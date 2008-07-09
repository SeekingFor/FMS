#ifndef _ipaddressacl_
#define _ipaddressacl_

#include <Poco/Net/IPAddress.h>
#include <vector>
#include <string>

class IPAddressACL
{
public:
	IPAddressACL();

	const bool IsAllowed(const Poco::Net::IPAddress &addr);
	const bool IsAllowed(const std::string &addrstr);

	const bool Add(const std::string &aclentry);

	void SetAllowByDefault(const bool allowbydefault)	{ m_allowbydefault=allowbydefault; }
	const bool GetAllowByDefault() const				{ return m_allowbydefault; }

private:
	const std::string CreateMask(const int maskbits);

	// Poco 1.3.0 + has Poco::Net::IPAddress::mask - but 1.2.9 does not
	Poco::Net::IPAddress MaskAddress(const Poco::Net::IPAddress &addr, const Poco::Net::IPAddress &mask);

	struct entry
	{
		entry(const bool allow, const Poco::Net::IPAddress &mask, const Poco::Net::IPAddress &addr):m_allow(allow),m_mask(mask),m_addr(addr)	{}
		bool m_allow;
		Poco::Net::IPAddress m_mask;
		Poco::Net::IPAddress m_addr;
	};

	bool m_allowbydefault;	// allow or deny hosts if not explicitly defined

	std::vector<entry> m_entries;

};

#endif	// _acl_
