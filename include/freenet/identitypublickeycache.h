#ifndef _identitypublickeycache_
#define _identitypublickeycache_

#include "../idatabase.h"
#include <Poco/LRUCache.h>

#include <map>

/*
	MUST only be used in the Freenet thread
*/

class IdentityPublicKeyCache:public IDatabase
{
public:
	IdentityPublicKeyCache(SQLite3DB::DB *db):IDatabase(db)	{ }

	const bool PublicKey(const long identityid, std::string &publickey);

private:

	struct minrequest
	{
	public:
		const bool operator()(const std::pair<long,long> &a, const std::pair<long,long> &b) const
		{
			return a.second<b.second;
		}
	};

	static Poco::LRUCache<long,std::string> m_cache;

};

#endif	// _identitypublickeycache_
