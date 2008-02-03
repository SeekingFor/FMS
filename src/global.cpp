#include "../include/global.h"
#include "../include/datetime.h"
#include "../include/logfile.h"
#include "../include/option.h"
#include "../include/stringfunctions.h"
#include "../include/db/sqlite3db.h"
#include "../include/freenet/freenetmasterthread.h"
#include "../include/nntp/nntplistener.h"
#include "../include/http/httpthread.h"

#ifdef _WIN32
	#include <winsock2.h>
#endif

#ifdef XMEM
	#include <xmem.h>
#endif

void SetupDB()
{

	DateTime date;
	SQLite3DB::DB *db=SQLite3DB::DB::Instance();

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
				LastInsertedBoardList	DATETIME,\
				InsertingMessageList	BOOL CHECK(InsertingMessageList IN('true','false')) DEFAULT 'false',\
				LastInsertedMessageList	DATETIME\
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
				PublicKey			TEXT UNIQUE,\
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
				IdentityID			INTEGER,\
				FromName			TEXT,\
				MessageDate			DATE,\
				MessageTime			TIME,\
				Subject				TEXT,\
				MessageUUID			TEXT UNIQUE,\
				ReplyBoardID		INTEGER,\
				Body				TEXT\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblMessageReplyTo(\
				MessageID			INTEGER,\
				ReplyToMessageUUID	TEXT,\
				ReplyOrder			INTEGER\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblMessageBoard(\
				MessageID			INTEGER,\
				BoardID				INTEGER\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblMessageListRequests(\
				IdentityID			INTEGER,\
				Day					DATE,\
				RequestIndex		INTEGER,\
				Found				BOOL CHECK(Found IN('true','false')) DEFAULT 'false'\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblMessageRequests(\
				IdentityID			INTEGER,\
				Day					DATE,\
				RequestIndex		INTEGER,\
				FromMessageList		BOOL CHECK(FromMessageList IN('true','false')) DEFAULT 'false',\
				Found				BOOL CHECK(Found IN('true','false')) DEFAULT 'false'\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblMessageInserts(\
				LocalIdentityID		INTEGER,\
				Day					DATE,\
				InsertIndex			INTEGER,\
				MessageUUID			TEXT UNIQUE,\
				MessageXML			TEXT,\
				Inserted			BOOL CHECK(Inserted IN('true','false')) DEFAULT 'false'\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblMessageListInserts(\
				LocalIdentityID		INTEGER,\
				Day					DATE,\
				InsertIndex			INTEGER,\
				Inserted			BOOL CHECK(Inserted IN('true','false')) DEFAULT 'false'\
				);");

	// MessageInserter will insert a record into this temp table which the MessageListInserter will query for and insert a MessageList when needed
	db->Execute("CREATE TEMPORARY TABLE IF NOT EXISTS tmpMessageListInsert(\
				LocalIdentityID		INTEGER,\
				Date				DATETIME\
				);");

	// low / high / message count for each board
	db->Execute("CREATE VIEW IF NOT EXISTS vwBoardStats AS \
				SELECT tblBoard.BoardID AS 'BoardID', IFNULL(MIN(MessageID),0) AS 'LowMessageID', IFNULL(MAX(MessageID),0) AS 'HighMessageID', COUNT(MessageID) AS 'MessageCount' \
				FROM tblBoard LEFT JOIN tblMessageBoard ON tblBoard.BoardID=tblMessageBoard.BoardID \
				WHERE MessageID>=0 OR MessageID IS NULL \
				GROUP BY tblBoard.BoardID;");

	// calculates peer trust
	db->Execute("CREATE VIEW IF NOT EXISTS vwCalculatedPeerTrust AS \
				SELECT TargetIdentityID, \
				ROUND(SUM(MessageTrust*(LocalMessageTrust/100.0))/SUM(LocalMessageTrust/100.0),0) AS 'PeerMessageTrust', \
				ROUND(SUM(TrustListTrust*(LocalTrustListTrust/100.0))/SUM(LocalTrustListTrust/100.0),0) AS 'PeerTrustListTrust' \
				FROM tblPeerTrust INNER JOIN tblIdentity ON tblPeerTrust.IdentityID=tblIdentity.IdentityID \
				WHERE LocalTrustListTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalTrustListTrust') \
				AND ( PeerTrustListTrust IS NULL OR PeerTrustListTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerTrustListTrust') ) \
				GROUP BY TargetIdentityID;");

	// update PeerTrustLevel when deleting a record from tblPeerTrust
	db->Execute("CREATE TRIGGER IF NOT EXISTS trgDeleteOntblPeerTrust AFTER DELETE ON tblPeerTrust \
				FOR EACH ROW \
				BEGIN \
					UPDATE tblIdentity SET PeerMessageTrust=(SELECT PeerMessageTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=old.TargetIdentityID), PeerTrustListTrust=(SELECT PeerTrustListTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=old.TargetIdentityID) WHERE IdentityID=old.TargetIdentityID;\
				END;");

	// update PeerTrustLevel when inserting a record into tblPeerTrust
	db->Execute("CREATE TRIGGER IF NOT EXISTS trgInsertOntblPeerTrust AFTER INSERT ON tblPeerTrust \
				FOR EACH ROW \
				BEGIN \
					UPDATE tblIdentity SET PeerMessageTrust=(SELECT PeerMessageTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=new.TargetIdentityID), PeerTrustListTrust=(SELECT PeerTrustListTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=new.TargetIdentityID) WHERE IdentityID=new.TargetIdentityID;\
				END;");

	// update PeerTrustLevel when updating a record in tblPeerTrust
	db->Execute("CREATE TRIGGER IF NOT EXISTS trgUpdateOntblPeerTrust AFTER UPDATE ON tblPeerTrust \
				FOR EACH ROW \
				BEGIN \
					UPDATE tblIdentity SET PeerMessageTrust=(SELECT PeerMessageTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=old.TargetIdentityID), PeerTrustListTrust=(SELECT PeerTrustListTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=old.TargetIdentityID) WHERE IdentityID=old.TargetIdentityID;\
					UPDATE tblIdentity SET PeerMessageTrust=(SELECT PeerMessageTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=new.TargetIdentityID), PeerTrustListTrust=(SELECT PeerTrustListTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=new.TargetIdentityID) WHERE IdentityID=new.TargetIdentityID;\
				END;");

	// recalculate all Peer TrustLevels when updating Local TrustLevels on tblIdentity - doesn't really need to be all, but rather all identities the updated identity has a trust level for.  It's easier to update everyone for now.
	db->Execute("CREATE TRIGGER IF NOT EXISTS trgUpdateLocalTrustLevels AFTER UPDATE OF LocalMessageTrust,LocalTrustListTrust ON tblIdentity \
				FOR EACH ROW \
				BEGIN \
					UPDATE tblIdentity SET PeerMessageTrust=(SELECT PeerMessageTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=IdentityID), PeerTrustListTrust=(SELECT PeerTrustListTrust FROM vwCalculatedPeerTrust WHERE TargetIdentityID=IdentityID);\
				END;");

	db->Execute("CREATE TRIGGER IF NOT EXISTS trgDeleteMessage AFTER DELETE ON tblMessage \
				FOR EACH ROW \
				BEGIN \
					DELETE FROM tblMessageBoard WHERE tblMessageBoard.MessageID=old.MessageID;\
					DELETE FROM tblMessageReplyTo WHERE tblMessageReplyTo.MessageID=old.MessageID;\
				END;");

	db->Execute("CREATE TRIGGER IF NOT EXISTS trgDeleteIdentity AFTER DELETE ON tblIdentity \
				FOR EACH ROW \
				BEGIN \
					DELETE FROM tblIdentityRequests WHERE IdentityID=old.IdentityID;\
					DELETE FROM tblIntroductionPuzzleRequests WHERE IdentityID=old.IdentityID;\
					DELETE FROM tblMessageListRequests WHERE IdentityID=old.IdentityID;\
					DELETE FROM tblMessageRequests WHERE IdentityID=old.IdentityID;\
					DELETE FROM tblPeerTrust WHERE IdentityID=old.IdentityID;\
					DELETE FROM tblTrustListRequests WHERE IdentityID=old.IdentityID;\
				END;");

	db->Execute("CREATE TRIGGER IF NOT EXISTS trgDeleteLocalIdentity AFTER DELETE ON tblLocalIdentity \
				FOR EACH ROW \
				BEGIN \
					DELETE FROM tblIdentityIntroductionInserts WHERE LocalIdentityID=old.LocalIdentityID;\
					DELETE FROM tblIntroductionPuzzleInserts WHERE LocalIdentityID=old.LocalIdentityID;\
					DELETE FROM tblLocalIdentityInserts WHERE LocalIdentityID=old.LocalIdentityID;\
					DELETE FROM tblMessageInserts WHERE LocalIdentityID=old.LocalIdentityID;\
					DELETE FROM tblMessageListInserts WHERE LocalIdentityID=old.LocalIdentityID;\
					DELETE FROM tblTrustListInserts WHERE LocalIdentityID=old.LocalIdentityID;\
				END;");

	// delete introduction puzzles that were half-way inserted
	db->Execute("DELETE FROM tblIntroductionPuzzleInserts WHERE Day IS NULL AND InsertIndex IS NULL;");

	// delete stale introduction puzzles (2 or more days old)
	date.SetToGMTime();
	date.Add(0,0,0,-2);
	db->Execute("DELETE FROM tblIntroductionPuzzleInserts WHERE Day<='"+date.Format("%Y-%m-%d")+"';");
	db->Execute("DELETE FROM tblIntroductionPuzzleRequests WHERE Day<='"+date.Format("%Y-%m-%d")+"';");

	// insert SomeDude's public key
	date.SetToGMTime();
	db->Execute("INSERT INTO tblIdentity(PublicKey,DateAdded) VALUES('SSK@NuBL7aaJ6Cn4fB7GXFb9Zfi8w1FhPyW3oKgU9TweZMw,iXez4j3qCpd596TxXiJgZyTq9o-CElEuJxm~jNNZAuA,AQACAAE/','"+date.Format("%Y-%m-%d %H:%M:%S")+"');");

}

void SetupDefaultOptions()
{
	// OptionValue should always be inserted as a string, even if the option really isn't a string - just to keep the field data type consistent

	std::ostringstream tempstr;	// must set tempstr to "" between db inserts
	SQLite3DB::DB *db=SQLite3DB::DB::Instance();
	SQLite3DB::Statement st=db->Prepare("INSERT INTO tblOption(Option,OptionValue,OptionDescription) VALUES(?,?,?);");

	// LogLevel
	tempstr.str("");
	tempstr << LogFile::LOGLEVEL_DEBUG;
	st.Bind(0,"LogLevel");
	st.Bind(1,tempstr.str());
	st.Bind(2,"The maximum logging level that will be written to file.  0=Fatal Errors, 1=Errors, 2=Warnings, 3=Informational Messages, 4=Debug Messages.  Higher levels will include all messages from the previous levels.");
	st.Step();
	st.Reset();

	// NNTPListenPort
	st.Bind(0,"NNTPListenPort");
	st.Bind(1,"1119");
	st.Bind(2,"The port that the NNTP service will listen for incoming connections.");
	st.Step();
	st.Reset();

	// NNTPBindAddresses
	st.Bind(0,"NNTPBindAddresses");
	st.Bind(1,"localhost");
	st.Bind(2,"A comma separated list of valid IPv4 or IPv6 addresses/hostnames that the NNTP service will try to bind to.");
	st.Step();
	st.Reset();

	st.Bind(0,"NNTPAllowPost");
	st.Bind(1,"true");
	st.Bind(2,"Allow posting messages from NNTP.  Setting to false will make the newsgroups read only.");
	st.Step();
	st.Reset();

	// StartNNTP
	st.Bind(0,"StartNNTP");
	st.Bind(1,"true");
	st.Bind(2,"Start NNTP server.");
	st.Step();
	st.Reset();

	st.Bind(0,"StartHTTP");
	st.Bind(1,"true");
	st.Bind(2,"Start HTTP server.");
	st.Step();
	st.Reset();

	st.Bind(0,"HTTPListenPort");
	st.Bind(1,"8080");
	st.Bind(2,"Port HTTP server will listen on.");
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
	st.Bind(1,"127.0.0.1");
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

	st.Bind(0,"MaxMessageListRequests");
	st.Bind(1,"5");
	st.Bind(2,"Maximum number of concurrent requests for new Message Lists");
	st.Step();
	st.Reset();

	st.Bind(0,"MaxMessageRequests");
	st.Bind(1,"20");
	st.Bind(2,"Maximum number of concurrent requests for new Messages");
	st.Step();
	st.Reset();

	st.Bind(0,"MinLocalMessageTrust");
	st.Bind(1,"50");
	st.Bind(2,"Specifies a local message trust level that a peer must have before its messages will be downloaded.");
	st.Step();
	st.Reset();

	st.Bind(0,"MinPeerMessageTrust");
	st.Bind(1,"30");
	st.Bind(2,"Specifies a peer message trust level that a peer must have before its messages will be downloaded.");
	st.Step();
	st.Reset();

	st.Bind(0,"MinLocalTrustListTrust");
	st.Bind(1,"50");
	st.Bind(2,"Specifies a local trust list trust level that a peer must have before its trust list will be included in the weighted average.  Any peers below this number will be excluded from the results.");
	st.Step();
	st.Reset();

	st.Bind(0,"MinPeerTrustListTrust");
	st.Bind(1,"30");
	st.Bind(2,"Specifies a peer trust list trust level that a peer must have before its trust list will be included in the weighted average.  Any peers below this number will be excluded from the results.");
	st.Step();
	st.Reset();

	st.Bind(0,"MessageDownloadMaxDaysBackward");
	st.Bind(1,"5");
	st.Bind(2,"The maximum number of days backward that messages will be downloaded from each identity");
	st.Step();
	st.Reset();

	st.Bind(0,"MessageListDaysBackward");
	st.Bind(1,"5");
	st.Bind(2,"The number of days backward that messages you have inserted will appear in your MessageLists");
	st.Step();
	st.Reset();

}

void SetupLogFile()
{
	DateTime date;
	std::string configval;
	int loglevel;

	date.SetToGMTime();

	LogFile::Instance()->SetFileName("fms-"+date.Format("%Y-%m-%d")+".log");
	LogFile::Instance()->OpenFile();
	LogFile::Instance()->SetWriteNewLine(true);
	LogFile::Instance()->SetWriteDate(true);
	LogFile::Instance()->SetWriteLogLevel(true);

	if(Option::Instance()->Get("LogLevel",configval)==false)
	{
		configval="4";
		Option::Instance()->Set("LogLevel",configval);
	}
	if(StringFunctions::Convert(configval,loglevel)==false)
	{
		loglevel=LogFile::LOGLEVEL_DEBUG;
		Option::Instance()->Set("LogLevel",loglevel);
	}
	LogFile::Instance()->SetLogLevel((LogFile::LogLevel)loglevel);
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

void ShutdownThreads(std::vector<PThread::Thread *> &threads)
{
	std::vector<PThread::Thread *>::iterator i;
	for(i=threads.begin(); i!=threads.end(); i++)
	{
/*		if((*i)->wait(1)==false)
		{
			try
			{
				(*i)->interrupt();
			}
			catch(...)
			{
			}
		}
		*/
		(*i)->Cancel();
	}

	for(i=threads.begin(); i!=threads.end(); i++)
	{
		LogFile::Instance()->WriteLog(LogFile::LOGLEVEL_DEBUG,"ShutdownThreads waiting for thread to exit.");
		//(*i)->wait();
		(*i)->Join();
		delete (*i);
	}

	threads.clear();

}

void StartThreads(std::vector<PThread::Thread *> &threads)
{
	std::string startfreenet;
	std::string startnntp;
	std::string starthttp;

	if(Option::Instance()->Get("StartFreenetUpdater",startfreenet)==false)
	{
		startfreenet="true";
		Option::Instance()->Set("StartFreenetUpdater","true");
	}

	if(Option::Instance()->Get("StartNNTP",startnntp)==false)
	{
		startnntp="true";
		Option::Instance()->Set("StartNNTP","true");
	}

	if(Option::Instance()->Get("StartHTTP",starthttp)==false)
	{
		starthttp="true";
		Option::Instance()->Set("StartHTTP","true");
	}

	if(startfreenet=="true")
	{
		PThread::Thread *t=new PThread::Thread(new FreenetMasterThread());
		threads.push_back(t);
	}

	if(startnntp=="true")
	{
		PThread::Thread *t=new PThread::Thread(new NNTPListener());
		threads.push_back(t);
	}

	if(starthttp=="true")
	{
		PThread::Thread *t=new PThread::Thread(new HTTPThread());
		threads.push_back(t);
	}

}
