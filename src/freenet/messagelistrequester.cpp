#include "../../include/freenet/messagelistrequester.h"
#include "../../include/freenet/messagelistxml.h"

#ifdef XMEM
	#include <xmem.h>
#endif

MessageListRequester::MessageListRequester()
{
	Initialize();
}

MessageListRequester::MessageListRequester(FCPv2 *fcp):IIndexRequester<long>(fcp)
{
	Initialize();
}

const bool MessageListRequester::HandleAllData(FCPMessage &message)
{	
	DateTime now;
	SQLite3DB::Statement st;
	SQLite3DB::Statement trustst;
	std::vector<std::string> idparts;
	long datalength;
	std::vector<char> data;
	MessageListXML xml;
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

		SQLite3DB::Statement st=m_db->Prepare("SELECT IdentityID FROM tblMessageRequests WHERE IdentityID=? AND Day=? AND RequestIndex=?;");
		SQLite3DB::Statement mst=m_db->Prepare("INSERT INTO tblMessageRequests(IdentityID,Day,RequestIndex,FromMessageList) VALUES(?,?,?,'true');");
		for(long i=0; i<xml.MessageCount(); i++)
		{
			st.Bind(0,identityid);
			st.Bind(1,xml.GetDate(i));
			st.Bind(2,xml.GetIndex(i));
			st.Step();
			if(st.RowReturned()==false)
			{
				mst.Bind(0,identityid);
				mst.Bind(1,xml.GetDate(i));
				mst.Bind(2,xml.GetIndex(i));
				mst.Step();
				mst.Reset();
			}
			st.Reset();
		}
		mst.Finalize();
		st.Finalize();

		st=m_db->Prepare("INSERT INTO tblMessageListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'true');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"MessageListRequester::HandleAllData parsed MessageList XML file : "+message["Identifier"]);
	}
	else
	{
		// bad data - mark index
		st=m_db->Prepare("INSERT INTO tblMessageListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"MessageListRequester::HandleAllData error parsing MessageList XML file : "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;

}

const bool MessageListRequester::HandleGetFailed(FCPMessage &message)
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
		st=m_db->Prepare("INSERT INTO tblMessageListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"MessageListRequester::HandleGetFailed fatal error requesting "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;
}

void MessageListRequester::Initialize()
{
	m_fcpuniquename="MessageListRequester";
	std::string tempval;
	Option::instance()->Get("MaxMessageListRequests",tempval);
	StringFunctions::Convert(tempval,m_maxrequests);
	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"Option MaxMessageListRequests is currently set at "+tempval+".  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->WriteLog(LogFile::LOGLEVEL_WARNING,"Option MaxMessageListRequests is currently set at "+tempval+".  This value might be incorrectly configured.");
	}
}

void MessageListRequester::PopulateIDList()
{
	DateTime date;
	int id;

	date.SetToGMTime();

	// select identities we want to query (we've seen them today) - sort by their trust level (descending) with secondary sort on how long ago we saw them (ascending)
	SQLite3DB::Statement st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen>='"+date.Format("%Y-%m-%d")+"' AND LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust') ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
	st.Step();

	m_ids.clear();

	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[id]=false;
		st.Step();
	}
}

void MessageListRequester::StartRequest(const long &id)
{
	DateTime now;
	FCPMessage message;
	std::string publickey;
	int index;
	std::string indexstr;
	std::string identityidstr;

	SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblIdentity WHERE IdentityID=?;");
	st.Bind(0,id);
	st.Step();

	if(st.RowReturned())
	{
		st.ResultText(0,publickey);

		now.SetToGMTime();

		SQLite3DB::Statement st2=m_db->Prepare("SELECT MAX(RequestIndex) FROM tblMessageListRequests WHERE Day=? AND IdentityID=?;");
		st2.Bind(0,now.Format("%Y-%m-%d"));
		st2.Bind(1,id);
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
		StringFunctions::Convert(id,identityidstr);

		message.SetName("ClientGet");
		message["URI"]=publickey+m_messagebase+"|"+now.Format("%Y-%m-%d")+"|MessageList|"+indexstr+".xml";
		message["Identifier"]=m_fcpuniquename+"|"+identityidstr+"|"+indexstr+"|"+message["URI"];
		message["ReturnType"]="direct";
		message["MaxSize"]="1000000";			// 1 MB

		m_fcp->SendMessage(message);

		m_requesting.push_back(id);
	}
	st.Finalize();

	m_ids[id]=true;
}
