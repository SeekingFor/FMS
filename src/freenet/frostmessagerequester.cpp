#include "../../include/freenet/frostmessagerequester.h"
#include "../../include/freenet/frostidentity.h"
#include "../../include/freenet/frostmessagexml.h"
#include "../../include/threadbuilder.h"
#include "../../include/message.h"

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
	if(idparts.size()>3)
	{
		return idparts[1]+"|"+idparts[2]+"|"+idparts[3];
	}
	return std::string("");
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
	bool constraintfailure=false;
	std::vector<std::pair<long,long> > buildthreads;
	ThreadBuilder tb(m_db);
	SQLite3DB::Transaction trans(m_db);

	m_log->trace("FrostMessageRequester::HandleAllData started handling "+message["Identifier"]);

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

	trans.Begin(SQLite3DB::Transaction::TRANS_IMMEDIATE);

	// mark this index as received
	SQLite3DB::Statement st=m_db->Prepare("INSERT OR IGNORE INTO tblFrostMessageRequests(BoardID,Day,RequestIndex,Found) VALUES(?,?,?,'true');");
	st.Bind(0,idparts[3]);
	st.Bind(1,idparts[1]);
	st.Bind(2,idparts[2]);
	trans.Step(st);

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

		if(validmessage==true && trans.IsSuccessful()==true)
		{
			std::string nntpbody="";
			nntpbody=xml.GetBody();
			int linemaxbytes=Message::LineMaxBytes(nntpbody);

			st=m_db->Prepare("INSERT INTO tblMessage(FromName,MessageDate,MessageTime,Subject,MessageUUID,ReplyBoardID,Body,InsertDate,MessageIndex,BodyLineMaxBytes,MessageSource) VALUES(?,?,?,?,?,?,?,?,?,?,?);");
			st.Bind(0,xml.GetFrostAuthor());
			st.Bind(1,xml.GetDate());
			st.Bind(2,xml.GetTime());
			st.Bind(3,xml.GetSubject());
			st.Bind(4,xml.GetMessageID());
			st.Bind(5,idparts[3]);
			st.Bind(6,nntpbody);
			st.Bind(7,idparts[1]);
			st.Bind(8,idparts[2]);
			st.Bind(9,linemaxbytes);
			st.Bind(10,Message::SOURCE_FROST);
			inserted=trans.Step(st,true);
			// constraint failure, we'll still mark the index as retrieved later
			if(trans.IsSuccessful()==false && (trans.GetLastError() & SQLITE_CONSTRAINT)==SQLITE_CONSTRAINT)
			{
				constraintfailure=true;
			}
			long messageid=st.GetLastInsertRowID();

			if(inserted==true)
			{

				SQLite3DB::Statement latestmessagest=m_db->Prepare("UPDATE tblBoard SET LatestMessageID=(SELECT tblMessage.MessageID FROM tblMessage INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID WHERE tblMessageBoard.BoardID=tblBoard.BoardID ORDER BY tblMessage.MessageDate DESC, tblMessage.MessageTime DESC LIMIT 0,1) WHERE tblBoard.BoardID=?;");
				SQLite3DB::Statement forumst=m_db->Prepare("SELECT Forum FROM tblBoard WHERE BoardID=?;");
				st=m_db->Prepare("INSERT INTO tblMessageBoard(MessageID,BoardID) VALUES(?,?);");
				for(std::vector<std::string>::iterator i=boards.begin(); i!=boards.end(); i++)
				{
					long boardid=0;
					StringFunctions::Convert(idparts[3],boardid);
					st.Bind(0,messageid);
					st.Bind(1,boardid);
					trans.Step(st);
					trans.Reset(st);

					forumst.Bind(0,boardid);
					if(trans.Step(forumst))
					{
						std::string isforum("false");
						forumst.ResultText(0,isforum);
						if(isforum=="true")
						{
							buildthreads.push_back(std::pair<long,long>(messageid,boardid));
							latestmessagest.Bind(0,boardid);
							trans.Step(latestmessagest);
							trans.Reset(latestmessagest);
						}
					}
					trans.Reset(forumst);

				}
				trans.Finalize(st);

				st=m_db->Prepare("INSERT INTO tblMessageReplyTo(MessageID,ReplyToMessageUUID,ReplyOrder) VALUES(?,?,?);");
				for(std::map<long,std::string>::iterator j=replyto.begin(); j!=replyto.end(); j++)
				{
					st.Bind(0,messageid);
					st.Bind(1,(*j).second);
					st.Bind(2,(*j).first);
					trans.Step(st);
					trans.Reset(st);
				}
				trans.Finalize(st);

				st=m_db->Prepare("INSERT INTO tblMessageFileAttachment(MessageID,Key,Size) VALUES(?,?,?);");
				std::vector<MessageXML::fileattachment> fileattachments=xml.GetFileAttachments();
				for(std::vector<MessageXML::fileattachment>::iterator i=fileattachments.begin(); i!=fileattachments.end(); i++)
				{
					st.Bind(0,messageid);
					st.Bind(1,(*i).m_key);
					st.Bind(2,(*i).m_size);
					trans.Step(st);
					trans.Reset(st);
				}

				m_log->debug("FrostMessageRequester::HandleAllData parsed Message XML file : "+message["Identifier"]);

			}
			else	// couldn't insert - was already in database
			{
				m_log->debug("FrostMessageRequester::HandleAllData could not insert message into database.  SQLite error "+trans.GetLastErrorStr());
			}

			trans.Finalize(st);

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

	trans.Commit();

	if(trans.IsSuccessful()==false && constraintfailure==false)
	{
		m_log->error("FrostMessageRequester::HandleAllData transaction failed with SQLite error:"+trans.GetLastErrorStr()+" SQL="+trans.GetErrorSQL());
	}

	if(constraintfailure==true)
	{
		st=m_db->Prepare("INSERT INTO tblFrostMessageRequests(BoardID,Day,RequestIndex,Found) VALUES(?,?,?,'true');");
		st.Bind(0,idparts[3]);
		st.Bind(1,idparts[1]);
		st.Bind(2,idparts[2]);
		st.Step();
	}

	for(std::vector<std::pair<long,long> >::const_iterator i=buildthreads.begin(); i!=buildthreads.end(); i++)
	{
		tb.Build((*i).first,(*i).second,true);
	}

	RemoveFromRequestList(idparts[1]+"|"+idparts[2]+"|"+idparts[3]);
	// also delete from list so the list doesn't grow too large
	m_ids.erase(idparts[1]+"|"+idparts[2]+"|"+idparts[3]);

	return true;

}

const bool FrostMessageRequester::HandleGetFailed(FCPv2::Message &message)
{
	std::vector<std::string> idparts;
	StringFunctions::Split(message["Identifier"],"|",idparts);

	if(message["Fatal"]=="true")
	{
		if(message["Code"]!="25")
		{
			// insert index so we won't try it again
			SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblFrostMessageRequests(BoardID,Day,RequestIndex,Found) VALUES(?,?,?,'true');");
			st.Bind(0,idparts[1]);
			st.Bind(1,idparts[3]);
			st.Bind(2,idparts[2]);
			st.Step();
		}
	}

	m_log->trace("FrostMessageRequester::HandleGetFailed handled failure "+message["Code"]+" of "+message["Identifier"]);

	RemoveFromRequestList(idparts[1]+"|"+idparts[2]+"|"+idparts[3]);
	// also delete from list so the list doesn't grow too large
	m_ids.erase(idparts[1]+"|"+idparts[2]+"|"+idparts[3]);

	return true;

}

void FrostMessageRequester::Initialize()
{
	m_fcpuniquename="FrostMessageRequester";
	std::string tempval("");
	m_maxrequests=0;
	m_reverserequest=true;
	Option option(m_db);

	option.GetInt("FrostMaxMessageRequests",m_maxrequests);
	option.Get("FrostMaxMessageRequests",tempval);
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
	option.Get("FrostMessageMaxDaysBackward",tempval);
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

void FrostMessageRequester::PopulateBoardRequestIDs(SQLite3DB::Transaction &trans, const long boardid, const Poco::DateTime &date)
{
	long expectedindex=0;
	std::string boardidstr("");
	std::string lastid("");
	std::string day("");

	StringFunctions::Convert(boardid,boardidstr);
	day=Poco::DateTimeFormatter::format(date,"%Y-%m-%d");

	m_log->trace("FrostMessageRequester::PopulateBoardRequestIDs populating requests for "+day+" for board id "+boardidstr);

	SQLite3DB::Statement st=m_db->Prepare("SELECT RequestIndex FROM tblFrostMessageRequests WHERE BoardID=? AND Day=? ORDER BY RequestIndex ASC;");
	st.Bind(0,boardid);
	st.Bind(1,day);

	trans.Step(st);
	while(st.RowReturned() && trans.IsSuccessful())
	{
		int thisindex=-1;

		st.ResultInt(0,thisindex);

		// fill in indexes we haven't downloaded yet
		if(expectedindex<thisindex)
		{
			for(long i=expectedindex; i<thisindex; i++)
			{
				std::string istr="";
				StringFunctions::Convert(i,istr);
				m_ids[day+"|"+istr+"|"+boardidstr];
				lastid=day+"|"+istr+"|"+boardidstr;
			}
		}

		expectedindex=thisindex+1;
		st.Step();
	}

	// fill in remaining indexes
	for(long i=expectedindex; i<=expectedindex+m_maxindexesforward; i++)
	{
		std::string istr="";
		StringFunctions::Convert(i,istr);	
		m_ids[day+"|"+istr+"|"+boardidstr];
		lastid=day+"|"+istr+"|"+boardidstr;
	}

	// we'll use the flag to indicate that when we start this request, we want to populate indexes for the previous day for this board
	// the flag is only set on the last index for a board for a given day.
	m_ids[lastid].m_flag=true;

}

void FrostMessageRequester::PopulateIDList()
{

	Poco::DateTime date;
	long expectedindex;
	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardID, BoardName FROM tblBoard WHERE BoardName LIKE ? || '%' AND SaveReceivedMessages='true';");
	SQLite3DB::Statement st2=m_db->Prepare("SELECT Day, RequestIndex FROM tblFrostMessageRequests WHERE BoardID=? AND Day=? ORDER BY RequestIndex ASC;");
	SQLite3DB::Transaction trans(m_db);

	// only selects, deferred OK
	trans.Begin();

	st.Bind(0,m_boardprefix);
	trans.Step(st);

	while(st.RowReturned() && trans.IsSuccessful())
	{
		int boardid=-1;
		//std::string boardname="";
		std::string lastid="";

		st.ResultInt(0,boardid);
		//st.ResultText(1,boardname);

		PopulateBoardRequestIDs(trans,boardid,date);

		trans.Step(st);
	}

	trans.Finalize(st);
	trans.Finalize(st2);
	trans.Commit();

}

void FrostMessageRequester::StartRequest(const std::string &id)
{
	Poco::DateTime date;
	int tz=0;
	FCPv2::Message message;
	std::vector<std::string> idparts;

	StringFunctions::Split(id,"|",idparts);

	Poco::DateTimeParser::tryParse(idparts[0],date,tz);

	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardName, FrostPublicKey FROM tblBoard WHERE BoardID=?;");
	st.Bind(0,idparts[2]);
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

	// populate previous day indexes for this board
	if(m_ids[id].m_flag)
	{
		// populate previous day indexes for this board
		Poco::DateTime today;
		SQLite3DB::Transaction trans(m_db);
		date-=Poco::Timespan(1,0,0,0,0);
		Poco::Timespan ts=today-date;

		trans.Begin();

		if(ts.days()<=m_maxdaysbackward)
		{
			long boardid=0;
			StringFunctions::Convert(idparts[2],boardid);
			PopulateBoardRequestIDs(trans,boardid,date);
		}

		trans.Commit();

	}

}
