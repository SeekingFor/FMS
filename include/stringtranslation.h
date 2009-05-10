#ifndef _stringtranslation_
#define _stringtranslation_

#include <Poco/Util/PropertyFileConfiguration.h>
#include "translationpropertyfile.h"

class StringTranslation
{
public:

	const bool LoadDefaultTranslation(const std::string &path);
	const bool LoadLocalizedTranslation(const std::string &path);

	const std::string Get(const std::string &id, const std::string &defaultvalue="?????");

	void SetDefault(const std::string &id, const std::string &value);
	void SaveDefaultTranslation(const std::string &path);

private:

	TranslationPropertyFile m_default;
	TranslationPropertyFile m_localized;

};

#endif	// _stringtranslation_
