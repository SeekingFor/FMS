#ifndef _identityexportxml_
#define _identityexportxml_

#include "../ifmsxmldocument.h"

class IdentityExportXML:public IFMSXMLDocument
{
public:
	IdentityExportXML();

	std::string GetXML();
	const bool ParseXML(const std::string &xml);

	void AddIdentity(const std::string &name, const std::string &publickey, const std::string &privatekey, const bool singleuse=false, const bool publishtrustlist=false, const bool publishboardlist=false, const bool publishfreesite=false);

	const long GetCount()		{ return m_identities.size(); }

	const std::string GetName(const long index);
	const std::string GetPublicKey(const long index);
	const std::string GetPrivateKey(const long index);
	const bool GetSingleUse(const long index);
	const bool GetPublishTrustList(const long index);
	const bool GetPublishBoardList(const long index);
	const bool GetPublishFreesite(const long index);

private:
	void Initialize();

	struct identity
	{
		identity(const std::string &name, const std::string &publickey, const std::string &privatekey, const bool singleuse, const bool publishtrustlist, const bool publishboardlist, const bool publishfreesite):m_name(name),m_publickey(publickey),m_privatekey(privatekey),m_singleuse(singleuse),m_publishtrustlist(publishtrustlist),m_publishboardlist(publishboardlist),m_publishfreesite(publishfreesite)	{}
		std::string m_name;
		std::string m_publickey;
		std::string m_privatekey;
		bool m_singleuse;
		bool m_publishtrustlist;
		bool m_publishboardlist;
		bool m_publishfreesite;
	};

	std::vector<identity> m_identities;

};

#endif	// _identityexportxml_
