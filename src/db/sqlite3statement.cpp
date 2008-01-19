#include "../../include/db/sqlite3db/sqlite3statement.h"

#ifdef XMEM
	#include <xmem.h>
#endif

namespace SQLite3DB
{

std::map<sqlite3_stmt *, long> Statement::m_statementcount;

Statement::Statement()
{
	m_statement=NULL;
	m_parametercount=0;
	m_resultcolumncount=0;
	m_rowreturned=false;
}

Statement::Statement(sqlite3_stmt *statement)
{
	m_statement=statement;
	m_parametercount=sqlite3_bind_parameter_count(m_statement);
	m_resultcolumncount=sqlite3_column_count(m_statement);
	m_rowreturned=false;
	
	if(m_statement)
	{
		m_statementcount[m_statement]++;
	}
}

Statement::Statement(const Statement &rhs)
{
	m_statement=NULL;
	m_parametercount=0;
	m_resultcolumncount=0;
	m_rowreturned=false;
	*this=rhs;
}

Statement::~Statement()
{
	
	Finalize();
	
	std::vector<char *>::iterator i;
	for(i=textptrs.begin(); i!=textptrs.end(); i++)
	{
		if((*i))
		{
			delete [] (*i);
		}
	}
}

const bool Statement::Bind(const int column)
{
	if(Valid() && column>=0 && column<m_parametercount)
	{
		ZThread::Guard<ZThread::Mutex> g(DB::instance()->m_mutex);
		if(sqlite3_bind_null(m_statement,column+1)==SQLITE_OK)
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

const bool Statement::Bind(const int column, const int value)
{
	if(Valid() && column>=0 && column<m_parametercount)
	{
		ZThread::Guard<ZThread::Mutex> g(DB::instance()->m_mutex);
		if(sqlite3_bind_int(m_statement,column+1,value)==SQLITE_OK)
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

const bool Statement::Bind(const int column, const double value)
{
	if(Valid() && column>=0 && column<m_parametercount)
	{
		ZThread::Guard<ZThread::Mutex> g(DB::instance()->m_mutex);
		if(sqlite3_bind_double(m_statement,column+1,value)==SQLITE_OK)
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

const bool Statement::Bind(const int column, const std::string &value)
{
	if(Valid() && column>=0 && column<m_parametercount)
	{
		char *text=new char[value.size()+1];
		strncpy(text,value.c_str(),value.size());
		text[value.size()]=NULL;
		textptrs.push_back(text);
		ZThread::Guard<ZThread::Mutex> g(DB::instance()->m_mutex);
		if(sqlite3_bind_text(m_statement,column+1,text,value.size(),NULL)==SQLITE_OK)
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

const bool Statement::Bind(const int column, const void *data, const int length)
{
	if(Valid() && column>=0 && column<m_parametercount)
	{
		ZThread::Guard<ZThread::Mutex> g(DB::instance()->m_mutex);
		if(sqlite3_bind_blob(m_statement,column+1,data,length,NULL)==SQLITE_OK)
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

void Statement::Finalize()
{
	if(m_statement)
	{
		ZThread::Guard<ZThread::Mutex> g(DB::instance()->m_mutex);
		m_statementcount[m_statement]--;
		if(m_statementcount[m_statement]<=0)
		{
			m_statementcount.erase(m_statement);
			sqlite3_finalize(m_statement);
		}
		m_statement=NULL;
	}
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

		if(m_statement)
		{
			ZThread::Guard<ZThread::Mutex> g(DB::instance()->m_mutex);
			m_statementcount[m_statement]++;
		}
	}
	return *this;
}

const bool Statement::Reset()
{
	if(Valid())
	{
		ZThread::Guard<ZThread::Mutex> g(DB::instance()->m_mutex);
		if(sqlite3_reset(m_statement)==SQLITE_OK)
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

const bool Statement::ResultBlob(const int column, void *data, int &length)
{
	if(Valid() && column>=0 && column<m_resultcolumncount)
	{
		ZThread::Guard<ZThread::Mutex> g(DB::instance()->m_mutex);
		data=(void *)sqlite3_column_blob(m_statement,column);
		length=sqlite3_column_bytes(m_statement,column);
		return true;
	}
	else
	{
		return false;
	}
}

const bool Statement::ResultDouble(const int column, double &result)
{
	if(Valid() && column>=0 && column<m_resultcolumncount)
	{
		ZThread::Guard<ZThread::Mutex> g(DB::instance()->m_mutex);
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
		ZThread::Guard<ZThread::Mutex> g(DB::instance()->m_mutex);
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
		ZThread::Guard<ZThread::Mutex> g(DB::instance()->m_mutex);
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
		ZThread::Guard<ZThread::Mutex> g(DB::instance()->m_mutex);
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
		ZThread::Guard<ZThread::Mutex> g(DB::instance()->m_mutex);
		int result=sqlite3_step(m_statement);
		if(result==SQLITE_OK || result==SQLITE_ROW || result==SQLITE_DONE)
		{
			if(result==SQLITE_ROW)
			{
				m_rowreturned=true;
			}
			if(saveinsertrowid)
			{
				m_lastinsertrowid=sqlite3_last_insert_rowid(DB::instance()->GetDB());
			}
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

const bool Statement::Valid()
{
	ZThread::Guard<ZThread::Mutex> g(DB::instance()->m_mutex);
	return m_statement ? true : false ;
}

}	// namespace
