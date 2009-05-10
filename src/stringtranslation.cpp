#include "../include/stringtranslation.h"

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
		return true;
	}
	catch(...)
	{
		return false;
	}
}

const std::string StringTranslation::Get(const std::string &id, const std::string &defaultvalue)
{
	if(m_localized.hasProperty(id))
	{
		return m_localized.getString(id,defaultvalue);
	}
	else
	{
		return m_default.getString(id,defaultvalue);
	}
}

void StringTranslation::SaveDefaultTranslation(const std::string &path)
{
	m_default.save(path);
}

void StringTranslation::SetDefault(const std::string &id, const std::string &value)
{
	m_default.setString(id,value);
}
