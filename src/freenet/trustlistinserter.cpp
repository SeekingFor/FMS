#include "../../include/freenet/trustlistinserter.h"
#include "../../include/option.h"
#include "../../include/freenet/trustlistxml.h"
#include "../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

TrustListInserter::TrustListInserter()
{
	Initialize();
}

TrustListInserter::TrustListInserter(FCPv2 *fcp):IFCPConnected(fcp)
{
	Initialize();
}

void TrustListInserter::CheckForNeededInsert()
{
	DateTime date;
	date.SetToGMTime();
	date.Add(0,0,-1);
	SQLite3DB::Recordset rs=m_db->Query("SELECT LocalIdentityID, PrivateKey FROM tblLocalIdentity WHERE PrivateKey IS NOT NULL AND PrivateKey <> '' AND PublishTrustList='true' AND InsertingTrustList='false' AND (LastInsertedTrustList<='"+date.Format("%Y-%m-%d %H:%M:%S")+"' OR LastInsertedTrustList IS NULL);");

	if(rs.Empty()==false)
	{
		StartInsert(rs.GetInt(0),rs.GetField(1));
	}
}

void TrustListInserter::FCPConnected()
{
	m_db->Execute("UPDATE tblLocalIdentity SET InsertingTrustList='false';");
}

void TrustListInserter::FCPDisconnected()
{

}

const bool TrustListInserter::HandleMessage(FCPMessage &message)
{

	if(message["Identifier"].find("TrustListInserter")==0)
	{
		
		DateTime now;
		std::vector<std::string> idparts;

		now.SetToGMTime();
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
			m_db->Execute("UPDATE tblLocalIdentity SET InsertingTrustList='false', LastInsertedTrustList='"+now.Format("%Y-%m-%d %H:%M:%S")+"' WHERE LocalIdentityID="+idparts[1]+";");
			m_db->Execute("INSERT INTO tblTrustListInserts(LocalIdentityID,Day,InsertIndex) VALUES("+idparts[1]+",'"+idparts[4]+"',"+idparts[2]+");");
			m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"TrustListInserter::HandleMessage inserted TrustList xml");
			return true;
		}

		if(message.GetName()=="PutFailed")
		{
			m_db->Execute("UPDATE tblLocalIdentity SET InsertingTrustList='false' WHERE LocalIdentityID="+idparts[1]+";");
			m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"TrustListInserter::HandleMessage failure inserting TrustList xml.  Code="+message["Code"]+" Description="+message["CodeDescription"]);
			
			// if code 9 (collision), then insert index into inserted table
			if(message["Code"]=="9")
			{
				m_db->Execute("INSERT INTO tblTrustListInserts(LocalIdentityID,Day,InsertIndex) VALUES("+idparts[1]+",'"+idparts[4]+"',"+idparts[2]+");");
			}
			
			return true;
		}

	}

	return false;
}

void TrustListInserter::Initialize()
{
	Option::instance()->Get("MessageBase",m_messagebase);
	m_lastchecked.SetToGMTime();
}

void TrustListInserter::Process()
{
	DateTime now;
	now.SetToGMTime();

	// check every minute
	if(m_lastchecked<=(now-(1.0/1440.0)))
	{
		CheckForNeededInsert();
		m_lastchecked=now;
	}
}

void TrustListInserter::RegisterWithThread(FreenetMasterThread *thread)
{
	thread->RegisterFCPConnected(this);
	thread->RegisterFCPMessageHandler(this);
	thread->RegisterPeriodicProcessor(this);
}

void TrustListInserter::StartInsert(const long localidentityid, const std::string &privatekey)
{
	FCPMessage message;
	TrustListXML xml;
	std::string data;
	std::string datasizestr;
	std::string publickey;
	int messagetrust;
	int trustlisttrust;
	DateTime now;
	int index;
	std::string indexstr;
	std::string localidentityidstr;

	now.SetToGMTime();
	
	// build the xml file
	SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey, LocalMessageTrust, LocalTrustListTrust FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey<>'';");
	st.Step();
	while(st.RowReturned())
	{
		st.ResultText(0,publickey);
		st.ResultInt(1,messagetrust);
		st.ResultInt(2,trustlisttrust);
		xml.AddTrust(publickey,messagetrust,trustlisttrust);
		st.Step();
	}

	// get next insert index
	st=m_db->Prepare("SELECT MAX(InsertIndex) FROM tblTrustListInserts WHERE LocalIdentityID=? AND Day=?;");
	st.Bind(0,localidentityid);
	st.Bind(1,now.Format("%Y-%m-%d"));
	st.Step();

	index=0;
	if(st.RowReturned() && st.ResultNull(0)==false)
	{
		st.ResultInt(0,index);
		index++;
	}

	StringFunctions::Convert(localidentityid,localidentityidstr);
	StringFunctions::Convert(index,indexstr);

	data=xml.GetXML();
	StringFunctions::Convert(data.size(),datasizestr);

	message.SetName("ClientPut");
	message["URI"]=privatekey+m_messagebase+"|"+now.Format("%Y-%m-%d")+"|TrustList|"+indexstr+".xml";
	message["Identifier"]="TrustListInserter|"+localidentityidstr+"|"+indexstr+"|"+message["URI"];
	message["UploadFrom"]="direct";
	message["DataLength"]=datasizestr;
	m_fcp->SendMessage(message);
	m_fcp->SendRaw(data.c_str(),data.size());

	m_db->Execute("UPDATE tblLocalIdentity SET InsertingTrustList='true' WHERE LocalIdentityID="+localidentityidstr+";");

}