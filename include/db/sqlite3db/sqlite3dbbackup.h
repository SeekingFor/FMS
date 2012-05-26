#ifndef _sqlite3backup_
#define _sqlite3backup_

#include "sqlite3db.h"

namespace SQLite3DB
{

class DBBackup
{
public:
	DBBackup(DB *source, DB *dest);
	~DBBackup();

	const int Step(const int pages);

private:
	sqlite3_backup *m_backup;

};	// class DBBackup

}	// namespace

#endif	// _sqlite3backup_
