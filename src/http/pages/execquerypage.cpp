#include "../../../include/http/pages/execquerypage.h"
#include "../../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string ExecQueryPage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
{
	std::string content="";
	std::string query="";

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="execute" && queryvars.find("query")!=queryvars.end() && (*queryvars.find("query")).second!="" && ValidateFormPassword(queryvars))
	{
		query=(*queryvars.find("query")).second.GetData();
		SQLite3DB::Recordset rs=m_db->Query(query);

		content+="<table>";
		if(rs.Count()>0)
		{
			content+="<tr>";
			for(int i=0; i<rs.Cols(); i++)
			{
				content+="<th>";
				if(rs.GetColumnName(i))
				{
					content+=rs.GetColumnName(i);
				}
				content+="</th>";
			}
			content+="<tr>";
		}
		else if(m_db->GetLastResult()!=SQLITE_OK)
		{
			std::string error="";
			m_db->GetLastError(error);
			content+="<tr><td>"+SanitizeOutput(error)+"</td></tr>";
		}
		while(!rs.AtEnd())
		{
			content+="<tr>";
			for(int i=0; i<rs.Cols(); i++)
			{
				content+="<td>";
				if(rs.GetField(i))
				{
					content+=SanitizeOutput(std::string(rs.GetField(i)));
				}
				content+="</td>";
			}
			content+="</tr>";
			rs.Next();
		}
		content+="</table>";
	}

	content+="<h2>"+m_trans->Get("web.page.execquery.title")+"</h2>";
	content+="<form name=\"frmquery\" method=\"POST\">";
	content+=CreateFormPassword();
	content+="<input type=\"hidden\" name=\"formaction\" value=\"execute\">";
	content+="<textarea name=\"query\" rows=\"10\" cols=\"80\">"+StringFunctions::Replace(query,"<","&lt;")+"</textarea>";
	content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.execquery.executequery")+"\">";
	content+="</form>";

	return content;
}

const bool ExecQueryPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("execquery.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
