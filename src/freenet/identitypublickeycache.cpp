#include "../../include/freenet/identitypublickeycache.h"

#include <algorithm>

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
				m_cache.update(identityid,publickey);
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
