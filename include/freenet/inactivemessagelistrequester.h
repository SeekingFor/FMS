#ifndef _inactivemessagelistrequester_
#define _inactivemessagelistrequester_

#include "messagelistrequester.h"

#include <map>

class InactiveMessageListRequester:public MessageListRequester
{
public:
	InactiveMessageListRequester();
	InactiveMessageListRequester(FCPv2::Connection *fcp);

private:
	virtual void Initialize();
	virtual void PopulateIDList();

	bool m_localtrustoverrides;
	bool m_savetonewboards;
	long m_messagedownloadmaxdaysbackward;

};

#endif	// _inactivemessagelistrequester_
