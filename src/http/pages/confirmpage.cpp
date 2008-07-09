#include "../../../include/http/pages/confirmpage.h"
#include "../../../include/stringfunctions.h"

const std::string ConfirmPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="";
	std::string target="";
	std::string confirmdescription="";
	
	if(queryvars.find("targetpage")!=queryvars.end())
	{
		target=(*queryvars.find("targetpage")).second;
	}
	if(queryvars.find("confirmdescription")!=queryvars.end())
	{
		confirmdescription=(*queryvars.find("confirmdescription")).second;
	}
	
	content+="<h1>Confirm</h1>";
	content+=confirmdescription+"<br>";
	content+="<form name=\"confirm\" method=\"POST\" action=\""+target+"\">";
	for(std::map<std::string,std::string>::const_iterator i=queryvars.begin(); i!=queryvars.end(); i++)
	{
		if((*i).first!="targetpage" && (*i).first!="confirmdescription")
		{
			content+="<input type=\"hidden\" name=\""+(*i).first+"\" value=\""+(*i).second+"\">";	
		}
	}
	content+="<input type=\"submit\" value=\"Continue\">";
	content+="</form>";
	content+="<form name=\"cancel\" method=\"POST\" action=\""+target+"\">";
	content+="<input type=\"submit\" value=\"Cancel\">";
	content+="</form>";

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
}


const bool ConfirmPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("confirm.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
