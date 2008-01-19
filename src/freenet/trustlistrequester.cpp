#include "../../include/freenet/trustlistrequester.h"
#include "../../include/option.h"
#include "../../include/stringfunctions.h"
#include "../../include/freenet/trustlistxml.h"

#ifdef XMEM
	#include <xmem.h>
#endif

TrustListRequester::TrustListRequester()
{
	Initialize();
}

TrustListRequester::TrustListRequester(FCPv2 *fcp):IFCPConnected(fcp)
{
	Initialize();
}

void TrustListRequester::FCPConnected()
{
	m_requesting.clear();
	PopulateIDList();
}

void TrustListRequester::FCPDisconnected()
{
	
}

const bool TrustListRequester::HandleAllData(FCPMessage &message)
{
	DateTime now;
	SQLite3DB::Statement st;
	SQLite3DB::Statement trustst;
	std::vector<std::string> idparts;
	long datalength;
	std::vector<char> data;
	TrustListXML xml;
	long identityid;
	long index;

	now.SetToGMTime();
	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(message["DataLength"],datalength);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[2],index);

	// wait for all data to be received from connection
	while(m_fcp->Connected() && m_fcp->ReceiveBufferSize()<datalength)
	{
		m_fcp->Update(1);
	}

	// if we got disconnected- return immediately
	if(m_fcp->Connected()==false)
	{
		return false;
	}

	// receive the file
	data.resize(datalength);
	m_fcp->ReceiveRaw(&data[0],datalength);

	// parse file into xml and update the database
	if(xml.ParseXML(std::string(data.begin(),data.end()))==true)
	{

		// drop all existing peer trust from this identity - we will rebuild it when we go through each trust in the xml file
		st=m_db->Prepare("DELETE FROM tblPeerTrust WHERE IdentityID=?;");
		st.Bind(0,identityid);
		st.Step();
		st.Finalize();

		st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey=?;");
		trustst=m_db->Prepare("INSERT INTO tblPeerTrust(IdentityID,TargetIdentityID,MessageTrust,TrustListTrust) VALUES(?,?,?,?);");
		// loop through all trust entries in xml and add to database if we don't already know them
		for(long i=0; i<xml.TrustCount(); i++)
		{
			int id;
			std::string identity;
			identity=xml.GetIdentity(i);

			st.Bind(0,identity);
			st.Step();
			if(st.RowReturned()==false)
			{
				m_db->ExecuteInsert("INSERT INTO tblIdentity(PublicKey,DateAdded) VALUES('"+identity+"','"+now.Format("%Y-%m-%d %H:%M:%S")+"');",(long &)id);
			}
			else
			{
				st.ResultInt(0,id);
			}
			st.Reset();

			//insert trust for this identity
			trustst.Bind(0,identityid);
			trustst.Bind(1,id);
			trustst.Bind(2,xml.GetMessageTrust(i));
			trustst.Bind(3,xml.GetTrustListTrust(i));
			trustst.Step();
			trustst.Reset();

		}
		trustst.Finalize();
		st.Finalize();

		st=m_db->Prepare("INSERT INTO tblTrustListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'true');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"TrustListRequester::HandleAllData parsed TrustList XML file : "+message["Identifier"]);
	}
	else
	{
		// bad data - mark index
		st=m_db->Prepare("INSERT INTO tblTrustListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"TrustListRequester::HandleAllData error parsing TrustList XML file : "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;

}

const bool TrustListRequester::HandleGetFailed(FCPMessage &message)
{
	DateTime now;
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long identityid;
	long index;

	now.SetToGMTime();
	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[2],index);	

	// if this is a fatal error - insert index into database so we won't try to download this index again
	if(message["Fatal"]=="true")
	{
		st=m_db->Prepare("INSERT INTO tblTrustListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"TrustListRequester::HandleGetFailed fatal error requesting "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;

}

const bool TrustListRequester::HandleMessage(FCPMessage &message)
{

	if(message["Identifier"].find("TrustListRequester")==0)
	{
		if(message.GetName()=="DataFound")
		{
			return true;
		}

		if(message.GetName()=="AllData")
		{
			return HandleAllData(message);
		}

		if(message.GetName()=="GetFailed")
		{
			return HandleGetFailed(message);
		}

		if(message.GetName()=="IdentifierCollision")
		{
			// remove one of the ids from the requesting list
			long identityid=0;
			std::vector<std::string> idparts;
			StringFunctions::Split(message["Identifier"],"|",idparts);
			StringFunctions::Convert(idparts[1],identityid);
			RemoveFromRequestList(identityid);
			return true;
		}
	}

	return false;
}

void TrustListRequester::Initialize()
{
	std::string tempval="";
	Option::instance()->Get("MaxIdentityRequests",tempval);
	StringFunctions::Convert(tempval,m_maxrequests);
	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"Option MaxTrustListRequests is currently set at "+tempval+".  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->WriteLog(LogFile::LOGLEVEL_WARNING,"Option MaxTrustListRequests is currently set at "+tempval+".  This value might be incorrectly configured.");
	}
	Option::instance()->Get("MessageBase",m_messagebase);
	m_tempdate.SetToGMTime();
}

void TrustListRequester::PopulateIDList()
{
	DateTime date;
	int id;

	date.SetToGMTime();

	// select identities we want to query (we've seen them today and they are publishing trust list) - sort by their trust level (descending) with secondary sort on how long ago we saw them (ascending)
	SQLite3DB::Statement st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen>='"+date.Format("%Y-%m-%d")+"' AND PublishTrustList='true' AND LocalTrustListTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalTrustListTrust') ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
	st.Step();

	m_ids.clear();

	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[id]=false;
		st.Step();
	}
}

void TrustListRequester::Process()
{
	// max is the smaller of the config value or the total number of identities we will request from
	long max=m_maxrequests>m_ids.size() ? m_ids.size() : m_maxrequests;

	// try to keep up to max requests going
	if(m_requesting.size()<max)
	{
		std::map<long,bool>::iterator i=m_ids.begin();
		while(i!=m_ids.end() && (*i).second==true)
		{
			i++;
		}

		if(i!=m_ids.end())
		{
			StartRequest((*i).first);
		}
		else
		{
			// we requested from all ids in the list, repopulate the list
			PopulateIDList();
		}
	}
	// special case - if there were 0 identities on the list when we started then we will never get a chance to repopulate the list
	// this will recheck for ids every minute
	DateTime now;
	now.SetToGMTime();
	if(m_tempdate<(now-(1.0/1440.0)))
	{
		PopulateIDList();
		m_tempdate=now;
	}

}

void TrustListRequester::RegisterWithThread(FreenetMasterThread *thread)
{
	thread->RegisterFCPConnected(this);
	thread->RegisterFCPMessageHandler(this);
	thread->RegisterPeriodicProcessor(this);
}

void TrustListRequester::RemoveFromRequestList(const long identityid)
{
	std::vector<long>::iterator i=m_requesting.begin();
	while(i!=m_requesting.end() && (*i)!=identityid)
	{
		i++;
	}
	if(i!=m_requesting.end())
	{
		m_requesting.erase(i);
	}
}

void TrustListRequester::StartRequest(const long identityid)
{
	DateTime now;
	FCPMessage message;
	std::string publickey;
	int index;
	std::string indexstr;
	std::string identityidstr;

	SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblIdentity WHERE IdentityID=?;");
	st.Bind(0,identityid);
	st.Step();

	if(st.RowReturned())
	{
		st.ResultText(0,publickey);

		now.SetToGMTime();

		SQLite3DB::Statement st2=m_db->Prepare("SELECT MAX(RequestIndex) FROM tblTrustListRequests WHERE Day=? AND IdentityID=?;");
		st2.Bind(0,now.Format("%Y-%m-%d"));
		st2.Bind(1,identityid);
		st2.Step();

		index=0;
		if(st2.RowReturned())
		{
			if(st2.ResultNull(0)==false)
			{
				st2.ResultInt(0,index);
				index++;
			}
		}
		st2.Finalize();

		StringFunctions::Convert(index,indexstr);
		StringFunctions::Convert(identityid,identityidstr);

		message.SetName("ClientGet");
		message["URI"]=publickey+m_messagebase+"|"+now.Format("%Y-%m-%d")+"|TrustList|"+indexstr+".xml";
		message["Identifier"]="TrustListRequester|"+identityidstr+"|"+indexstr+"|"+message["URI"];
		message["ReturnType"]="direct";
		message["MaxSize"]="1000000";			// 1 MB

		m_fcp->SendMessage(message);

		m_requesting.push_back(identityid);
	}
	st.Finalize();

	m_ids[identityid]=true;

}
