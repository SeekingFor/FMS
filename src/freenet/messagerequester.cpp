#include "../../include/freenet/messagerequester.h"
#include "../../include/freenet/messagexml.h"

#ifdef XMEM
	#include <xmem.h>
#endif

MessageRequester::MessageRequester()
{
	Initialize();
}

MessageRequester::MessageRequester(FCPv2 *fcp):IIndexRequester<std::string>(fcp)
{
	Initialize();
}

const long MessageRequester::GetBoardID(const std::string &boardname)
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardID FROM tblBoard WHERE BoardName=?;");
	st.Bind(0,boardname);
	st.Step();

	if(st.RowReturned())
	{
		int boardid;
		st.ResultInt(0,boardid);
		return boardid;
	}
	else
	{
		DateTime now;
		now.SetToGMTime();
		st=m_db->Prepare("INSERT INTO tblBoard(BoardName,DateAdded) VALUES(?,?);");
		st.Bind(0,boardname);
		st.Bind(1,now.Format("%Y-%m-%d %H:%M:%S"));
		st.Step(true);
		return st.GetLastInsertRowID();
	}	
}

const std::string MessageRequester::GetIdentityName(const long identityid)
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT Name,PublicKey FROM tblIdentity WHERE IdentityID=?;");
	st.Bind(0,identityid);
	st.Step();
	if(st.RowReturned())
	{
		std::vector<std::string> keyparts;
		std::string key;
		std::string name;
		st.ResultText(0,name);
		st.ResultText(1,key);
		
		StringFunctions::SplitMultiple(key,"@,",keyparts);
		
		if(keyparts.size()>1)
		{
			return name+"@"+keyparts[1];
		}
		else
		{
			return name+"@invalidpublickey";
		}
	}
	else
	{
		return "";
	}
}

const bool MessageRequester::HandleAllData(FCPMessage &message)
{
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long datalength;
	std::vector<char> data;
	MessageXML xml;
	long identityid;
	long index;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(message["DataLength"],datalength);
	StringFunctions::Convert(idparts[2],identityid);
	StringFunctions::Convert(idparts[4],index);

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

	// mark this index as received
	st=m_db->Prepare("UPDATE tblMessageRequests SET Found='true' WHERE IdentityID=? AND Day=? AND RequestIndex=?;");
	st.Bind(0,identityid);
	st.Bind(1,idparts[3]);
	st.Bind(2,index);
	st.Step();
	st.Finalize();

	// parse file into xml and update the database
	if(xml.ParseXML(std::string(data.begin(),data.end()))==true)
	{
		std::vector<std::string> boards=xml.GetBoards();

		if(boards.size()<=0)
		{
			m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"MessageRequester::HandleAllData Message XML did not contain any boards! "+message["Identifier"]);
			return true;
		}
		if(xml.GetReplyBoard()=="")
		{
			m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"MessageRequester::HandleAllData Message XML did not contain a reply board! "+message["Identifier"]);
			return true;
		}

		st=m_db->Prepare("INSERT INTO tblMessage(IdentityID,FromName,MessageDate,MessageTime,Subject,MessageUUID,ReplyBoardID,Body) VALUES(?,?,?,?,?,?,?,?);");
		st.Bind(0,identityid);
		st.Bind(1,GetIdentityName(identityid));
		st.Bind(2,xml.GetDate());
		st.Bind(3,xml.GetTime());
		st.Bind(4,xml.GetSubject());
		st.Bind(5,xml.GetMessageID());
		st.Bind(6,GetBoardID(xml.GetReplyBoard()));
		st.Bind(7,xml.GetBody());
		st.Step(true);
		int messageid=st.GetLastInsertRowID();

		st=m_db->Prepare("INSERT INTO tblMessageBoard(MessageID,BoardID) VALUES(?,?);");
		for(std::vector<std::string>::iterator i=boards.begin(); i!=boards.end(); i++)
		{
			st.Bind(0,messageid);
			st.Bind(1,GetBoardID((*i)));
			st.Step();
			st.Reset();
		}
		st.Finalize();

		st=m_db->Prepare("INSERT INTO tblMessageReplyTo(MessageID,ReplyToMessageUUID,ReplyOrder) VALUES(?,?,?);");
		std::map<long,std::string> replyto=xml.GetInReplyTo();
		for(std::map<long,std::string>::iterator j=replyto.begin(); j!=replyto.end(); j++)
		{
			st.Bind(0,messageid);
			st.Bind(1,(*j).second);
			st.Bind(2,(*j).first);
			st.Step();
			st.Reset();
		}
		st.Finalize();

		m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"MessageRequester::HandleAllData parsed Message XML file : "+message["Identifier"]);

	}
	else
	{
		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"MessageRequester::HandleAllData error parsing Message XML file : "+message["Identifier"]);
	}

	RemoveFromRequestList(idparts[1]);

	return true;
}

const bool MessageRequester::HandleGetFailed(FCPMessage &message)
{
	DateTime now;
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	std::string requestid;
	long index;
	long identityid;

	now.SetToGMTime();
	StringFunctions::Split(message["Identifier"],"|",idparts);
	requestid=idparts[1];
	StringFunctions::Convert(idparts[2],identityid);
	StringFunctions::Convert(idparts[4],index);

	// if this is a fatal error - insert index into database so we won't try to download this index again
	if(message["Fatal"]=="true")
	{
		st=m_db->Prepare("UPDATE tblMessageRequests SET Found='true' WHERE IdentityID=? AND Day=? AND RequestIndex=?;");
		st.Bind(0,identityid);
		st.Bind(1,idparts[3]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"MessageRequester::HandleGetFailed fatal error requesting "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(requestid);

	return true;
}

void MessageRequester::Initialize()
{
	m_fcpuniquename="MessageRequester";
	std::string tempval;
	Option::Instance()->Get("MaxMessageRequests",tempval);
	StringFunctions::Convert(tempval,m_maxrequests);
	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"Option MaxMessageRequests is currently set at "+tempval+".  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->WriteLog(LogFile::LOGLEVEL_WARNING,"Option MaxMessageRequests is currently set at "+tempval+".  This value might be incorrectly configured.");
	}
	Option::Instance()->Get("MessageDownloadMaxDaysBackward",tempval);
	StringFunctions::Convert(tempval,m_maxdaysbackward);
	if(m_maxdaysbackward<0)
	{
		m_maxdaysbackward=0;
		m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"Option MessageDownloadMaxDaysBackward is currently set at "+tempval+".  It must be 0 or greater.");
	}
	if(m_maxdaysbackward>30)
	{
		m_log->WriteLog(LogFile::LOGLEVEL_WARNING,"Option MessageDownloadMaxDaysBackward is currently set at "+tempval+".  This value might be incorrectly configured.");
	}
}

void MessageRequester::PopulateIDList()
{
	DateTime date;
	std::string val1;
	std::string val2;
	std::string val3;
	std::string sql;

	date.SetToGMTime();
	date.Add(0,0,0,-m_maxdaysbackward);

	sql="SELECT tblIdentity.IdentityID,Day,RequestIndex ";
	sql+="FROM tblMessageRequests INNER JOIN tblIdentity ON tblMessageRequests.IdentityID=tblIdentity.IdentityID ";
	sql+="WHERE tblIdentity.LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust') AND FromMessageList='true' AND Found='false' AND Day>='"+date.Format("%Y-%m-%d")+"' ";
	sql+="AND (tblIdentity.PeerMessageTrust IS NULL OR tblIdentity.PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')) ";
	sql+=";";

	SQLite3DB::Statement st=m_db->Prepare(sql);
	st.Step();

	m_ids.clear();

	while(st.RowReturned())
	{
		st.ResultText(0,val1);
		st.ResultText(1,val2);
		st.ResultText(2,val3);
		if(m_ids.find(val1+"*"+val2+"*"+val3)==m_ids.end())
		{
			m_ids[val1+"*"+val2+"*"+val3]=false;
		}
		st.Step();
	}

}

void MessageRequester::StartRequest(const std::string &requestid)
{
	FCPMessage message;
	std::vector<std::string> parts;
	std::string tempval;
	long identityid;
	std::string date;
	std::string indexstr;
	std::string publickey;

	StringFunctions::Split(requestid,"*",parts);
	StringFunctions::Convert(parts[0],identityid);
	StringFunctions::Convert(parts[1],date);
	indexstr=parts[2];

	SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblIdentity WHERE IdentityID=?;");
	st.Bind(0,identityid);
	st.Step();

	if(st.RowReturned())
	{
		st.ResultText(0,publickey);

		message.SetName("ClientGet");
		message["URI"]=publickey+m_messagebase+"|"+date+"|Message|"+indexstr+".xml";
		message["Identifier"]=m_fcpuniquename+"|"+requestid+"|"+parts[0]+"|"+parts[1]+"|"+parts[2]+"|"+message["URI"];
		message["ReturnType"]="direct";
		message["MaxSize"]="1000000";		// 1 MB

		m_fcp->SendMessage(message);

		m_requesting.push_back(requestid);
	}
	
	m_ids[requestid]=true;

}
