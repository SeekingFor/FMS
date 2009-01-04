#include "../../include/freenet/identityinserter.h"
#include "../../include/freenet/identityxml.h"
#include "../../include/stringfunctions.h"
#include "../../include/option.h"

#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>

#ifdef XMEM
	#include <xmem.h>
#endif

IdentityInserter::IdentityInserter()
{
	Initialize();
}

IdentityInserter::IdentityInserter(FCPv2::Connection *fcp):IFCPConnected(fcp)
{
	Initialize();
}

void IdentityInserter::CheckForNeededInsert()
{
	Poco::DateTime now;
	Poco::DateTime date;

	// set date to 1 hour back
	date-=Poco::Timespan(0,1,0,0,0);

	// Because of importance of Identity.xml, if we are now at the next day we immediately want to insert identities so change the date back to 12:00 AM so we find all identities not inserted yet today
	if(date.day()!=now.day())
	{
		date=now;
		date.assign(date.year(),date.month(),date.day(),0,0,0);
	}

	SQLite3DB::Recordset rs=m_db->Query("SELECT LocalIdentityID FROM tblLocalIdentity WHERE PrivateKey IS NOT NULL AND PrivateKey <> '' AND InsertingIdentity='false' AND (LastInsertedIdentity<'"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S")+"' OR LastInsertedIdentity IS NULL) ORDER BY LastInsertedIdentity;");
	
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

const bool IdentityInserter::HandleMessage(FCPv2::Message &message)
{

	if(message["Identifier"].find("IdentityInserter")==0)
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
			// a little hack here - if we just inserted index yesterday and it is now the next day - we would have inserted todays date not yesterdays as LastInsertedIdentity.
			// If this is the case, we will skip updating LastInsertedIdentity so that we can insert this identity again for today
			Poco::DateTime lastdate;
			int tzdiff=0;
			Poco::DateTimeParser::tryParse("%Y-%m-%d",idparts[4],lastdate,tzdiff);

			if(lastdate.day()==now.day())
			{
				m_db->Execute("UPDATE tblLocalIdentity SET InsertingIdentity='false', LastInsertedIdentity='"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S")+"' WHERE LocalIdentityID="+idparts[1]+";");
			}
			else
			{
				m_db->Execute("UPDATE tblLocalIdentity SET InsertingIdentity='false' WHERE LocalIdentityID="+idparts[1]+";");
			}
			m_db->Execute("INSERT INTO tblLocalIdentityInserts(LocalIdentityID,Day,InsertIndex) VALUES("+idparts[1]+",'"+idparts[4]+"',"+idparts[2]+");");
			m_log->debug("IdentityInserter::HandleMessage inserted Identity xml");
			return true;
		}

		if(message.GetName()=="PutFailed")
		{
			m_db->Execute("UPDATE tblLocalIdentity SET InsertingIdentity='false' WHERE LocalIdentityID="+idparts[1]+";");
			m_log->debug("IdentityInserter::HandleMessage failure inserting Identity xml.  Code="+message["Code"]+" Description="+message["CodeDescription"]);
			
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
	m_lastchecked=Poco::Timestamp();
}

void IdentityInserter::Process()
{
	Poco::DateTime now;

	if(m_lastchecked<(now-Poco::Timespan(0,0,1,0,0)))
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
	Poco::DateTime date;
	std::string idstring;

	StringFunctions::Convert(localidentityid,idstring);

	SQLite3DB::Recordset rs=m_db->Query("SELECT Name,PrivateKey,SingleUse,PublishTrustList,PublishBoardList,PublishFreesite,FreesiteEdition FROM tblLocalIdentity WHERE LocalIdentityID="+idstring+";");

	if(rs.Empty()==false)
	{
		IdentityXML idxml;
		FCPv2::Message mess;
		Poco::DateTime now;
		std::string messagebase;
		std::string data;
		std::string datasizestr;
		std::string privatekey;
		long index=0;
		std::string indexstr;
		std::string singleuse="false";
		std::string publishtrustlist="false";
		std::string publishboardlist="false";
		std::string freesiteedition="";
		int edition=-1;

		now=Poco::Timestamp();

		SQLite3DB::Recordset rs2=m_db->Query("SELECT MAX(InsertIndex) FROM tblLocalIdentityInserts WHERE LocalIdentityID="+idstring+" AND Day='"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"';");
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

		if(rs.GetField(5) && rs.GetField(6))
		{
			if(std::string(rs.GetField(5))=="true")
			{
				freesiteedition=rs.GetField(6);
				StringFunctions::Convert(freesiteedition,edition);
				idxml.SetFreesiteEdition(edition);
			}
		}

		data=idxml.GetXML();
		StringFunctions::Convert(data.size(),datasizestr);

		mess.SetName("ClientPut");
		mess["URI"]=privatekey+messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|Identity|"+indexstr+".xml";
		mess["Identifier"]="IdentityInserter|"+idstring+"|"+indexstr+"|"+mess["URI"];
		mess["UploadFrom"]="direct";
		mess["DataLength"]=datasizestr;
		m_fcp->Send(mess);
		m_fcp->Send(std::vector<char>(data.begin(),data.end()));

		m_db->Execute("UPDATE tblLocalIdentity SET InsertingIdentity='true' WHERE LocalIdentityID="+idstring+";");

	}
}
