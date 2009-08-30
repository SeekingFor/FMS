#include "../../include/freenet/messagelistrequester.h"
#include "../../include/freenet/messagelistxml.h"
#include "../../include/unicode/unicodestring.h"
#include "../../include/global.h"

#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>
#include <Poco/Timestamp.h>

#ifdef XMEM
	#include <xmem.h>
#endif

MessageListRequester::MessageListRequester(SQLite3DB::DB *db):IIndexRequester<long>(db)
{
	Initialize();
}

MessageListRequester::MessageListRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexRequester<long>(db,fcp)
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

const bool MessageListRequester::CheckDateWithinMaxDays(const std::string &datestr) const
{
	Poco::DateTime checkdate;
	Poco::DateTime date;
	int tzdiff=0;
	if(Poco::DateTimeParser::tryParse(datestr,date,tzdiff))
	{
		checkdate-=Poco::Timespan(m_messagedownloadmaxdaysbackward,0,0,0,0);
		if(checkdate<=date)
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

void MessageListRequester::GetBoardList(std::map<std::string,bool> &boards, const bool forceload)
{
	// only query database when forced, or an 30 minutes have passed since last query
	if(forceload==true || m_boardscacheupdate+Poco::Timespan(0,0,30,0,0)<=Poco::DateTime())
	{
		m_boardscache.clear();
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
				m_boardscache[boardname]=true;
			}
			else
			{
				m_boardscache[boardname]=false;
			}

			st.Step();
		}
		m_boardscacheupdate=Poco::DateTime();
	}

	boards=m_boardscache;

}

const long MessageListRequester::GetIDFromIdentifier(const std::string &identifier)
{
	long id;
	std::vector<std::string> idparts;
	StringFunctions::Split(identifier,"|",idparts);
	StringFunctions::Convert(idparts[1],id);
	return id;
}

const bool MessageListRequester::HandleAllData(FCPv2::Message &message)
{	
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long datalength;
	std::vector<char> data;
	MessageListXML xml;
	long identityid;
	long fromidentityid;
	long index;
	std::map<std::string,bool> boards;	// list of boards and if we will save messages for that board or not
	std::map<std::string,long> identityids;	// list of identity public keys and their id in the database
	bool addmessage=false;
	std::string boardsstr="";
	std::string datestr="";
	std::vector<std::string> dateparts;

	GetBoardList(boards);

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(message["DataLength"],datalength);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[2],index);

	fromidentityid=identityid;

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

		m_db->Execute("BEGIN;");

		SQLite3DB::Statement spk=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey=?;");
		SQLite3DB::Statement mst=m_db->Prepare("INSERT INTO tblMessageRequests(IdentityID,Day,RequestIndex,FromMessageList,FromIdentityID) VALUES(?,?,?,'true',?);");
		SQLite3DB::Statement ust=m_db->Prepare("UPDATE tblMessageRequests SET FromIdentityID=? WHERE IdentityID=? AND Day=? AND RequestIndex=?;");

		for(long i=0; i<xml.MessageCount(); i++)
		{

			// go through each board the message was posted to and see if we are saving messages to that board
			// if the board isn't found, see if we are saving messages to new boards
			boardsstr="";
			addmessage=false;
			std::vector<std::string> messageboards=xml.GetBoards(i);
			for(std::vector<std::string>::iterator j=messageboards.begin(); j!=messageboards.end(); j++)
			{
				UnicodeString boardname((*j));
				boardname.Trim(MAX_BOARD_NAME_LENGTH);
				(*j)=boardname.NarrowString();

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
				m_log->error(m_fcpuniquename+"::HandleAllData date for message is in future! "+xml.GetDate(i));
			}

			if(addmessage==true && CheckDateWithinMaxDays(xml.GetDate(i))==false)
			{
				addmessage=false;
			}

			if(addmessage==true)
			{
				mst.Bind(0,identityid);
				mst.Bind(1,xml.GetDate(i));
				mst.Bind(2,xml.GetIndex(i));
				mst.Bind(3,identityid);
				mst.Step();
				mst.Reset();

				// We need to update ID here, in case this index was already inserted from another
				// identity's message list.  This doesn't reset try count - maybe we should if the from
				// identity was another identity
				ust.Bind(0,identityid);
				ust.Bind(1,identityid);
				ust.Bind(2,xml.GetDate(i));
				ust.Bind(3,xml.GetIndex(i));
				ust.Step();
				ust.Reset();

				m_requestindexcache[xml.GetDate(i)][identityid].insert(xml.GetIndex(i));

			}
			else
			{
				//m_log->trace("MessageListRequester::HandleAllData will not download message posted to "+boardsstr+" on "+xml.GetDate(i));
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
					UnicodeString boardname((*j));
					boardname.Trim(MAX_BOARD_NAME_LENGTH);
					(*j)=boardname.NarrowString();

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
					m_log->error(m_fcpuniquename+"::HandleAllData date for external message is in future! "+xml.GetExternalDate(i));
				}

				if(addmessage==true && CheckDateWithinMaxDays(xml.GetExternalDate(i))==false)
				{
					addmessage=false;
				}

				if(addmessage==true)
				{
					int thisidentityid=0;
					if(identityids.find(xml.GetExternalIdentity(i))!=identityids.end())
					{
						thisidentityid=identityids[xml.GetExternalIdentity(i)];
					}
					else
					{
						spk.Bind(0,xml.GetExternalIdentity(i));
						spk.Step();

						if(spk.RowReturned())
						{
							spk.ResultInt(0,thisidentityid);
							identityids[xml.GetExternalIdentity(i)]=thisidentityid;
						}

						spk.Reset();
					}

					if(thisidentityid!=0 && m_requestindexcache[xml.GetExternalDate(i)][thisidentityid].find(xml.GetExternalIndex(i))==m_requestindexcache[xml.GetExternalDate(i)][thisidentityid].end())
					{
						mst.Bind(0,thisidentityid);
						mst.Bind(1,xml.GetExternalDate(i));
						mst.Bind(2,xml.GetExternalIndex(i));
						mst.Bind(3,fromidentityid);
						mst.Step();
						mst.Reset();

						m_requestindexcache[xml.GetExternalDate(i)][thisidentityid].insert(xml.GetExternalIndex(i));
					}
				}
				else
				{
					//m_log->trace("MessageListRequester::HandleAllData will not download external message posted to "+boardsstr+" from " + xml.GetExternalIdentity(i) + " on " + xml.GetExternalDate(i));
				}
			}
		}

		st=m_db->Prepare("INSERT INTO tblMessageListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'true');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		spk.Finalize();
		mst.Finalize();
		ust.Finalize();

		m_db->Execute("COMMIT;");

		m_log->debug(m_fcpuniquename+"::HandleAllData parsed MessageList XML file : "+message["Identifier"]);
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

		m_log->error(m_fcpuniquename+"::HandleAllData error parsing MessageList XML file : "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	// keep 2 days of request indexes in the cache
	while(m_requestindexcache.size()>2)
	{
		m_requestindexcache.erase(m_requestindexcache.begin());
	}

	return true;

}

const bool MessageListRequester::HandleGetFailed(FCPv2::Message &message)
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

		m_log->error(m_fcpuniquename+"::HandleGetFailed fatal error code="+message["Code"]+" requesting "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;
}

void MessageListRequester::Initialize()
{
	m_fcpuniquename="ActiveMessageListRequester";
	std::string tempval("");
	m_maxrequests=0;
	Option option(m_db);

	option.GetInt("MaxMessageListRequests",m_maxrequests);

	// active identities get 1/2 of the max requests option + any remaining if not evenly divisible - inactive identities get 1/2
	m_maxrequests=(m_maxrequests/2)+(m_maxrequests%2);

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
	option.Get("LocalTrustOverridesPeerTrust",tempval);
	if(tempval=="true")
	{
		m_localtrustoverrides=true;
	}
	else
	{
		m_localtrustoverrides=false;
	}

	tempval="";
	option.Get("SaveMessagesFromNewBoards",tempval);
	if(tempval=="true")
	{
		m_savetonewboards=true;
	}
	else
	{
		m_savetonewboards=false;
	}

	m_messagedownloadmaxdaysbackward=5;
	tempval="5";
	option.Get("MessageDownloadMaxDaysBackward",tempval);
	StringFunctions::Convert(tempval,m_messagedownloadmaxdaysbackward);

	m_boardscacheupdate=Poco::DateTime()-Poco::Timespan(1,0,0,0,0);

}

void MessageListRequester::PopulateIDList()
{
	Poco::DateTime date;
	Poco::DateTime yesterday=date-Poco::Timespan(1,0,0,0,0);
	int id;

	SQLite3DB::Statement st;

	// select identities we want to query (we've seen them today) - sort by their trust level (descending) with secondary sort on how long ago we saw them (ascending)
	if(m_localtrustoverrides==false)
	{
		st=m_db->Prepare("SELECT tblIdentity.IdentityID FROM tblIdentity INNER JOIN vwIdentityStats ON tblIdentity.IdentityID=vwIdentityStats.IdentityID WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen>='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"' AND (vwIdentityStats.LastMessageDate>='"+Poco::DateTimeFormatter::format(yesterday,"%Y-%m-%d")+"') AND (LocalMessageTrust IS NULL OR LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust')) AND (PeerMessageTrust IS NULL OR PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')) AND FailureCount<=(SELECT OptionValue FROM tblOption WHERE Option='MaxFailureCount') ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
	}
	else
	{
		st=m_db->Prepare("SELECT tblIdentity.IdentityID FROM tblIdentity INNER JOIN vwIdentityStats ON tblIdentity.IdentityID=vwIdentityStats.IdentityID WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen>='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"' AND (vwIdentityStats.LastMessageDate>='"+Poco::DateTimeFormatter::format(yesterday,"%Y-%m-%d")+"') AND (LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust') OR (LocalMessageTrust IS NULL AND (PeerMessageTrust IS NULL OR PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')))) AND FailureCount<=(SELECT OptionValue FROM tblOption WHERE Option='MaxFailureCount') ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
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

void MessageListRequester::StartRedirectRequest(FCPv2::Message &message)
{
	std::vector<std::string> parts;
	std::string indexstr="";
	std::string identityidstr="";
	std::string datestr="";
	FCPv2::Message newmessage;

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

	m_fcp->Send(newmessage);

}

void MessageListRequester::StartRequest(const long &id)
{
	Poco::DateTime now;
	FCPv2::Message message;
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
		message["PriorityClass"]=m_defaultrequestpriorityclassstr;
		message["ReturnType"]="direct";
		message["MaxSize"]="1000000";

		m_fcp->Send(message);

		m_requesting.push_back(id);
	}
	st.Finalize();

	m_ids[id]=true;
}
