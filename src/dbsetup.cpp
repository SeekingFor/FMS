#include "../include/dbsetup.h"
#include "../include/dbconversions.h"
#include "../include/option.h"
#include "../include/db/sqlite3db.h"

#include <Poco/DateTime.h>
#include <Poco/Timespan.h>
#include <Poco/DateTimeFormatter.h>

void SetupDB(SQLite3DB::DB *db)
{

	Poco::DateTime date;
	std::string tempval="";

	db->Execute("CREATE TABLE IF NOT EXISTS tblDBVersion(\
				Major				INTEGER,\
				Minor				INTEGER\
				);");

	SQLite3DB::Statement st=db->Prepare("SELECT Major,Minor FROM tblDBVersion;");
	st.Step();
	if(st.RowReturned())
	{
		int major;
		int minor;
		st.ResultInt(0,major);
		st.ResultInt(1,minor);
		st.Finalize();
		if(major==1 && minor==0)
		{
			ConvertDB0100To0101(db);
			major=1;
			minor=1;
		}
		if(major==1 && (minor==1 || minor==2))
		{
			ConvertDB0101To0103(db);
			major=1;
			minor=3;
		}
		if(major==1 && minor==3)
		{
			ConvertDB0103To0104(db);
			major=1;
			minor=4;
		}
		if(major==1 && minor==4)
		{
			ConvertDB0104To0105(db);
			major=1;
			minor=5;
		}
		if(major==1 && minor==5)
		{
			ConvertDB0105To0106(db);
			major=1;
			minor=6;
		}
		if(major==1 && minor==6)
		{
			ConvertDB0106To0107(db);
			major=1;
			minor=7;
		}
		if(major==1 && minor==7)
		{
			ConvertDB0107To0108(db);
			major=1;
			minor=8;
		}
		if(major==1 && minor==8)
		{
			ConvertDB0108To0109(db);
			major=1;
			minor=9;
		}
		if(major==1 && minor==9)
		{
			ConvertDB0109To0110(db);
			major=1;
			minor=10;
		}
		if(major==1 && minor==10)
		{
			ConvertDB0110To0111(db);
			major=1;
			minor=11;
		}
		if(major==1 && minor==11)
		{
			ConvertDB0111To0112(db);
			major=1;
			minor=12;
		}
		if(major==1 && minor==12)
		{
			ConvertDB0112To0113(db);
			major=1;
			minor=13;
		}
		if(major==1 && minor==13)
		{
			ConvertDB0113To0114(db);
			major=1;
			minor=14;
		}
		if(major==1 && minor==14)
		{
			ConvertDB0114To0115(db);
			major=1;
			minor=15;
		}
		if(major==1 && minor==15)
		{
			ConvertDB0115To0116(db);
			major=1;
			minor=16;
		}
		if(major==1 && minor==16)
		{
			ConvertDB0116To0117(db);
			major=1;
			minor=17;
		}
		if(major==1 && minor==17)
		{
			ConvertDB0117To0118(db);
			major=1;
			minor=18;
		}
	}
	else
	{
		db->Execute("INSERT INTO tblDBVersion(Major,Minor) VALUES(1,18);");
	}

	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=18;");

	db->Execute("CREATE TABLE IF NOT EXISTS tblFMSVersion(\
				Major				INTEGER,\
				Minor				INTEGER,\
				Release				INTEGER,\
				Notes				TEXT,\
				Changes				TEXT,\
				PageKey				TEXT,\
				SourceKey			TEXT\
				);");

	db->Execute("CREATE UNIQUE INDEX IF NOT EXISTS idxFMSVersion_Version ON tblFMSVersion(Major,Minor,Release);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblOption(\
				Option				TEXT UNIQUE,\
				OptionValue			TEXT NOT NULL,\
				OptionDescription	TEXT,\
				Section				TEXT,\
				SortOrder			INTEGER,\
				ValidValues			TEXT,\
				DisplayType			TEXT CHECK (DisplayType IN ('textbox','textarea','select','multiselect')) DEFAULT 'textbox',\
				DisplayParam1		TEXT,\
				DisplayParam2		TEXT,\
				Mode				TEXT CHECK (Mode IN ('simple','advanced')) DEFAULT 'simple'\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblLocalIdentity(\
				LocalIdentityID			INTEGER PRIMARY KEY,\
				Name					TEXT,\
				PublicKey				TEXT UNIQUE,\
				PrivateKey				TEXT UNIQUE,\
				SingleUse				BOOL CHECK(SingleUse IN('true','false')) DEFAULT 'false',\
				PublishTrustList		BOOL CHECK(PublishTrustList IN('true','false')) DEFAULT 'false',\
				PublishBoardList		BOOL CHECK(PublishBoardList IN('true','false')) DEFAULT 'false',\
				PublishFreesite			BOOL CHECK(PublishFreesite IN('true','false')) DEFAULT 'false',\
				FreesiteEdition			INTEGER,\
				InsertingIdentity		BOOL CHECK(InsertingIdentity IN('true','false')) DEFAULT 'false',\
				LastInsertedIdentity	DATETIME,\
				InsertingPuzzle			BOOL CHECK(InsertingPuzzle IN('true','false')) DEFAULT 'false',\
				LastInsertedPuzzle		DATETIME,\
				InsertingTrustList		BOOL CHECK(InsertingTrustList IN('true','false')) DEFAULT 'false',\
				LastInsertedTrustList	DATETIME,\
				LastInsertedBoardList	DATETIME,\
				LastInsertedMessageList	DATETIME,\
				LastInsertedFreesite	DATETIME,\
				DateCreated				DATETIME,\
				MinMessageDelay			INTEGER DEFAULT 0,\
				MaxMessageDelay			INTEGER DEFAULT 0\
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

	/*
		PurgeDate is not used yet
	*/
	db->Execute("CREATE TABLE IF NOT EXISTS tblIdentity(\
				IdentityID				INTEGER PRIMARY KEY,\
				PublicKey				TEXT UNIQUE,\
				Name					TEXT,\
				SingleUse				BOOL CHECK(SingleUse IN('true','false')) DEFAULT 'false',\
				PublishTrustList		BOOL CHECK(PublishTrustList IN('true','false')) DEFAULT 'false',\
				PublishBoardList		BOOL CHECK(PublishBoardList IN('true','false')) DEFAULT 'false',\
				FreesiteEdition			INTEGER,\
				DateAdded				DATETIME,\
				LastSeen				DATETIME,\
				LocalMessageTrust		INTEGER CHECK(LocalMessageTrust BETWEEN 0 AND 100) DEFAULT NULL,\
				PeerMessageTrust		INTEGER CHECK(PeerMessageTrust BETWEEN 0 AND 100) DEFAULT NULL,\
				LocalTrustListTrust		INTEGER CHECK(LocalTrustListTrust BETWEEN 0 AND 100) DEFAULT NULL,\
				PeerTrustListTrust		INTEGER CHECK(PeerTrustListTrust BETWEEN 0 AND 100) DEFAULT NULL,\
				AddedMethod				TEXT,\
				Hidden					BOOL CHECK(Hidden IN('true','false')) DEFAULT 'false',\
				PurgeDate				DATETIME,\
				FailureCount			INTEGER CHECK(FailureCount>=0) DEFAULT 0\
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

	db->Execute("CREATE TABLE IF NOT EXISTS tblIdentityTrust(\
				LocalIdentityID			INTEGER,\
				IdentityID				INTEGER,\
				LocalMessageTrust		INTEGER CHECK(LocalMessageTrust BETWEEN 0 AND 100) DEFAULT NULL,\
				MessageTrustComment		TEXT,\
				LocalTrustListTrust		INTEGER CHECK(LocalTrustListTrust BETWEEN 0 AND 100) DEFAULT NULL,\
				TrustListTrustComment	TEXT\
				);");

	db->Execute("CREATE UNIQUE INDEX IF NOT EXISTS idxIdentityTrust_IDs ON tblIdentityTrust(LocalIdentityID,IdentityID);");

	db->Execute("CREATE TRIGGER IF NOT EXISTS trgInsertOnIdentityTrust AFTER INSERT ON tblIdentityTrust \
				FOR EACH ROW \
				BEGIN \
					UPDATE tblIdentity SET LocalMessageTrust=(SELECT MAX(LocalMessageTrust) FROM tblIdentityTrust WHERE tblIdentityTrust.IdentityID=new.IdentityID GROUP BY tblIdentityTrust.IdentityID), LocalTrustListTrust=(SELECT MAX(LocalTrustListTrust) FROM tblIdentityTrust WHERE tblIdentityTrust.IdentityID=new.IdentityID GROUP BY tblIdentityTrust.IdentityID) WHERE tblIdentity.IdentityID=new.IdentityID; \
				END;");

	db->Execute("CREATE TRIGGER IF NOT EXISTS trgUpdateOnIdentityTrust AFTER UPDATE OF LocalMessageTrust,LocalTrustListTrust ON tblIdentityTrust \
				FOR EACH ROW \
				BEGIN \
					UPDATE tblIdentity SET LocalMessageTrust=(SELECT MAX(LocalMessageTrust) FROM tblIdentityTrust WHERE tblIdentityTrust.IdentityID=new.IdentityID GROUP BY tblIdentityTrust.IdentityID), LocalTrustListTrust=(SELECT MAX(LocalTrustListTrust) FROM tblIdentityTrust WHERE tblIdentityTrust.IdentityID=new.IdentityID GROUP BY tblIdentityTrust.IdentityID) WHERE tblIdentity.IdentityID=new.IdentityID; \
				END;");

	db->Execute("CREATE TRIGGER IF NOT EXISTS trgDeleteOnIdentityTrust AFTER DELETE ON tblIdentityTrust \
				FOR EACH ROW \
				BEGIN \
					UPDATE tblIdentity SET LocalMessageTrust=(SELECT MAX(LocalMessageTrust) FROM tblIdentityTrust WHERE tblIdentityTrust.IdentityID=old.IdentityID GROUP BY tblIdentityTrust.IdentityID), LocalTrustListTrust=(SELECT MAX(LocalTrustListTrust) FROM tblIdentityTrust WHERE tblIdentityTrust.IdentityID=old.IdentityID GROUP BY tblIdentityTrust.IdentityID) WHERE tblIdentity.IdentityID=old.IdentityID; \
				END;");

	db->Execute("CREATE TABLE IF NOT EXISTS tblPeerTrust(\
				IdentityID				INTEGER,\
				TargetIdentityID		INTEGER,\
				MessageTrust			INTEGER CHECK(MessageTrust BETWEEN 0 AND 100),\
				TrustListTrust			INTEGER CHECK(TrustListTrust BETWEEN 0 AND 100),\
				MessageTrustComment		TEXT,\
				TrustListTrustComment	TEXT\
				);");

	db->Execute("CREATE INDEX IF NOT EXISTS idxPeerTrust_IdentityID ON tblPeerTrust (IdentityID);");
	db->Execute("CREATE INDEX IF NOT EXISTS idxPeerTrust_TargetIdentityID ON tblPeerTrust (TargetIdentityID);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblBoard(\
				BoardID					INTEGER PRIMARY KEY,\
				BoardName				TEXT UNIQUE,\
				BoardDescription		TEXT,\
				DateAdded				DATETIME,\
				SaveReceivedMessages	BOOL CHECK(SaveReceivedMessages IN('true','false')) DEFAULT 'true',\
				AddedMethod				TEXT,\
				Forum					TEXT CHECK(Forum IN('true','false')) DEFAULT 'false'\
				);");

	db->Execute("INSERT INTO tblBoard(BoardName,BoardDescription,DateAdded,AddedMethod,Forum) VALUES('fms','Freenet Message System','2007-12-01 12:00:00','Seed Board','true');");
	db->Execute("INSERT INTO tblBoard(BoardName,BoardDescription,DateAdded,AddedMethod,Forum) VALUES('freenet','Discussion about Freenet','2007-12-01 12:00:00','Seed Board','true');");
	db->Execute("INSERT INTO tblBoard(BoardName,BoardDescription,DateAdded,AddedMethod,Forum) VALUES('public','Public discussion','2007-12-01 12:00:00','Seed Board','true');");
	db->Execute("INSERT INTO tblBoard(BoardName,BoardDescription,DateAdded,AddedMethod,Forum) VALUES('test','Test board','2007-12-01 12:00:00','Seed Board','true');");

	db->Execute("CREATE TABLE IF NOT EXISTS tblMessage(\
				MessageID			INTEGER PRIMARY KEY,\
				IdentityID			INTEGER,\
				FromName			TEXT,\
				MessageDate			DATE,\
				MessageTime			TIME,\
				Subject				TEXT,\
				MessageUUID			TEXT UNIQUE,\
				ReplyBoardID		INTEGER,\
				Body				TEXT,\
				InsertDate			DATE,\
				MessageIndex		INTEGER,\
				Read				INTEGER CHECK(Read IN(0,1)) DEFAULT 0\
				);");

	db->Execute("CREATE INDEX IF NOT EXISTS idxMessage_IdentityID ON tblMessage (IdentityID);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblMessageReplyTo(\
				MessageID			INTEGER,\
				ReplyToMessageUUID	TEXT,\
				ReplyOrder			INTEGER\
				);");

	db->Execute("CREATE INDEX IF NOT EXISTS idxMessageReplyTo_MessageID ON tblMessageReplyTo (MessageID);");
	db->Execute("CREATE INDEX IF NOT EXISTS idxMessageReplyTo_ReplyToMessageUUID ON tblMessageReplyTo (ReplyToMessageUUID);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblMessageBoard(\
				MessageID			INTEGER,\
				BoardID				INTEGER\
				);");

	db->Execute("CREATE INDEX IF NOT EXISTS idxMessageBoard_MessageID ON tblMessageBoard (MessageID);");
	db->Execute("CREATE INDEX IF NOT EXISTS idxMessageBoard_BoardID ON tblMessageBoard (BoardID);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblMessageListRequests(\
				IdentityID			INTEGER,\
				Day					DATE,\
				RequestIndex		INTEGER,\
				Found				BOOL CHECK(Found IN('true','false')) DEFAULT 'false'\
				);");

	/*
		Key is for anonymous messages (future)
	*/
	db->Execute("CREATE TABLE IF NOT EXISTS tblMessageRequests(\
				IdentityID			INTEGER,\
				Day					DATE,\
				RequestIndex		INTEGER,\
				FromMessageList		BOOL CHECK(FromMessageList IN('true','false')) DEFAULT 'false',\
				Found				BOOL CHECK(Found IN('true','false')) DEFAULT 'false',\
				Tries				INTEGER DEFAULT 0,\
				Key					TEXT,\
				FromIdentityID		INTEGER\
				);");

	db->Execute("CREATE UNIQUE INDEX IF NOT EXISTS idxMessageRequest ON tblMessageRequests(IdentityID,Day,RequestIndex);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblMessageInserts(\
				LocalIdentityID		INTEGER,\
				Day					DATE,\
				InsertIndex			INTEGER,\
				MessageUUID			TEXT UNIQUE,\
				MessageXML			TEXT,\
				Inserted			BOOL CHECK(Inserted IN('true','false')) DEFAULT 'false',\
				SendDate			DATETIME\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblFileInserts(\
				FileInsertID		INTEGER PRIMARY KEY,\
				MessageUUID			TEXT,\
				FileName			TEXT,\
				Key					TEXT,\
				Size				INTEGER,\
				MimeType			TEXT,\
				Data				BLOB\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblMessageListInserts(\
				LocalIdentityID		INTEGER,\
				Day					DATE,\
				InsertIndex			INTEGER,\
				Inserted			BOOL CHECK(Inserted IN('true','false')) DEFAULT 'false'\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblAdministrationBoard(\
				BoardID						INTEGER UNIQUE,\
				ModifyLocalMessageTrust		INTEGER,\
				ModifyLocalTrustListTrust	INTEGER\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblBoardListInserts(\
				LocalIdentityID		INTEGER,\
				Day					DATE,\
				InsertIndex			INTEGER,\
				Inserted			BOOL CHECK(Inserted IN('true','false')) DEFAULT 'false'\
				);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblBoardListRequests(\
				IdentityID			INTEGER,\
				Day					DATE,\
				RequestIndex		INTEGER,\
				Found				BOOL CHECK(Found IN('true','false')) DEFAULT 'false'\
				);");

#ifdef FROST_SUPPORT

	db->Execute("CREATE TABLE IF NOT EXISTS tblFrostMessageRequests(\
				BoardID				INTEGER,\
				Day					DATE,\
				RequestIndex		INTEGER,\
				Found				BOOL CHECK(Found IN('true','false')) DEFAULT 'false',\
				Tries				INTEGER DEFAULT 0\
				);");

	db->Execute("CREATE UNIQUE INDEX IF NOT EXISTS idxFrostMessageRequest ON tblFrostMessageRequests(BoardID,Day,RequestIndex);");

#endif

	// begin thread db schema
	db->Execute("CREATE TABLE IF NOT EXISTS tblThread(\
				ThreadID		INTEGER PRIMARY KEY,\
				BoardID			INTEGER,\
				FirstMessageID	INTEGER,\
				LastMessageID	INTEGER\
				);");

	db->Execute("CREATE INDEX IF NOT EXISTS idxThread_BoardID ON tblThread(BoardID);");
	db->Execute("CREATE INDEX IF NOT EXISTS idxThread_FirstMessageID ON tblThread(FirstMessageID);");
	db->Execute("CREATE INDEX IF NOT EXISTS idxThread_LastMessageID ON tblThread(LastMessageID);");

	db->Execute("CREATE TABLE IF NOT EXISTS tblThreadPost(\
				ThreadID		INTEGER,\
				MessageID		INTEGER,\
				PostOrder		INTEGER\
				);");

	db->Execute("CREATE UNIQUE INDEX IF NOT EXISTS idxThreadPost_ThreadMessage ON tblThreadPost(ThreadID,MessageID);");
	db->Execute("CREATE INDEX IF NOT EXISTS idxThreadPost_MessageID ON tblThreadPost(MessageID);");

	db->Execute("CREATE TRIGGER IF NOT EXISTS trgDeleteOnThread AFTER DELETE ON tblThread\
				FOR EACH ROW\
				BEGIN\
					DELETE FROM tblThreadPost WHERE ThreadID=old.ThreadID;\
				END;");
	// end thread db schema

	// low / high / message count for each board
	db->Execute("CREATE VIEW IF NOT EXISTS vwBoardStats AS \
				SELECT tblBoard.BoardID AS 'BoardID', IFNULL(MIN(MessageID),0) AS 'LowMessageID', IFNULL(MAX(MessageID),0) AS 'HighMessageID', COUNT(MessageID) AS 'MessageCount' \
				FROM tblBoard LEFT JOIN tblMessageBoard ON tblBoard.BoardID=tblMessageBoard.BoardID \
				WHERE MessageID>=0 OR MessageID IS NULL \
				GROUP BY tblBoard.BoardID;");

	// calculates peer trust
	// do the (MessageTrust+1)*LocalTrustListTrust/(MessageTrust+1)/100.0 - so if MessageTrust or TrustListTrust is NULL, the calc will be NULL and it won't be included at all in the average
	// need the +1 so that when the values are 0 the result is not 0
	db->Execute("DROP VIEW IF EXISTS vwCalculatedPeerTrust;");
	db->Execute("CREATE VIEW IF NOT EXISTS vwCalculatedPeerTrust AS \
				SELECT TargetIdentityID, \
				ROUND(SUM(MessageTrust*(LocalTrustListTrust/100.0))/SUM(((MessageTrust+1)*LocalTrustListTrust/(MessageTrust+1))/100.0),0) AS 'PeerMessageTrust', \
				ROUND(SUM(TrustListTrust*(LocalTrustListTrust/100.0))/SUM(((TrustListTrust+1)*LocalTrustListTrust/(TrustListTrust+1))/100.0),0) AS 'PeerTrustListTrust' \
				FROM tblPeerTrust INNER JOIN tblIdentity ON tblPeerTrust.IdentityID=tblIdentity.IdentityID \
				WHERE LocalTrustListTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalTrustListTrust') \
				AND ( PeerTrustListTrust IS NULL OR PeerTrustListTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerTrustListTrust') ) \
				GROUP BY TargetIdentityID;");

	db->Execute("CREATE VIEW IF NOT EXISTS vwIdentityStats AS \
				SELECT tblIdentity.IdentityID, COUNT(tblMessage.MessageID) AS MessageCount, MIN(tblMessage.MessageDate) AS FirstMessageDate, MAX(tblMessage.MessageDate) AS LastMessageDate \
				FROM tblIdentity LEFT JOIN tblMessage ON tblIdentity.IdentityID=tblMessage.IdentityID \
				GROUP BY tblIdentity.IdentityID;");

	/*
		These peer trust calculations are too CPU intensive to be triggers - they were called every time a new trust list was processed
		All trust levels will now be recalculated every hour in the PeriodicDBMaintenance class
	*/
	// drop existing triggers
	db->Execute("DROP TRIGGER IF EXISTS trgDeleteOntblPeerTrust;");
	db->Execute("DROP TRIGGER IF EXISTS trgInsertOntblPeerTrust;");
	db->Execute("DROP TRIGGER IF EXISTS trgUpdateOntblPeerTrust;");
	db->Execute("DROP TRIGGER IF EXISTS trgUpdateLocalTrustLevels;");
/*
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
*/

	db->Execute("CREATE TRIGGER IF NOT EXISTS trgDeleteMessage AFTER DELETE ON tblMessage \
				FOR EACH ROW \
				BEGIN \
					DELETE FROM tblMessageBoard WHERE tblMessageBoard.MessageID=old.MessageID;\
					DELETE FROM tblMessageReplyTo WHERE tblMessageReplyTo.MessageID=old.MessageID;\
				END;");

	db->Execute("DROP TRIGGER IF EXISTS trgDeleteIdentity;");
	db->Execute("CREATE TRIGGER IF NOT EXISTS trgDeleteIdentity AFTER DELETE ON tblIdentity \
				FOR EACH ROW \
				BEGIN \
					DELETE FROM tblIdentityRequests WHERE IdentityID=old.IdentityID;\
					DELETE FROM tblIntroductionPuzzleRequests WHERE IdentityID=old.IdentityID;\
					DELETE FROM tblMessageListRequests WHERE IdentityID=old.IdentityID;\
					DELETE FROM tblMessageRequests WHERE IdentityID=old.IdentityID;\
					DELETE FROM tblPeerTrust WHERE IdentityID=old.IdentityID;\
					DELETE FROM tblTrustListRequests WHERE IdentityID=old.IdentityID;\
					DELETE FROM tblIdentityTrust WHERE IdentityID=old.IdentityID;\
				END;");

	db->Execute("DROP TRIGGER IF EXISTS trgDeleteLocalIdentity;");
	db->Execute("CREATE TRIGGER IF NOT EXISTS trgDeleteLocalIdentity AFTER DELETE ON tblLocalIdentity \
				FOR EACH ROW \
				BEGIN \
					DELETE FROM tblIdentityIntroductionInserts WHERE LocalIdentityID=old.LocalIdentityID;\
					DELETE FROM tblIntroductionPuzzleInserts WHERE LocalIdentityID=old.LocalIdentityID;\
					DELETE FROM tblLocalIdentityInserts WHERE LocalIdentityID=old.LocalIdentityID;\
					DELETE FROM tblMessageInserts WHERE LocalIdentityID=old.LocalIdentityID;\
					DELETE FROM tblMessageListInserts WHERE LocalIdentityID=old.LocalIdentityID;\
					DELETE FROM tblTrustListInserts WHERE LocalIdentityID=old.LocalIdentityID;\
					DELETE FROM tblIdentityTrust WHERE LocalIdentityID=old.LocalIdentityID;\
				END;");

	db->Execute("CREATE TRIGGER IF NOT EXISTS trgDeleteBoard AFTER DELETE ON tblBoard \
				FOR EACH ROW \
				BEGIN \
					DELETE FROM tblMessageBoard WHERE BoardID=old.BoardID;\
				END;");

	// delete introduction puzzles that were half-way inserted
	db->Execute("DELETE FROM tblIntroductionPuzzleInserts WHERE Day IS NULL AND InsertIndex IS NULL;");

	// delete stale introduction puzzles (2 or more days old)
	date-=Poco::Timespan(2,0,0,0,0);
	db->Execute("DELETE FROM tblIntroductionPuzzleInserts WHERE Day<='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"';");
	db->Execute("DELETE FROM tblIntroductionPuzzleRequests WHERE Day<='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"';");

	date=Poco::Timestamp();
	// insert SomeDude's public key
	db->Execute("INSERT INTO tblIdentity(PublicKey,DateAdded,LocalTrustListTrust,AddedMethod) VALUES('SSK@NuBL7aaJ6Cn4fB7GXFb9Zfi8w1FhPyW3oKgU9TweZMw,iXez4j3qCpd596TxXiJgZyTq9o-CElEuJxm~jNNZAuA,AQACAAE/','"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S")+"',50,'Seed Identity');");
	// insert Shadow Panther's public key - haven't seen in a while - disabling for now
	//db->Execute("INSERT INTO tblIdentity(PublicKey,DateAdded,AddedMethod) VALUES('SSK@~mimyB1kmH4f7Cgsd2wM2Qv2NxrZHRMM6IY8~7EWRVQ,fxTKkR0TYhgMYb-vEGAv55sMOxCGD2xhE4ZxWHxdPz4,AQACAAE/','"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S")+"','Seed Identity');");
	// insert garfield's public key -haven't seen in a while - disabling for now
	//db->Execute("INSERT INTO tblIdentity(PublicKey,DateAdded,AddedMethod) VALUES('SSK@T8l1IEGU4-PoASFzgc2GYhIgRzUvZsKdoQWeuLHuTmM,QLxAPfkGis8l5NafNpSCdbxzXhBlu9WL8svcqJw9Mpo,AQACAAE/','"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S")+"','Seed Identity');");
	// insert alek's public key - haven't seen in a while - disabling for now
	//db->Execute("INSERT INTO tblIdentity(PublicKey,DateAdded,AddedMethod) VALUES('SSK@lTjeI6V0lQsktXqaqJ6Iwk4TdsHduQI54rdUpHfhGbg,0oTYfrxxx8OmdU1~60gqpf3781qzEicM4Sz97mJsBM4,AQACAAE/','"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S")+"','Seed Identity');");
	// insert Luke771's public key - haven't seen in a while - disabling for now
	//db->Execute("INSERT INTO tblIdentity(PublicKey,DateAdded,AddedMethod) VALUES('SSK@mdXK~ZVlfTZhF1SLBrvZ--i0vOsOpa~w9wv~~psQ-04,gXonsXKc7aexKSO8Gt8Fwre4Qgmmbt2WueO7VzxNKkk,AQACAAE/','"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S")+"','Seed Identity');");
	// insert falafel's public key
	db->Execute("INSERT INTO tblIdentity(PublicKey,DateAdded,AddedMethod) VALUES('SSK@IxVqeqM0LyYdTmYAf5z49SJZUxr7NtQkOqVYG0hvITw,RM2wnMn5zAufCMt5upkkgq25B1elfBAxc7htapIWg1c,AQACAAE/','"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S")+"','Seed Identity');");
	// insert cptn_insano's public key
	db->Execute("INSERT INTO tblIdentity(PublicKey,DateAdded,AddedMethod) VALUES('SSK@bloE1LJ~qzSYUkU2nt7sB9kq060D4HTQC66pk5Q8NpA,DOOASUnp0kj6tOdhZJ-h5Tk7Ka50FSrUgsH7tCG1usU,AQACAAE/','"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S")+"','Seed Identity');");
	// insert Flink's public key - haven't seen in a while - disabling for now
	//db->Execute("INSERT INTO tblIdentity(PublicKey,DateAdded,AddedMethod) VALUES('SSK@q2TtkNBOuuniyJ56~8NSopCs3ttwe5KlB31ugZtWmXA,6~PzIupS8YK7L6oFNpXGKJmHT2kBMDfwTg73nHdNur8,AQACAAE/','"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S")+"','Seed Identity');");
	// insert Kane's public key
	db->Execute("INSERT INTO tblIdentity(PublicKey,DateAdded,AddedMethod) VALUES('SSK@Ofm~yZivDJ5Z2fSzZbMiLEUUQaIc0KHRdZMBTaPLO6I,WLm4s4hNbOOurJ6ijfOq4odz7-dN7uTUvYxJRwWnlMI,AQACAAE/','"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S")+"','Seed Identity');");
	// inserts boardstat's public key
	db->Execute("INSERT INTO tblIdentity(PublicKey,DateAdded,AddedMethod) VALUES('SSK@aYWBb6zo2AM13XCNhsmmRKMANEx6PG~C15CWjdZziKA,X1pAG4EIqR1gAiyGFVZ1iiw-uTlh460~rFACJ7ZHQXk,AQACAAE/','"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S")+"','Seed Identity');");

	// TODO remove sometime after 0.1.17
	FixCapitalBoardNames(db);

	// run analyze - may speed up some queries
	db->Execute("ANALYZE;");

}

const bool VerifyDB(SQLite3DB::DB *db)
{
	SQLite3DB::Statement st=db->Prepare("PRAGMA integrity_check;");
	st.Step();
	if(st.RowReturned())
	{
		std::string res="";
		st.ResultText(0,res);
		if(res=="ok")
		{
			return true;
		}
		else
		{
			// try to reindex and vacuum database in case of index corruption
			st=db->Prepare("REINDEX;");
			st.Step();
			st=db->Prepare("VACUUM;");
			st.Step();

			// check integrity again
			st=db->Prepare("PRAGMA integrity_check;");
			st.Step();
			st.ResultText(0,res);
			if(res=="ok")
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
}

const std::string TestDBIntegrity(SQLite3DB::DB *db)
{
	std::string result="";

	SQLite3DB::Statement st=db->Prepare("PRAGMA integrity_check;");
	st.Step();
	while(st.RowReturned())
	{
		std::string text="";
		st.ResultText(0,text);
		result+=text;
		st.Step();
	}
	return result;
}
