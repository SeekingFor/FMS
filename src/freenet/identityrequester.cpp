#include "../../include/freenet/identityrequester.h"
#include "../../include/freenet/identityxml.h"
#include "../../include/stringfunctions.h"
#include "../../include/option.h"

#include <Poco/DateTime.h>
#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>
#include <Poco/DateTimeFormatter.h>

#ifdef XMEM
	#include <xmem.h>
#endif

IdentityRequester::IdentityRequester(SQLite3DB::DB *db):IIndexRequester<long>(db)
{
	Initialize();
}

IdentityRequester::IdentityRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexRequester<long>(db,fcp)
{
	Initialize();
}

const bool IdentityRequester::HandleAllData(FCPv2::Message &message)
{
	Poco::DateTime now;
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long datalength;
	std::vector<char> data;
	IdentityXML xml;
	long identityid;
	long index;
	std::string name="";

	now=Poco::Timestamp();
	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(message["DataLength"],datalength);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[2],index);

	// wait for all data to be received from connection
	m_fcp->WaitForBytes(1000,datalength);

	// if we got disconnected- return immediately
	if(m_fcp->IsConnected()==false)
	{
		return false;
	}

	// receive the file
	m_fcp->Receive(data,datalength);

	// parse file into xml and update the database
	if(data.size()>0 && xml.ParseXML(std::string(data.begin(),data.end()))==true)
	{

		st=m_db->Prepare("UPDATE tblIdentity SET Name=?, SingleUse=?, LastSeen=?, PublishTrustList=?, PublishBoardList=?, FreesiteEdition=? WHERE IdentityID=?");
		name=xml.GetName();
		if(name.size()>40)
		{
			name.erase(40);
		}
		st.Bind(0,name);
		if(xml.GetSingleUse()==true)
		{
			st.Bind(1,"true");
		}
		else
		{
			st.Bind(1,"false");
		}
		st.Bind(2,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
		if(xml.GetPublishTrustList()==true)
		{
			st.Bind(3,"true");
		}
		else
		{
			st.Bind(3,"false");
		}
		if(xml.GetPublishBoardList()==true)
		{
			st.Bind(4,"true");
		}
		else
		{
			st.Bind(4,"false");
		}
		if(xml.GetFreesiteEdition()>=0)
		{
			st.Bind(5,xml.GetFreesiteEdition());
		}
		else
		{
			st.Bind(5);
		}
		st.Bind(6,identityid);
		st.Step();
		st.Finalize();

		st=m_db->Prepare("INSERT INTO tblIdentityRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'true');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->debug(m_fcpuniquename+"::HandleAllData parsed Identity XML file : "+message["Identifier"]);
	}
	else
	{
		// bad data - mark index
		st=m_db->Prepare("INSERT INTO tblIdentityRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->error(m_fcpuniquename+"::HandleAllData error parsing Identity XML file : "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;

}

const bool IdentityRequester::HandleGetFailed(FCPv2::Message &message)
{
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long identityid;
	long index;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[2],index);	

	// if this is a fatal error - insert index into database so we won't try to download this index again
	if(message["Fatal"]=="true")
	{
		st=m_db->Prepare("INSERT INTO tblIdentityRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->error(m_fcpuniquename+"::HandleGetFailed fatal error requesting "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;

}

void IdentityRequester::Initialize()
{
	m_fcpuniquename="KnownIdentityRequester";
	Option option(m_db);
	option.GetInt("MaxIdentityRequests",m_maxrequests);

	// known identities get 4/5 + any remaining if not evenly divisible - unknown identities get 1/5 of the max requests option
	m_maxrequests=((m_maxrequests*4)/5)+(m_maxrequests%5);

	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->error("Option MaxIdentityRequests is currently set at less than 1.  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->warning("Option MaxIdentityRequests is currently set at more than 100.  This value might be incorrectly configured.");
	}
}

void IdentityRequester::PopulateIDList()
{
	Poco::DateTime date;
	int id;

	date.assign(date.year(),date.month(),date.day(),0,0,0);

	// select identities we want to query (haven't seen yet today) - sort by their trust level (descending) with secondary sort on how long ago we saw them (ascending)
	SQLite3DB::Statement st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen IS NOT NULL AND LastSeen<'"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S")+"' ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
	st.Step();

	m_ids.clear();

	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[id]=false;
		st.Step();
	}
}

void IdentityRequester::StartRequest(const long &identityid)
{
	Poco::DateTime now;
	FCPv2::Message message;
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

		now=Poco::Timestamp();

		SQLite3DB::Statement st2=m_db->Prepare("SELECT MAX(RequestIndex) FROM tblIdentityRequests WHERE Day=? AND IdentityID=?;");
		st2.Bind(0,Poco::DateTimeFormatter::format(now,"%Y-%m-%d"));
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
		message["URI"]=publickey+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|Identity|"+indexstr+".xml";
		message["Identifier"]=m_fcpuniquename+"|"+identityidstr+"|"+indexstr+"|"+message["URI"];
		message["ReturnType"]="direct";
		message["MaxSize"]="10000";			// 10 KB

		m_fcp->Send(message);

		m_requesting.push_back(identityid);
	}
	st.Finalize();

	m_ids[identityid]=true;

}
