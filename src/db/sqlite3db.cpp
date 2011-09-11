#include "../../include/db/sqlite3db.h"
#include "../../include/stringfunctions.h"

#ifdef QUERY_LOG
#include <Poco/Logger.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FileChannel.h>
#include "../../include/stringfunctions.h"
#endif

#ifdef XMEM
	#include <xmem.h>
#endif

namespace SQLite3DB
{

bool SQLite3DB::DB::m_profiling=false;
std::map<std::string,SQLite3DB::DB::ProfileData> SQLite3DB::DB::m_profiledata;
Poco::FastMutex SQLite3DB::DB::m_profilemutex;

DB::DB()
{
	Initialize();
}

DB::DB(const std::string &filename)
{
	Initialize();
	Open(filename);
}

DB::~DB()
{
	if(IsOpen())
	{
		Close();
	}
}

const bool DB::Close()
{
	if(IsOpen())
	{
		sqlite3_stmt *st=0;
		while((st=sqlite3_next_stmt(m_db,0))!=0)
		{
			sqlite3_finalize(st);
		}

		m_lastresult=sqlite3_close(m_db);
		if(m_lastresult==SQLITE_OK)
		{
			m_db=NULL;
			return true;
		}
		else
		{
			std::string errmsg("");
			int err=GetLastExtendedError(errmsg);
			HandleError(err,errmsg,"DB::Close");
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool DB::Execute(const std::string &sql)
{
	if(IsOpen())
	{
		m_lastresult=sqlite3_exec(m_db,sql.c_str(),NULL,NULL,NULL);
#ifdef QUERY_LOG
		Poco::Logger::get("querylog").information("Execute : "+sql);
#endif
		if(m_lastresult==SQLITE_OK)
		{
			return true;
		}
		else
		{
			std::string errmsg("");
			int err=GetLastExtendedError(errmsg);
			HandleError(err,errmsg,"DB::Execute");
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool DB::ExecuteInsert(const std::string &sql, long &insertid)
{
	if(IsOpen())
	{
		m_lastresult=sqlite3_exec(m_db,sql.c_str(),NULL,NULL,NULL);
#ifdef QUERY_LOG
		Poco::Logger::get("querylog").information("Execute Insert : "+sql);
#endif
		if(m_lastresult==SQLITE_OK)
		{
			insertid=sqlite3_last_insert_rowid(m_db);
			return true;
		}
		else
		{
			std::string errmsg("");
			int err=GetLastExtendedError(errmsg);
			HandleError(err,errmsg,"DB::ExecuteInsert");
			return false;
		}
	}
	else
	{
		return false;
	}
}

const int DB::GetLastError(std::string &errormessage)
{
	if(IsOpen())
	{
		int errcode=sqlite3_errcode(m_db);
		const char *errmsg=sqlite3_errmsg(m_db);
		if(errmsg)
		{
			errormessage=errmsg;
		}
		return errcode;
	}
	else
	{
		return SQLITE_OK;
	}
}

const int DB::GetLastExtendedError(std::string &errormessage)
{
	if(IsOpen())
	{
		int errcode=sqlite3_extended_errcode(m_db);
		const char *errmsg=sqlite3_errmsg(m_db);
		if(errmsg)
		{
			errormessage=errmsg;
		}
		return errcode;
	}
	else
	{
		return SQLITE_OK;
	}
}

void DB::GetProfileData(std::map<std::string,ProfileData> &profiledata, const bool cleardata)
{
	Poco::ScopedLock<Poco::FastMutex> guard(m_profilemutex);
	profiledata=m_profiledata;
	if(cleardata==true)
	{
		m_profiledata.clear();
	}
}

void DB::HandleError(const int extendederrorcode, const std::string &errormessage, const std::string &extramessage)
{
	// which errors are really fatal?
	if((extendederrorcode & SQLITE_NOMEM)==SQLITE_NOMEM)
	{
		throw Exception("SQLite3DB SQLITE_NOMEM memory allocation exception. "+errormessage+" "+extramessage);
	}
	else if((extendederrorcode & SQLITE_IOERR_NOMEM)==SQLITE_IOERR_NOMEM)
	{
		throw Exception("SQLite3DB SQLITE_IOERR_NOMEM memory allocation exception. "+errormessage+" "+extramessage);
	}
}

void DB::Initialize()
{
	m_db=NULL;
	m_lastresult=SQLITE_OK;
}

const bool DB::IsOpen() const
{
	return m_db ? true : false;
}

const bool DB::Open(const std::string &filename)
{
	if(IsOpen()==true)
	{
		Close();	
	}
	if(IsOpen()==false)
	{
		m_lastresult=sqlite3_open(filename.c_str(),&m_db);
		if(m_lastresult==SQLITE_OK)
		{
			if(IsProfiling())
			{
				StartProfiling();
			}
			return true;
		}
		else
		{
			std::string errmsg("");
			int err=GetLastExtendedError(errmsg);
			HandleError(err,errmsg,"DB::Open");
			return false;
		}
	}
	else
	{
		return false;
	}
}

Statement DB::Prepare(const std::string &sql)
{
	if(IsOpen())
	{
		sqlite3_stmt *statement=NULL;
		m_lastresult=sqlite3_prepare_v2(m_db,sql.c_str(),sql.size(),&statement,NULL);
#ifdef QUERY_LOG
		size_t temp=reinterpret_cast<size_t>(statement);
		std::string tempstr("");
		StringFunctions::Convert(temp,tempstr);
		Poco::Logger::get("querylog").information("Prepare : "+sql+" "+tempstr);
#endif
		if(m_lastresult==SQLITE_OK)
		{
			return Statement(statement,this);
		}
		else
		{
			std::string errmsg("");
			int err=GetLastExtendedError(errmsg);
			HandleError(err,errmsg,"DB::Prepare");
			return Statement();
		}
	}
	else
	{
		return Statement();
	}
}

void DB::ProfileCallback(void *db, const char *sql, sqlite3_uint64 tottime)
{
	Poco::ScopedLock<Poco::FastMutex> guard(m_profilemutex);

	if(sql && sql[0]!=0)
	{
		std::string ssql(sql);
		ssql=StringFunctions::Replace(ssql,"\t"," ");
		m_profiledata[ssql].m_count++;
		m_profiledata[ssql].m_time+=tottime;
	}
}

Recordset DB::Query(const std::string &sql)
{
	if(IsOpen())
	{
		char **rs=NULL;
		int rows,cols;
		m_lastresult=sqlite3_get_table(m_db,sql.c_str(),&rs,&rows,&cols,NULL);
#ifdef QUERY_LOG
		Poco::Logger::get("querylog").information("Query : "+sql);
#endif
		if(m_lastresult==SQLITE_OK)
		{
			return Recordset(rs,rows,cols);
		}
		else
		{
			sqlite3_free_table(rs);
			std::string errmsg("");
			int err=GetLastExtendedError(errmsg);
			HandleError(err,errmsg,"DB::Query");
			return Recordset();
		}
	}
	else
	{
		return Recordset();
	}
}

const int DB::SetBusyTimeout(const int ms)
{
	if(IsOpen())
	{
		m_lastresult=sqlite3_busy_timeout(m_db,ms);
		return m_lastresult;
	}
	else
	{
		return SQLITE_ERROR;
	}
}

void DB::StartProfiling()
{
	Poco::ScopedLock<Poco::FastMutex> guard(m_profilemutex);
	sqlite3_profile(m_db,DB::ProfileCallback,this);
	m_profiling=true;
}

}	// namespace
