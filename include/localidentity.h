#ifndef _localidentity_
#define _localidentity_

#include "idatabase.h"

class LocalIdentity:public IDatabase
{
public:
	LocalIdentity(SQLite3DB::DB *db);

	const bool Load(const int id);
	const bool Load(const std::string &name);

	const int GetID() const					{ return m_id; }
	const std::string GetName() const		{ return m_name; }
	const std::string GetPublicKey() const	{ return m_publickey; }
	const std::string GetPrivateKey() const	{ return m_privatekey; }
	const bool IsActive() const				{ return m_active; }

private:
	void Initialize();

	int m_id;
	std::string m_name;
	std::string m_publickey;
	std::string m_privatekey;
	bool m_active;
};

#endif	// _localidentity_
