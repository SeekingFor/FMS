#include "../../include/freenet/identityinserter.h"
#include "../../include/freenet/identityxml.h"
#include "../../include/stringfunctions.h"
#include "../../include/option.h"

#ifdef XMEM
	#include <xmem.h>
#endif

IdentityInserter::IdentityInserter()
{
	Initialize();
}

IdentityInserter::IdentityInserter(FCPv2 *fcp):IFCPConnected(fcp)
{
	Initialize();
}

void IdentityInserter::CheckForNeededInsert()
{
	DateTime now;
	DateTime date;
	now.SetToGMTime();
	date.SetToGMTime();
	// set date to 1 hour back
	date.Add(0,0,-1);

	// Because of importance of Identity.xml, if we are now at the next day we immediately want to insert identities so change the date back to 12:00 AM so we find all identities not inserted yet today
	if(date.GetDay()!=now.GetDay())
	{
		date=now;
		date.SetHour(0);
		date.SetMinute(0);
		date.SetSecond(0);
	}

	SQLite3DB::Recordset rs=m_db->Query("SELECT LocalIdentityID FROM tblLocalIdentity WHERE PrivateKey IS NOT NULL AND PrivateKey <> '' AND InsertingIdentity='false' AND (LastInsertedIdentity<'"+date.Format("%Y-%m-%d %H:%M:%S")+"' OR LastInsertedIdentity IS NULL) ORDER BY LastInsertedIdentity;");
	
	if(rs.Empty()==false)
	{
		StartInsert(rs.GetInt(0));
	}

}

void IdentityInserter::FCPConnected()
{
	m_db->Execute("UPDATE tblLocalIdentity SET InsertingIdentity='false';");
}


void IdentityInserter::FCPDisconnected()
{
	
}

const bool IdentityInserter::HandleMessage(FCPMessage &message)
{

	if(message["Identifier"].find("IdentityInserter")==0)
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
			// a little hack here - if we just inserted index yesterday and it is now the next day - we would have inserted todays date not yesterdays as LastInsertedIdentity.
			// If this is the case, we will skip updating LastInsertedIdentity so that we can insert this identity again for today
			DateTime lastdate;
			lastdate.Set(idparts[4]);
			if(lastdate.GetDay()==now.GetDay())
			{
				m_db->Execute("UPDATE tblLocalIdentity SET InsertingIdentity='false', LastInsertedIdentity='"+now.Format("%Y-%m-%d %H:%M:%S")+"' WHERE LocalIdentityID="+idparts[1]+";");
			}
			else
			{
				m_db->Execute("UPDATE tblLocalIdentity SET InsertingIdentity='false' WHERE LocalIdentityID="+idparts[1]+";");
			}
			m_db->Execute("INSERT INTO tblLocalIdentityInserts(LocalIdentityID,Day,InsertIndex) VALUES("+idparts[1]+",'"+idparts[4]+"',"+idparts[2]+");");
			m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"IdentityInserter::HandleMessage inserted Identity xml");
			return true;
		}

		if(message.GetName()=="PutFailed")
		{
			m_db->Execute("UPDATE tblLocalIdentity SET InsertingIdentity='false' WHERE LocalIdentityID="+idparts[1]+";");
			m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"IdentityInserter::HandleMessage failure inserting Identity xml.  Code="+message["Code"]+" Description="+message["CodeDescription"]);
			
			// if code 9 (collision), then insert index into inserted table
			if(message["Code"]=="9")
			{
				m_db->Execute("INSERT INTO tblLocalIdentityInserts(LocalIdentityID,Day,InsertIndex) VALUES("+idparts[1]+",'"+idparts[4]+"',"+idparts[2]+");");
			}
			
			return true;
		}

	}

	return false;

}

void IdentityInserter::Initialize()
{
	m_lastchecked.SetToGMTime();
}

void IdentityInserter::Process()
{
	DateTime now;
	now.SetToGMTime();

	if(m_lastchecked<(now-(1.0/1440.0)))
	{
		CheckForNeededInsert();
		m_lastchecked=now;
	}

}

void IdentityInserter::RegisterWithThread(FreenetMasterThread *thread)
{
	thread->RegisterFCPConnected(this);
	thread->RegisterFCPMessageHandler(this);
	thread->RegisterPeriodicProcessor(this);
}

void IdentityInserter::StartInsert(const long localidentityid)
{
	DateTime date;
	std::string idstring;

	StringFunctions::Convert(localidentityid,idstring);
	date.SetToGMTime();

	SQLite3DB::Recordset rs=m_db->Query("SELECT Name,PrivateKey,SingleUse,PublishTrustList,PublishBoardList FROM tblLocalIdentity WHERE LocalIdentityID="+idstring+";");

	if(rs.Empty()==false)
	{
		IdentityXML idxml;
		FCPMessage mess;
		DateTime now;
		std::string messagebase;
		std::string data;
		std::string datasizestr;
		std::string privatekey;
		long index=0;
		std::string indexstr;
		std::string singleuse="false";
		std::string publishtrustlist="false";
		std::string publishboardlist="false";

		now.SetToGMTime();

		SQLite3DB::Recordset rs2=m_db->Query("SELECT MAX(InsertIndex) FROM tblLocalIdentityInserts WHERE LocalIdentityID="+idstring+" AND Day='"+now.Format("%Y-%m-%d")+"';");
		if(rs2.Empty()==false)
		{
			if(rs2.GetField(0)==NULL)
			{
				index=0;
			}
			else
			{
				index=rs2.GetInt(0)+1;
			}
		}
		StringFunctions::Convert(index,indexstr);

		Option::Instance()->Get("MessageBase",messagebase);

		if(rs.GetField(0))
		{
			idxml.SetName(rs.GetField(0));
		}

		if(rs.GetField(1))
		{
			privatekey=rs.GetField(1);
		}

		if(rs.GetField(2))
		{
			singleuse=rs.GetField(2);
		}
		singleuse=="true" ? idxml.SetSingleUse(true) : idxml.SetSingleUse(false);

		if(rs.GetField(3))
		{
			publishtrustlist=rs.GetField(3);
		}
		publishtrustlist=="true" ? idxml.SetPublishTrustList(true) : idxml.SetPublishTrustList(false);

		if(rs.GetField(4))
		{
			publishboardlist=rs.GetField(4);
		}
		publishboardlist=="true" ? idxml.SetPublishBoardList(true) : idxml.SetPublishBoardList(false);

		data=idxml.GetXML();
		StringFunctions::Convert(data.size(),datasizestr);

		mess.SetName("ClientPut");
		mess["URI"]=privatekey+messagebase+"|"+now.Format("%Y-%m-%d")+"|Identity|"+indexstr+".xml";
		mess["Identifier"]="IdentityInserter|"+idstring+"|"+indexstr+"|"+mess["URI"];
		mess["UploadFrom"]="direct";
		mess["DataLength"]=datasizestr;
		m_fcp->SendMessage(mess);
		m_fcp->SendRaw(data.c_str(),data.size());

		m_db->Execute("UPDATE tblLocalIdentity SET InsertingIdentity='true' WHERE LocalIdentityID="+idstring+";");

	}
}
