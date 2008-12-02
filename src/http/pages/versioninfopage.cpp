#include "../../../include/http/pages/versioninfopage.h"
#include "../../../include/global.h"
#include "../../../include/stringfunctions.h"

#include <string>

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string VersionInfoPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="";

	bool hascriteria=false;
	std::string major=VERSION_MAJOR;
	std::string minor=VERSION_MINOR;
	std::string release=VERSION_RELEASE;
	std::string sql="";

	if(queryvars.find("Major")!=queryvars.end())
	{
		major=(*queryvars.find("Major")).second;
		hascriteria=true;
	}
	if(queryvars.find("Minor")!=queryvars.end())
	{
		minor=(*queryvars.find("Minor")).second;
		hascriteria=true;
	}
	if(queryvars.find("Release")!=queryvars.end())
	{
		release=(*queryvars.find("Release")).second;
		hascriteria=true;
	}

	sql="SELECT Notes, Changes, Major, Minor, Release FROM tblFMSVersion ";
	if(hascriteria==true)
	{
		sql+="WHERE Major=? AND Minor=? AND Release=? ";
	}
	sql+="ORDER BY Major DESC, Minor DESC, Release DESC;";
	SQLite3DB::Statement st=m_db->Prepare(sql);
	if(hascriteria==true)
	{
		st.Bind(0,major);
		st.Bind(1,minor);
		st.Bind(2,release);
	}
	st.Step();

	while(st.RowReturned())
	{
		std::string notes="";
		std::string changes="";

		st.ResultText(0,notes);
		st.ResultText(1,changes);
		st.ResultText(2,major);
		st.ResultText(3,minor);
		st.ResultText(4,release);

		content+="<div style=\"margin-bottom:20px;\">";
		content+="<h2>Release "+major+"."+minor+"."+release+"</h2>";
		content+="<h3>Notes</h3>";
		content+=StringFunctions::Replace(notes,"\n","<br>");
		content+="<h3>Changes</h3>";
		content+=StringFunctions::Replace(changes,"\n","<br>");
		content+="</div>";
		st.Step();
	}

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
}
