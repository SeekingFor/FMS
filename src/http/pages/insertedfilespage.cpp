#include "../../../include/http/pages/insertedfilespage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/option.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string InsertedFilesPage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
{
	std::string content="<h2>"+m_trans->Get("web.page.insertedfiles.title")+"</h2>";

	Option option(m_db);
	std::string node="localhost";
	option.Get("FProxyHost",node);
	std::string fproxyport="8888";
	option.Get("FProxyPort",fproxyport);
	std::string fproxyprotocol("http");
	option.Get("FProxyProtocol",fproxyprotocol);

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="removefile" && queryvars.find("fileid")!=queryvars.end() && ValidateFormPassword(queryvars))
	{
		SQLite3DB::Statement del=m_db->Prepare("DELETE FROM tblFileInserts WHERE FileInsertID=?;");
		del.Bind(0,(*queryvars.find("fileid")).second.GetData());
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

		content+="<a href=\""+fproxyprotocol+"://"+node+":"+fproxyport+"/"+StringFunctions::UriEncode(key)+"\">"+SanitizeOutput(filename)+"</a> - "+sizestr+" bytes";
		content+="<form name=\"frmRemove"+insertidstr+"\" method=\"POST\">";
		content+=CreateFormPassword();
		content+="<input type=\"hidden\" name=\"formaction\" value=\"removefile\">";
		content+="<input type=\"hidden\" name=\"fileid\" value=\""+insertidstr+"\">";
		content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.insertedfiles.remove")+"\">";
		content+="</form>";
		content+="<br>";

		st.Step();
	}

	return content;
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
