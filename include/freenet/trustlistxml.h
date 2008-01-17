#ifndef _trustlistxml_
#define _trustlistxml_

#include "../ifmsxmldocument.h"
#include "../ilogger.h"

#include <vector>

class TrustListXML:public IFMSXMLDocument,public ILogger
{
public:
	TrustListXML();

	std::string GetXML();
	
	const bool ParseXML(const std::string &xml);

	void ClearTrust()			{ m_trust.clear(); }

	void AddTrust(const std::string &identity, const long messagetrust, const long trustlisttrust);

	const long TrustCount()		{ return m_trust.size(); }
	std::string GetIdentity(const long index);
	long GetMessageTrust(const long index);
	long GetTrustListTrust(const long index);

private:
	struct trust
	{
		trust(const std::string &identity, const long messagetrust, const long trustlisttrust):m_identity(identity),m_messagetrust(messagetrust),m_trustlisttrust(trustlisttrust) {}
		std::string m_identity;
		long m_messagetrust;
		long m_trustlisttrust;
	};

	void Initialize();

	std::vector<trust> m_trust;
	
};

#endif	// _trustlistxml_
