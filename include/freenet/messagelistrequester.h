#ifndef _messagelistrequester_
#define _messagelistrequester_

#include "iindexrequester.h"

#include <map>

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
	void GetBoardList(std::map<std::string,bool> &boards);
	const bool CheckDateNotFuture(const std::string &datestr) const;
	const bool CheckDateWithinMaxDays(const std::string &datestr) const;

	bool m_localtrustoverrides;
	bool m_savetonewboards;
	long m_messagedownloadmaxdaysbackward;

};

#endif	// _messagelistrequester_
