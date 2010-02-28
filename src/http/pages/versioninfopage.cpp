#include "../../../include/http/pages/versioninfopage.h"
#include "../../../include/global.h"
#include "../../../include/stringfunctions.h"

#include <string>

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string VersionInfoPage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
{
	std::string content="";

	bool hascriteria=false;
	std::string major=VERSION_MAJOR;
	std::string minor=VERSION_MINOR;
	std::string release=VERSION_RELEASE;
	std::string sql="";

	if(queryvars.find("Major")!=queryvars.end())
	{
		major=(*queryvars.find("Major")).second.GetData();
		hascriteria=true;
	}
	if(queryvars.find("Minor")!=queryvars.end())
	{
		minor=(*queryvars.find("Minor")).second.GetData();
		hascriteria=true;
	}
	if(queryvars.find("Release")!=queryvars.end())
	{
		release=(*queryvars.find("Release")).second.GetData();
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
		content+="<h2>"+m_trans->Get("web.page.versioninfo.release")+" "+major+"."+minor+"."+release+"</h2>";
		content+="<h3>"+m_trans->Get("web.page.versioninfo.notes")+"</h3>";
		content+=StringFunctions::Replace(notes,"\n","<br>");
		content+="<h3>"+m_trans->Get("web.page.versioninfo.changes")+"</h3>";
		content+=StringFunctions::Replace(changes,"\n","<br>");
		content+="</div>";
		st.Step();
	}

	return content;
}
