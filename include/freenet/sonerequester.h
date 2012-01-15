#ifndef _sonerequester_
#define _sonerequester_

#include "iindexrequester.h"

#include <set>
#include <map>

class SoneRequester:public IIndexRequester<std::pair<long,long> >
{
public:
	SoneRequester(SQLite3DB::DB *db);
	SoneRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

protected:
	virtual void Initialize();
	virtual void PopulateIDList();
	const std::pair<long,long> GetIDFromIdentifier(const std::string &identifier);
	void StartRequest(const std::pair<long,long> &inputpair);
	const bool HandleAllData(FCPv2::Message &message);
	const bool HandleGetFailed(FCPv2::Message &message);

	std::string CleanupSone(const std::string &text);
	std::string CleanupSubject(const std::string &text);
	const std::string GetIdentityName(const long identityid);

	bool m_localtrustoverrides;
	std::string m_soneboardname;
	int m_soneboardid;
	int m_deletemessagesolderthan;
	std::set<std::string> m_soneids;	// sone ids we already have in database - could get large, so change to cache in future

};

#endif	// _sonerequester_
