#include "../../../include/http/pages/optionspage.h"
#include "../../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string OptionsPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="<h2 style=\"text-align:center;\">Options</h2>\r\n";
	content+="<form name=\"frmoptions\" method=\"POST\"><input type=\"hidden\" name=\"formaction\" value=\"save\">";
	content+="<table><tr><th>Option</th><th>Value</th><th>Description</th></tr>";

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="save")
	{
		SQLite3DB::Statement update=m_db->Prepare("UPDATE tblOption SET OptionValue=? WHERE Option=?;");
		std::vector<std::string> options;
		std::vector<std::string> oldvalues;
		std::vector<std::string> newvalues;
		CreateArgArray(queryvars,"option",options);
		CreateArgArray(queryvars,"oldvalue",oldvalues);
		CreateArgArray(queryvars,"value",newvalues);

		for(int i=0; i<options.size(); i++)
		{
			if(oldvalues[i]!=newvalues[i])
			{
				update.Bind(0,newvalues[i]);
				update.Bind(1,options[i]);
				update.Step();
				update.Reset();
			}
		}
	}

	SQLite3DB::Statement st=m_db->Prepare("SELECT Option,OptionValue,OptionDescription FROM tblOption;");
	st.Step();

	int count=0;
	std::string countstr;
	while(st.RowReturned())
	{
		std::string option;
		std::string value;
		std::string description;

		st.ResultText(0,option);
		st.ResultText(1,value);
		st.ResultText(2,description);

		StringFunctions::Convert(count,countstr);
		content+="<tr>";
		content+="<td valign=\"top\"><input type=\"hidden\" name=\"option["+countstr+"]\" value=\""+option+"\">"+option+"</td>";
		content+="<td valign=\"top\"><input type=\"hidden\" name=\"oldvalue["+countstr+"]\" value=\""+value+"\">";
		content+="<input type=\"text\" name=\"value["+countstr+"]\" value=\""+value+"\"></td>";
		content+="<td valign=\"top\">"+description+"</td>";
		content+="</tr>";
		st.Step();
		count++;
	}

	content+="<tr><td colspan=\"3\"><center><input type=\"submit\" value=\"Save\"></form></td></tr>";
	content+="</table>";
	
	return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"+StringFunctions::Replace(m_template,"[CONTENT]",content);
}

const bool OptionsPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("options.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
