#ifndef _messagerequester_
#define _messagerequester_

#include "iindexrequester.h"

class MessageRequester:public IIndexRequester<std::string>
{
public:
	MessageRequester(SQLite3DB::DB *db);
	MessageRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);

private:
	void Initialize();
	void PopulateIDList();
	const std::string GetIDFromIdentifier(const std::string &identifier);
	void StartRequest(const std::string &requestid);
	const bool HandleAllData(FCPv2::Message &message);
	const bool HandleGetFailed(FCPv2::Message &message);

	const long GetBoardInfo(SQLite3DB::Transaction &trans, const std::string &boardname, const std::string &identityname, bool &forum);
	const bool SaveToBoard(const std::string &boardname);
	const std::string GetIdentityName(const long identityid);

	int m_maxdaysbackward;
	int m_maxpeermessages;
	int m_maxboardspermessage;
	bool m_savemessagesfromnewboards;
	bool m_localtrustoverrides;

	static std::string m_validuuidchars;
	
};

#endif	// _messagerequester_
