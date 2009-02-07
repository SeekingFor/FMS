#ifndef _messagelistrequester_
#define _messagelistrequester_

#include "iindexrequester.h"

#include <map>
#include <set>

#include <Poco/DateTime.h>

class MessageListRequester:public IIndexRequester<long>
{
public:
	MessageListRequester(SQLite3DB::DB *db);
	MessageListRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	virtual void Initialize();
	virtual void PopulateIDList();
	void StartRequest(const long &id);
	void StartRedirectRequest(FCPv2::Message &message);
	const bool HandleAllData(FCPv2::Message &message);
	const bool HandleGetFailed(FCPv2::Message &message);
	void GetBoardList(std::map<std::string,bool> &boards, const bool forceload=false);
	const bool CheckDateNotFuture(const std::string &datestr) const;
	const bool CheckDateWithinMaxDays(const std::string &datestr) const;

	bool m_localtrustoverrides;
	bool m_savetonewboards;
	long m_messagedownloadmaxdaysbackward;

	std::map<std::string,bool> m_boardscache;
	Poco::DateTime m_boardscacheupdate;			// last time we updated the boards cache

	std::map<std::string,std::map<long,std::set<long> > > m_requestindexcache;	// date - identity id - index

};

#endif	// _messagelistrequester_
