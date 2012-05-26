#ifndef _sqlite3db_
#define _sqlite3db_

#include <sqlite3.h>
#include <string>
#include <map>
#include "../sqlite3db.h"

#include <Poco/Mutex.h>

#if SQLITE_VERSION_NUMBER<3006006
#error "Your version of SQLite is too old!  3.6.6.2 or later is required."
#endif

#if SQLITE_VERSION_NUMBER==3006014
#error "SQLite 3.6.14 does not work with FMS!"
#endif

namespace SQLite3DB
{

enum {
	ResultCodeMask = 0xff // mask extended error code
};

class Exception
{
public:
	Exception(const std::string &message):m_message(message)	{ }
	const std::string what() const { return m_message; }

private:
	std::string m_message;
};

class DB
{
public:
	DB();
	DB(const std::string &filename);
	~DB();

	struct ProfileData
	{
		ProfileData():m_count(0),m_time(0)	{ }

		sqlite3_uint64 m_count;
		sqlite3_uint64 m_time;
	};
	
	const bool Open(const std::string &filename);
	const bool Close();
	
	const int GetLastResult() { return m_lastresult; }			// gets result of last action taken - standard sqlite3 return codes
	const int GetLastError(std::string &errormessage);			// gets last error of this database connection
	const int GetLastExtendedError(std::string &errormessage);	// gets last extended error of this database connection
	
	const bool IsOpen() const;
	
	const bool Execute(const std::string &sql);	// executes a statement returing true if successful
	const bool ExecuteInsert(const std::string &sql, long &insertid);	// call when inserting data and the insertid of the row inserted is needed, otherwise Execute can be called if the row id is not needed
	Recordset Query(const std::string &sql);		// executes a statement returning a recordset
	Statement Prepare(const std::string &sql);	// prepares a statement returning the statement object

	const int SetBusyTimeout(const int ms);		// sets busy timeout in ms.  SQLite will wait for a lock up to this many ms before returning SQLITE_BUSY

	sqlite3 *GetDB() { return m_db; }

	void StartProfiling();
	const bool IsProfiling() const	{ Poco::ScopedLock<Poco::FastMutex> guard(m_profilemutex); return m_profiling; }
	void GetProfileData(std::map<std::string,ProfileData> &profiledata, const bool cleardata=false);

	static void HandleError(const int extendederrorcode, const std::string &errormessage, const std::string &extramessage="");

private:
	void Initialize();

	static void ProfileCallback(void *db, const char *sql, sqlite3_uint64 tottime);
	
	sqlite3 *m_db;
	int m_lastresult;

	static bool m_profiling;
	static std::map<std::string,ProfileData> m_profiledata;
	static Poco::FastMutex m_profilemutex;

};

}	// namespace

#endif	// _sqlite3db_
