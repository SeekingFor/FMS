#include "../../../include/http/pages/insertedfilespage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/option.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string InsertedFilesPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="<h2>Inserted Files</h2>";

	std::string node="localhost";
	Option::Instance()->Get("FCPHost",node);
	std::string fproxyport="8888";
	Option::Instance()->Get("FProxyPort",fproxyport);


	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="removefile" && queryvars.find("fileid")!=queryvars.end())
	{
		SQLite3DB::Statement del=m_db->Prepare("DELETE FROM tblFileInserts WHERE FileInsertID=?;");
		del.Bind(0,(*queryvars.find("fileid")).second);
		del.Step();
	}

	SQLite3DB::Statement st=m_db->Prepare("SELECT Key,FileName,Size,FileInsertID FROM tblFileInserts WHERE Key IS NOT NULL ORDER BY FileName");
	st.Step();

	while(st.RowReturned())
	{
		std::string key="";
		std::string filename="";
		std::string sizestr="";
		std::string insertidstr="";

		st.ResultText(0,key);
		st.ResultText(1,filename);
		st.ResultText(2,sizestr);
		st.ResultText(3,insertidstr);

		content+="<a href=\"http://"+node+":"+fproxyport+"/"+StringFunctions::UriEncode(key)+"\">"+SanitizeOutput(filename)+"</a> - "+sizestr+" bytes";
		content+="<form name=\"frmRemove"+insertidstr+"\" method=\"POST\">";
		content+="<input type=\"hidden\" name=\"formaction\" value=\"removefile\">";
		content+="<input type=\"hidden\" name=\"fileid\" value=\""+insertidstr+"\">";
		content+="<input type=\"submit\" value=\"Remove\">";
		content+="</form>";
		content+="<br>";

		st.Step();
	}

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
}

const bool InsertedFilesPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("insertedfiles.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
