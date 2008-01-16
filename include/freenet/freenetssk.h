#ifndef _freenetssk_
#define _freenetssk_

#include <string>

class FreenetSSK
{
public:

	const bool ValidBaseKey(const std::string &key) const;
	const bool ValidPublicKey() const;
	const bool ValidPrivateKey() const;

	const bool SetPublicKey(const std::string &publickey);
	const bool SetPrivateKey(const std::string &privatekey);

private:

	std::string m_publickey;
	std::string m_privatekey;
	
};

#endif	// _freenetssk_
