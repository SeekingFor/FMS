#include "../include/dbconversions.h"
#include "../include/db/sqlite3db.h"
#include "../include/stringfunctions.h"

#include <Poco/Timestamp.h>
#include <Poco/DateTimeFormatter.h>

void ConvertDB0100To0101(SQLite3DB::DB *db)
{
	// added unique constraint to public and private key
	db->Execute("CREATE TEMPORARY TABLE tblLocalIdentityTemp AS SELECT * FROM tblLocalIdentity;");
	db->Execute("DROP TABLE IF EXISTS tblLocalIdentity;");
	db->Execute("CREATE TABLE IF NOT EXISTS tblLocalIdentity(\
				LocalIdentityID			INTEGER PRIMARY KEY,\
				Name					TEXT,\
				PublicKey				TEXT UNIQUE,\
				PrivateKey				TEXT UNIQUE,\
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
	db->Execute("INSERT INTO tblLocalIdentity SELECT * FROM tblLocalIdentityTemp;");
	db->Execute("DROP TABLE IF EXISTS tblLocalIdentityTemp;");
	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=1;");
}

void ConvertDB0101To0103(SQLite3DB::DB *db)
{
	// remove default 50 from trust fields and set default to NULL
	db->Execute("CREATE TEMPORARY TABLE tblIdentityTemp AS SELECT * FROM tblIdentity;");
	db->Execute("DROP TABLE IF EXISTS tblIdentity;");
	db->Execute("CREATE TABLE IF NOT EXISTS tblIdentity(\
				IdentityID			INTEGER PRIMARY KEY,\
				PublicKey			TEXT UNIQUE,\
				Name				TEXT,\
				SingleUse			BOOL CHECK(SingleUse IN('true','false')) DEFAULT 'false',\
				PublishTrustList	BOOL CHECK(PublishTrustList IN('true','false')) DEFAULT 'false',\
				PublishBoardList	BOOL CHECK(PublishBoardList IN('true','false')) DEFAULT 'false',\
				DateAdded			DATETIME,\
				LastSeen			DATETIME,\
				LocalMessageTrust	INTEGER CHECK(LocalMessageTrust BETWEEN 0 AND 100) DEFAULT NULL,\
				PeerMessageTrust	INTEGER CHECK(PeerMessageTrust BETWEEN 0 AND 100) DEFAULT NULL,\
				LocalTrustListTrust	INTEGER CHECK(LocalTrustListTrust BETWEEN 0 AND 100) DEFAULT NULL,\
				PeerTrustListTrust	INTEGER CHECK(PeerTrustListTrust BETWEEN 0 AND 100) DEFAULT NULL\
				);");
	db->Execute("INSERT INTO tblIdentity SELECT * FROM tblIdentityTemp;");
	db->Execute("DROP TABLE IF EXISTS tblIdentityTemp;");

	// add SaveReceivedMessages field to tblBoard
	db->Execute("ALTER TABLE tblBoard ADD COLUMN SaveReceivedMessages	BOOL CHECK(SaveReceivedMessages IN('true','false')) DEFAULT 'true';");

	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=3;");
}

void ConvertDB0103To0104(SQLite3DB::DB *db)
{
	// add MessageIndex to tblMessage
	Poco::Timestamp date;
	db->Execute("ALTER TABLE tblMessage ADD COLUMN MessageIndex	INTEGER;");
	db->Execute("CREATE UNIQUE INDEX IF NOT EXISTS idxMessageRequest ON tblMessageRequests(IdentityID,Day,RequestIndex);");
	db->Execute("ALTER TABLE tblLocalIdentity ADD COLUMN DateCreated DATETIME;");
	db->Execute("UPDATE tblLocalIdentity SET DateCreated='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S")+"' WHERE DateCreated IS NULL;");
	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=4;");
}

void ConvertDB0104To0105(SQLite3DB::DB *db)
{
	// add AddedMethod, MessageTrustComment, TrustListTrustComment to tblIdentity
	// add MessageTrustComment,TrustListTrustComment to tblPeerTrust
	db->Execute("ALTER TABLE tblIdentity ADD COLUMN AddedMethod TEXT;");
	db->Execute("ALTER TABLE tblIdentity ADD COLUMN MessageTrustComment TEXT;");
	db->Execute("ALTER TABLE tblIdentity ADD COLUMN TrustListTrustComment TEXT;");
	db->Execute("ALTER TABLE tblPeerTrust ADD COLUMN MessageTrustComment TEXT;");
	db->Execute("ALTER TABLE tblPeerTrust ADD COLUMN TrustListTrustComment TEXT;");
	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=5;");
}

void ConvertDB0105To0106(SQLite3DB::DB *db)
{
	// add Publish Freesite
	db->Execute("ALTER TABLE tblLocalIdentity ADD COLUMN PublishFreesite BOOL CHECK(PublishFreesite IN('true','false')) DEFAULT 'false';");
	db->Execute("ALTER TABLE tblLocalIdentity ADD COLUMN LastInsertedFreesite DATETIME;");
	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=6;");
}

void ConvertDB0106To0107(SQLite3DB::DB *db)
{
	// add AddedMethod to tblBoard
	db->Execute("ALTER TABLE tblBoard ADD COLUMN AddedMethod TEXT;");
	db->Execute("ALTER TABLE tblIdentity ADD COLUMN Hidden BOOL CHECK(Hidden IN('true','false')) DEFAULT 'false';");
	db->Execute("UPDATE tblIdentity SET Hidden='false' WHERE Hidden IS NULL;");
	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=7;");
}

void ConvertDB0107To0108(SQLite3DB::DB *db)
{
	// add FreesiteEdition to tblLocalIdentity and tblIdentity
	db->Execute("ALTER TABLE tblLocalIdentity ADD COLUMN FreesiteEdition INTEGER;");
	db->Execute("ALTER TABLE tblIdentity ADD COLUMN FreesiteEdition INTEGER;");
	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=8;");
}

void ConvertDB0108To0109(SQLite3DB::DB *db)
{
	db->Execute("CREATE TABLE IF NOT EXISTS tblFileInserts(\
			FileInsertID		INTEGER PRIMARY KEY,\
			MessageUUID			TEXT,\
			FileName			TEXT,\
			Key					TEXT,\
			Size				INTEGER,\
			Data				BLOB\
			);");
	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=9;");
}

void ConvertDB0109To0110(SQLite3DB::DB *db)
{
	db->Execute("ALTER TABLE tblFileInserts ADD COLUMN MimeType TEXT;");
	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=10;");
}

void ConvertDB0110To0111(SQLite3DB::DB *db)
{
	/*
	Drop MessageTrustComment, TrustListTrustComment FROM tblIdentity

	Drop InsertingMessageList, InsertingBoardList FROM tblLocalIdentity
	Add MinMessageDelay, MaxMessageDelay to tblLocalIdentity Default 0

	Add SendDate to tblMessageInserts
	*/

	db->Execute("ALTER TABLE tblMessageInserts ADD COLUMN SendDate DATETIME;");

	db->Execute("CREATE TEMPORARY TABLE tblLocalIdentityTemp AS SELECT * FROM tblLocalIdentity;");
	db->Execute("DROP TABLE IF EXISTS tblLocalIdentity;");
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
	db->Execute("INSERT INTO tblLocalIdentity SELECT LocalIdentityID,Name,PublicKey,PrivateKey,SingleUse,PublishTrustList,PublishBoardList,PublishFreesite,FreesiteEdition,InsertingIdentity,LastInsertedIdentity,InsertingPuzzle,LastInsertedPuzzle,InsertingTrustList,LastInsertedTrustList,LastInsertedBoardList,LastInsertedMessageList,LastInsertedFreesite,DateCreated,0,0 FROM tblLocalIdentityTemp;");
	db->Execute("DROP TABLE IF EXISTS tblLocalIdentityTemp;");

	db->Execute("CREATE TEMPORARY TABLE tblIdentityTemp AS SELECT * FROM tblIdentity;");
	db->Execute("DROP TABLE IF EXISTS tblIdentity;");
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
				Hidden					BOOL CHECK(Hidden IN('true','false')) DEFAULT 'false'\
				);");
	db->Execute("INSERT INTO tblIdentity SELECT IdentityID,PublicKey,Name,SingleUse,PublishTrustList,PublishBoardList,FreesiteEdition,DateAdded,LastSeen,LocalMessageTrust,PeerMessageTrust,LocalTrustListTrust,PeerTrustListTrust,AddedMethod,Hidden FROM tblIdentityTemp;");
	db->Execute("DROP TABLE IF EXISTS tblIdentityTemp;");

	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=11;");
}

void ConvertDB0111To0112(SQLite3DB::DB *db)
{
	/*
		Add Section, SortOrder, ValidValues to tblOption
		Add PurgeDate to tblIdentity
	*/

	db->Execute("ALTER TABLE tblOption ADD COLUMN Section TEXT;");
	db->Execute("ALTER TABLE tblOption ADD COLUMN SortOrder INTEGER;");
	db->Execute("ALTER TABLE tblOption ADD COLUMN ValidValues TEXT;");

	db->Execute("ALTER TABLE tblIdentity ADD COLUMN PurgeDate DATETIME;");

	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=12;");
}

void ConvertDB0112To0113(SQLite3DB::DB *db)
{
	// Add Tries and Key (for anonymous messages) to tblMessageRequests	

	db->Execute("ALTER TABLE tblMessageRequests ADD COLUMN Tries INTEGER DEFAULT 0;");
	db->Execute("ALTER TABLE tblMessageRequests ADD COLUMN Key TEXT;");

	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=13;");
}

void ConvertDB0113To0114(SQLite3DB::DB *db)
{

	db->Execute("ALTER TABLE tblBoard ADD COLUMN Forum TEXT CHECK(Forum IN ('true','false')) DEFAULT 'false';");
	db->Execute("ALTER TABLE tblMessage ADD COLUMN Read INTEGER CHECK(Read IN (0,1)) DEFAULT 0;");

	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=14;");
}

void ConvertDB0114To0115(SQLite3DB::DB *db)
{

	db->Execute("ALTER TABLE tblOption ADD COLUMN DisplayType TEXT CHECK (DisplayType IN ('textbox','textarea','select','multiselect')) DEFAULT 'textbox';");
	db->Execute("ALTER TABLE tblOption ADD COLUMN DisplayParam1 TEXT;");
	db->Execute("ALTER TABLE tblOption ADD COLUMN DisplayParam2 TEXT;");
	db->Execute("ALTER TABLE tblOption ADD COLUMN Mode TEXT CHECK (Mode IN ('simple','advanced')) DEFAULT 'simple';");

	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=15;");
}

void ConvertDB0115To0116(SQLite3DB::DB *db)
{
	// Add FromIdentityID to tblMessageRequests so we know who we got this index from
	// Add FailureCount

	db->Execute("ALTER TABLE tblMessageRequests ADD COLUMN FromIdentityID INTEGER;");
	db->Execute("ALTER TABLE tblIdentity ADD COLUMN FailureCount INTEGER CHECK(FailureCount>=0) DEFAULT 0;");

	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=16;");
}

void ConvertDB0116To0117(SQLite3DB::DB *db)
{
	// Add InsertDate to tblMessage

	db->Execute("ALTER TABLE tblMessage ADD COLUMN InsertDate DATE;");
	db->Execute("DROP INDEX IF EXISTS idxThreadPost_ThreadID;");

	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=17;");
}

void ConvertDB0117To0118(SQLite3DB::DB *db)
{

	db->Execute("UPDATE tblOption SET OptionValue='news' WHERE Option='FrostMessageBase' AND OptionValue='frost';");

	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=18;");
}

void ConvertDB0118To0119(SQLite3DB::DB *db)
{
	db->Execute("ALTER TABLE tblMessageBoard ADD COLUMN BoardMessageID INTEGER;");
	db->Execute("ALTER TABLE tblBoard ADD COLUMN NextMessageID INTEGER NOT NULL DEFAULT 1;");

	// TODO - fill in missing BoardMessageIDs
	db->Execute("BEGIN;");
	SQLite3DB::Statement boardst=db->Prepare("SELECT BoardID FROM tblBoard;");
	SQLite3DB::Statement boardmessagest=db->Prepare("SELECT MessageID FROM tblMessageBoard WHERE BoardID=? ORDER BY MessageID;");
	SQLite3DB::Statement updatemessageboardst=db->Prepare("UPDATE tblMessageBoard SET BoardMessageID=? WHERE MessageID=? AND BoardID=?;");
	SQLite3DB::Statement updateboardst=db->Prepare("UPDATE tblBoard SET NextMessageID=? WHERE BoardID=?;");
	boardst.Step();
	while(boardst.RowReturned())
	{
		int boardmessageid=1;
		int boardid=0;
		boardst.ResultInt(0,boardid);

		boardmessagest.Bind(0,boardid);
		boardmessagest.Step();
		while(boardmessagest.RowReturned())
		{
			int messageid=0;
			boardmessagest.ResultInt(0,messageid);

			updatemessageboardst.Bind(0,boardmessageid++);
			updatemessageboardst.Bind(1,messageid);
			updatemessageboardst.Bind(2,boardid);
			updatemessageboardst.Step();
			updatemessageboardst.Reset();

			boardmessagest.Step();
		}
		boardmessagest.Reset();

		updateboardst.Bind(0,boardmessageid);
		updateboardst.Bind(1,boardid);
		updateboardst.Step();
		updateboardst.Reset();

		boardst.Step();
	}
	db->Execute("COMMIT;");

	db->Execute("DROP INDEX IF EXISTS idxMessageBoard_BoardID;");
	db->Execute("DROP VIEW IF EXISTS vwBoardStats;");
	db->Execute("INSERT INTO tblOption(Option,OptionValue) VALUES('UniqueBoardMessageIDs','false');");

	db->Execute("UPDATE tblDBVersion SET Major=1, Minor=19;");
}

void FixCapitalBoardNames(SQLite3DB::DB *db)
{

	SQLite3DB::Statement st=db->Prepare("SELECT BoardID,BoardName FROM tblBoard WHERE BoardID NOT IN (SELECT BoardID FROM tblAdministrationBoard);");
	SQLite3DB::Statement st2=db->Prepare("SELECT BoardID FROM tblBoard WHERE BoardName=?;");
	SQLite3DB::Statement del=db->Prepare("DELETE FROM tblBoard WHERE BoardID=?;");
	SQLite3DB::Statement upd=db->Prepare("UPDATE tblBoard SET BoardName=? WHERE BoardID=?;");
	SQLite3DB::Statement upd2=db->Prepare("UPDATE tblMessage SET ReplyBoardID=? WHERE ReplyBoardID=?;");
	SQLite3DB::Statement upd3=db->Prepare("UPDATE tblMessageBoard SET BoardID=? WHERE BoardID=?;");

	st.Step();
	while(st.RowReturned())
	{
		int boardid=0;
		int newboardid=0;
		std::string name="";
		std::string lowername="";

		st.ResultInt(0,boardid);
		st.ResultText(1,name);

		lowername=name;
		StringFunctions::LowerCase(lowername,lowername);
       
		if(name!=lowername)
		{
			st2.Bind(0,lowername);
			st2.Step();

			if(st2.RowReturned())
			{
				st2.ResultInt(0,newboardid);

				upd2.Bind(0,newboardid);
				upd2.Bind(1,boardid);
				upd2.Step();
				upd2.Reset();

				upd3.Bind(0,newboardid);
				upd3.Bind(1,boardid);
				upd3.Step();
				upd3.Reset();

				del.Bind(0,boardid);
				del.Step();
				del.Reset();
			}
			else
			{
				upd.Bind(0,lowername);
				upd.Bind(1,boardid);
				upd.Step();
				upd.Reset();
			}

			st2.Reset();
		}
       
		st.Step();
	}

}
