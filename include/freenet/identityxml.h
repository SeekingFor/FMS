#ifndef _identityxml_
#define _identityxml_

#include "../ifmsxmldocument.h"

class IdentityXML:public IFMSXMLDocument
{
public:

	IdentityXML();

	std::string GetXML();
	const bool ParseXML(const std::string &xml);

	const std::string GetName() const				{ return m_name; }
	void SetName(const std::string &name)			{ m_name=name; }

	const bool GetPublishTrustList() const			{ return m_publishtrustlist; }
	void SetPublishTrustList(const bool publish)	{ m_publishtrustlist=publish; }

	const bool GetPublishBoardList() const			{ return m_publishboardlist; }
	void SetPublishBoardList(const bool publish)	{ m_publishboardlist=publish; }

	const bool GetSingleUse() const					{ return m_singleuse; }
	void SetSingleUse(const bool singleuse)			{ m_singleuse=singleuse; }

	const int GetFreesiteEdition() const			{ return m_freesiteedition; }
	void SetFreesiteEdition(const int edition)		{ m_freesiteedition=edition; }

	const std::string GetSignature() const			{ return m_signature; }
	void SetSignature(const std::string &signature)	{ m_signature=signature; }

	const std::string GetAvatar() const				{ return m_avatar; }
	void SetAvatar(const std::string &avatar)		{ m_avatar=avatar; }

private:
	void Initialize();

	std::string m_name;
	bool m_publishtrustlist;
	bool m_publishboardlist;
	bool m_singleuse;
	int m_freesiteedition;
	std::string m_signature;
	std::string m_avatar;

};

#endif	// _identityxml_
