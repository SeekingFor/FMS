#include "../../include/freenet/trustlistinserter.h"
#include "../../include/option.h"
#include "../../include/freenet/trustlistxml.h"
#include "../../include/stringfunctions.h"

#include <Poco/Timespan.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>

#ifdef XMEM
	#include <xmem.h>
#endif

TrustListInserter::TrustListInserter(SQLite3DB::DB *db):IDatabase(db)
{
	Initialize();
}

TrustListInserter::TrustListInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp):IDatabase(db),IFCPConnected(fcp)
{
	Initialize();
}

void TrustListInserter::CheckForNeededInsert()
{
	Poco::DateTime date;
	int currentday=date.day();
	date-=Poco::Timespan(0,6,0,0,0);
	// insert trust lists every 6 hours - if 6 hours ago was different day then set to midnight of current day to insert list today ASAP
	if(currentday!=date.day())
	{
		date.assign(date.year(),date.month(),currentday,0,0,0);
	}

	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID, PrivateKey FROM tblLocalIdentity WHERE tblLocalIdentity.Active='true' AND PrivateKey IS NOT NULL AND PrivateKey <> '' AND PublishTrustList='true' AND InsertingTrustList='false' AND (LastInsertedTrustList<=? OR LastInsertedTrustList IS NULL);");
	st.Bind(0,Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S"));
	st.Step();

	if(st.RowReturned())
	{
		int lid=0;
		std::string pkey("");
		st.ResultInt(0,lid);
		st.ResultText(1,pkey);
		StartInsert(lid,pkey);
	}

}

void TrustListInserter::FCPConnected()
{
	m_db->Execute("UPDATE tblLocalIdentity SET InsertingTrustList='false';");
}

void TrustListInserter::FCPDisconnected()
{

}

const bool TrustListInserter::HandleMessage(FCPv2::Message &message)
{

	if(message["Identifier"].find("TrustListInserter")==0)
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
			// non USK
			if(idparts[0]=="TrustListInserter")
			{
				m_db->Execute("UPDATE tblLocalIdentity SET InsertingTrustList='false', LastInsertedTrustList='"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S")+"' WHERE LocalIdentityID="+idparts[1]+";");
				m_db->Execute("INSERT INTO tblTrustListInserts(LocalIdentityID,Day,InsertIndex) VALUES("+idparts[1]+",'"+idparts[4]+"',"+idparts[2]+");");
				m_log->debug("TrustListInserter::HandleMessage inserted TrustList xml");
			}
			return true;
		}

		if(message.GetName()=="PutFailed" && idparts[0]=="TrustListInserter")
		{
			// non USK
			if(idparts[0]=="TrustListInserter")
			{
				m_db->Execute("UPDATE tblLocalIdentity SET InsertingTrustList='false' WHERE LocalIdentityID="+idparts[1]+";");
				m_log->debug("TrustListInserter::HandleMessage failure inserting TrustList xml.  Code="+message["Code"]+" Description="+message["CodeDescription"]);
			
				// if code 9 (collision), then insert index into inserted table
				if(message["Code"]=="9")
				{
					m_db->Execute("INSERT INTO tblTrustListInserts(LocalIdentityID,Day,InsertIndex) VALUES("+idparts[1]+",'"+idparts[4]+"',"+idparts[2]+");");
				}
			}
			return true;
		}

	}

	return false;
}

void TrustListInserter::Initialize()
{
	Option option(m_db);
	option.Get("MessageBase",m_messagebase);
	m_lastchecked=Poco::Timestamp();
}

void TrustListInserter::Process()
{
	Poco::DateTime now;

	// check every minute
	if(m_lastchecked<=(now-Poco::Timespan(0,0,1,0,0)))
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
	FCPv2::Message message;
	TrustListXML xml;
	std::string data;
	std::string datasizestr;
	std::string publickey;
	int messagetrust;
	int trustlisttrust;
	Poco::DateTime now,date,dateminus30,tempdate;
	int index;
	std::string indexstr;
	std::string localidentityidstr;
	std::string messagetrustcomment="";
	std::string trustlisttrustcomment="";
	int identityid=-1;
	int count=0;
	bool add=false;
	std::string dateadded="";

	dateminus30-=Poco::Timespan(30,0,0,0,0);

	// insert all identities not in trust list already
	m_db->Execute("INSERT INTO tblIdentityTrust(LocalIdentityID,IdentityID) SELECT LocalIdentityID,IdentityID FROM tblLocalIdentity,tblIdentity WHERE LocalIdentityID || '_' || IdentityID NOT IN (SELECT LocalIdentityID || '_' || IdentityID FROM tblIdentityTrust);");

	// select statement for last message date for an identity
	SQLite3DB::Statement countst=m_db->Prepare("SELECT COUNT(*) FROM tblMessage WHERE IdentityID=? AND MessageDate>=?;");

	// build the xml file - we only want to add identities that we recently saw, otherwise we could be inserting a ton of identities
	date-=Poco::Timespan(15,0,0,0,0);	// identities seen in last 15 days - the maintenance page lets us delete identities not seen in 20 days, so this gives us a window where the identity won't be deleted and then found in a trust list and readded immediately
	
	m_db->Execute("BEGIN;");
	
	//SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey, LocalMessageTrust, LocalTrustListTrust, MessageTrustComment, TrustListTrustComment FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey<>'' AND LastSeen>=?;");
	// we want to order by public key so we can't do identity correllation based on the sequence of identities in the list.
	SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey, tblIdentityTrust.LocalMessageTrust, tblIdentityTrust.LocalTrustListTrust, tblIdentityTrust.MessageTrustComment, tblIdentityTrust.TrustListTrustComment, tblIdentity.IdentityID, tblIdentity.DateAdded FROM tblIdentity INNER JOIN tblIdentityTrust ON tblIdentity.IdentityID=tblIdentityTrust.IdentityID WHERE PublicKey IS NOT NULL AND PublicKey<>'' AND LastSeen>=? AND tblIdentityTrust.LocalIdentityID=? ORDER BY PublicKey;");
	st.Bind(0,Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
	st.Bind(1,localidentityid);
	st.Step();
	while(st.RowReturned())
	{
		st.ResultText(0,publickey);
		if(st.ResultNull(1)==false)
		{
			st.ResultInt(1,messagetrust);
		}
		else
		{
			messagetrust=-1;
		}
		if(st.ResultNull(2)==false)
		{
			st.ResultInt(2,trustlisttrust);
		}
		else
		{
			trustlisttrust=-1;
		}
		st.ResultText(3,messagetrustcomment);
		st.ResultText(4,trustlisttrustcomment);
		identityid=-1;
		st.ResultInt(5,identityid);
		dateadded="";
		st.ResultText(6,dateadded);

		add=false;
		
		// Add identities to list regardless of last message sent date
		// If we saw them recently, then they are added, no exceptions
		add=true;
		/*
		// add the identity to the trust list if they have posted a message in the last 30 days
		countst.Bind(0,identityid);
		countst.Bind(1,Poco::DateTimeFormatter::format(dateminus30,"%Y-%m-%d"));
		countst.Step();
		if(countst.RowReturned())
		{
			count=0;
			countst.ResultInt(0,count);
			if(count>0)
			{
				add=true;
			}
		}
		countst.Reset();

		// no messages in last 30 days - add the identity if we learned about them less than 5 days ago
		if(add==false && dateadded!="")
		{
			int tzdiff=0;
			if(Poco::DateTimeParser::tryParse(dateadded,tempdate,tzdiff)==false)
			{
				tempdate=Poco::Timestamp();
				m_log->fatal("TrustListInserter::StartInsert could not parse date "+dateadded);
			}
			if(tempdate>=(now-Poco::Timespan(5,0,0,0,0)))
			{
				add=true;
			}
		}
		*/

		if(add==true)
		{
			xml.AddTrust(publickey,messagetrust,trustlisttrust,messagetrustcomment,trustlisttrustcomment);
		}

		st.Step();
	}

	m_db->Execute("COMMIT;");

	// get next insert index
	st=m_db->Prepare("SELECT MAX(InsertIndex) FROM tblTrustListInserts WHERE LocalIdentityID=? AND Day=?;");
	st.Bind(0,localidentityid);
	st.Bind(1,Poco::DateTimeFormatter::format(now,"%Y-%m-%d"));
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
	message["URI"]=privatekey+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|TrustList|"+indexstr+".xml";
	message["Identifier"]="TrustListInserter|"+localidentityidstr+"|"+indexstr+"|"+message["URI"];
	message["UploadFrom"]="direct";
	message["DataLength"]=datasizestr;
	message["Metadata.ContentType"]="";
	m_fcp->Send(message);
	m_fcp->Send(std::vector<char>(data.begin(),data.end()));

	// insert to USK - not used, but don't remove code yet
	/*
	message.Clear();
	message.SetName("ClientPutComplexDir");
	message["URI"]="USK"+privatekey.substr(3)+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y.%m.%d")+"|TrustList/0/";
	message["Identifier"]="TrustListInserterUSK|"+message["URI"];
	message["DefaultName"]="TrustList.xml";
	message["Files.0.Name"]="TrustList.xml";
	message["Files.0.UplaodFrom"]="direct";
	message["Files.0.DataLength"]=datasizestr;
	m_fcp->Send(message);
	m_fcp->Send(std::vector<char>(data.begin(),data.end()));
	*/

	m_db->Execute("UPDATE tblLocalIdentity SET InsertingTrustList='true' WHERE LocalIdentityID="+localidentityidstr+";");

}
