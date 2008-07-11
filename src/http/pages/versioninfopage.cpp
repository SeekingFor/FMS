#include "../../../include/http/pages/versioninfopage.h"
#include "../../../include/global.h"
#include "../../../include/stringfunctions.h"

#include <string>

const std::string VersionInfoPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="";

	std::string major=VERSION_MAJOR;
	std::string minor=VERSION_MINOR;
	std::string release=VERSION_RELEASE;

	if(queryvars.find("Major")!=queryvars.end())
	{
		major=(*queryvars.find("Major")).second;
	}
	if(queryvars.find("Minor")!=queryvars.end())
	{
		minor=(*queryvars.find("Minor")).second;
	}
	if(queryvars.find("Release")!=queryvars.end())
	{
		release=(*queryvars.find("Release")).second;
	}

	SQLite3DB::Statement st=m_db->Prepare("SELECT Notes, Changes FROM tblFMSVersion WHERE Major=? AND Minor=? AND Release=?;");
	st.Bind(0,major);
	st.Bind(1,minor);
	st.Bind(2,release);
	st.Step();

	if(st.RowReturned())
	{
		std::string notes="";
		std::string changes="";

		st.ResultText(0,notes);
		st.ResultText(1,changes);

		content+="<h2>Release "+major+"."+minor+"."+release+"</h2>";
		content+="<h3>Notes</h3>";
		content+=StringFunctions::Replace(notes,"\n","<br>");
		content+="<h3>Changes</h3>";
		content+=StringFunctions::Replace(changes,"\n","<br>");
	}

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
}

const bool VersionInfoPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("versioninfo.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
