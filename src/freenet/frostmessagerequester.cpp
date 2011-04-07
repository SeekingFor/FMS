#include "../../include/freenet/frostmessagerequester.h"
#include "../../include/freenet/frostidentity.h"
#include "../../include/freenet/frostmessagexml.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeParser.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/Timespan.h>

FrostMessageRequester::FrostMessageRequester(SQLite3DB::DB *db):IIndexRequester<std::string>(db)
{
	Initialize();
}

FrostMessageRequester::FrostMessageRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexRequester<std::string>(db,fcp)
{
	Initialize();
}

const std::string FrostMessageRequester::GetIDFromIdentifier(const std::string &identifier)
{
	std::vector<std::string> idparts;
	StringFunctions::Split(identifier,"|",idparts);
	return idparts[1]+"|"+idparts[2]+"|"+idparts[3];
}

const bool FrostMessageRequester::HandleAllData(FCPv2::Message &message)
{
	std::vector<std::string> idparts;
	std::vector<char> data;
	long datalength=0;
	FrostMessageXML xml;
	FrostIdentity frostid;
	bool validmessage=true;
	bool inserted=false;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(message["DataLength"],datalength);

	// wait for all data to be received from connection
	m_fcp->WaitForBytes(1000,datalength);

	// if we got disconnected- return immediately
	if(m_fcp->IsConnected()==false)
	{
		return false;
	}

	// receive the file
	m_fcp->Receive(data,datalength);

	m_db->Execute("BEGIN;");

	// mark this index as received
	SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblFrostMessageRequests(BoardID,Day,RequestIndex,Found) VALUES(?,?,?,'true');");
	st.Bind(0,idparts[1]);
	st.Bind(1,idparts[3]);
	st.Bind(2,idparts[2]);
	st.Step();

	if(data.size()>0 && xml.ParseXML(std::string(data.begin(),data.end()))==true)
	{
		std::vector<std::string> boards=xml.GetBoards();
		std::map<long,std::string> replyto=xml.GetInReplyTo();
		std::string messageuuid(xml.GetMessageID());

		if(xml.GetFrostAuthor()!="Anonymous" && frostid.FromPublicKey(xml.GetFrostPublicKey())==false)
		{
			m_log->debug("FrostMessageRequester::HandleAllData error with public key "+xml.GetFrostPublicKey());
			validmessage=false;
		}

		if(validmessage && xml.GetFrostAuthor()!="Anonymous" && frostid.VerifyAuthor(xml.GetFrostAuthor())==false)
		{
			m_log->debug("FrostMessageRequester::HandleAllData error with author "+xml.GetFrostAuthor());
			validmessage=false;
		}

		std::string contentv2=xml.GetSignableContentV2();
		if(validmessage && xml.GetFrostAuthor()!="Anonymous" && frostid.VerifySignature(std::vector<unsigned char>(contentv2.begin(),contentv2.end()),xml.GetFrostSignatureV2())==false)
		{
			m_log->debug("FrostMessageRequester::HandleAllData error with signature "+xml.GetFrostSignatureV2());
			validmessage=false;
		}

		if(xml.GetFrostAuthor()=="Anonymous" && m_saveanonymous==false)
		{
			validmessage=false;
		}

		// make sure there isn't any weird chars in uuid
		if(messageuuid.find_first_of("\":;><")!=std::string::npos)
		{
			m_log->error("FrostMessageRequester::HandleAllData Message ID contained unexpected characters : "+message["Identifier"]);
			validmessage=false;
		}

		if(validmessage==true)
		{
			std::string nntpbody="";
			nntpbody=xml.GetBody();

			st=m_db->Prepare("INSERT INTO tblMessage(FromName,MessageDate,MessageTime,Subject,MessageUUID,ReplyBoardID,Body,InsertDate,MessageIndex) VALUES(?,?,?,?,?,?,?,?,?);");
			st.Bind(0,xml.GetFrostAuthor());
			st.Bind(1,xml.GetDate());
			st.Bind(2,xml.GetTime());
			st.Bind(3,xml.GetSubject());
			st.Bind(4,xml.GetMessageID());
			st.Bind(5,idparts[1]);
			st.Bind(6,nntpbody);
			st.Bind(7,idparts[3]);
			st.Bind(8,idparts[2]);
			inserted=st.Step(true);
			long messageid=st.GetLastInsertRowID();

			if(inserted==true)
			{

				st=m_db->Prepare("INSERT INTO tblMessageBoard(MessageID,BoardID) VALUES(?,?);");
				for(std::vector<std::string>::iterator i=boards.begin(); i!=boards.end(); i++)
				{
					st.Bind(0,messageid);
					st.Bind(1,idparts[1]);
					st.Step();
					st.Reset();
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

				st=m_db->Prepare("INSERT INTO tblMessageFileAttachment(MessageID,Key,Size) VALUES(?,?,?);");
				std::vector<MessageXML::fileattachment> fileattachments=xml.GetFileAttachments();
				for(std::vector<MessageXML::fileattachment>::iterator i=fileattachments.begin(); i!=fileattachments.end(); i++)
				{
					st.Bind(0,messageid);
					st.Bind(1,(*i).m_key);
					st.Bind(2,(*i).m_size);
					st.Step();
					st.Reset();
				}

				m_log->debug("FrostMessageRequester::HandleAllData parsed Message XML file : "+message["Identifier"]);

			}
			else	// couldn't insert - was already in database
			{
				std::string errmsg;
				m_db->GetLastError(errmsg);
				m_log->debug("FrostMessageRequester::HandleAllData could not insert message into database.  SQLite error "+errmsg);
			}

			st.Finalize();

		}	// if validmessage
		else
		{
			m_log->debug("FrostMessageRequester::HandleAllData invalid message "+message["Identifier"]);
		}

	}
	else
	{
		m_log->error("FrostMessageRequester::HandleAllData error parsing FrostMessage XML file : "+message["Identifier"]);
	}

	m_db->Execute("COMMIT;");

	RemoveFromRequestList(idparts[1]+"|"+idparts[2]+"|"+idparts[3]);

	return true;

}

const bool FrostMessageRequester::HandleGetFailed(FCPv2::Message &message)
{
	std::vector<std::string> idparts;
	StringFunctions::Split(message["Identifier"],"|",idparts);

	if(message["Fatal"]=="true")
	{
		// insert index so we won't try it again
		SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblFrostMessageRequests(BoardID,Day,RequestIndex,Found) VALUES(?,?,?,'true');");
		st.Bind(0,idparts[1]);
		st.Bind(1,idparts[3]);
		st.Bind(2,idparts[2]);
		st.Step();
	}

	m_log->trace("FrostMessageRequester::HandleGetFailed handled failure "+message["Code"]+" of "+message["Identifier"]);

	RemoveFromRequestList(idparts[1]+"|"+idparts[2]+"|"+idparts[3]);

	return true;

}

void FrostMessageRequester::Initialize()
{
	m_fcpuniquename="FrostMessageRequester";
	std::string tempval("");
	m_maxrequests=0;
	Option option(m_db);

	option.GetInt("FrostMaxMessageRequests",m_maxrequests);
	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->error("Option FrostMaxMessageRequests is currently set at "+tempval+".  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->warning("Option FrostMaxMessageRequests is currently set at "+tempval+".  This value might be incorrectly configured.");
	}

	m_maxdaysbackward=0;
	option.GetInt("FrostMessageMaxDaysBackward",m_maxdaysbackward);
	if(m_maxdaysbackward<0)
	{
		m_maxdaysbackward=0;
		m_log->error("Option FrostMessageMaxDaysBackward is currently set at "+tempval+".  It must be 0 or greater.");
	}
	if(m_maxdaysbackward>30)
	{
		m_log->warning("Option FrostMessageMaxDaysBackward is currently set at "+tempval+".  This value might be incorrectly configured.");
	}

	m_boardprefix="";
	option.Get("FrostBoardPrefix",m_boardprefix);

	m_frostmessagebase="";
	option.Get("FrostMessageBase",m_frostmessagebase);

	m_saveanonymous=false;
	option.Get("FrostSaveAnonymousMessages",tempval);
	if(tempval=="true")
	{
		m_saveanonymous=true;
	}

	m_maxindexesforward=4;

}

void FrostMessageRequester::PopulateIDList()
{

	Poco::DateTime pastdate;
	long expectedindex;
	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardID, BoardName FROM tblBoard WHERE BoardName LIKE ? || '%' AND SaveReceivedMessages='true';");
	SQLite3DB::Statement st2=m_db->Prepare("SELECT Day, RequestIndex FROM tblFrostMessageRequests WHERE BoardID=? AND Day=? ORDER BY RequestIndex ASC;");

	m_db->Execute("BEGIN;");

	st.Bind(0,m_boardprefix);
	st.Step();

	while(st.RowReturned())
	{
		int boardid=-1;
		std::string boardidstr="";
		std::string boardname="";

		st.ResultInt(0,boardid);
		st.ResultText(1,boardname);

		StringFunctions::Convert(boardid,boardidstr);

		for(long backdays=0; backdays<=m_maxdaysbackward; backdays++)
		{

			pastdate=Poco::DateTime()-Poco::Timespan(backdays,0,0,0,0);
			std::string day=Poco::DateTimeFormatter::format(pastdate,"%Y-%m-%d");

			st2.Bind(0,boardid);
			st2.Bind(1,day);
			st2.Step();

			expectedindex=0;
			while(st2.RowReturned())
			{
				int thisindex=-1;
				st2.ResultText(0,day);
				st2.ResultInt(1,thisindex);

				// fill in indexes we haven't downloaded yet
				if(expectedindex<thisindex)
				{
					for(long i=expectedindex; i<thisindex; i++)
					{
						std::string istr="";
						StringFunctions::Convert(i,istr);
						m_ids[boardidstr+"|"+istr+"|"+day].m_requested=false;
					}
				}

				expectedindex=thisindex+1;

				st2.Step();

			}
			st2.Reset();

			// fill in remaining indexes
			for(long i=expectedindex; i<=expectedindex+m_maxindexesforward; i++)
			{
				std::string istr="";
				StringFunctions::Convert(i,istr);	
				m_ids[boardidstr+"|"+istr+"|"+day].m_requested=false;
			}

		}

		st.Step();
	}

	m_db->Execute("COMMIT;");

}

void FrostMessageRequester::StartRequest(const std::string &id)
{
	Poco::DateTime date;
	int tz=0;
	FCPv2::Message message;
	std::vector<std::string> idparts;

	StringFunctions::Split(id,"|",idparts);

	Poco::DateTimeParser::tryParse(idparts[2],date,tz);

	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardName, FrostPublicKey FROM tblBoard WHERE BoardID=?;");
	st.Bind(0,idparts[0]);
	st.Step();

	if(st.RowReturned())
	{
		std::string boardname="";
		std::string publickey="";

		st.ResultText(0,boardname);
		// erase prefix from the board name
		if(m_boardprefix.size()>0)
		{
			boardname.erase(0,m_boardprefix.size());
		}
		st.ResultText(1,publickey);
		if(!(publickey.find("SSK@")==0 && publickey.rfind("/")==(publickey.size()-1)))
		{
			publickey="";
		}

		message.SetName("ClientGet");
		if(publickey=="")
		{
			message["URI"]="KSK@frost|message|"+m_frostmessagebase+"|"+Poco::DateTimeFormatter::format(date,"%Y.%n.%e")+"-"+boardname+"-"+idparts[1]+".xml";
		}
		else
		{
			message["URI"]=publickey+boardname+"|"+Poco::DateTimeFormatter::format(date,"%Y.%n.%e")+"-"+idparts[1]+".xml";
		}
		message["Identifier"]=m_fcpuniquename+"|"+id+"|"+message["URI"];
		message["PriorityClass"]=m_defaultrequestpriorityclassstr;
		message["ReturnType"]="direct";
		message["MaxSize"]="1000000";		// 1 MB

		m_fcp->Send(message);

		StartedRequest(id,message["Identifier"]);
	}
	else
	{
		m_log->error("FrostMessageRequester::StartRequest unable to find db record for "+id);
	}

	m_ids[id].m_requested=true;

}
