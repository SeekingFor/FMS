#include "../../../include/http/pages/translatepage.h"
#include <vector>
#include <algorithm>

const std::string TranslatePage::GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content("");
	std::vector<std::string> keys;
	int page=1;
	std::string id("");
	bool gotonextuntranslated=false;

	m_trans->GetDefaultKeys(keys);

	content+="<h2>"+m_trans->Get("web.page.translate.title")+"</h2>";

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="showtranslate" && queryvars.find("id")!=queryvars.end() && ValidateFormPassword(queryvars))
	{
		page=2;
		id=(*queryvars.find("id")).second;
	}
	else if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="translate" && queryvars.find("id")!=queryvars.end() && queryvars.find("translation")!=queryvars.end() && ValidateFormPassword(queryvars))
	{
		if(std::find(keys.begin(),keys.end(),(*queryvars.find("id")).second)!=keys.end())
		{
			if((*queryvars.find("translation")).second!="")
			{
				m_trans->SetLocalized((*queryvars.find("id")).second,(*queryvars.find("translation")).second);
			}
			else
			{
				m_trans->EraseLocalized((*queryvars.find("id")).second);
			}
			m_trans->SaveLocalizedTranslation();
		}
		if(queryvars.find("chkgotonextuntranslated")!=queryvars.end() && (*queryvars.find("chkgotonextuntranslated")).second=="true")
		{
			id="";
			std::vector<std::string>::const_iterator thisid=std::find(keys.begin(),keys.end(),(*queryvars.find("id")).second);
			if(thisid!=keys.end())
			{
				thisid++;
				for(std::vector<std::string>::const_iterator i=thisid; i!=keys.end();)
				{
					if(m_trans->TranslationExists((*i))==false)
					{
						id=(*i);
						i=keys.end();
					}
					else
					{
						i++;
					}
				}
			}
			if(id!="")
			{
				page=2;
				gotonextuntranslated=true;
			}
		}
	}

	if(page==1)
	{
		content+="<table class=\"small90\">";
		for(std::vector<std::string>::const_iterator i=keys.begin(); i!=keys.end(); i++)
		{
			content+="<tr>";
			content+="<td style=\"vertical-align:top;\"><strong>"+(*i)+"</strong></td>";
			content+="<td><form name=\"frmtranslate\" method=\"post\">"+CreateFormPassword()+"<input type=\"hidden\" name=\"formaction\" value=\"showtranslate\"><input type=\"hidden\" name=\"id\" value=\""+(*i)+"\"><input type=\"submit\" value=\""+m_trans->Get("web.page.translate.translatebutton")+"\"></form></td>";
			content+="</tr>";
			content+="<tr>";
			content+="<td style=\"vertical-align:top;\">"+SanitizeTextAreaOutput(m_trans->GetDefault((*i)))+"</td>";
			if(m_trans->TranslationExists((*i))==true)
			{
				if(m_trans->GetLocalized((*i))!="")
				{
					content+="<td style=\"vertical-align:top;\">"+SanitizeTextAreaOutput(m_trans->GetLocalized((*i)))+"</td>";
				}
				else
				{
					content+="<td class=\"translationneeded\"></td>";
				}
			}
			else
			{
				content+="<td class=\"translationneeded\"></td>";
			}
			content+="</tr>";
		}
		content+="</table>";
	}
	else if(page==2)
	{
		content+="<div>";
		content+="<strong>"+id+"</strong>";
		content+="</div>";
		content+="<div>";
		content+=m_trans->GetDefault(id);
		content+="</div>";
		content+="<form name=\"frmtranslate\" method=\"post\">";
		content+=CreateFormPassword();
		content+="<input type=\"hidden\" name=\"formaction\" value=\"translate\">";
		content+="<input type=\"hidden\" name=\"id\" value=\""+id+"\">";
		content+="<div>";
		if(m_trans->TranslationExists(id))
		{
			content+="<textarea name=\"translation\" cols=\"80\" rows=\"15\">"+SanitizeTextAreaOutput(m_trans->GetLocalized(id))+"</textarea>";
		}
		else
		{
			content+="<textarea name=\"translation\" cols=\"80\" rows=\"15\"></textarea>";
		}
		content+="</div>";
		content+="<input type=\"checkbox\" name=\"chkgotonextuntranslated\" value=\"true\"";
		if(gotonextuntranslated==true)
		{
			content+=" CHECKED";
		}
		content+=">"+m_trans->Get("web.page.translate.gotonextuntranslated");
		content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.translate.translatebutton")+"\">";
		content+="</form>";
	}

	return content;
}
