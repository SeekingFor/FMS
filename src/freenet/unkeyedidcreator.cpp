#include "../../include/freenet/unkeyedidcreator.h"
#include "../../include/stringfunctions.h"

#include <Poco/Timestamp.h>

#include <sstream>

#ifdef XMEM
	#include <xmem.h>
#endif

UnkeyedIDCreator::UnkeyedIDCreator(SQLite3DB::DB *db):IDatabase(db)
{
	Initialize();
}

UnkeyedIDCreator::UnkeyedIDCreator(SQLite3DB::DB *db, FCPv2::Connection *fcp):IDatabase(db),IFCPConnected(fcp)
{
	Initialize();
}

void UnkeyedIDCreator::FCPConnected()
{
	m_waiting=false;
}

void UnkeyedIDCreator::FCPDisconnected()
{
	m_waiting=false;
}

void UnkeyedIDCreator::CheckForUnkeyedID()
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID FROM tblLocalIdentity WHERE PublicKey IS NULL OR PrivateKey IS NULL OR PublicKey='' OR PrivateKey='';");
	st.Step();

	if(st.RowReturned())
	{
		int id;
		st.ResultInt(0, id);

		std::string idstring;
		StringFunctions::Convert(id,idstring);

		FCPv2::Message message;
		message.SetName("GenerateSSK");
		message["Identifier"]="UnkeyedIDRequest|"+idstring;
		m_fcp->Send(message);

		m_waiting=true;
		st.Reset();

	}

	// set last checked time to now
	m_lastchecked=Poco::Timestamp();

}

const bool UnkeyedIDCreator::HandleMessage(FCPv2::Message &message)
{
	if(message["Identifier"].find("UnkeyedIDRequest")==0)
	{

		if(message.GetName()=="SSKKeypair")
		{

			long id;
			std::vector<std::string> idparts;
			StringFunctions::Split(message["Identifier"],"|",idparts);

			if(idparts.size()>1)
			{
				if(StringFunctions::Convert(idparts[1],id)==false)
				{
					id=0;
				}
				SaveKeys(id,message["RequestURI"],message["InsertURI"]);
			}

			m_log->information("UnkeyedIDCreator::HandleMessage received keypair");

			m_waiting=false;

			return true;
		}

	}

	return false;
}

void UnkeyedIDCreator::Initialize()
{
	m_waiting=false;
	m_lastchecked=Poco::Timestamp();
}

void UnkeyedIDCreator::Process()
{
	Poco::DateTime now;

	// only perform check every minute (1/1440 of 1 day)
	if(m_waiting==false && m_lastchecked<(now-Poco::Timespan(0,0,1,0,0)))
	{
		CheckForUnkeyedID();
	}
}

void UnkeyedIDCreator::RegisterWithThread(FreenetMasterThread *thread)
{
	thread->RegisterFCPConnected(this);
	thread->RegisterFCPMessageHandler(this);
	thread->RegisterPeriodicProcessor(this);
}

void UnkeyedIDCreator::SaveKeys(const long localidentityid, const std::string &publickey, const std::string &privatekey)
{
	SQLite3DB::Statement st=m_db->Prepare("UPDATE tblLocalIdentity SET PublicKey=?, PrivateKey=? WHERE LocalIdentityID=?;");
	st.Bind(0,publickey);
	st.Bind(1,privatekey);
	st.Bind(2,localidentityid);
	st.Step();
}
