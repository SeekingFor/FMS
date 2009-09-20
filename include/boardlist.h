#ifndef _boardlist_
#define _boardlist_

#include "board.h"
#include "ilogger.h"
#include "idatabase.h"

#include <vector>

/**
	\brief 
*/
class BoardList:public std::vector<Board>,public ILogger,public IDatabase
{
public:
	BoardList(SQLite3DB::DB *db):IDatabase(db)		{}
	
	/**
		\brief Loads all known boards
	*/
	void Load();

	/**
		\brief Loads boards added after specified date
	*/
	void LoadNew(const std::string &date);
	
private:

};

#endif	// _boardlist_
