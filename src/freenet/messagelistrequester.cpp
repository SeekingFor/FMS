#include "../../include/freenet/messagelistrequester.h"
#include "../../include/freenet/messagelistxml.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/Timestamp.h>

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

const bool MessageListRequester::CheckDateNotFuture(const std::string &datestr) const
{
	std::vector<std::string> dateparts;
	int year=0;
	int month=0;
	int day=0;
	Poco::DateTime today;

	StringFunctions::Split(datestr,"-",dateparts);
	if(dateparts.size()==3)
	{
		StringFunctions::Convert(dateparts[0],year);
		StringFunctions::Convert(dateparts[1],month);
		StringFunctions::Convert(dateparts[2],day);
		if(today.year()>year || (today.year()==year && today.month()>month) || (today.year()==year && today.month()==month && today.day()>=day))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

}

void MessageListRequester::GetBoardList(std::map<std::string,bool> &boards)
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardName, SaveReceivedMessages FROM tblBoard;");
	st.Step();
	while(st.RowReturned())
	{
		std::string boardname="";
		std::string tempval="";
		st.ResultText(0,boardname);
		st.ResultText(1,tempval);

		if(tempval=="true")
		{
			boards[boardname]=true;
		}
		else
		{
			boards[boardname]=false;
		}

		st.Step();
	}
}

const bool MessageListRequester::HandleAllData(FCPMessage &message)
{	
	SQLite3DB::Statement st;
	SQLite3DB::Statement trustst;
	std::vector<std::string> idparts;
	long datalength;
	std::vector<char> data;
	MessageListXML xml;
	long identityid;
	long index;
	std::map<std::string,bool> boards;	// list of boards and if we will save messages for that board or not
	bool addmessage=false;
	std::string boardsstr="";
	std::string datestr="";
	std::vector<std::string> dateparts;

	GetBoardList(boards);

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
		SQLite3DB::Statement spk=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey=?;");
		SQLite3DB::Statement mst=m_db->Prepare("INSERT INTO tblMessageRequests(IdentityID,Day,RequestIndex,FromMessageList) VALUES(?,?,?,'true');");
		for(long i=0; i<xml.MessageCount(); i++)
		{

			// go through each board the message was posted to and see if we are saving messages to that board
			// if the board isn't found, see if we are saving messages to new boards
			boardsstr="";
			addmessage=false;
			std::vector<std::string> messageboards=xml.GetBoards(i);
			for(std::vector<std::string>::iterator j=messageboards.begin(); j!=messageboards.end(); j++)
			{
				if(boards.find((*j))!=boards.end())
				{
					if(boards[(*j)]==true)
					{
						addmessage=true;
					}
				}
				else if(m_savetonewboards==true)
				{
					addmessage=true;
				}
				if(j!=messageboards.begin())
				{
					boardsstr+=", ";
				}
				boardsstr+=(*j);
			}

			if(CheckDateNotFuture(xml.GetDate(i))==false)
			{
				addmessage=false;
				m_log->error("MessageListRequester::HandleAllData date for message is in future! "+xml.GetDate(i));
			}

			if(addmessage==true)
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
			else
			{
				m_log->trace("MessageListRequester::HandleAllData will not download message posted to "+boardsstr);
			}
		}

		// insert external message indexes
		for(long i=0; i<xml.ExternalMessageCount(); i++)
		{
			if(xml.GetExternalType(i)=="Keyed")
			{
				// go through each board the message was posted to and see if we are saving messages to that board
				// if the board isn't found, see if we are saving messages to new boards
				boardsstr="";
				addmessage=false;
				std::vector<std::string> messageboards=xml.GetExternalBoards(i);
				for(std::vector<std::string>::iterator j=messageboards.begin(); j!=messageboards.end(); j++)
				{
					if(boards.find((*j))!=boards.end())
					{
						if(boards[(*j)]==true)
						{
							addmessage=true;
						}
					}
					else if(m_savetonewboards==true)
					{
						addmessage=true;
					}
					if(j!=messageboards.begin())
					{
						boardsstr+=", ";
					}
					boardsstr+=(*j);
				}

				if(CheckDateNotFuture(xml.GetExternalDate(i))==false)
				{
					addmessage=false;
					m_log->error("MessageListRequester::HandleAllData date for external message is in future! "+xml.GetExternalDate(i));
				}

				if(addmessage==true)
				{
					spk.Bind(0,xml.GetExternalIdentity(i));
					spk.Step();
					if(spk.RowReturned())
					{
						int thisidentityid=0;
						spk.ResultInt(0,thisidentityid);
						mst.Bind(0,thisidentityid);
						mst.Bind(1,xml.GetExternalDate(i));
						mst.Bind(2,xml.GetExternalIndex(i));
						mst.Step();
						mst.Reset();
					}
					spk.Reset();
				}
				else
				{
					m_log->trace("MessageListRequester::HandleAllData will not download external message posted to "+boardsstr+" from " + xml.GetExternalIdentity(i));
				}
			}
		}

		st=m_db->Prepare("INSERT INTO tblMessageListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'true');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->debug("MessageListRequester::HandleAllData parsed MessageList XML file : "+message["Identifier"]);
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

		m_log->error("MessageListRequester::HandleAllData error parsing MessageList XML file : "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;

}

const bool MessageListRequester::HandleGetFailed(FCPMessage &message)
{
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long identityid;
	long index;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[2],index);	

	// code 27 - permanent redirect
	if(message["Code"]=="27")
	{
		StartRedirectRequest(message);
		return true;
	}

	// if this is a fatal error - insert index into database so we won't try to download this index again
	if(message["Fatal"]=="true")
	{
		st=m_db->Prepare("INSERT INTO tblMessageListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->error("MessageListRequester::HandleGetFailed fatal error code="+message["Code"]+" requesting "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;
}

void MessageListRequester::Initialize()
{
	m_fcpuniquename="MessageListRequester";
	std::string tempval="";

	m_maxrequests=0;
	Option::Instance()->GetInt("MaxMessageListRequests",m_maxrequests);
	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->error("Option MaxMessageListRequests is currently set at "+tempval+".  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->warning("Option MaxMessageListRequests is currently set at "+tempval+".  This value might be incorrectly configured.");
	}

	tempval="";
	Option::Instance()->Get("LocalTrustOverridesPeerTrust",tempval);
	if(tempval=="true")
	{
		m_localtrustoverrides=true;
	}
	else
	{
		m_localtrustoverrides=false;
	}

	tempval="";
	Option::Instance()->Get("SaveMessagesFromNewBoards",tempval);
	if(tempval=="true")
	{
		m_savetonewboards=true;
	}
	else
	{
		m_savetonewboards=false;
	}

}

void MessageListRequester::PopulateIDList()
{
	Poco::DateTime date;
	int id;

	SQLite3DB::Statement st;

	// select identities we want to query (we've seen them today) - sort by their trust level (descending) with secondary sort on how long ago we saw them (ascending)
	if(m_localtrustoverrides==false)
	{
		st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen>='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"' AND (LocalMessageTrust IS NULL OR LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust')) AND (PeerMessageTrust IS NULL OR PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')) ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
	}
	else
	{
		st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen>='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"' AND (LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust') OR (LocalMessageTrust IS NULL AND (PeerMessageTrust IS NULL OR PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')))) ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
	}
	st.Step();

	m_ids.clear();

	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[id]=false;
		st.Step();
	}
}

void MessageListRequester::StartRedirectRequest(FCPMessage &message)
{
	std::vector<std::string> parts;
	std::string indexstr="";
	std::string identityidstr="";
	std::string datestr="";
	FCPMessage newmessage;

	// get the new edition #
	StringFunctions::Split(message["RedirectURI"],"/",parts);
	//edition # is 2nd to last part
	if(parts.size()>2)
	{
		indexstr=parts[parts.size()-2];
	}

	// get identityid
	parts.clear();
	StringFunctions::Split(message["Identifier"],"|",parts);
	if(parts.size()>1)
	{
		identityidstr=parts[1];
	}
	if(parts.size()>4)
	{
		datestr=parts[4];
	}

	newmessage.SetName("ClientGet");
	newmessage["URI"]=StringFunctions::UriDecode(message["RedirectURI"]);
	newmessage["Identifier"]=m_fcpuniquename+"|"+identityidstr+"|"+indexstr+"|_|"+datestr+"|"+newmessage["URI"];
	newmessage["ReturnType"]="direct";
	newmessage["MaxSize"]="1000000";

	m_fcp->SendMessage(newmessage);

}

void MessageListRequester::StartRequest(const long &id)
{
	Poco::DateTime now;
	FCPMessage message;
	std::string publickey;
	int index=0;
	std::string indexstr;
	std::string identityidstr;

	SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblIdentity WHERE IdentityID=?;");
	st.Bind(0,id);
	st.Step();

	if(st.RowReturned())
	{
		st.ResultText(0,publickey);

		now=Poco::Timestamp();

		SQLite3DB::Statement st2=m_db->Prepare("SELECT MAX(RequestIndex) FROM tblMessageListRequests WHERE Day=? AND IdentityID=?;");
		st2.Bind(0,Poco::DateTimeFormatter::format(now,"%Y-%m-%d"));
		st2.Bind(1,id);
		st2.Step();

		index=0;
		if(st2.RowReturned())
		{
			if(st2.ResultNull(0)==false)
			{
				st2.ResultInt(0,index);
				// don't increment index here - the node will let us know if there is a new edition
				// 2008-05-31 - well actually the node isn't reliably retreiving the latest edition for USKs, so we DO need to increment the index
				index++;
			}
		}
		st2.Finalize();

		StringFunctions::Convert(index,indexstr);
		StringFunctions::Convert(id,identityidstr);

		message.SetName("ClientGet");
		message["URI"]="USK"+publickey.substr(3)+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y.%m.%d")+"|MessageList/"+indexstr+"/MessageList.xml";
		message["Identifier"]=m_fcpuniquename+"|"+identityidstr+"|"+indexstr+"|_|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|"+message["URI"];
		message["ReturnType"]="direct";
		message["MaxSize"]="1000000";

		m_fcp->SendMessage(message);

		m_requesting.push_back(id);
	}
	st.Finalize();

	m_ids[id]=true;
}
