#include "../../../include/http/pages/execquerypage.h"
#include "../../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string ExecQueryPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="";
	std::string query="";

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="execute" && queryvars.find("query")!=queryvars.end() && (*queryvars.find("query")).second!="")
	{
		query=(*queryvars.find("query")).second;
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
		while(!rs.AtEnd())
		{
			content+="<tr>";
			for(int i=0; i<rs.Cols(); i++)
			{
				content+="<td>";
				if(rs.GetField(i))
				{
					content+=rs.GetField(i);
				}
				content+="</td>";
			}
			content+="</tr>";
			rs.Next();
		}
		content+="</table>";
	}

	content+="<h2>Execute Query</h2>";
	content+="<form name=\"frmquery\" method=\"POST\">";
	content+="<input type=\"hidden\" name=\"formaction\" value=\"execute\">";
	content+="<textarea name=\"query\" rows=\"10\" cols=\"80\">"+StringFunctions::Replace(query,"<","&lt;")+"</textarea>";
	content+="<input type=\"submit\" value=\"Execute Query\">";
	content+="</form>";

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
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
