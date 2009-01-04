#include "../../include/freenet/identityintroductioninserter.h"
#include "../../include/freenet/identityintroductionxml.h"
#include "../../include/stringfunctions.h"
#include "../../include/hex.h"
#include "../../include/option.h"

#include <Poco/SHA1Engine.h>

#ifdef XMEM
	#include <xmem.h>
#endif

IdentityIntroductionInserter::IdentityIntroductionInserter()
{
	Initialize();
}

IdentityIntroductionInserter::IdentityIntroductionInserter(FCPv2::Connection *fcp):IFCPConnected(fcp)
{
	Initialize();
}

void IdentityIntroductionInserter::CheckForNewInserts()
{
	SQLite3DB::Recordset rs=m_db->Query("SELECT LocalIdentityID, Day, UUID, Solution FROM tblIdentityIntroductionInserts WHERE Inserted='false';");
	if(!rs.Empty())
	{
		if(rs.GetField(0) && rs.GetField(1) && rs.GetField(2))
		{
			StartInsert(rs.GetInt(0),rs.GetField(1),rs.GetField(2),rs.GetField(3));
		}
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
				m_db->Execute("UPDATE tblIntroductionPuzzleRequests SET Found='false' WHERE UUID='"+idparts[3]+"';");
				m_log->warning("IdentityIntroductionInserter::HandleMessage received fatal error trying to insert IdentityIntroduction "+idparts[3]);
			}
			m_inserting=false;
			return true;
		}

		if(message.GetName()=="PutSuccessful")
		{
			m_db->Execute("UPDATE tblIdentityIntroductionInserts SET Inserted='true' WHERE UUID='"+idparts[3]+"';");
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
	Option::Instance()->Get("MessageBase",m_messagebase);
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

		m_fcp->Send(message);
		m_fcp->Send(std::vector<char>(data.begin(),data.end()));

		m_inserting=true;
	}
	else
	{
		m_log->debug("IdentityIntroductionInserter::StartInsert could not find a public key for identity.  It is probably a new identity that doesn't have a key yet.");
	}

}
