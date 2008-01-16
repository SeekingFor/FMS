#include "../../include/freenet/freenetssk.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const bool FreenetSSK::SetPrivateKey(const std::string &privatekey)
{
	if(ValidBaseKey(privatekey))
	{
		m_privatekey=privatekey;
		return true;
	}
	else
	{
		return false;
	}
}

const bool FreenetSSK::SetPublicKey(const std::string &publickey)
{
	if(ValidBaseKey(publickey))
	{
		m_publickey=publickey;
		return true;
	}
	else
	{
		return false;
	}
}

const bool FreenetSSK::ValidBaseKey(const std::string &key) const
{
	if(key.size()==0)
	{
		return false;
	}
	if(key.find("SSK@")!=0)
	{
		return false;
	}
	if(key.size()>0 && key.find("/")!=key.size()-1)
	{
		return false;
	}
	return true;
}

const bool FreenetSSK::ValidPrivateKey() const
{
	return ValidBaseKey(m_privatekey);
}

const bool FreenetSSK::ValidPublicKey() const
{
	return ValidBaseKey(m_publickey);
}
