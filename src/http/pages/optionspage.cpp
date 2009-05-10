#include "../../../include/http/pages/optionspage.h"
#include "../../../include/stringfunctions.h"

#include <Poco/Path.h>

#ifdef XMEM
	#include <xmem.h>
#endif

int OptionsPage::m_mode=1;

const std::string OptionsPage::CreateDropDown(const std::string &option, const std::string &name, const std::vector<std::string> &items, const std::string &selecteditem, const std::string &param1, const std::string &param2)
{
	std::string rval("");

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

		rval+=m_trans->Get("web.option."+option+"."+(*i));

		rval+="</option>";
		i++;
		if(i!=items.end())
		{
			i++;
		}
	}

	rval+="</select>";

	return rval;
}

const std::string OptionsPage::CreateTextArea(const std::string &name, const std::string &currentvalue, const std::string &param1, const std::string &param2)
{
	std::string html("");

	html+="<textarea name=\""+name+"\"";
	if(param1!="")
	{
		html+=" cols=\""+param1+"\"";
	}
	if(param2!="")
	{
		html+=" rows=\""+param2+"\"";
	}
	html+=">";
	html+=SanitizeTextAreaOutput(currentvalue);
	html+="</textarea>";

	return html;
}

const std::string OptionsPage::CreateTextBox(const std::string &name, const std::string &currentvalue, const std::string &param1, const std::string &param2)
{
	std::string html("");

	html+="<input type=\"text\" name=\""+name+"\" value=\""+currentvalue+"\"";
	if(param1!="")
	{
		html+=" size=\""+param1+"\"";
	}
	if(param2!="")
	{
		html+=" maxlength=\""+param2+"\"";
	}
	html+=">";

	return html;

}

const std::string OptionsPage::GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content("");
	std::string sql("");

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="save" && ValidateFormPassword(queryvars))
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

				// load new language immediately
				if(options[i]=="Language")
				{
					Poco::Path tdir;
					tdir.pushDirectory("translations");
					tdir=tdir.makeAbsolute();
					tdir.setFileName(newvalues[i]);
					m_trans->LoadLocalizedTranslation(tdir.toString());
				}
			}
		}

	}

	if(queryvars.find("mode")!=queryvars.end())
	{
		if((*queryvars.find("mode")).second=="2")
		{
			m_mode=2;
		}
		else
		{
			m_mode=1;
		}
	}

	content+="<h2 style=\"text-align:center;\">"+m_trans->Get("web.page.options.title")+"</h2>\r\n";
	content+="<div style=\"text-align:center;\">";
	if(m_mode==1)
	{
		content+=m_trans->Get("web.page.options.simple")+" | <a href=\""+m_pagename+"?mode=2\">"+m_trans->Get("web.page.options.advanced")+"</a>";
	}
	else
	{
		content+="<a href=\""+m_pagename+"?mode=1\">"+m_trans->Get("web.page.options.simple")+"</a> | "+m_trans->Get("web.page.options.advanced")+"</a>";
	}
	content+="</div>";

	content+="<form name=\"frmoptions\" method=\"POST\"><input type=\"hidden\" name=\"formaction\" value=\"save\">";
	content+=CreateFormPassword();
	content+="<table>\r\n";

	if(m_mode==1)
	{
		sql="SELECT Option,OptionValue,OptionDescription,Section,ValidValues,DisplayType,DisplayParam1,DisplayParam2 FROM tblOption WHERE Mode='simple' ORDER BY SortOrder;";
	}
	else
	{
		sql="SELECT Option,OptionValue,OptionDescription,Section,ValidValues,DisplayType,DisplayParam1,DisplayParam2 FROM tblOption ORDER BY SortOrder;";
	}
	SQLite3DB::Statement st=m_db->Prepare(sql);
	st.Step();

	int count=0;
	std::string countstr;
	std::string lastsection("");
	while(st.RowReturned())
	{
		std::string option("");
		std::string value("");
		std::string description("");
		std::string section("");
		std::string validvalues("");
		std::vector<std::string> validvaluevec;
		std::string displaytype("");
		std::string displayparam1("");
		std::string displayparam2("");

		st.ResultText(0,option);
		st.ResultText(1,value);
		st.ResultText(2,description);
		st.ResultText(3,section);
		if(st.ResultNull(4)==false)
		{
			st.ResultText(4,validvalues);
			StringFunctions::Split(validvalues,"|",validvaluevec);
		}
		st.ResultText(5,displaytype);
		st.ResultText(6,displayparam1);
		st.ResultText(7,displayparam2);

		if(section!=lastsection)
		{
			content+="<tr>";
			content+="<td colspan=\"3\"><h3>"+m_trans->Get("web.option.section."+section)+"</h3></td>";
			content+="</tr>";
			lastsection=section;
		}

		StringFunctions::Convert(count,countstr);
		content+="<tr>";
		content+="<td valign=\"top\" class=\"optionname\"><input type=\"hidden\" name=\"option["+countstr+"]\" value=\""+option+"\">"+option+"</td>";
		content+="<td valign=\"top\"><input type=\"hidden\" name=\"oldvalue["+countstr+"]\" value=\""+value+"\">";

		if(displaytype=="textbox")
		{
			content+=CreateTextBox("value["+countstr+"]",value,displayparam1,displayparam2);
		}
		else if(displaytype=="select")
		{
			if(validvaluevec.size()==4 && validvaluevec[0]=="true" && validvaluevec[2]=="false")
			{
				content+=CreateTrueFalseDropDown("value["+countstr+"]",value);	
			}
			else
			{
				content+=CreateDropDown(option,"value["+countstr+"]",validvaluevec,value,displayparam1,displayparam2);
			}
		}
		else if(displaytype=="textarea")
		{
			content+=CreateTextArea("value["+countstr+"]",value,displayparam1,displayparam2);
		}
		else
		{
			content+="Currently Unsupported";
		}

		/*
		if(validvaluevec.size()>0)
		{
			content+=CreateDropDown("value["+countstr+"]",validvaluevec,value);
		}
		else if(value!="true" && value!="false")
		{
			content+="<input type=\"text\" name=\"value["+countstr+"]\" value=\""+value+"\">";
		}
		else
		{
			content+=CreateTrueFalseDropDown("value["+countstr+"]",value);
		}
		*/

		content+="</td></tr>\r\n";
		content+="<tr><td valign=\"top\" class=\"optiondescription\" colspan=\"2\">"+m_trans->Get("web.option."+option+".description")+"</td>";
		content+="</tr>\r\n";
		st.Step();
		count++;
	}
	content+="<input type=\"hidden\" name=\"param[0]\" value=\"\">";
	content+="<tr><td colspan=\"3\"><center><input type=\"submit\" value=\""+m_trans->Get("web.page.options.save")+"\"></form></td></tr>";
	content+="<tr><td colspan=\"3\"><center><strong>"+m_trans->Get("web.page.options.requirerestart")+"</strong></center></td></tr>";

	content+="</table>";
	
	return content;
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
