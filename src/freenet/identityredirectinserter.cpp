#include "../../include/freenet/identityredirectinserter.h"
#include "../../include/stringfunctions.h"
#include "../../include/option.h"

IdentityRedirectInserter::IdentityRedirectInserter(SQLite3DB::DB *db):IDatabase(db)
{
	Initialize();
}

IdentityRedirectInserter::IdentityRedirectInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp):IDatabase(db),IFCPConnected(fcp)
{
	Initialize();
}

void IdentityRedirectInserter::CheckForNeededInsert()
{

	m_log->trace("IdentityRedirectInserter::CheckForNeededInsert");

	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID, Redirect FROM tmpLocalIdentityRedirectInsert ORDER BY RANDOM() LIMIT 0,1;");
	st.Step();
	
	if(st.RowReturned())
	{
		int localidentityid=0;
		std::string redirect("");
		
		st.ResultInt(0, localidentityid);
		st.ResultText(1, redirect);
		st.Reset();

		StartInsert(localidentityid,redirect);
	}
	else
	{
		m_log->trace("IdentityRedirectInserter::CheckForNeededInsert rs is empty");
		int err=0;
		std::string error("");

		err=m_db->GetLastError(error);
		if(err!=SQLITE_OK && err!=SQLITE_DONE)
		{
			m_log->trace("IdentityRedirectInserter::CheckForNeededInsert db error "+error);
		}
	}

}

void IdentityRedirectInserter::FCPConnected()
{
	m_lastchecked=Poco::Timestamp();
}


void IdentityRedirectInserter::FCPDisconnected()
{
	
}

const bool IdentityRedirectInserter::HandleMessage(FCPv2::Message &message)
{

	if(message["Identifier"].find("IdentityRedirectInserter")==0)
	{
		Poco::DateTime now;
		std::vector<std::string> idparts;

		StringFunctions::Split(message["Identifier"],"|",idparts);

		// no action for URIGenerated
		if(message.GetName()=="URIGenerated")
		{
			return true;
		}

		// no action for IdentifierCollision
		if(message.GetName()=="IdentifierCollision")
		{
			return true;
		}

		if(message.GetName()=="PutSuccessful")
		{
			m_log->debug("IdentityRedirectInserter::HandleMessage PutSuccessful for "+message["Identifier"]);

			if(idparts.size()>2)
			{
				SQLite3DB::Statement st=m_db->Prepare("DELETE FROM tmpLocalIdentityRedirectInsert WHERE LocalIdentityID=?;");
				st.Bind(0,idparts[1]);
				st.Step();
			}

			return true;
		}

		if(message.GetName()=="PutFailed")
		{
			m_log->error("IdentityRedirectInserter::HandleMessage PutFailed for "+message["Identifier"]);
			return true;
		}

	}

	return false;

}

void IdentityRedirectInserter::Initialize()
{
	m_lastchecked=Poco::Timestamp();
}

void IdentityRedirectInserter::Process()
{
	Poco::DateTime now=Poco::Timestamp();

	if(m_lastchecked<(now-Poco::Timespan(0,0,30,0,0)))
	{
		CheckForNeededInsert();
		m_lastchecked=now;
	}
}

void IdentityRedirectInserter::RegisterWithThread(FreenetMasterThread *thread)
{
	thread->RegisterFCPConnected(this);
	thread->RegisterFCPMessageHandler(this);
	thread->RegisterPeriodicProcessor(this);
}

void IdentityRedirectInserter::StartInsert(const long localidentityid, const std::string &redirect)
{
	std::string idstring("");

	StringFunctions::Convert(localidentityid,idstring);

	SQLite3DB::Statement st=m_db->Prepare("SELECT PrivateKey FROM tblLocalIdentity WHERE LocalIdentityID=?");
	st.Bind(0, localidentityid);
	st.Step();

	if(st.RowReturned())
	{
		std::string privatekey("");
		std::string messagebase("");
		Option option(m_db);

		option.Get("MessageBase",messagebase);

		st.ResultText(0, privatekey);

		FCPv2::Message mess("ClientPut");
		mess["URI"]="USK"+privatekey.substr(3)+messagebase+"|IdentityRedirect/0/";
		mess["Identifier"]="IdentityRedirectInserter|"+idstring+"|"+mess["URI"];
		mess["UploadFrom"]="redirect";
		mess["TargetURI"]=redirect;
		m_fcp->Send(mess);

		m_log->debug("IdentityRedirectInserter::StartInsert inserting redirect for "+idstring+" to "+redirect);
	}
}
