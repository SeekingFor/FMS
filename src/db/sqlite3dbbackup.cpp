#include "../../include/db/sqlite3db/sqlite3dbbackup.h"

namespace SQLite3DB
{

DBBackup::DBBackup(DB *source, DB *dest):m_backup(0)
{
	m_backup=sqlite3_backup_init(dest->GetDB(),"main",source->GetDB(),"main");
}

DBBackup::~DBBackup()
{
	if(m_backup)
	{
		sqlite3_backup_finish(m_backup);
		m_backup=0;
	}
}

const int DBBackup::Step(const int pages)
{
	if(m_backup)
	{
		int rval=sqlite3_backup_step(m_backup,pages);
		if(rval==SQLITE_OK || rval==SQLITE_BUSY || rval==SQLITE_LOCKED)
		{
			return sqlite3_backup_remaining(m_backup);
		}
		else if(rval==SQLITE_DONE)
		{
			return 0;
		}
	}

	return -1;

}

}	// namespace
