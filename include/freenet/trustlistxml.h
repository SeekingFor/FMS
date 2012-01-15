#ifndef _trustlistxml_
#define _trustlistxml_

#include "../ifmsxmldocument.h"
#include "../ilogger.h"

#include <vector>

// trust of -1 will mean NULL trust
class TrustListXML:public IFMSXMLDocument,public ILogger
{
private:
	struct trust
	{
		trust(const std::string &identity, const long messagetrust, const long trustlisttrust, const std::string &messagetrustcomment, const std::string &trustlisttrustcomment, bool isfms, bool iswot):m_identity(identity),m_messagetrust(messagetrust),m_trustlisttrust(trustlisttrust),m_messagetrustcomment(messagetrustcomment),m_trustlisttrustcomment(trustlisttrustcomment),m_isfms(isfms),m_iswot(iswot) {}
		std::string m_identity;
		long m_messagetrust;
		long m_trustlisttrust;
		std::string m_messagetrustcomment;
		std::string m_trustlisttrustcomment;
		bool m_isfms;
		bool m_iswot;
	};
public:
	TrustListXML();

	std::string GetXML();
	
	const bool ParseXML(const std::string &xml);

	void ClearTrust()			{ m_trust.clear(); }

	void AddTrust(const std::string &identity, const long messagetrust, const long trustlisttrust, const std::string &messagetrustcomment, const std::string &trustlisttrustcomment, bool isfms, bool iswot);

	const std::vector<trust>::size_type TrustCount()		{ return m_trust.size(); }
	std::string GetIdentity(const long index);
	long GetMessageTrust(const long index);
	long GetTrustListTrust(const long index);
	std::string GetMessageTrustComment(const long index);
	std::string GetTrustListTrustComment(const long index);
	bool GetIsFMS(const long index);
	bool GetIsWOT(const long index);

private:
	void Initialize();

	std::vector<trust> m_trust;
	
};

#endif	// _trustlistxml_
