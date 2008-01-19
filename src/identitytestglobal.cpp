#include "../include/identitytestglobal.h"
#include "../include/datetime.h"
#include "../include/logfile.h"
#include "../include/option.h"
#include "../include/stringfunctions.h"
#include "../include/db/sqlite3db.h"
#include "../include/freenet/freenetmasterthread.h"

#ifdef _WIN32
	#include <winsock2.h>
#endif

#ifdef XMEM
	#include <xmem.h>
#endif

void SetupDB()
{

	SQLite3DB::DB *db=SQLite3DB::DB::instance();

	db->Open("fms.db3");
	db->SetBusyTimeout(10000);		// set timeout to 10 seconds
	db->Execute("VACUUM;");

	db->Execute("CREATE TABLE IF NOT EXISTS tblOption(\
				Option				TEXT UNIQUE,\
				OptionValue			TEXT NOT NULL,\
				OptionDescription	TEXT\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblLocalIdentity(\
				LocalIdentityID			INTEGER PRIMARY KEY,\
				Name					TEXT,\
				PublicKey				TEXT,\
				PrivateKey				TEXT,\
				SingleUse				BOOL CHECK(SingleUse IN('true','false')) DEFAULT 'false',\
				PublishTrustList		BOOL CHECK(PublishTrustList IN('true','false')) DEFAULT 'false',\
				PublishBoardList		BOOL CHECK(PublishBoardList IN('true','false')) DEFAULT 'false',\
				InsertingIdentity		BOOL CHECK(InsertingIdentity IN('true','false')) DEFAULT 'false',\
				LastInsertedIdentity	DATETIME,\
				InsertingPuzzle			BOOL CHECK(InsertingPuzzle IN('true','false')) DEFAULT 'false',\
				LastInsertedPuzzle		DATETIME,\
				InsertingTrustList		BOOL CHECK(InsertingTrustList IN('true','false')) DEFAULT 'false',\
				LastInsertedTrustList	DATETIME,\
				InsertingBoardList		BOOL CHECK(InsertingBoardList IN('true','false')) DEFAULT 'false',\
				LastInsertedBoardList	DATETIME\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblLocalIdentityInserts(\
				LocalIdentityID		INTEGER,\
				Day					DATE,\
				InsertIndex			INTEGER\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblTrustListInserts(\
				LocalIdentityID		INTEGER,\
				Day					DATE,\
				InsertIndex			INTEGER\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblTrustListRequests(\
				IdentityID			INTEGER,\
				Day					DATE,\
				RequestIndex		INTEGER,\
				Found				BOOL CHECK(Found IN('true','false')) DEFAULT 'false'\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblIntroductionPuzzleInserts(\
				UUID				TEXT UNIQUE,\
				LocalIdentityID		INTEGER,\
				Day					DATE,\
				InsertIndex			INTEGER,\
				Type				TEXT,\
				MimeType			TEXT,\
				PuzzleData			TEXT,\
				PuzzleSolution		TEXT,\
				FoundSolution		BOOL CHECK(FoundSolution IN('true','false')) DEFAULT 'false'\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblIdentity(\
				IdentityID			INTEGER PRIMARY KEY,\
				PublicKey			TEXT,\
				Name				TEXT,\
				SingleUse			BOOL CHECK(SingleUse IN('true','false')) DEFAULT 'false',\
				PublishTrustList	BOOL CHECK(PublishTrustList IN('true','false')) DEFAULT 'false',\
				PublishBoardList	BOOL CHECK(PublishBoardList IN('true','false')) DEFAULT 'false',\
				DateAdded			DATETIME,\
				LastSeen			DATETIME,\
				LocalMessageTrust	INTEGER CHECK(LocalMessageTrust BETWEEN 0 AND 100) DEFAULT 50,\
				PeerMessageTrust	INTEGER CHECK(PeerMessageTrust BETWEEN 0 AND 100) DEFAULT 50,\
				LocalTrustListTrust	INTEGER CHECK(LocalTrustListTrust BETWEEN 0 AND 100) DEFAULT 50,\
				PeerTrustListTrust	INTEGER CHECK(PeerTrustListTrust BETWEEN 0 AND 100) DEFAULT 50\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblIdentityRequests(\
				IdentityID			INTEGER,\
				Day					DATE,\
				RequestIndex		INTEGER,\
				Found				BOOL CHECK(Found IN('true','false')) DEFAULT 'false'\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblIntroductionPuzzleRequests(\
				IdentityID			INTEGER,\
				Day					DATE,\
				RequestIndex		INTEGER,\
				Found				BOOL CHECK(Found IN('true','false')) DEFAULT 'false',\
				UUID				TEXT UNIQUE,\
				Type				TEXT,\
				MimeType			TEXT,\
				PuzzleData			TEXT\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblIdentityIntroductionInserts(\
				LocalIdentityID		INTEGER,\
				Day					DATE,\
				UUID				TEXT UNIQUE,\
				Solution			TEXT,\
				Inserted			BOOL CHECK(Inserted IN('true','false')) DEFAULT 'false'\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblPeerTrust(\
				IdentityID			INTEGER,\
				TargetIdentityID	INTEGER,\
				MessageTrust		INTEGER CHECK(MessageTrust BETWEEN 0 AND 100),\
				TrustListTrust		INTEGER CHECK(TrustListTrust BETWEEN 0 AND 100)\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblBoard(\
				BoardID				INTEGER PRIMARY KEY,\
				BoardName			TEXT UNIQUE,\
				BoardDescription	TEXT,\
				DateAdded			DATETIME\
				);");

	db->Execute("INSERT INTO tblBoard(BoardName,BoardDescription,DateAdded) VALUES('fms','Freenet Message System','2007-12-01');");
	db->Execute("INSERT INTO tblBoard(BoardName,BoardDescription,DateAdded) VALUES('freenet','Discussion about Freenet','2007-12-01');");
	db->Execute("INSERT INTO tblBoard(BoardName,BoardDescription,DateAdded) VALUES('public','Public discussion','2007-12-01');");
	db->Execute("INSERT INTO tblBoard(BoardName,BoardDescription,DateAdded) VALUES('test','Test board','2007-12-01');");

	db->Execute("CREATE TABLE IF NOT EXISTS tblMessage(\
				MessageID			INTEGER PRIMARY KEY,\
				MessageDate			DATE,\
				MessageTime			TIME,\
				Subject				TEXT,\
				MessageUUID			TEXT UNIQUE,\
				ReplyBoardID		INTEGER,\
				Body				TEXT\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblMessageReplyTo(\
				MessageID			INTEGER,\
				ReplyToMessageID	INTEGER,\
				Order				INTEGER\
				);");
	
	db->Execute("CREATE TABLE IF NOT EXISTS tblMessageBoard(\
				MessageID			INTEGER,\
				BoardID				INTEGER\
				);");

	// low / high / message count for each board
	db->Execute("CREATE VIEW IF NOT EXISTS vwBoardStats AS \
				SELECT tblBoard.BoardID AS 'BoardID', IFNULL(MIN(MessageID),0) AS 'LowMessageID', IFNULL(MAX(MessageID),0) AS 'HighMessageID', COUNT(MessageID) AS 'MessageCount' \
				FROM tblBoard LEFT JOIN tblMessageBoard ON tblBoard.BoardID=tblMessageBoard.BoardID \
				GROUP BY tblBoard.BoardID;");

	// calculates peer trust
	db->Execute("CREATE VIEW IF NOT EXISTS vwCalculatedPeerTrust AS \
				SELECT TargetIdentityID, \
				ROUND(SUM(MessageTrust*(LocalMessageTrust/100.0))/SUM(LocalMessageTrust/100.0),0) AS 'PeerMessageTrust', \
				ROUND(SUM(TrustListTrust*(LocalTrustListTrust/100.0))/SUM(LocalTrustListTrust/100.0),0) AS 'PeerTrustListTrust' \
				FROM tblPeerTrust INNER JOIN tblIdentity ON tblPeerTrust.IdentityID=tblIdentity.IdentityID \
				WHERE LocalTrustListTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalTrustListTrust') \
				GROUP BY TargetIdentityID;");

	// update PeerTrustLevel when deleting a record from tblPeerTrust
	db->Execute("CREATE TRIGGER trgDeleteOntblPeerTrust AFTER DELETE ON tblPeerTrust \
				FOR EACH ROW \
				BEGIN \
					UPDATE tblIdentity SET PeerMessageTrust=(SELECT PeerMessageTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=old.TargetIdentityID), PeerTrustListTrust=(SELECT PeerTrustListTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=old.TargetIdentityID) WHERE IdentityID=old.TargetIdentityID;\
				END;");

	// update PeerTrustLevel when inserting a record into tblPeerTrust
	db->Execute("CREATE TRIGGER trgInsertOntblPeerTrust AFTER INSERT ON tblPeerTrust \
				FOR EACH ROW \
				BEGIN \
					UPDATE tblIdentity SET PeerMessageTrust=(SELECT PeerMessageTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=new.TargetIdentityID), PeerTrustListTrust=(SELECT PeerTrustListTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=new.TargetIdentityID) WHERE IdentityID=new.TargetIdentityID;\
				END;");

	// update PeerTrustLevel when updating a record in tblPeerTrust
	db->Execute("CREATE TRIGGER trgUpdateOntblPeerTrust AFTER UPDATE ON tblPeerTrust \
				FOR EACH ROW \
				BEGIN \
					UPDATE tblIdentity SET PeerMessageTrust=(SELECT PeerMessageTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=old.TargetIdentityID), PeerTrustListTrust=(SELECT PeerTrustListTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=old.TargetIdentityID) WHERE IdentityID=old.TargetIdentityID;\
					UPDATE tblIdentity SET PeerMessageTrust=(SELECT PeerMessageTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=new.TargetIdentityID), PeerTrustListTrust=(SELECT PeerTrustListTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=new.TargetIdentityID) WHERE IdentityID=new.TargetIdentityID;\
				END;");

	// recalculate all Peer TrustLevels when updating Local TrustLevels on tblIdentity - doesn't really need to be all, but rather all identities the updated identity has a trust level for.  It's easier to update everyone for now.
	db->Execute("CREATE TRIGGER trgUpdateLocalTrustLevels AFTER UPDATE OF LocalMessageTrust,LocalTrustListTrust ON tblIdentity \
				FOR EACH ROW \
				BEGIN \
					UPDATE tblIdentity SET PeerMessageTrust=(SELECT PeerMessageTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=IdentityID), PeerTrustListTrust=(SELECT PeerTrustListTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=IdentityID);\
				END;");

}

void SetupDefaultOptions()
{
	// OptionValue should always be inserted as a string, even if the option really isn't a string - just to keep the field data type consistent

	std::ostringstream tempstr;	// must set tempstr to "" between db inserts
	SQLite3DB::DB *db=SQLite3DB::DB::instance();
	SQLite3DB::Statement st=db->Prepare("INSERT INTO tblOption(Option,OptionValue,OptionDescription) VALUES(?,?,?);");

	// LogLevel
	tempstr.str("");
	tempstr << LogFile::LOGLEVEL_DEBUG;
	st.Bind(0,"LogLevel");
	st.Bind(1,tempstr.str());
	st.Bind(2,"The maximum logging level that will be written to file.  0=Fatal Errors, 1=Errors, 2=Warnings, 3=Informational Messages, 4=Debug Messages.  Higher levels will include all messages from the previous levels.");
	st.Step();
	st.Reset();

	// StartFreenetUpdater
	st.Bind(0,"StartFreenetUpdater");
	st.Bind(1,"true");
	st.Bind(2,"Start Freenet Updater thread.");
	st.Step();
	st.Reset();

	// FCPHost
	st.Bind(0,"FCPHost");
	st.Bind(1,"localhost");
	st.Bind(2,"Host name or address of Freenet node.");
	st.Step();
	st.Reset();

	// FCPPort
	st.Bind(0,"FCPPort");
	st.Bind(1,"9481");
	st.Bind(2,"The port that Freenet is listening for FCP connections on.");
	st.Step();
	st.Reset();

	st.Bind(0,"MessageBase");
	st.Bind(1,"fms");
	st.Bind(2,"A unique string shared by all clients who want to communicate with each other.  This should not be changed unless you want to create your own separate communications network.");
	st.Step();
	st.Reset();

	st.Bind(0,"MaxIdentityRequests");
	st.Bind(1,"5");
	st.Bind(2,"Maximum number of concurrent requests for new Identity xml files");
	st.Step();
	st.Reset();

	st.Bind(0,"MaxIdentityIntroductionRequests");
	st.Bind(1,"5");
	st.Bind(2,"Maximum number of concurrent identities requesting IdentityIntroduction xml files.  Each identity may have multiple requests pending.");
	st.Step();
	st.Reset();

	st.Bind(0,"MaxIntroductionPuzzleRequests");
	st.Bind(1,"5");
	st.Bind(2,"Maximum number of concurrent requests for new IntroductionPuzzle xml files");
	st.Step();
	st.Reset();

	st.Bind(0,"MaxTrustListRequests");
	st.Bind(1,"5");
	st.Bind(2,"Maximum number of concurrent requests for new Trust Lists");
	st.Step();
	st.Reset();

	st.Bind(0,"MinLocalTrustListTrust");
	st.Bind(1,"50");
	st.Bind(2,"Specifies a local trust list trust level that a peer must have before its trust list will be included in the weighted average.  Any peers below this number will be excluded from the results.");
	st.Step();
	st.Reset();

}

void SetupLogFile()
{
	DateTime date;
	std::string configval;
	int loglevel;

	date.SetToGMTime();

	LogFile::instance()->SetFileName("fms-"+date.Format("%Y-%m-%d")+".log");
	LogFile::instance()->OpenFile();
	LogFile::instance()->SetWriteNewLine(true);
	LogFile::instance()->SetWriteDate(true);
	LogFile::instance()->SetWriteLogLevel(true);

	if(Option::instance()->Get("LogLevel",configval)==false)
	{
		configval="4";
		Option::instance()->Set("LogLevel",configval);
	}
	if(StringFunctions::Convert(configval,loglevel)==false)
	{
		loglevel=LogFile::LOGLEVEL_DEBUG;
		Option::instance()->Set("LogLevel",loglevel);
	}
	LogFile::instance()->SetLogLevel((LogFile::LogLevel)loglevel);
}

void SetupNetwork()
{
#ifdef _WIN32
	WSAData wsadata;
	WSAStartup(MAKEWORD(2,2),&wsadata);
#endif
}

void ShutdownNetwork()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

void ShutdownThreads(std::vector<ZThread::Thread *> &threads)
{
	std::vector<ZThread::Thread *>::iterator i;
	for(i=threads.begin(); i!=threads.end(); i++)
	{
		if((*i)->wait(1)==false)
		{
			try
			{
				(*i)->interrupt();
			}
			catch(...)
			{
			}
		}
	}

	for(i=threads.begin(); i!=threads.end(); i++)
	{
		(*i)->wait();
		delete (*i);
	}

	threads.clear();

}

void StartThreads(std::vector<ZThread::Thread *> &threads)
{
	std::string startfreenet;
	std::string startnntp;

	if(Option::instance()->Get("StartFreenetUpdater",startfreenet)==false)
	{
		startfreenet="true";
		Option::instance()->Set("StartFreenetUpdater","true");
	}

	if(startfreenet=="true")
	{
		ZThread::Thread *t=new ZThread::Thread(new FreenetMasterThread());
		threads.push_back(t);
	}

}
