#include "../../../include/http/pages/optionspage.h"
#include "../../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string OptionsPage::CreateDropDown(const std::string &name, const std::vector<std::string> &items, const std::string &selecteditem)
{
	std::string rval="";

	rval+="<select name=\""+name+"\">";

	std::vector<std::string>::const_iterator i=items.begin();
	while(i!=items.end())
	{
		rval+="<option value=\""+(*i)+"\"";
		if((*i)==selecteditem)
		{
			rval+=" SELECTED";
		}
		rval+=">";
		i++;
		if(i!=items.end())
		{
			rval+=(*i);
			i++;
		}
		rval+="</option>";
	}

	rval+="</select>";

	return rval;
}

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

	SQLite3DB::Statement st=m_db->Prepare("SELECT Option,OptionValue,OptionDescription,Section,ValidValues FROM tblOption ORDER BY SortOrder;");
	st.Step();

	int count=0;
	std::string countstr;
	std::string lastsection="";
	while(st.RowReturned())
	{
		std::string option;
		std::string value;
		std::string description;
		std::string section;
		std::string validvalues;
		std::vector<std::string> validvaluevec;

		st.ResultText(0,option);
		st.ResultText(1,value);
		st.ResultText(2,description);
		st.ResultText(3,section);
		if(st.ResultNull(4)==false)
		{
			st.ResultText(4,validvalues);
			StringFunctions::Split(validvalues,"|",validvaluevec);
		}

		if(section!=lastsection)
		{
			content+="<tr>";
			content+="<td colspan=\"3\"><h3>"+section+"</h3></td>";
			content+="</tr>";
			lastsection=section;
		}

		StringFunctions::Convert(count,countstr);
		content+="<tr>";
		content+="<td valign=\"top\"><input type=\"hidden\" name=\"option["+countstr+"]\" value=\""+option+"\">"+option+"</td>";
		content+="<td valign=\"top\"><input type=\"hidden\" name=\"oldvalue["+countstr+"]\" value=\""+value+"\">";

		if(validvaluevec.size()>0)
		{
			content+=CreateDropDown("value["+countstr+"]",validvaluevec,value);
		}
		else if(value!="true" && value!="false")
		{
			content+="<input type=\"text\" name=\"value["+countstr+"]\" value=\""+value+"\"></td>";
		}
		else
		{
			content+=CreateTrueFalseDropDown("value["+countstr+"]",value);
		}

		content+="<td valign=\"top\">"+description+"</td>";
		content+="</tr>";
		st.Step();
		count++;
	}
content+="<input type=\"hidden\" name=\"param[0]\" value=\"\">";
	content+="<tr><td colspan=\"3\"><center><input type=\"submit\" value=\"Save\"></form></td></tr>";
	content+="</table>";
	
	return StringFunctions::Replace(m_template,"[CONTENT]",content);
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
