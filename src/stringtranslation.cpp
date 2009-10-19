#include "../include/stringtranslation.h"
#include <map>

StringTranslation::StringTranslation():m_localizedloaded(false),m_localizedpath("")
{

}

void StringTranslation::EraseLocalized(const std::string &id)
{
	std::vector<std::string> keys;
	std::map<std::string,std::string> vals;

	if(m_localizedloaded==true)
	{
		m_localized.GetKeys(keys);

		for(std::vector<std::string>::const_iterator i=keys.begin(); i!=keys.end(); i++)
		{
			if((*i)!=id)
			{
				vals[(*i)]=GetLocalized((*i),"");
			}
		}

		m_localized.clear();

		for(std::map<std::string,std::string>::const_iterator i=vals.begin(); i!=vals.end(); i++)
		{
			SetLocalized((*i).first,(*i).second);
		}

	}
}

const bool StringTranslation::LoadDefaultTranslation(const std::string &path)
{
	m_default.clear();
	try
	{
		m_default.load(path);
		return true;
	}
	catch(...)
	{
		return false;
	}
}

const bool StringTranslation::LoadLocalizedTranslation(const std::string &path)
{
	m_localized.clear();
	try
	{
		m_localized.load(path);
		m_localizedpath=path;
		m_localizedloaded=true;
		return true;
	}
	catch(...)
	{
		m_localizedloaded=false;
		return false;
	}
}

const std::string StringTranslation::Get(const std::string &id, const std::string &defaultvalue) const
{
	if(m_localized.hasProperty(id) && m_localized.getString(id,defaultvalue)!="")
	{
		return m_localized.getString(id,defaultvalue);
	}
	else
	{
		return m_default.getString(id,defaultvalue);
	}
}

const std::string StringTranslation::GetDefault(const std::string &id, const std::string &defaultvalue) const
{
	return m_default.getString(id,defaultvalue);
}

const std::string StringTranslation::GetLocalized(const std::string &id, const std::string &defaultvalue) const
{
	return m_localized.getString(id,defaultvalue);
}

void StringTranslation::GetDefaultKeys(std::vector<std::string> &keys) const
{
	m_default.GetKeys(keys);
}

void StringTranslation::SaveDefaultTranslation(const std::string &path)
{
	m_default.save(path);
}

void StringTranslation::SaveLocalizedTranslation()
{
	if(m_localizedloaded==true && m_localizedpath!="")
	{
		m_localized.save(m_localizedpath);
	}
}

void StringTranslation::SetDefault(const std::string &id, const std::string &value)
{
	m_default.setString(id,value);
}

void StringTranslation::SetLocalized(const std::string &id, const std::string &value)
{
	if(m_localizedloaded==true)
	{
		m_localized.setString(id,value);
	}
}

const bool StringTranslation::IDExists(const std::string &id) const
{
	if(m_default.hasProperty(id))
	{
		return true;
	}
	else
	{
		return false;
	}
}

const bool StringTranslation::TranslationExists(const std::string &id) const
{
	if(m_localized.hasProperty(id))
	{
		return true;
	}
	else
	{
		return false;
	}
}
