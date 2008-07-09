#ifndef _identityxml_
#define _identityxml_

#include "../ifmsxmldocument.h"

class IdentityXML:public IFMSXMLDocument
{
public:

	IdentityXML();

	std::string GetXML();
	const bool ParseXML(const std::string &xml);

	const std::string GetName()						{ return m_name; }
	void SetName(const std::string &name)			{ m_name=name; }

	const bool GetPublishTrustList()				{ return m_publishtrustlist; }
	void SetPublishTrustList(const bool publish)	{ m_publishtrustlist=publish; }

	const bool GetPublishBoardList()				{ return m_publishboardlist; }
	void SetPublishBoardList(const bool publish)	{ m_publishboardlist=publish; }

	const bool GetSingleUse()						{ return m_singleuse; }
	void SetSingleUse(const bool singleuse)			{ m_singleuse=singleuse; }

	const int GetFreesiteEdition()					{ return m_freesiteedition; }
	void SetFreesiteEdition(const int edition)		{ m_freesiteedition=edition; }

private:
	void Initialize();

	std::string m_name;
	bool m_publishtrustlist;
	bool m_publishboardlist;
	bool m_singleuse;
	int m_freesiteedition;

};

#endif	// _identityxml_
