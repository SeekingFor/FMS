#include "../../include/freenet/identitypublickeycache.h"

#include <algorithm>
#ifndef POCO_VERSION	// Used to be in Foundation.h, as of 1.4 it's in Version.h, but not included by default
#include <Poco/Version.h>
#endif

Poco::LRUCache<long,std::string> IdentityPublicKeyCache::m_cache(10000);

const bool IdentityPublicKeyCache::PublicKey(const long identityid, std::string &publickey)
{
	if(m_cache.has(identityid)==false)
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblIdentity WHERE IdentityID=?;");
		st.Bind(0,identityid);
		st.Step();
		if(st.RowReturned())
		{
			if(st.ResultText(0,publickey) && publickey!="")
			{
#if POCO_VERSION < 0x01040300	// previous Poco releases don't have update method
				m_cache.add(identityid,publickey);
#else
				m_cache.update(identityid,publickey);
#endif
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		publickey=(*m_cache.get(identityid));
		return true;
	}
}
