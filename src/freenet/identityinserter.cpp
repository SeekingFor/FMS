#include "../../include/freenet/identityinserter.h"
#include "../../include/freenet/identityxml.h"
#include "../../include/stringfunctions.h"
#include "../../include/option.h"

#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>

#ifdef XMEM
	#include <xmem.h>
#endif

IdentityInserter::IdentityInserter(SQLite3DB::DB *db):IDatabase(db)
{
	Initialize();
}

IdentityInserter::IdentityInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp):IDatabase(db),IFCPConnected(fcp)
{
	Initialize();
}

void IdentityInserter::CheckForNeededInsert()
{
	Poco::DateTime now;
	Poco::DateTime date;
	Poco::DateTime nowplus30min;

	// always set date to midnight of current day, so we'll only insert 1 identity file per day
	date.assign(date.year(),date.month(),date.day(),0,0,0);
	nowplus30min+=Poco::Timespan(0,0,30,0,0);

	// +30 minutes is the next day - we want to select identities that haven't inserted yet in the new day so set date to midnight of the next day
	if(nowplus30min.day()!=now.day())
	{
		date=nowplus30min;
		date.assign(date.year(),date.month(),date.day(),0,0,0);
	}

	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID FROM tblLocalIdentity WHERE tblLocalIdentity.Active='true' AND PrivateKey IS NOT NULL AND PrivateKey <> '' AND InsertingIdentity='false' AND (LastInsertedIdentity<? OR LastInsertedIdentity IS NULL) ORDER BY LastInsertedIdentity;");
	st.Bind(0,Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S"));
	st.Step();

	if(st.RowReturned())
	{
		int lid=0;
		st.ResultInt(0,lid);
		StartInsert(lid);
		// if +30 minutes is the next day, we also insert the identity.xml file to that day
		if(now.day()!=nowplus30min.day())
		{
			StartInsert(lid,1);
		}
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
		m_lastreceivedmessage=now;
		int localidentityid, insertindex;
		StringFunctions::Convert(idparts[1], localidentityid);
		StringFunctions::Convert(idparts[2], insertindex);

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
	
			// do check to make sure this is the non-editioned SSK - we ignore failure/success for editioned SSK for now
			if(message["Identifier"].find(".xml")!=std::string::npos)
			{
				// a little hack here - if we just inserted index yesterday and it is now the next day - we would have inserted todays date not yesterdays as LastInsertedIdentity.
				// If this is the case, we will skip updating LastInsertedIdentity so that we can insert this identity again for today
				Poco::DateTime lastdate;
				int tzdiff=0;
				Poco::DateTimeParser::tryParse("%Y-%m-%d",idparts[4],lastdate,tzdiff);
				SQLite3DB::Statement st;
				if(lastdate.day()==now.day())
				{
					st=m_db->Prepare("UPDATE tblLocalIdentity SET InsertingIdentity='false', LastInsertedIdentity=? WHERE LocalIdentityID=?;");
					st.Bind(0, Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
					st.Bind(1, localidentityid);
					st.Step();
				}
				// we inserted to a future date - set date to midnight of that day
				else if(lastdate.year()>now.year() || (lastdate.year()==now.year() && lastdate.month()>now.month()) || (lastdate.year()==now.year() && lastdate.month()==now.month() && lastdate.day()>now.day()))
				{
					st=m_db->Prepare("UPDATE tblLocalIdentity SET InsertingIdentity='false', LastInsertedIdentity=? WHERE LocalIdentityID=?");
					st.Bind(0, Poco::DateTimeFormatter::format(lastdate,"%Y-%m-%d 00:00:00"));
					st.Bind(1, localidentityid);
					st.Step();
				}
				else
				{
					st=m_db->Prepare("UPDATE tblLocalIdentity SET InsertingIdentity='false' WHERE LocalIdentityID=?");
					st.Bind(0, localidentityid);
					st.Step();
				}
				st=m_db->Prepare("INSERT INTO tblLocalIdentityInserts(LocalIdentityID,Day,InsertIndex) VALUES(?, ?, ?)");
				st.Bind(0, localidentityid);
				st.Bind(1, idparts[4]);
				st.Bind(2, insertindex);
				st.Step();
				
				st=m_db->Prepare("INSERT OR REPLACE INTO tmpLocalIdentityRedirectInsert(LocalIdentityID,Redirect) VALUES(?,?);");
				st.Bind(0,localidentityid);
				st.Bind(1,StringFunctions::UriDecode(message["URI"]));
				st.Step();
				
				m_log->debug("IdentityInserter::HandleMessage inserted Identity xml");
			}
			else
			{
				m_log->trace("IdentityInserter::HandleMessage inserted editioned Identity xml");
			}
			return true;
		}

		if(message.GetName()=="PutFailed")
		{
			// do check to make sure this is the non-editioned SSK - we ignore failure/success for editioned SSK for now
			if(message["Identifier"].find(".xml")!=std::string::npos)
			{
				SQLite3DB::Statement st=m_db->Prepare("UPDATE tblLocalIdentity SET InsertingIdentity='false' WHERE LocalIdentityID=?");
				st.Bind(0, localidentityid);
				st.Step();
				m_log->debug("IdentityInserter::HandleMessage failure inserting Identity xml.  Code="+message["Code"]+" Description="+message["CodeDescription"]);
				
				// if code 9 (collision), then insert index into inserted table
				if(message["Code"]=="9")
				{
					st=m_db->Prepare("INSERT INTO tblLocalIdentityInserts(LocalIdentityID,Day,InsertIndex) VALUES(?, ?, ?)");
					st.Bind(0, localidentityid);
					st.Bind(1, idparts[4]);
					st.Bind(2, insertindex);
					st.Step();
				}
			}
			else
			{
				m_log->trace("IdentityInserter::HandleMessage PutFailed for editioned SSK error code "+message["Code"]+ " id "+message["Identifier"]);
			}
			
			return true;
		}

	}

	return false;

}

void IdentityInserter::Initialize()
{
	m_lastchecked=Poco::Timestamp();
	m_lastreceivedmessage=Poco::Timestamp();
}

void IdentityInserter::Process()
{
	Poco::DateTime now;

	if(m_lastchecked<(now-Poco::Timespan(0,0,1,0,0)))
	{
		CheckForNeededInsert();
		m_lastchecked=now;
	}

	if(m_lastreceivedmessage<(now-Poco::Timespan(0,0,10,0,0)))
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID FROM tblLocalIdentity WHERE InsertingIdentity='true';");
		st.Step();
		if(st.RowReturned())
		{
			m_log->debug("IdentityInserter::Process 10 minutes have passed without an insert response from the node.  Restarting inserts.");
			m_db->Execute("UPDATE tblLocalIdentity SET InsertingIdentity='false';");
		}
		m_lastreceivedmessage=now;
	}

}

void IdentityInserter::RegisterWithThread(FreenetMasterThread *thread)
{
	thread->RegisterFCPConnected(this);
	thread->RegisterFCPMessageHandler(this);
	thread->RegisterPeriodicProcessor(this);
}

void IdentityInserter::StartInsert(const long localidentityid, const int dayoffset)
{
	std::string idstring;

	StringFunctions::Convert(localidentityid,idstring);

	SQLite3DB::Statement st=m_db->Prepare("SELECT Name,PrivateKey,SingleUse,PublishTrustList,PublishBoardList,PublishFreesite,FreesiteEdition,Signature,FMSAvatar FROM tblLocalIdentity WHERE LocalIdentityID=?");
	st.Bind(0, localidentityid);
	st.Step();

	if(st.RowReturned())
	{
		IdentityXML idxml;
		FCPv2::Message mess;
		Poco::DateTime date;
		std::string messagebase;
		std::string data;
		std::string datasizestr;
		std::string privatekey;
		int index=0;
		std::string indexstr;
		std::string singleuse="false";
		std::string publishtrustlist="false";
		std::string publishboardlist="false";
		std::string publishfreesite="false";
		int edition=-1;
		std::string name="";
		std::string signature="";
		std::string avatar="";

		date=Poco::Timestamp();
		date+=Poco::Timespan(dayoffset,0,0,0,0);

		// FIXME convert to nested SELECT
		SQLite3DB::Statement st2=m_db->Prepare("SELECT MAX(InsertIndex)+1 FROM tblLocalIdentityInserts WHERE LocalIdentityID=? AND Day=?;");
		st2.Bind(0, localidentityid);
		st2.Bind(1, Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
		st2.Step();
		if(st2.RowReturned())
		{
			st2.ResultInt(0, index); // NULL converted to 0
			st2.Reset();
		}
		StringFunctions::Convert(index,indexstr);

		Option option(m_db);
		option.Get("MessageBase",messagebase);

		st.ResultText(0, name);
		if(!name.empty())
		{
			idxml.SetName(name);
		}

		st.ResultText(7, signature);
		if(!signature.empty())
		{
			idxml.SetSignature(signature);
		}

		st.ResultText(8,avatar);
		if(!avatar.empty())
		{
			idxml.SetAvatar(avatar);
		}

		st.ResultText(1, privatekey);

		st.ResultText(2, singleuse);
		singleuse=="true" ? idxml.SetSingleUse(true) : idxml.SetSingleUse(false);

		st.ResultText(3, publishtrustlist);
		publishtrustlist=="true" ? idxml.SetPublishTrustList(true) : idxml.SetPublishTrustList(false);

		st.ResultText(4, publishboardlist);
		publishboardlist=="true" ? idxml.SetPublishBoardList(true) : idxml.SetPublishBoardList(false);

		st.ResultText(5, publishfreesite);
		if(publishfreesite=="true" && !st.ResultNull(6))
		{
			st.ResultInt(6, edition);
			if (edition >= 0)
			{
				idxml.SetFreesiteEdition(edition);
			}
		}

		data=idxml.GetXML();
		StringFunctions::Convert(data.size(),datasizestr);

		mess.SetName("ClientPut");
		mess["URI"]=privatekey+messagebase+"|"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"|Identity|"+indexstr+".xml";
		mess["Identifier"]="IdentityInserter|"+idstring+"|"+indexstr+"|"+mess["URI"];
		mess["UploadFrom"]="direct";
		mess["DataLength"]=datasizestr;
		mess["Metadata.ContentType"]="";
		mess["ExtraInsertsSingleBlock"]="2";
		m_fcp->Send(mess);
		m_fcp->Send(std::vector<char>(data.begin(),data.end()));

		/*
		// test insert as editioned SSK
		mess.Clear();
		mess.SetName("ClientPut");
		mess["URI"]=privatekey+messagebase+"|"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"|Identity-"+indexstr;
		mess["Identifier"]="IdentityInserter|"+mess["URI"];
		mess["UploadFrom"]="direct";
		mess["DataLength"]=datasizestr;
		m_fcp->Send(mess);
		m_fcp->Send(std::vector<char>(data.begin(),data.end()));
		*/

		SQLite3DB::Statement upd=m_db->Prepare("UPDATE tblLocalIdentity SET InsertingIdentity='true' WHERE LocalIdentityID=?;");
		upd.Bind(0, localidentityid);
		upd.Step();

	}
}
