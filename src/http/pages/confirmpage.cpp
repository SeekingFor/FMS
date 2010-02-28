#include "../../../include/http/pages/confirmpage.h"
#include "../../../include/stringfunctions.h"

const std::string ConfirmPage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
{
	std::string content="";
	std::string target="";
	std::string confirmdescription="";
	
	if(queryvars.find("targetpage")!=queryvars.end())
	{
		target=(*queryvars.find("targetpage")).second.GetData();
	}
	if(queryvars.find("confirmdescription")!=queryvars.end())
	{
		confirmdescription=(*queryvars.find("confirmdescription")).second.GetData();
	}
	
	content+="<h1>"+m_trans->Get("web.page.confirm.title")+"</h1>";
	content+=confirmdescription+"<br>";
	content+="<form name=\"confirm\" method=\"POST\" action=\""+target+"\">";
	for(std::map<std::string,QueryVar>::const_iterator i=queryvars.begin(); i!=queryvars.end(); i++)
	{
		if((*i).first!="targetpage" && (*i).first!="confirmdescription")
		{
			content+="<input type=\"hidden\" name=\""+(*i).first+"\" value=\""+(*i).second.GetData()+"\">";	
		}
	}
	content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.confirm.continue")+"\">";
	content+="</form>";
	content+="<form name=\"cancel\" method=\"POST\" action=\""+target+"\">";
	content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.confirm.cancel")+"\">";
	content+="</form>";

	return content;
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
