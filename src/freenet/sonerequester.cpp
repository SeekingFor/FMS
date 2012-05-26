#include "../../include/freenet/sonerequester.h"
#include "../../include/freenet/sonexml.h"
#include "../../include/freenet/identitypublickeycache.h"
#include "../../include/unicode/unicodestring.h"
#include "../../include/threadbuilder.h"
#include "../../include/message.h"

#include <Poco/DateTimeFormatter.h>

SoneRequester::SoneRequester(SQLite3DB::DB *db):IIndexRequester<std::pair<long,long> >(db)
{
	Initialize();
}

SoneRequester::SoneRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexRequester<std::pair<long,long> >(db,fcp)
{
	Initialize();
}

std::string SoneRequester::CleanupSone(const std::string &text)
{
	return text;
}

std::string SoneRequester::CleanupSubject(const std::string &text)
{
	std::string returnstring=text;

	//get rid of sone://, @sone://, and @ sone://
	std::string::size_type pos=returnstring.find("sone://");
	while(pos!=std::string::npos)
	{
		// find next space after sone:// - must be done before finding @ space
		std::string::size_type pos2=returnstring.find(' ',pos);

		if(pos>0 && returnstring[pos-1]=='@')
		{
			pos--;
		}
		else if(pos>1 && returnstring[pos-2]=='@' && returnstring[pos-1]==' ')
		{
			pos-=2;
		}

		if(pos2!=std::string::npos)
		{
			returnstring.erase(pos,(pos2+1)-pos);
		}
		else
		{
			returnstring.erase(pos);
		}

		pos=returnstring.find("sone://");

	}

	//trim \t \r \n off beginning of string
	pos=returnstring.find_first_not_of("\t\r\n");
	if(pos>0 && pos!=std::string::npos)
	{
		returnstring.erase(0,pos);
	}

	// trim off space at beginning of string
	pos=returnstring.find_first_not_of(" ");
	if(pos>0 && pos!=std::string::npos)
	{
		returnstring.erase(0,pos);
	}

	pos=returnstring.find_first_of("\t\r\n");
	if(pos!=std::string::npos)
	{
		returnstring.erase(pos);
	}

	return returnstring;
}

const std::string SoneRequester::GetIdentityName(const long identityid)
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

const std::pair<long,long> SoneRequester::GetIDFromIdentifier(const std::string &identifier)
{
	long identityid;
	long identityorder;

	std::vector<std::string> idparts;
	StringFunctions::Split(identifier,"|",idparts);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[3],identityorder);

	return std::pair<long,long>(identityorder,identityid);
}

const bool SoneRequester::HandleAllData(FCPv2::Message &message)
{
	std::vector<std::string> idparts;
	std::vector<char> data;
	std::vector<char>::size_type datalength=0;
	SQLite3DB::Transaction trans(m_db);
	std::string idstr("");
	std::string indexstr("");
	SoneXML xml;
	long identityid;
	long index;
	long lastindex;
	long identityorder;
	std::vector<std::pair<long,long> > buildthreads;
	ThreadBuilder tb(m_db);
	Poco::DateTime now;

	trans.Begin(SQLite3DB::Transaction::TRANS_IMMEDIATE);

	m_log->debug(m_fcpuniquename+"::HandleAllData for "+message["Identifier"]);

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(message["DataLength"],datalength);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[2],index);
	StringFunctions::Convert(idparts[3],identityorder);
	StringFunctions::Convert(idparts[4],lastindex);

	if(idparts.size()>3)
	{
		idstr=idparts[1];
		indexstr=idparts[2];
	}

	// wait for all data to be received from connection
	m_fcp->WaitForBytes(1000,datalength);

	// if we got disconnected- return immediately
	if(m_fcp->IsConnected()==false)
	{
		return false;
	}

	m_fcp->Receive(data,datalength);

	// only continue if we haven't already downloaded the Sones at this index
	if(index>lastindex)
	{

		if(data.size()>0 && xml.ParseXML(std::string(data.begin(),data.end()))==true)
		{
			SQLite3DB::Statement findst=m_db->Prepare("SELECT MessageID FROM tblMessage WHERE MessageUUID=?;");
			SQLite3DB::Statement insertmst=m_db->Prepare("INSERT INTO tblMessage(IdentityID,FromName,MessageDate,MessageTime,Subject,MessageUUID,ReplyBoardID,Body,BodyLineMaxBytes,MessageSource) VALUES(?,?,?,?,?,?,?,?,?,?);");
			SQLite3DB::Statement insertmrst=m_db->Prepare("INSERT INTO tblMessageReplyTo(MessageID,ReplyToMessageUUID,ReplyOrder) VALUES(?,?,?);");
			SQLite3DB::Statement insertbst=m_db->Prepare("INSERT INTO tblMessageBoard(MessageID,BoardID) VALUES(?,?);");
			SQLite3DB::Statement latestmessagest=m_db->Prepare("UPDATE tblBoard SET LatestMessageID=(SELECT tblMessage.MessageID FROM tblMessage INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID WHERE tblMessageBoard.BoardID=tblBoard.BoardID ORDER BY tblMessage.MessageDate DESC, tblMessage.MessageTime DESC LIMIT 0,1) WHERE tblBoard.BoardID=?;");

			std::string fromname=GetIdentityName(identityid);

			for(std::vector<SoneXML::message>::const_iterator i=xml.GetSones().begin(); i!=xml.GetSones().end(); i++)
			{
				if(m_deletemessagesolderthan==-1 || ((*i).m_time+Poco::Timespan(m_deletemessagesolderthan,0,0,0,0))>=now)
				{
					if(m_soneids.find((*i).m_id)==m_soneids.end())
					{
						int messageid=0;
						findst.Bind(0,(*i).m_id+"@sone");
						trans.Step(findst);
						if(findst.RowReturned())
						{
							m_soneids.insert((*i).m_id);
							trans.Reset(findst);
						}
						else	// don't already have sone, insert it into db
						{
							trans.Reset(findst);
							std::string text(CleanupSone((*i).m_message));
							int linemaxbytes=Message::LineMaxBytes(text);
							UnicodeString subject=CleanupSubject(text);
							if(subject.CharacterCount()>30)
							{
								subject.Trim(30);
								subject+="...";
							}
							if(subject.CharacterCount()==0)
							{
								subject=" ";
							}

							insertmst.Bind(0,identityid);
							insertmst.Bind(1,fromname);
							insertmst.Bind(2,Poco::DateTimeFormatter::format((*i).m_time,"%Y-%m-%d"));
							insertmst.Bind(3,Poco::DateTimeFormatter::format((*i).m_time,"%H:%M:%S"));
							insertmst.Bind(4,subject.NarrowString());
							insertmst.Bind(5,(*i).m_id+"@sone");
							insertmst.Bind(6,m_soneboardid);
							insertmst.Bind(7,text);
							insertmst.Bind(8,linemaxbytes);
							insertmst.Bind(9,Message::SOURCE_SONE);
							if(trans.Step(insertmst,true)==true)
							{
								messageid=insertmst.GetLastInsertRowID();
								buildthreads.push_back(std::pair<long,long>(messageid,m_soneboardid));
							}
							else
							{
								m_log->trace(m_fcpuniquename+"::HandleAllData SQL error inserting message : "+trans.GetLastErrorStr());
							}
							trans.Reset(insertmst);

							if((*i).m_replyto!="")
							{
								insertmrst.Bind(0,messageid);
								insertmrst.Bind(1,(*i).m_replyto+"@sone");
								insertmrst.Bind(2,0);
								trans.Step(insertmrst);
								trans.Reset(insertmrst);
							}

							insertbst.Bind(0,messageid);
							insertbst.Bind(1,m_soneboardid);
							trans.Step(insertbst);
							trans.Reset(insertbst);

							latestmessagest.Bind(0,m_soneboardid);
							trans.Step(latestmessagest);
							trans.Reset(latestmessagest);

						}
					}
				}

			}

		}
		else
		{
			SQLite3DB::Statement failst=m_db->Prepare("UPDATE tblIdentity SET FailureCount=FailureCount+1 WHERE IdentityID=?;");
			failst.Bind(0,identityid);
			trans.Step(failst);
			trans.Finalize(failst);
			m_log->error(m_fcpuniquename+"::HandleAllData trouble with "+message["Identifier"]);
		}

		SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIdentity SET SoneLastRequest=datetime('now'), SoneLastSeen=datetime('now'), SoneLastIndex=? WHERE IdentityID=?;");
		st.Bind(0,index);
		st.Bind(1,identityid);
		trans.Step(st);
		trans.Finalize(st);

	}
	else	// we've already downloaded these Sones
	{
		SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIdentity SET SoneLastRequest=datetime('now') WHERE IdentityID=?;");
		st.Bind(0,identityid);
		trans.Step(st);
		trans.Finalize(st);
	}

	trans.Commit();

	for(std::vector<std::pair<long,long> >::const_iterator i=buildthreads.begin(); i!=buildthreads.end(); i++)
	{
		tb.Build((*i).first,(*i).second,true);
	}

	// remove this identityid from request list
	RemoveFromRequestList(std::pair<long,long>(identityorder,identityid));

	return true;
}

const bool SoneRequester::HandleGetFailed(FCPv2::Message &message)
{
	std::vector<std::string> idparts;
	long identityid;
	long index;
	long identityorder;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[2],index);
	StringFunctions::Convert(idparts[3],identityorder);

	RemoveFromRequestList(std::pair<long,long>(identityorder,identityid));

	// code 27 = new edition
	if(message["Code"]=="27")
	{
		std::vector<std::string> uriparts;
		StringFunctions::Split(message["RedirectURI"],"/",uriparts);

		// 0       1    2        3+
		// USK@foo/site/edition[/path]
		if(uriparts.size()>2 && uriparts[0].compare(0, 4, "USK@") == 0)
		{
			int newedition=0;
			if(StringFunctions::Convert(uriparts[2],newedition)==true)
			{
				SQLite3DB::Statement st=m_db->Prepare("UPDATE tblWOTIdentityProperty SET Value=? WHERE IdentityID=? AND Property='Sone.LatestEdition';");
				st.Bind(0,newedition);
				st.Bind(1,identityid);
				st.Step();
				st.Reset();

				// start the request again with the new edition
				StartRequest(std::pair<long,long>(identityorder,identityid));
			}
		}

	}
	else
	{
		SQLite3DB::Statement failst=m_db->Prepare("UPDATE tblIdentity SET FailureCount=FailureCount+1, SoneLastRequest=datetime('now') WHERE IdentityID=?;");
		failst.Bind(0,identityid);
		failst.Step();
	}

	m_log->trace(m_fcpuniquename+"::HandleGetFailed handled failure "+message["Code"]+" for "+message["Identifier"]);

	return true;

}

void SoneRequester::Initialize()
{
	Option option(m_db);

	m_fcpuniquename="SoneRequester";
	option.GetInt("SoneMaxRequests",m_maxrequests);
	option.Get("SoneBoardName",m_soneboardname);
	option.GetBool("LocalTrustOverridesPeerTrust",m_localtrustoverrides);
	option.GetInt("DeleteMessagesOlderThan",m_deletemessagesolderthan);

	// recent sones get 1/2, inactive sones get 1/2 + any remaining if not evenly divisible
	m_maxrequests=(m_maxrequests/2)+(m_maxrequests%2);

	if(m_maxrequests>100)
	{
		m_log->warning("Option SoneMaxRequests is currently set at more than 100.  This value might be incorrectly configured.");
	}

	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardID FROM tblBoard WHERE BoardName=?;");
	st.Bind(0,m_soneboardname);
	st.Step();
	if(st.RowReturned()==true)
	{
		st.ResultInt(0,m_soneboardid);
	}
	else
	{
		SQLite3DB::Statement inst=m_db->Prepare("INSERT INTO tblBoard(BoardName,DateAdded) VALUES(?,datetime('now'));");
		inst.Bind(0,m_soneboardname);
		inst.Step(true);
		m_soneboardid=inst.GetLastInsertRowID();
	}

}

void SoneRequester::PopulateIDList()
{
	SQLite3DB::Transaction trans(m_db);

	// only selects, deferred OK
	trans.Begin();

	std::string sql;

	sql="SELECT tblIdentity.IdentityID ";
	sql+="FROM tblIdentity INNER JOIN tblWOTIdentityContext ON tblIdentity.IdentityID=tblWOTIdentityContext.IdentityID ";
	sql+="WHERE Context='Sone' ";
	if(m_localtrustoverrides==false)
	{
		sql+="AND (tblIdentity.LocalMessageTrust IS NULL OR tblIdentity.LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust')) ";
		sql+="AND (tblIdentity.PeerMessageTrust IS NULL OR tblIdentity.PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')) ";
	}
	else
	{
		sql+="AND (tblIdentity.LocalMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalMessageTrust') OR (tblIdentity.LocalMessageTrust IS NULL AND (tblIdentity.PeerMessageTrust IS NULL OR tblIdentity.PeerMessageTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerMessageTrust')))) ";
	}
	sql+="AND tblIdentity.Name <> '' AND tblIdentity.FailureCount<=(SELECT OptionValue FROM tblOption WHERE Option='MaxFailureCount') ";
	sql+="AND (tblIdentity.SoneLastSeen IS NULL OR tblIdentity.SoneLastSeen<=datetime('now','-1 hours')) ";
	sql+="ORDER BY tblIdentity.SoneLastRequest DESC ";
	sql+=";";

	int count=0;
	std::string countstr("");
	SQLite3DB::Statement st=m_db->Prepare(sql);
	st.Step();

	m_ids.clear();

	while(st.RowReturned())
	{
		int id=0;
		st.ResultInt(0,id);
		m_ids[std::pair<long,long>(count,id)].m_requested=false;
		st.Step();
		count++;
	}

	trans.Finalize(st);
	trans.Commit();

	StringFunctions::Convert(count,countstr);
	m_log->trace(m_fcpuniquename+"::PopulateIDList populated "+countstr+" ids");

}

void SoneRequester::StartRequest(const std::pair<long,long> &inputpair)
{
	const long identityorder=inputpair.first;
	const long identityid=inputpair.second;
	std::string publickey("");
	std::string lastindexstr("");
	int index=0;
	std::string indexstr("");
	std::string identityidstr("");
	std::string identityorderstr("");
	IdentityPublicKeyCache pkcache(m_db);
	FCPv2::Message message;

	if(pkcache.PublicKey(identityid,publickey) && publickey.size()>4)
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT MAX(IFNULL(Value,0),IFNULL(tblIdentity.SoneLastIndex+1,0)), IFNULL(tblIdentity.SoneLastIndex,0) FROM tblWOTIdentityProperty INNER JOIN tblIdentity ON tblWOTIdentityProperty.IdentityID=tblIdentity.IdentityID WHERE tblIdentity.IdentityID=? AND Property='Sone.LatestEdition';");
		st.Bind(0,identityid);
		st.Step();
		if(st.RowReturned())
		{
			st.ResultInt(0,index);
			st.ResultText(1,lastindexstr);
			StringFunctions::Convert(index,indexstr);
			StringFunctions::Convert(identityid,identityidstr);
			StringFunctions::Convert(identityorder,identityorderstr);

			message.SetName("ClientGet");
			message["URI"]="USK"+publickey.substr(3)+"Sone/"+indexstr+"/sone.xml";
			message["Identifier"]=m_fcpuniquename+"|"+identityidstr+"|"+indexstr+"|"+identityorderstr+"|"+lastindexstr+"|"+message["URI"];
			message["ReturnType"]="direct";
			message["MaxSize"]="10000000";	// The sone file can be large
			m_fcp->Send(message);

			StartedRequest(inputpair,message["Identifier"]);

			m_log->trace(m_fcpuniquename+"::StartRequest started request for "+message["Identifier"]);
		}
	}

	m_ids[inputpair].m_requested=true;

}
