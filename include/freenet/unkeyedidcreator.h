#ifndef _unkeyedidcreator_
#define _unkeyedidcreatorr_

#include "../ilogger.h"
#include "../idatabase.h"
#include "ifreenetregistrable.h"
#include "ifcpconnected.h"
#include "ifcpmessagehandler.h"
#include "iperiodicprocessor.h"

#include <Poco/DateTime.h>


/**
	\brief Looks for any unkeyed Local Identities and requests SSK keys for them
*/
class UnkeyedIDCreator:public IFreenetRegistrable,public IFCPConnected,public IFCPMessageHandler,public IDatabase,public IPeriodicProcessor,public ILogger
{
public:
	UnkeyedIDCreator(SQLite3DB::DB *db);
	UnkeyedIDCreator(SQLite3DB::DB *db, FCPv2::Connection *fcp);

	const bool HandleMessage(FCPv2::Message &message);

	void FCPDisconnected();
	void FCPConnected();

	void Process();

	void RegisterWithThread(FreenetMasterThread *thread);
	
private:
	void Initialize();
	void CheckForUnkeyedID();
	void SaveKeys(const long localidentityid, const std::string &publickey, const std::string &privatekey);

	Poco::DateTime m_lastchecked;
	bool m_waiting;
};

#endif	// _unkeyedidcreator_
