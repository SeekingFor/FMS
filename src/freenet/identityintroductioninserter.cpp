#include "../../include/freenet/identityintroductioninserter.h"
#include "../../include/freenet/identityintroductionxml.h"
#include "../../include/xyssl/sha1.h"
#include "../../include/stringfunctions.h"
#include "../../include/hex.h"
#include "../../include/option.h"

#ifdef XMEM
	#include <xmem.h>
#endif

IdentityIntroductionInserter::IdentityIntroductionInserter()
{
	Initialize();
}

IdentityIntroductionInserter::IdentityIntroductionInserter(FCPv2 *fcp):IFCPConnected(fcp)
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

const bool IdentityIntroductionInserter::HandleMessage(FCPMessage &message)
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
				m_log->WriteLog(LogFile::LOGLEVEL_WARNING,"IdentityIntroductionInserter::HandleMessage received fatal error trying to insert IdentityIntroduction "+idparts[3]);
			}
			m_inserting=false;
			return true;
		}

		if(message.GetName()=="PutSuccessful")
		{
			m_db->Execute("UPDATE tblIdentityIntroductionInserts SET Inserted='true' WHERE UUID='"+idparts[3]+"';");
			m_inserting=false;
			m_log->WriteLog(LogFile::LOGLEVEL_INFO,"IdentityIntroductionInserter::HandleMessage successfully inserted IdentityIntroduction "+idparts[3]);
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
	Option::instance()->Get("MessageBase",m_messagebase);
}

void IdentityIntroductionInserter::Process()
{
	DateTime now;
	now.SetToGMTime();

	// only do 1 insert at a time
	if(!m_inserting && m_lastchecked<(now-(1.0/1440.0)))
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
	FCPMessage message;
	IdentityIntroductionXML xml;
	std::string publickey;
	std::string data;
	std::string datasizestr;
	std::vector<unsigned char> hash;
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

		hash.resize(20);
		sha1((unsigned char *)solution.c_str(),solution.size(),&hash[0]);
		Hex::Encode(hash,encodedhash);

		message.SetName("ClientPut");
		message["URI"]="KSK@"+m_messagebase+"|"+day+"|"+UUID+"|"+encodedhash+".xml";
		message["Identifier"]="IdentityIntroductionInserter|"+message["URI"];
		message["UploadFrom"]="direct";
		message["DataLength"]=datasizestr;

		m_fcp->SendMessage(message);
		m_fcp->SendRaw(data.c_str(),data.size());

		m_inserting=true;
	}
	else
	{
		m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"IdentityIntroductionInserter::StartInsert could not find a public key for identity.  It is probably a new identity that doesn't have a key yet.");
	}

}