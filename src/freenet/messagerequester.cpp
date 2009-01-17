#include "../../include/freenet/messagerequester.h"
#include "../../include/freenet/messagexml.h"

#include <algorithm>

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/Timespan.h>

#ifdef XMEM
	#include <xmem.h>
#endif

MessageRequester::MessageRequester(SQLite3DB::DB *db):IIndexRequester<std::string>(db)
{
	Initialize();
}

MessageRequester::MessageRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexRequester<std::string>(db,fcp)
{
	Initialize();
}

const long MessageRequester::GetBoardID(const std::string &boardname, const std::string &identityname)
{
	std::string lowerboard=boardname;
	StringFunctions::LowerCase(lowerboard,lowerboard);
	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardID FROM tblBoard WHERE BoardName=?;");
	st.Bind(0,lowerboard);
	st.Step();

	if(st.RowReturned())
	{
		int boardid;
		st.ResultInt(0,boardid);
		return boardid;
	}
	else
	{
		Poco::DateTime now;
		st=m_db->Prepare("INSERT INTO tblBoard(BoardName,DateAdded,SaveReceivedMessages,AddedMethod) VALUES(?,?,?,?);");
		st.Bind(0,boardname);
		st.Bind(1,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
		if(m_savemessagesfromnewboards)
		{
			st.Bind(2,"true");
		}
		else
		{
			st.Bind(2,"false");
		}
		st.Bind(3,"Message from "+identityname);
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

const bool MessageRequester::HandleAllData(FCPv2::Message &message)
{
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long datalength;
	std::vector<char> data;
	MessageXML xml;
	long identityid;
	long index;
	bool inserted=false;
	bool validmessage=true;
	long savetoboardcount=0;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(message["DataLength"],datalength);
	StringFunctions::Convert(idparts[2],identityid);
	StringFunctions::Convert(idparts[4],index);

	// wait for all data to be received from connection
	m_fcp->WaitForBytes(1000,datalength);

	// if we got disconnected- return immediately
	if(m_fcp->IsConnected()==false)
	{
		return false;
	}

	// receive the file
	m_fcp->Receive(data,datalength);

	// mark this index as received
	st=m_db->Prepare("UPDATE tblMessageRequests SET Found='true' WHERE IdentityID=? AND Day=? AND RequestIndex=?;");
	st.Bind(0,identityid);
	st.Bind(1,idparts[3]);
	st.Bind(2,index);
	st.Step();
	st.Finalize();

	// parse file into xml and update the database
	if(data.size()>0 && xml.ParseXML(std::string(data.begin(),data.end()))==true)
	{
		std::vector<std::string> boards=xml.GetBoards();
		std::map<long,std::string> replyto=xml.GetInReplyTo();

		if(boards.size()>m_maxboardspermessage)
		{
			boards.resize(m_maxboardspermessage);
		}

		if(boards.size()<=0)
		{
			m_log->error("MessageRequester::HandleAllData Message XML did not contain any boards! "+message["Identifier"]);
			// remove this identityid from request list
			RemoveFromRequestList(idparts[1]);			
			return true;
		}
		if(xml.GetReplyBoard()=="")
		{
			m_log->error("MessageRequester::HandleAllData Message XML did not contain a reply board! "+message["Identifier"]);
			// remove this identityid from request list
			RemoveFromRequestList(idparts[1]);			
			return true;
		}

		// make sure the reply board is on the board list we are saving - if not, replace the last element of boards with the reply board
		if(xml.GetReplyBoard()!="" && std::find(boards.begin(),boards.end(),xml.GetReplyBoard())==boards.end() && boards.size()>0)
		{
			boards[boards.size()-1]=xml.GetReplyBoard();
		}

		// make sure domain of message id match 43 characters of public key of identity (remove - and ~) - if not, discard message
		// implement after 0.1.12 is released
		st=m_db->Prepare("SELECT PublicKey FROM tblIdentity WHERE IdentityID=?;");
		st.Bind(0,identityid);
		st.Step();
		if(st.RowReturned())
		{
			std::vector<std::string> uuidparts;
			std::vector<std::string> keyparts;
			std::string keypart="";
			std::string publickey="";

			st.ResultText(0,publickey);

			StringFunctions::SplitMultiple(publickey,"@,",keyparts);
			StringFunctions::SplitMultiple(xml.GetMessageID(),"@",uuidparts);

			if(uuidparts.size()>1 && keyparts.size()>1)
			{
				keypart=StringFunctions::Replace(StringFunctions::Replace(keyparts[1],"-",""),"~","");
				if(keypart!=uuidparts[1])
				{
					m_log->error("MessageRequester::HandleAllData MessageID in Message doesn't match public key of identity : "+message["Identifier"]);
					validmessage=false;
				}
			}
			else
			{
				m_log->error("MessageRequester::HandleAllData Error with identity's public key or Message ID : "+message["Identifier"]);
				validmessage=false;
			}
		}
		else
		{
			m_log->error("MessageRequester::HandleAllData Error couldn't find identity : "+message["Identifier"]);
			validmessage=false;
		}

		// make sure we will at least save to 1 board before inserting message
		savetoboardcount=0;
		for(std::vector<std::string>::iterator bi=boards.begin(); bi!=boards.end(); bi++)
		{
			if(SaveToBoard((*bi)))
			{
				savetoboardcount++;
			}
		}

		if(validmessage && savetoboardcount>0)
		{
			std::string nntpbody="";
			nntpbody=xml.GetBody();

			//add file keys/sizes to body
			std::vector<MessageXML::fileattachment> fileattachments=xml.GetFileAttachments();
			if(fileattachments.size()>0)
			{
				nntpbody+="\r\nAttachments";
			}
			for(std::vector<MessageXML::fileattachment>::iterator i=fileattachments.begin(); i!=fileattachments.end(); i++)
			{
				std::string sizestr="0";
				StringFunctions::Convert((*i).m_size,sizestr);

				nntpbody+="\r\n"+(*i).m_key;
				nntpbody+="\r\n"+sizestr+" bytes";
				nntpbody+="\r\n";
			}

			m_db->Execute("BEGIN;");

			st=m_db->Prepare("INSERT INTO tblMessage(IdentityID,FromName,MessageDate,MessageTime,Subject,MessageUUID,ReplyBoardID,Body,MessageIndex) VALUES(?,?,?,?,?,?,?,?,?);");
			st.Bind(0,identityid);
			st.Bind(1,GetIdentityName(identityid));
			st.Bind(2,xml.GetDate());
			st.Bind(3,xml.GetTime());
			st.Bind(4,xml.GetSubject());
			st.Bind(5,xml.GetMessageID());
			st.Bind(6,GetBoardID(xml.GetReplyBoard(),GetIdentityName(identityid)));
			st.Bind(7,nntpbody);
			st.Bind(8,index);
			inserted=st.Step(true);
			int messageid=st.GetLastInsertRowID();

			if(inserted==true)
			{

				st=m_db->Prepare("INSERT INTO tblMessageBoard(MessageID,BoardID) VALUES(?,?);");
				for(std::vector<std::string>::iterator i=boards.begin(); i!=boards.end(); i++)
				{
					if(SaveToBoard((*i)))
					{
						st.Bind(0,messageid);
						st.Bind(1,GetBoardID((*i),GetIdentityName(identityid)));
						st.Step();
						st.Reset();
					}
				}
				st.Finalize();

				st=m_db->Prepare("INSERT INTO tblMessageReplyTo(MessageID,ReplyToMessageUUID,ReplyOrder) VALUES(?,?,?);");
				for(std::map<long,std::string>::iterator j=replyto.begin(); j!=replyto.end(); j++)
				{
					st.Bind(0,messageid);
					st.Bind(1,(*j).second);
					st.Bind(2,(*j).first);
					st.Step();
					st.Reset();
				}
				st.Finalize();

				m_log->debug("MessageRequester::HandleAllData parsed Message XML file : "+message["Identifier"]);

			}
			else	// couldn't insert - was already in database
			{
				//m_log->WriteLog(LogFile::LOGLEVEL_ERROR,"MessageRequester::HandleAddData could not insert message into database.  "+message["Identifier"]);
			}

			m_db->Execute("COMMIT;");

		}	// if validmessage
	}
	else
	{
		m_log->error("MessageRequester::HandleAllData error parsing Message XML file : "+message["Identifier"]);
	}

	RemoveFromRequestList(idparts[1]);

	return true;
}

const bool MessageRequester::HandleGetFailed(FCPv2::Message &message)
{
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	std::string requestid;
	long index;
	long identityid;

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

		m_log->error("MessageRequester::HandleGetFailed fatal error requesting "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(requestid);

	return true;
}

void MessageRequester::Initialize()
{
	m_fcpuniquename="MessageRequester";
	std::string tempval("");
	m_maxrequests=0;
	Option option(m_db);

	option.GetInt("MaxMessageRequests",m_maxrequests);
	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->error("Option MaxMessageRequests is currently set at "+tempval+".  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->warning("Option MaxMessageRequests is currently set at "+tempval+".  This value might be incorrectly configured.");
	}

	m_maxdaysbackward=0;
	option.GetInt("MessageDownloadMaxDaysBackward",m_maxdaysbackward);
	if(m_maxdaysbackward<0)
	{
		m_maxdaysbackward=0;
		m_log->error("Option MessageDownloadMaxDaysBackward is currently set at "+tempval+".  It must be 0 or greater.");
	}
	if(m_maxdaysbackward>30)
	{
		m_log->warning("Option MessageDownloadMaxDaysBackward is currently set at "+tempval+".  This value might be incorrectly configured.");
	}

	m_maxpeermessages=0;
	option.GetInt("MaxPeerMessagesPerDay",m_maxpeermessages);
	if(m_maxpeermessages<1)
	{
		m_maxpeermessages=1;
		m_log->error("Option MaxPeerMessagesPerDay is currently set at "+tempval+".  It must be 1 or greater.");
	}
	if(m_maxpeermessages<20 || m_maxpeermessages>1000)
	{
		m_log->warning("Option MaxPeerMessagesPerDay is currently set at "+tempval+".  This value might be incorrectly configured.  The suggested value is 200.");
	}

	m_maxboardspermessage=0;
	option.GetInt("MaxBoardsPerMessage",m_maxboardspermessage);
	if(m_maxboardspermessage<1)
	{
		m_maxboardspermessage=1;
		m_log->error("Option MaxBoardsPerMessage is currently set at "+tempval+".  It must be 1 or greater.");
	}
	if(m_maxboardspermessage>20)
	{
		m_log->warning("Option MaxBoardsPerMessage is currently set at "+tempval+".  This value might be incorrectly configured.");
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

void MessageRequester::PopulateIDList()
{
	Poco::DateTime date;
	std::string val1;
	std::string val2;
	std::string val3;
	std::string sql;
	long requestindex;

	date-=Poco::Timespan(m_maxdaysbackward,0,0,0,0);

	sql="SELECT tblIdentity.IdentityID,Day,RequestIndex ";
	sql+="FROM tblMessageRequests INNER JOIN tblIdentity ON tblMessageRequests.IdentityID=tblIdentity.IdentityID ";
	sql+="WHERE FromMessageList='true' AND Found='false' AND Day>='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"' ";
	if(m_localtrustoverrides==false)
	{
		sql+="AND (tblIdentity.LocalMessageTrust IS NULL OR tblIdentity.LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust')) ";
		sql+="AND (tblIdentity.PeerMessageTrust IS NULL OR tblIdentity.PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')) ";
	}
	else
	{
		sql+="AND (tblIdentity.LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust') OR (tblIdentity.LocalMessageTrust IS NULL AND (tblIdentity.PeerMessageTrust IS NULL OR tblIdentity.PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')))) ";
	}
	sql+="AND tblIdentity.Name <> '' ";
	// sort by day descending - in case there is a bunch of messages on a day that keep timing out, we will eventually get to the next day and hopefully find messages there
	// secondary ascending sort on tries
	// tertiary sort on request index (so we get low indexes first)
	sql+="ORDER BY tblMessageRequests.Day DESC, tblMessageRequests.Tries ASC, tblMessageRequests.RequestIndex ASC ";
	sql+=";";

	SQLite3DB::Statement st=m_db->Prepare(sql);
	st.Step();

	m_ids.clear();

	while(st.RowReturned())
	{
		st.ResultText(0,val1);
		st.ResultText(1,val2);
		st.ResultText(2,val3);

		requestindex=0;
		StringFunctions::Convert(val3,requestindex);

		// only continue if index is < max messages we will accept from a peer
		if(requestindex<m_maxpeermessages)
		{
			if(m_ids.find(val1+"*"+val2+"*"+val3)==m_ids.end())
			{
				m_ids[val1+"*"+val2+"*"+val3]=false;
			}
		}
		st.Step();
	}

}

const bool MessageRequester::SaveToBoard(const std::string &boardname)
{
	bool save=true;
	SQLite3DB::Statement st=m_db->Prepare("SELECT SaveReceivedMessages FROM tblBoard WHERE BoardName=?;");
	st.Bind(0,boardname);
	st.Step();
	if(st.RowReturned())
	{
		std::string val="";
		st.ResultText(0,val);
		if(val=="true")
		{
			save=true;
		}
		else
		{
			save=false;
		}
	}
	return save;
}

void MessageRequester::StartRequest(const std::string &requestid)
{
	FCPv2::Message message;
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
		message["MaxRetries"]="-1";			// use ULPR since we are fairly sure message exists since the author says it does

		m_fcp->Send(message);

		m_requesting.push_back(requestid);

		// update tries
		st=m_db->Prepare("UPDATE tblMessageRequests SET Tries=Tries+1 WHERE IdentityID=? AND Day=? AND RequestIndex=?;");
		st.Bind(0,identityid);
		st.Bind(1,date);
		st.Bind(2,indexstr);
		st.Step();

		m_log->debug("MessageRequester::StartRequest requesting "+message["Identifier"]);
	}
	
	m_ids[requestid]=true;

}
