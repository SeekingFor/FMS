#ifndef _wotidentityxml_
#define _wotidentityxml_

#include "../ifmsxmldocument.h"

class WOTIdentityXML:public IFMSXMLDocument
{
public:
	WOTIdentityXML();

	struct trust
	{
		trust(const std::string &identity, const std::string &comment, const int &trust):m_identity(identity),m_comment(comment),m_trust(trust)	{}
		std::string m_identity;
		std::string m_comment;
		int m_trust;
	};

	std::string GetXML();
	const bool ParseXML(const std::string &xml);

	const std::string &GetName() const												{ return m_name; }
	const std::vector<std::string> &GetContexts() const								{ return m_contexts; }
	const std::vector<std::pair<std::string,std::string> > &GetProperties() const	{ return m_properties; }
	const std::vector<trust> &GetTrustList() const									{ return m_trustlist; }

private:
	void Initialize();

	std::string m_name;
	std::vector<std::string> m_contexts;
	std::vector<std::pair<std::string,std::string> > m_properties;
	std::vector<trust> m_trustlist;

};

#endif	// _wotidentityxml_
