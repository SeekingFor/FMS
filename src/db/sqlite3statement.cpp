#include "../../include/db/sqlite3db/sqlite3statement.h"

#ifdef QUERY_LOG
#include <Poco/Logger.h>
#include <Poco/FormattingChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FileChannel.h>
#include "../../include/stringfunctions.h"
#endif

#ifdef XMEM
	#include <xmem.h>
#endif

namespace SQLite3DB
{

Poco::FastMutex Statement::m_mutex;
std::map<sqlite3_stmt *, long> Statement::m_statementcount;

Statement::Statement():m_statement(0),m_parametercount(0),m_resultcolumncount(0),m_rowreturned(false),m_lastinsertrowid(-1),m_db(0)
{

}

Statement::Statement(sqlite3_stmt *statement, DB *db):m_statement(statement),m_rowreturned(false),m_lastinsertrowid(-1),m_db(db)
{
	m_parametercount=sqlite3_bind_parameter_count(m_statement);
	m_resultcolumncount=sqlite3_column_count(m_statement);

	if(m_statement)
	{
		Poco::ScopedLock<Poco::FastMutex> g(m_mutex);
		m_statementcount[m_statement]++;
	}
}

Statement::Statement(const Statement &rhs):m_statement(0),m_parametercount(0),m_resultcolumncount(0),m_rowreturned(false),m_lastinsertrowid(-1),m_db(0)
{
	*this=rhs;
}

Statement::~Statement()
{
	Finalize();
}

const bool Statement::Bind(const int column)
{
	if(Valid() && column>=0 && column<m_parametercount)
	{
		if(sqlite3_bind_null(m_statement,column+1)==SQLITE_OK)
		{
			return true;
		}
		else
		{
			HandleError("Statement::Bind NULL");
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool Statement::Bind(const int column, const int value)
{
	if(Valid() && column>=0 && column<m_parametercount)
	{
		if(sqlite3_bind_int(m_statement,column+1,value)==SQLITE_OK)
		{
			return true;
		}
		else
		{
			HandleError("Statement::Bind int");
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool Statement::Bind(const int column, const double value)
{
	if(Valid() && column>=0 && column<m_parametercount)
	{
		if(sqlite3_bind_double(m_statement,column+1,value)==SQLITE_OK)
		{
			return true;
		}
		else
		{
			HandleError("Statement::Bind double");
			return false;
		}
	}
	else
	{
		return false;
	}	
}

const bool Statement::Bind(const int column, const std::string &value)
{
	if(Valid() && column>=0 && column<m_parametercount)
	{
		if(sqlite3_bind_text(m_statement,column+1,value.c_str(),value.size(),SQLITE_TRANSIENT)==SQLITE_OK)
		{
			return true;
		}
		else
		{
			HandleError("Statement::Bind std::string");
			return false;
		}
	}
	else
	{
		return false;
	}	
}

const bool Statement::Bind(const int column, const void *data, const int length)
{
	if(Valid() && column>=0 && column<m_parametercount)
	{
		if(sqlite3_bind_blob(m_statement,column+1,data,length,SQLITE_TRANSIENT)==SQLITE_OK)
		{
			return true;
		}
		else
		{
			HandleError("Statement::Bind blob");
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool Statement::Finalize()
{
	bool ok=false;
	if(m_statement)
	{
		Poco::ScopedLock<Poco::FastMutex> g(m_mutex);
		m_statementcount[m_statement]--;
		if(m_statementcount[m_statement]<=0)
		{
			m_statementcount.erase(m_statement);
			if(sqlite3_finalize(m_statement)==SQLITE_OK)
			{
				ok=true;
			}
			else
			{
				HandleError("Statement::Finalize");
			}
		}
		m_statement=NULL;
	}
	return ok;
}

const std::string Statement::GetSQL() const
{
	if(m_statement)
	{
		const char *sql=sqlite3_sql(m_statement);
		if(sql)
		{
			return std::string(sql);
		}
	}
	return std::string("");
}

void Statement::HandleError(const std::string &extramessage)
{
	std::string errmsg("");
	int err=0;
	if(m_db)
	{
		err=m_db->GetLastExtendedError(errmsg);
	}
	else if(m_statement && sqlite3_db_handle(m_statement))
	{
		err=sqlite3_extended_errcode(sqlite3_db_handle(m_statement));
		const char *msg=sqlite3_errmsg(sqlite3_db_handle(m_statement));
		if(msg)
		{
			errmsg=std::string(msg);
		}
	}
	DB::HandleError(err,errmsg,extramessage);
}

Statement &Statement::operator=(const Statement &rhs)
{
	if(&rhs!=this)
	{
		Finalize();

		m_statement=rhs.m_statement;
		m_parametercount=rhs.m_parametercount;
		m_resultcolumncount=rhs.m_resultcolumncount;
		m_rowreturned=rhs.m_rowreturned;
		m_lastinsertrowid=rhs.m_lastinsertrowid;
		m_db=rhs.m_db;

		if(m_statement)
		{
			Poco::ScopedLock<Poco::FastMutex> g(m_mutex);
			m_statementcount[m_statement]++;
		}
	}
	return *this;
}

const bool Statement::Reset()
{
	if(Valid())
	{
		if(sqlite3_reset(m_statement)==SQLITE_OK)
		{
			m_rowreturned=false;
			return true;
		}
		else
		{
			HandleError("Statement::Reset");
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool Statement::ResultBlob(const int column, void *data, int &length)
{
	if(Valid() && column>=0 && column<m_resultcolumncount)
	{
		int bloblength=sqlite3_column_bytes(m_statement,column);
		if(bloblength>length)
		{
			bloblength=length;
		}
		if(bloblength<length)
		{
			length=bloblength;
		}
		const void *blobptr=sqlite3_column_blob(m_statement,column);
		if(blobptr)
		{
			std::copy((unsigned char *)blobptr,(unsigned char *)blobptr+bloblength,(unsigned char *)data);
		}
		else
		{
			length=0;
		}
		return true;
	}
	else
	{
		return false;
	}
}

const bool Statement::ResultBool(const int column, bool &result)
{
	if(ResultNull(column)==false)
	{
		std::string res("");
		if(ResultText(column,res))
		{
			if(res=="1" || res=="true" || res=="TRUE")
			{
				result=true;
				return true;
			}
			else if(res=="0" || res=="false" || res=="FALSE")
			{
				result=false;
				return true;
			}
		}
	}
	return false;
}

const bool Statement::ResultDouble(const int column, double &result)
{
	if(Valid() && column>=0 && column<m_resultcolumncount)
	{
		result=sqlite3_column_double(m_statement,column);
		return true;
	}
	else
	{
		return false;
	}
}

const bool Statement::ResultInt(const int column, int &result)
{
	if(Valid() && column>=0 && column<m_resultcolumncount)
	{
		result=sqlite3_column_int(m_statement,column);
		return true;
	}
	else
	{
		return false;
	}
}

const bool Statement::ResultNull(const int column)
{
	if(Valid() && column>=0 && column<m_resultcolumncount)
	{
		if(sqlite3_column_type(m_statement,column)==SQLITE_NULL)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool Statement::ResultText(const int column, std::string &result)
{
	if(Valid() && column>=0 && column<m_resultcolumncount)
	{
		const unsigned char *cresult=sqlite3_column_text(m_statement,column);
		if(cresult)
		{
			result=(char *)cresult;
		}
		else
		{
			result="";
		}
		return true;
	}
	else
	{
		return false;
	}
}

const bool Statement::Step(const bool saveinsertrowid)
{
	m_rowreturned=false;
	if(Valid())
	{
		int result=sqlite3_step(m_statement);
#ifdef QUERY_LOG
		size_t temp=reinterpret_cast<size_t>(m_statement);
		std::string tempstr("");
		StringFunctions::Convert(temp,tempstr);
		Poco::Logger::get("querylog").information("Step : "+tempstr);
#endif
		if(result==SQLITE_OK || result==SQLITE_ROW || result==SQLITE_DONE)
		{
			if(result==SQLITE_ROW)
			{
				m_rowreturned=true;
			}
			if(saveinsertrowid)
			{
				m_lastinsertrowid=sqlite3_last_insert_rowid(sqlite3_db_handle(m_statement));
			}
			return true;
		}
		else
		{
			HandleError("Statement::Step");
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool Statement::Valid()
{
	return m_statement ? true : false ;
}

}	// namespace
