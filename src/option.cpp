#include "../include/option.h"
#include "../include/db/sqlite3db.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const bool Option::Get(const std::string &option, std::string &value)
{
	SQLite3DB::Statement st=SQLite3DB::DB::instance()->Prepare("SELECT OptionValue FROM tblOption WHERE Option=?;");
	st.Bind(0,option);
	st.Step();
	if(st.RowReturned())
	{
		st.ResultText(0,value);
		return true;
	}
	else
	{
		return false;
	}
}