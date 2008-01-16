#ifndef _identityintroductionxml_
#define _identityintroductionxml_

#include "../ifmsxmldocument.h"

class IdentityIntroductionXML:public IFMSXMLDocument
{
public:
	IdentityIntroductionXML();

	std::string GetXML();

	const bool ParseXML(const std::string &xml);

	const std::string GetIdentity() const			{ return m_identity; }
	void SetIdentity(const std::string &identity)	{ m_identity=identity; }

private:
	void Initialize();
	
	std::string m_identity;
	
};

#endif	// _identityintroductionxml_
