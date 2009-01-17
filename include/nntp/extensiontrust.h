#ifndef _extension_trust_
#define _extension_trust_

#include "../idatabase.h"

class TrustExtension:public IDatabase
{
public:
	TrustExtension(SQLite3DB::DB *db);
	TrustExtension(SQLite3DB::DB *db, const int &localidentityid);

	struct trust
	{
		trust()	{}
		trust(const int localmessagetrust, const int peermessagetrust, const std::string &messagetrustcomment, const int localtrustlisttrust, const int peertrustlisttrust, const std::string &trustlisttrustcomment):m_localmessagetrust(localmessagetrust),m_peermessagetrust(peermessagetrust),m_messagetrustcomment(messagetrustcomment),m_localtrustlisttrust(localtrustlisttrust),m_peertrustlisttrust(peertrustlisttrust),m_trustlisttrustcomment(trustlisttrustcomment)	{}
		int m_localmessagetrust;
		int m_peermessagetrust;
		std::string m_messagetrustcomment;
		int m_localtrustlisttrust;
		int m_peertrustlisttrust;
		std::string m_trustlisttrustcomment;
	};

	void SetLocalIdentityID(const int id)	{ m_localidentityid=id; }

	const bool GetMessageTrust(const std::string &nntpname, int &trust);
	const bool GetTrustListTrust(const std::string &nntpname, int &trust);
	const bool GetPeerMessageTrust(const std::string &nntpname, int &trust);
	const bool GetPeerTrustListTrust(const std::string &nntpname, int &trust);

	const bool SetMessageTrust(const std::string &nntpname, const int trust);
	const bool SetTrustListTrust(const std::string &nntpname, const int trust);
	const bool SetMessageTrustComment(const std::string &nntpname, const std::string &comment);
	const bool SetTrustListTrustComment(const std::string &nntpname, const std::string &comment);

	const bool GetTrustList(std::map<std::string,trust> &trustlist);

private:

	const int GetIdentityID(const std::string &nntpname);	// return -1 if not found

	int m_localidentityid;

};

#endif	// _extension_trust
