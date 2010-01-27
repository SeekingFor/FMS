#ifndef _ithreaddatabase_
#define _ithreaddatabase_

#include "db/sqlite3db.h"

// each thread using the database must inherit from this class
class IThreadDatabase
{
public:
	IThreadDatabase():m_db(0)	{}
	virtual ~IThreadDatabase()
	{
		delete m_db;
	}
	
	void LoadDatabase()
	{
		if(m_db)
		{
			delete m_db;
		}
		m_db=new SQLite3DB::DB("fms.db3");
		m_db->SetBusyTimeout(40000);		// set timeout to 40 seconds
		m_db->Execute("PRAGMA temp_store=2;");	// store temporary tables in memory
		m_db->Execute("PRAGMA synchronous = FULL;");

		// MessageInserter will insert a record into this temp table which the MessageListInserter will query for and insert a MessageList when needed
		m_db->Execute("CREATE TEMPORARY TABLE IF NOT EXISTS tmpMessageListInsert(\
					MessageListInsertID	INTEGER PRIMARY KEY,\
					LocalIdentityID		INTEGER,\
					Date				DATETIME\
					);");

		// A temporary table that will hold a local identity id of the last identity who was loaded in the trust list page
		m_db->Execute("CREATE TEMPORARY TABLE IF NOT EXISTS tmpLocalIdentityPeerTrustPage(\
					LocalIdentityID		INTEGER\
					);");

		// Temporary table for form passwords
		m_db->Execute("CREATE TEMPORARY TABLE IF NOT EXISTS tmpFormPassword(\
					Date			DATETIME,\
					Password		TEXT\
					);");

	}
	
protected:
	SQLite3DB::DB *m_db;
};

#endif	// _ithreaddatabase_
