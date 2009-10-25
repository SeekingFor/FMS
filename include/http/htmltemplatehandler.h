#ifndef _html_template_handler_
#define _html_template_handler_

#include <map>
#include "../stringtranslation.h"

class HTMLTemplateHandler
{
public:

	const bool LoadTemplate(const std::string &templatepath);
	const bool GetSection(const std::string &section, std::string &result, const std::vector<std::string> &ignoredvars=std::vector<std::string>()) const;
	const int PerformReplacements(const std::string &text, const std::map<std::string,std::string> &varmap, std::string &result, const std::vector<std::string> &ignoredvars=std::vector<std::string>()) const;
	void PerformTranslations(const std::string &text, const StringTranslation &translations, std::string &result) const;

private:

	std::map<std::string,std::string> m_section;

};

#endif	// _html_template_handler_
