#ifndef _sqlite3transaction_
#define _sqlite3transaction_

#include "sqlite3db.h"
#include "sqlite3statement.h"

namespace SQLite3DB
{

class Transaction
{
public:

	enum TransactionType
	{
		TRANS_DEFERRED=0,
		TRANS_IMMEDIATE=1,
		TRANS_EXCLUSIVE=2
	};

	enum TransactionResult
	{
		TRANS_RESULT_NOTSTARTED=0,
		TRANS_RESULT_INPROGRESS=1,
		TRANS_RESULT_COMMIT=2,
		TRANS_RESULT_ROLLBACK=3,
		TRANS_RESULT_ERROR=4
	};

	Transaction();
	Transaction(DB *db);
	virtual ~Transaction();

	const bool Begin(const TransactionType transtype=TRANS_DEFERRED);
	const bool Commit(const int tries=10);
	const bool Rollback(const int tries=10);

	const bool Step(Statement &st, const bool saveinsertrowid=false);
	const bool Reset(Statement &st);
	const bool Finalize(Statement &st);
	
	const int GetTransactionLevel() const	{ return m_transactionlevel; }

	const bool IsSuccessful() const				{ return !m_error; }
	const int GetLastError() const				{ return m_lasterr; }
	const std::string GetLastErrorStr() const	{ return m_errorstr; }
	const std::string GetErrorSQL() const		{ return m_errorsql; }
	const int GetTransactionResult() const		{ return m_transactionresult; }
	void ClearError()							{ m_error=false; m_errorstr=""; m_errorsql=""; }

private:
	int m_transactionlevel;
	
	DB *m_db;
	bool m_error;
	int m_lasterr;
	std::string m_errorstr;
	std::string m_errorsql;
	int m_transactionresult;

};	//class

}	// namespace

#endif	// _sqlite3transaction_
