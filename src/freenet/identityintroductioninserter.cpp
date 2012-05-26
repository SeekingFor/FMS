#include "../../include/freenet/identityintroductioninserter.h"
#include "../../include/freenet/identityintroductionxml.h"
#include "../../include/stringfunctions.h"
#include "../../include/hex.h"
#include "../../include/option.h"

#include <Poco/SHA1Engine.h>

#ifdef XMEM
	#include <xmem.h>
#endif

IdentityIntroductionInserter::IdentityIntroductionInserter(SQLite3DB::DB *db):IDatabase(db)
{
	Initialize();
}

IdentityIntroductionInserter::IdentityIntroductionInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp):IDatabase(db),IFCPConnected(fcp)
{
	Initialize();
}

void IdentityIntroductionInserter::CheckForNewInserts()
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID, Day, UUID, Solution FROM tblIdentityIntroductionInserts WHERE Inserted='false' AND LocalIdentityID IS NOT NULL AND Day IS NOT NULL AND UUID IS NOT NULL;");
	st.Step();
	if(st.RowReturned())
	{
		int localidentityid;
		std::string day, uuid, solution;
		st.ResultInt(0, localidentityid);
		st.ResultText(1, day);
		st.ResultText(2, uuid);
		st.ResultText(3, solution);
		st.Reset();

		StartInsert(localidentityid,day,uuid,solution);
	}
}

void IdentityIntroductionInserter::FCPConnected()
{
	m_inserting=false;
}

void IdentityIntroductionInserter::FCPDisconnected()
{

}

const bool IdentityIntroductionInserter::HandleMessage(FCPv2::Message &message)
{

	if(message["Identifier"].find("IdentityIntroductionInserter")==0)
	{
		std::vector<std::string> idparts;
		StringFunctions::Split(message["Identifier"],"|",idparts);
		int localidentityid;
		StringFunctions::Convert(idparts[1],localidentityid);
		
		// no action for URIGenerated
		if(message.GetName()=="URIGenerated")
		{
			return true;
		}

		if(message.GetName()=="PutFailed")
		{
			// if fatal error, or data is already there - remove insert from database
			if(message["Fatal"]=="true" || message["Code"]=="9")
			{
				m_db->Execute("DELETE FROM tblIdentityIntroductionInserts WHERE UUID='"+idparts[3]+"';");
				// update the puzzle from the request table (set to not found) because we don't need it anymore and don't want to user tyring to solve it again
				SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIntroductionPuzzleRequests SET Found='false' WHERE UUID=?");
				st.Bind(0, idparts[3]);
				st.Step();
				m_log->warning("IdentityIntroductionInserter::HandleMessage received fatal error trying to insert IdentityIntroduction "+idparts[3]);
			}
			m_inserting=false;
			return true;
		}

		if(message.GetName()=="PutSuccessful")
		{
			SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIdentityIntroductionInserts SET Inserted='true' WHERE UUID=?;");
			st.Bind(0, idparts[3]);
			st.Step();
			m_inserting=false;
			m_log->information("IdentityIntroductionInserter::HandleMessage successfully inserted IdentityIntroduction "+idparts[3]);
			return true;
		}

		if(message.GetName()=="IdentifierCollision")
		{
			m_inserting=false;
			return true;
		}
	}

	return false;
}

void IdentityIntroductionInserter::Initialize()
{
	m_inserting=false;
	Option option(m_db);
	option.Get("MessageBase",m_messagebase);
}

void IdentityIntroductionInserter::Process()
{
	Poco::DateTime now;

	// only do 1 insert at a time
	if(!m_inserting && m_lastchecked<(now-Poco::Timespan(0,0,1,0,0)))
	{
		CheckForNewInserts();
		m_lastchecked=now;
	}
}

void IdentityIntroductionInserter::RegisterWithThread(FreenetMasterThread *thread)
{
	thread->RegisterFCPConnected(this);
	thread->RegisterFCPMessageHandler(this);
	thread->RegisterPeriodicProcessor(this);
}

void IdentityIntroductionInserter::StartInsert(const long localidentityid, const std::string &day, const std::string &UUID, const std::string &solution)
{
	FCPv2::Message message;
	IdentityIntroductionXML xml;
	std::string publickey;
	std::string data;
	std::string datasizestr;
	std::string encodedhash;
	
	SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblLocalIdentity WHERE PublicKey IS NOT NULL AND PublicKey<>'' AND LocalIdentityID=?;");
	st.Bind(0,localidentityid);
	st.Step();

	if(st.RowReturned())
	{
		st.ResultText(0,publickey);

		xml.SetIdentity(publickey);
		data=xml.GetXML();
		StringFunctions::Convert(data.size(),datasizestr);

		Poco::SHA1Engine sha1;
		sha1.update(solution);
		encodedhash=Poco::DigestEngine::digestToHex(sha1.digest());
		StringFunctions::UpperCase(encodedhash,encodedhash);

		message.SetName("ClientPut");
		message["URI"]="KSK@"+m_messagebase+"|"+day+"|"+UUID+"|"+encodedhash+".xml";
		message["Identifier"]="IdentityIntroductionInserter|"+message["URI"];
		message["UploadFrom"]="direct";
		message["DataLength"]=datasizestr;
		message["Metadata.ContentType"]="";

		m_fcp->Send(message);
		m_fcp->Send(std::vector<char>(data.begin(),data.end()));

		m_inserting=true;
	}
	else
	{
		m_log->debug("IdentityIntroductionInserter::StartInsert could not find a public key for identity.  It is probably a new identity that doesn't have a key yet.");
	}

}
