#ifndef _stringtranslation_
#define _stringtranslation_

#include "translationpropertyfile.h"
#include <vector>

class StringTranslation
{
public:
	StringTranslation();

	const bool LoadDefaultTranslation(const std::string &path);
	const bool LoadLocalizedTranslation(const std::string &path);

	const std::string Get(const std::string &id, const std::string &defaultvalue="?????") const;
	const std::string GetDefault(const std::string &id, const std::string &defaultvalue="?????") const;
	const std::string GetLocalized(const std::string &id, const std::string &defaultvalue="?????") const;

	const bool IDExists(const std::string &id) const;
	const bool TranslationExists(const std::string &id) const;

	void GetDefaultKeys(std::vector<std::string> &keys) const;
	void SetDefault(const std::string &id, const std::string &value);
	void SaveDefaultTranslation(const std::string &path);

	void EraseLocalized(const std::string &id);
	void SetLocalized(const std::string &id, const std::string &value);
	void SaveLocalizedTranslation();

private:

	TranslationPropertyFile m_default;
	TranslationPropertyFile m_localized;
	bool m_localizedloaded;
	std::string m_localizedpath;

};

#endif	// _stringtranslation_
