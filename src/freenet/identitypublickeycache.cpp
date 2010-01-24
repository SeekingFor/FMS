#include "../../include/freenet/identitypublickeycache.h"

#include <algorithm>

long IdentityPublicKeyCache::m_cachesize(1000);
std::map<long,std::string> IdentityPublicKeyCache::m_cache;
std::map<long,long> IdentityPublicKeyCache::m_requestcount;

const bool IdentityPublicKeyCache::PublicKey(const long identityid, std::string &publickey)
{
	if(m_cache.find(identityid)==m_cache.end())
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblIdentity WHERE IdentityID=?;");
		st.Bind(0,identityid);
		st.Step();
		if(st.RowReturned())
		{
			if(st.ResultText(0,publickey) && publickey!="")
			{
				m_cache[identityid]=publickey;
				m_requestcount[identityid]++;
				TrimCache();
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
		publickey=m_cache[identityid];
		m_requestcount[identityid]++;
		return true;
	}
}

void IdentityPublicKeyCache::TrimCache()
{
	while(m_requestcount.size()>m_cachesize)
	{
		std::map<long,long>::iterator i=std::min_element(m_requestcount.begin(),m_requestcount.end(),minrequest());
		if(i!=m_requestcount.end())
		{
			m_cache.erase((*i).first);
			m_requestcount.erase(i);
		}
		else
		{
			return;
		}
	}
}
