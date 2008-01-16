#ifndef _unkeyedidcreator_
#define _unkeyedidcreatorr_

#include "../ilogger.h"
#include "../datetime.h"
#include "../idatabase.h"
#include "ifreenetregistrable.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"


/**
	\brief Looks for any unkeyed Local Identities and requests SSK keys for them
*/
class UnkeyedIDCreator:public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IDatabase,public IPeriodicProcessor,public ILogger
{
public:
	UnkeyedIDCreator();
	UnkeyedIDCreator(FCPv2 *fcp);

	const bool HandleMessage(FCPMessage &message);

	void FCPDisconnected();
	void FCPConnected();

	void Process();

	void RegisterWithThread(FreenetMasterThread *thread);
	
private:
	void Initialize();
	void CheckForUnkeyedID();
	void SaveKeys(const long localidentityid, const std::string &publickey, const std::string &privatekey);

	DateTime m_lastchecked;
	bool m_waiting;
};

#endif	// _unkeyedidcreator_
