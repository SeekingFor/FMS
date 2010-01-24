#include "../../include/freenet/boardlistrequester.h"
#include "../../include/freenet/boardlistxml.h"
#include "../../include/freenet/identitypublickeycache.h"
#include "../../include/unicode/unicodestring.h"
#include "../../include/global.h"
#include "../../include/board.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>

#ifdef XMEM
	#include <xmem.h>
#endif

BoardListRequester::BoardListRequester(SQLite3DB::DB *db):IIndexRequester<long>(db)
{
	Initialize();
}

BoardListRequester::BoardListRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexRequester<long>(db,fcp)
{
	Initialize();
}

std::string BoardListRequester::GetIdentityName(const long identityid)
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

const long BoardListRequester::GetIDFromIdentifier(const std::string &identifier)
{
	long id;
	std::vector<std::string> idparts;
	StringFunctions::Split(identifier,"|",idparts);
	StringFunctions::Convert(idparts[1],id);
	return id;
}

const bool BoardListRequester::HandleAllData(FCPv2::Message &message)
{	
	Poco::DateTime now;
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long datalength;
	std::vector<char> data;
	BoardListXML xml;
	long identityid;
	long index;
	std::string identityname="";

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(message["DataLength"],datalength);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[2],index);

	identityname=GetIdentityName(identityid);

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

		SQLite3DB::Statement brd=m_db->Prepare("SELECT BoardID,BoardName,BoardDescription FROM tblBoard WHERE BoardName=?;");
		SQLite3DB::Statement ins=m_db->Prepare("INSERT INTO tblBoard(BoardName,BoardDescription,DateAdded,SaveReceivedMessages,AddedMethod) VALUES(?,?,?,?,?);");
		SQLite3DB::Statement upd=m_db->Prepare("UPDATE tblBoard SET BoardDescription=? WHERE BoardID=?;");
		for(long i=0; i<xml.GetCount(); i++)
		{
			int boardid;
			UnicodeString name(Board::FixBoardName(xml.GetName(i)));
			std::string dbdescription("");
			UnicodeString description(xml.GetDescription(i));

			description.Trim(MAX_BOARD_DESCRIPTION_LENGTH);

			brd.Bind(0,name.NarrowString());
			brd.Step();
			
			if(brd.RowReturned())
			{
				brd.ResultInt(0,boardid);
				brd.ResultText(2,dbdescription);
				if(dbdescription=="" && description.NarrowString()!="")
				{
					upd.Bind(0,description.NarrowString());
					upd.Bind(1,boardid);
					upd.Step();
					upd.Reset();
				}
			}
			else
			{
				ins.Bind(0,name.NarrowString());
				ins.Bind(1,description.NarrowString());
				ins.Bind(2,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
				if(m_savemessagesfromnewboards)
				{
					ins.Bind(3,"true");
				}
				else
				{
					ins.Bind(3,"false");
				}
				ins.Bind(4,"Board List of "+identityname);
				ins.Step();
				ins.Reset();
			}
			brd.Reset();
		}

		st=m_db->Prepare("INSERT INTO tblBoardListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'true');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_db->Execute("COMMIT;");

		m_log->debug("BoardListRequester::HandleAllData parsed BoardList XML file : "+message["Identifier"]);
	}
	else
	{
		// bad data - mark index
		st=m_db->Prepare("INSERT INTO tblBoardListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->error("BoardListRequester::HandleAllData error parsing BoardList XML file : "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;

}

const bool BoardListRequester::HandleGetFailed(FCPv2::Message &message)
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
		st=m_db->Prepare("INSERT INTO tblBoardListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->error("BoardListRequester::HandleGetFailed fatal error requesting "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;
}

void BoardListRequester::Initialize()
{
	std::string tempval="";

	m_fcpuniquename="BoardListRequester";
	m_maxrequests=0;
	Option option(m_db);

	option.GetInt("MaxBoardListRequests",m_maxrequests);
	if(m_maxrequests<0)
	{
		m_maxrequests=0;
		m_log->error("Option MaxBoardListRequests is currently set at "+tempval+".  It must be 0 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->warning("Option MaxBoardListRequests is currently set at "+tempval+".  This value might be incorrectly configured.");
	}

	option.Get("SaveMessagesFromNewBoards",tempval);
	if(tempval=="true")
	{
		m_savemessagesfromnewboards=true;
	}
	else
	{
		m_savemessagesfromnewboards=false;
	}

	option.Get("LocalTrustOverridesPeerTrust",tempval);
	if(tempval=="true")
	{
		m_localtrustoverrides=true;
	}
	else
	{
		m_localtrustoverrides=false;
	}

}

void BoardListRequester::PopulateIDList()
{
	int id;
	Poco::DateTime today;

	SQLite3DB::Statement st;

	if(m_localtrustoverrides==false)
	{
		st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen>='"+Poco::DateTimeFormatter::format(today,"%Y-%m-%d")+"' AND (LocalMessageTrust IS NULL OR LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust')) AND (PeerMessageTrust IS NULL OR PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')) AND PublishBoardList='true' AND FailureCount<=(SELECT OptionValue FROM tblOption WHERE Option='MaxFailureCount') ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
	}
	else
	{
		st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen>='"+Poco::DateTimeFormatter::format(today,"%Y-%m-%d")+"' AND (LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust') OR (LocalMessageTrust IS NULL AND (PeerMessageTrust IS NULL OR PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')))) AND PublishBoardList='true' AND FailureCount<=(SELECT OptionValue FROM tblOption WHERE Option='MaxFailureCount') ORDER BY LocalMessageTrust+LocalTrustListTrust DESC, LastSeen;");
	}

	m_ids.clear();

	m_db->Execute("BEGIN;");

	st.Step();
	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[id]=false;
		st.Step();
	}

	m_db->Execute("COMMIT;");

}

void BoardListRequester::StartRequest(const long &identityid)
{
	Poco::DateTime now;
	FCPv2::Message message;
	std::string publickey;
	std::string indexstr;
	int index;
	std::string identityidstr;
	IdentityPublicKeyCache pkcache(m_db);

	if(pkcache.PublicKey(identityid,publickey))
	{

		SQLite3DB::Statement st2=m_db->Prepare("SELECT MAX(RequestIndex) FROM tblBoardListRequests WHERE Day=? AND IdentityID=?;");
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
		message["URI"]=publickey+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|BoardList|"+indexstr+".xml";
		message["Identifier"]=m_fcpuniquename+"|"+identityidstr+"|"+indexstr+"|"+message["URI"];
		message["PriorityClass"]=m_defaultrequestpriorityclassstr;
		message["ReturnType"]="direct";
		message["MaxSize"]="100000";			// 100 KB

		m_fcp->Send(message);

		m_requesting.push_back(identityid);

	}

	m_ids[identityid]=true;

}
