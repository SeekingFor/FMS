#include "../include/message.h"
#include "../include/nntp/mime/Mime.h"
#include "../include/stringfunctions.h"
#include "../include/freenet/messagexml.h"
#include "../include/option.h"

#include <Poco/DateTimeParser.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/UUID.h>
#include <algorithm>

#ifdef DO_CHARSET_CONVERSION
	#include "../include/charsetconverter.h"
#endif

#ifdef XMEM
	#include <xmem.h>
#endif

Message::Message()
{
	Initialize();
}

Message::Message(const long messageid)
{
	Load(messageid);
}

const bool Message::CheckForAdministrationBoard(const std::vector<std::string> &boards)
{
	std::string name;
	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardName FROM tblBoard INNER JOIN tblAdministrationBoard ON tblBoard.BoardID=tblAdministrationBoard.BoardID;");
	st.Step();
	
	while(st.RowReturned())
	{
		st.ResultText(0,name);

		if(std::find(boards.begin(),boards.end(),name)!=boards.end())
		{
			return true;
		}
		
		st.Step();
	}

	return false;
}

const bool Message::Create(const long localidentityid, const long boardid, const std::string &subject, const std::string &body, const std::string &references)
{
	Initialize();

	Poco::UUIDGenerator uuidgen;
	Poco::UUID uuid;

	// get header info
	// date is always set to now regardless of what message has
	m_datetime=Poco::Timestamp();

	// messageuuid is always a unique id we generate regardless of message message-id
	try
	{
		uuid=uuidgen.createRandom();
		m_messageuuid=uuid.toString();
		StringFunctions::UpperCase(m_messageuuid,m_messageuuid);
	}
	catch(...)
	{
		m_log->fatal("Message::ParseNNTPMessage could not create UUID");
	}
	
	// get from
	SQLite3DB::Statement st=m_db->Prepare("SELECT Name FROM tblLocalIdentity WHERE LocalIdentityID=?;");
	st.Bind(0,localidentityid);
	st.Step();
	if(st.RowReturned())
	{
		st.ResultText(0,m_fromname);
	}

	// get boards posted to
	std::string boardname="";
	SQLite3DB::Statement boardst=m_db->Prepare("SELECT BoardName FROM tblBoard WHERE BoardID=?;");
	boardst.Bind(0,boardid);
	boardst.Step();
	if(boardst.RowReturned())
	{
		boardst.ResultText(0,boardname);
	}

	m_boards.push_back(boardname);
	m_replyboardname=boardname;

	m_subject=subject;

	m_body=body;

	if(references!="")
	{
		m_inreplyto[0]=references;
	}

	return true;
}

const int Message::FindLocalIdentityID(const std::string &name)
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID FROM tblLocalIdentity WHERE Name=?;");
	st.Bind(0,name);
	st.Step();
	if(st.RowReturned())
	{
		int result=-1;
		st.ResultInt(0,result);
		return result;
	}
	else
	{
		if(m_addnewpostfromidentities==true)
		{
			Poco::DateTime now;
			st=m_db->Prepare("INSERT INTO tblLocalIdentity(Name,DateCreated) VALUES(?,?);");
			st.Bind(0,name);
			st.Bind(1,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
			st.Step(true);
			return st.GetLastInsertRowID();
		}
		else
		{
			return -1;
		}
	}
}

const std::string Message::GetNNTPArticleID() const
{
	// old message - before 0.1.12 - doesn't have @domain so add @freenetproject.org
	if(m_messageuuid.find("@")==std::string::npos)
	{
		return "<"+m_messageuuid+"@freenetproject.org>";
	}
	else
	{
		return "<"+m_messageuuid+">";
	}
}

const std::string Message::GetNNTPBody() const
{
	return m_body;
}

const std::string Message::GetNNTPHeaders() const
{
	std::string rval("");

	rval+="From: "+m_fromname+"\r\n";
	rval+="Newsgroups: ";
	for(std::vector<std::string>::const_iterator i=m_boards.begin(); i!=m_boards.end(); i++)
	{
		if(i!=m_boards.begin())
		{
			rval+=",";
		}
		rval+=(*i);
	}
	rval+="\r\n";
	rval+="Subject: "+m_subject+"\r\n";
	// format time as  : Wdy, DD Mon YY HH:MM:SS TIMEZONE
	rval+="Date: "+Poco::DateTimeFormatter::format(m_datetime,"%w, %d %b %y %H:%M:%S -0000")+"\r\n";
	if(m_inreplyto.size()>0)
	{
		rval+="References: ";
		for(std::map<long,std::string>::const_reverse_iterator j=m_inreplyto.rbegin(); j!=m_inreplyto.rend(); j++)
		{
			if(j!=m_inreplyto.rend())
			{
				rval+=" ";
			}
			// old message - before 0.1.12 - doesn't have @domain so add @freenetproject.org
			if((*j).second.find("@")==std::string::npos)
			{
				rval+="<"+(*j).second+"@freenetproject.org>";
			}
			else
			{
				rval+="<"+(*j).second+">";
			}
		}
		rval+="\r\n";
	}
	rval+="Followup-To: "+m_replyboardname+"\r\n";
	rval+="Path: freenet\r\n";
	rval+="Message-ID: "+GetNNTPArticleID()+"\r\n";
	rval+="Content-Type: text/plain; charset=UTF-8\r\n";

	return rval;
}

void Message::HandleAdministrationMessage()
{
	// only continue if this message was actually a reply to another message
	if(m_inreplyto.size()>0)
	{
		int localidentityid=-1;
		int boardid=0;
		std::string boardname="";
		std::string identityname="";
		int identityid;
		int changemessagetrust=0;
		int changetrustlisttrust=0;
		int origmessagetrust=0;
		int origtrustlisttrust=0;
		SQLite3DB::Statement st=m_db->Prepare("SELECT tblBoard.BoardID,BoardName,ModifyLocalMessageTrust,ModifyLocalTrustListTrust FROM tblBoard INNER JOIN tblAdministrationBoard ON tblBoard.BoardID=tblAdministrationBoard.BoardID;");
		st.Step();

		localidentityid=FindLocalIdentityID(m_fromname);

		while(st.RowReturned() && localidentityid!=-1)
		{
			st.ResultInt(0,boardid);
			st.ResultText(1,boardname);
			st.ResultInt(2,changemessagetrust);
			st.ResultInt(3,changetrustlisttrust);

			if(std::find(m_boards.begin(),m_boards.end(),boardname)!=m_boards.end())
			{
				SQLite3DB::Statement origmess=m_db->Prepare("SELECT tblIdentity.IdentityID,tblIdentity.Name,tblIdentityTrust.LocalMessageTrust,tblIdentityTrust.LocalTrustListTrust FROM tblIdentity INNER JOIN tblMessage ON tblIdentity.IdentityID=tblMessage.IdentityID LEFT JOIN (SELECT IdentityID,LocalMessageTrust,LocalTrustListTrust FROM tblIdentityTrust WHERE LocalIdentityID=?) AS 'tblIdentityTrust' ON tblIdentity.IdentityID=tblIdentityTrust.IdentityID WHERE tblMessage.MessageUUID=?;");
				origmess.Bind(0,localidentityid);
				origmess.Bind(1,m_inreplyto[0]);
				origmess.Step();

				if(origmess.RowReturned())
				{
					origmess.ResultInt(0,identityid);
					origmess.ResultText(1,identityname);
					if(origmess.ResultNull(2)==false)
					{
						origmess.ResultInt(2,origmessagetrust);
					}
					else
					{
						//origmessagetrust=m_minlocalmessagetrust;
						origmessagetrust=50;
					}
					if(origmess.ResultNull(3)==false)
					{
						origmess.ResultInt(3,origtrustlisttrust);
					}
					else
					{
						//origtrustlisttrust=m_minlocaltrustlisttrust;
						origtrustlisttrust=50;
					}

					origmessagetrust+=changemessagetrust;
					origtrustlisttrust+=changetrustlisttrust;

					origmessagetrust<0 ? origmessagetrust=0 : false;
					origmessagetrust>100 ? origmessagetrust=100 : false;
					origtrustlisttrust<0 ? origtrustlisttrust=0 : false;
					origtrustlisttrust>100 ? origtrustlisttrust=100 : false;

					// make sure we have a record in tblIdentityTrust
					SQLite3DB::Statement ins=m_db->Prepare("INSERT INTO tblIdentityTrust(LocalIdentityID,IdentityID) VALUES(?,?);");
					ins.Bind(0,localidentityid);
					ins.Bind(1,identityid);
					ins.Step();

					// update new trust levels
					SQLite3DB::Statement update=m_db->Prepare("UPDATE tblIdentityTrust SET LocalMessageTrust=?, LocalTrustListTrust=? WHERE IdentityID=? AND LocalIdentityID=?;");
					update.Bind(0,origmessagetrust);
					update.Bind(1,origtrustlisttrust);
					update.Bind(2,identityid);
					update.Bind(3,localidentityid);
					update.Step();

					// insert message to show what id was changed and what current levels are
					int lastid=0;
					std::string messagebody;
					std::string messagetruststr="";
					std::string trustlisttruststr="";

					Poco::UUIDGenerator uuidgen;
					Poco::UUID uuid;

					try
					{
						uuid=uuidgen.createRandom();
					}
					catch(...)
					{
						m_log->fatal("Message::HandleAdministrationMessage could not generate a UUID");
					}

					Poco::DateTime now;
					StringFunctions::Convert(origmessagetrust,messagetruststr);
					StringFunctions::Convert(origtrustlisttrust,trustlisttruststr);
					messagebody="Trust List of "+m_fromname+"\r\n";
					messagebody="Trust Changed for "+identityname+"\r\n";
					messagebody+="Local Message Trust : "+messagetruststr+"\r\n";
					messagebody+="Local Trust List Trust : "+trustlisttruststr+"\r\n";
					SQLite3DB::Statement insert=m_db->Prepare("INSERT INTO tblMessage(FromName,MessageDate,MessageTime,Subject,MessageUUID,ReplyBoardID,Body) VALUES('FMS',?,?,?,?,?,?);");
					insert.Bind(0,Poco::DateTimeFormatter::format(now,"%Y-%m-%d"));
					insert.Bind(1,Poco::DateTimeFormatter::format(now,"%H:%M:%S"));
					insert.Bind(2,identityname+" Trust Changed");
					std::string uuidstr=uuid.toString();
					StringFunctions::UpperCase(uuidstr,uuidstr);
					insert.Bind(3,uuidstr);
					insert.Bind(4,boardid);
					insert.Bind(5,messagebody);
					insert.Step(true);
					lastid=insert.GetLastInsertRowID();

					insert=m_db->Prepare("INSERT INTO tblMessageBoard(MessageID,BoardID) VALUES(?,?);");
					insert.Bind(0,lastid);
					insert.Bind(1,boardid);
					insert.Step();

					m_log->debug("Message::HandleAdministrationMessage updated "+identityname+" to "+messagetruststr+" , "+trustlisttruststr);

				}
			}

			st.Step();
		}
	}

}

void Message::HandleChangeTrust()
{
	if(m_changemessagetrustonreply!=0 && m_inreplyto.size()>0)
	{
		int localidentityid=FindLocalIdentityID(m_fromname);
		if(localidentityid!=-1)
		{
			// make sure we have a record in tblIdentityTrust
			SQLite3DB::Statement ins=m_db->Prepare("INSERT INTO tblIdentityTrust(LocalIdentityID,IdentityID) VALUES(?,?);");

			SQLite3DB::Statement st=m_db->Prepare("SELECT tblIdentity.IdentityID,tblIdentityTrust.LocalMessageTrust FROM tblIdentity INNER JOIN tblMessage ON tblIdentity.IdentityID=tblMessage.IdentityID LEFT JOIN (SELECT IdentityID,LocalMessageTrust FROM tblIdentityTrust WHERE LocalIdentityID=?) AS 'tblIdentityTrust' ON tblIdentity.IdentityID=tblIdentityTrust.IdentityID WHERE tblMessage.MessageUUID=?;");
			st.Bind(0,localidentityid);
			st.Bind(1,m_inreplyto[0]);
			st.Step();
			if(st.RowReturned())
			{
				int identityid=0;
				int localmessagetrust=0;

				st.ResultInt(0,identityid);
				if(st.ResultNull(1)==false)
				{
					st.ResultInt(1,localmessagetrust);
				}
				else
				{
					//localmessagetrust=m_minlocalmessagetrust;
					localmessagetrust=50;
				}

				localmessagetrust+=m_changemessagetrustonreply;
				if(localmessagetrust<0)
				{
					localmessagetrust=0;
				}
				if(localmessagetrust>100)
				{
					localmessagetrust=100;
				}

				ins.Bind(0,localidentityid);
				ins.Bind(1,identityid);
				ins.Step();

				SQLite3DB::Statement st2=m_db->Prepare("UPDATE tblIdentityTrust SET LocalMessageTrust=? WHERE IdentityID=? AND LocalIdentityID=?;");
				st2.Bind(0,localmessagetrust);
				st2.Bind(1,identityid);
				st2.Bind(2,localidentityid);
				st2.Step();

			}
		}
	}
}

void Message::Initialize()
{
	std::string tempval="";
	m_messageid=-1;
	m_messageuuid="";
	m_subject="";
	m_body="";
	m_replyboardname="";
	m_datetime=Poco::Timestamp();
	m_fromname="";
	m_boards.clear();
	m_inreplyto.clear();
	m_fileattachments.clear();
	m_changemessagetrustonreply=0;
	Option::Instance()->Get("ChangeMessageTrustOnReply",tempval);
	StringFunctions::Convert(tempval,m_changemessagetrustonreply);
	Option::Instance()->Get("AddNewPostFromIdentities",tempval);
	if(tempval=="true")
	{
		m_addnewpostfromidentities=true;
	}
	else
	{
		m_addnewpostfromidentities=false;
	}
	tempval="50";
	Option::Instance()->Get("MinLocalMessageTrust",tempval);
	StringFunctions::Convert(tempval,m_minlocalmessagetrust);
	tempval="51";
	Option::Instance()->Get("MinLocalTrustListTrust",tempval);
	StringFunctions::Convert(tempval,m_minlocaltrustlisttrust);
}

const bool Message::Load(const long messageid, const long boardid)
{
	
	Initialize();

	std::string sql;
	
	sql="SELECT tblMessage.MessageID, MessageUUID, Subject, Body, tblBoard.BoardName, MessageDate, MessageTime, FromName FROM tblMessage INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID INNER JOIN tblBoard ON tblMessage.ReplyBoardID=tblBoard.BoardID WHERE tblMessage.MessageID=?";
	if(boardid!=-1)
	{
		sql+=" AND tblMessageBoard.BoardID=?";
	}
	sql+=";";

	SQLite3DB::Statement st=m_db->Prepare(sql);
	st.Bind(0,messageid);
	if(boardid!=-1)
	{
		st.Bind(1,boardid);
	}
	st.Step();

	if(st.RowReturned())
	{
		std::string tempdate;
		std::string temptime;
		int tempint=-1;
		st.ResultInt(0,tempint);
		m_messageid=tempint;
		st.ResultText(1,m_messageuuid);
		st.ResultText(2,m_subject);
		st.ResultText(3,m_body);
		st.ResultText(4,m_replyboardname);
		st.ResultText(5,tempdate);
		st.ResultText(6,temptime);
		st.ResultText(7,m_fromname);
		st.Finalize();

		int tzdiff=0;
		if(Poco::DateTimeParser::tryParse(tempdate + " " + temptime,m_datetime,tzdiff)==false)
		{
			m_log->error("Message::Load couldn't parse date/time "+tempdate+" "+temptime);
		}

		// strip off any \r\n in subject
		m_subject=StringFunctions::Replace(m_subject,"\r\n","");

		// get board list
		st=m_db->Prepare("SELECT tblBoard.BoardName FROM tblBoard INNER JOIN tblMessageBoard ON tblBoard.BoardID=tblMessageBoard.BoardID WHERE tblMessageBoard.MessageID=?;");
		st.Bind(0,messageid);
		st.Step();
		while(st.RowReturned())
		{
			std::string tempval;
			st.ResultText(0,tempval);
			m_boards.push_back(tempval);
			st.Step();
		}
		st.Finalize();

		// get in reply to list
		st=m_db->Prepare("SELECT ReplyToMessageUUID, ReplyOrder FROM tblMessageReplyTo INNER JOIN tblMessage ON tblMessageReplyTo.MessageID=tblMessage.MessageID WHERE tblMessage.MessageID=?;");
		st.Bind(0,messageid);
		st.Step();
		while(st.RowReturned())
		{
			std::string tempval;
			int tempint;
			st.ResultText(0,tempval);
			st.ResultInt(1,tempint);
			m_inreplyto[tempint]=tempval;
			st.Step();
		}
		st.Finalize();

		return true;
	}
	else
	{
		return false;
	}

}

const bool Message::Load(const std::string &messageuuid)
{

	std::string uuid=messageuuid;

	if(uuid.size()>0 && uuid[0]=='<')
	{
		uuid.erase(0,1);
	}
	if(uuid.size()>0 && uuid[uuid.size()-1]=='>')
	{
		uuid.erase(uuid.size()-1);
	}
	if(uuid.find("@freenetproject.org")!=std::string::npos)
	{
		uuid.erase(uuid.find("@freenetproject.org"));
	}

	SQLite3DB::Statement st=m_db->Prepare("SELECT MessageID FROM tblMessage WHERE MessageUUID=?;");
	st.Bind(0,uuid);
	st.Step();

	if(st.RowReturned())
	{
		int messageid;
		st.ResultInt(0,messageid);

		return Load(messageid);
	}
	else
	{
		return false;
	}
}

const bool Message::LoadNext(const long messageid, const long boardid)
{
	std::string sql="SELECT tblMessage.MessageID FROM tblMessage INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID WHERE tblMessage.MessageID>?";
	if(boardid!=-1)
	{
		sql+=" AND tblMessageBoard.BoardID=?";
	}
	sql+=";";

	SQLite3DB::Statement st=m_db->Prepare(sql);

	st.Bind(0,messageid);
	if(boardid!=-1)
	{
		st.Bind(1,boardid);
	}
	st.Step();

	if(st.RowReturned())
	{
		int result;
		st.ResultInt(0,result);
		return Load(result,boardid);
	}
	else
	{
		return false;
	}
}

const bool Message::LoadPrevious(const long messageid, const long boardid)
{
	std::string sql="SELECT tblMessage.MessageID FROM tblMessage INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID WHERE tblMessage.MessageID<?";
	if(boardid!=-1)
	{
		sql+=" AND tblMessageBoard.BoardID=?";
	}
	sql+=" ORDER BY tblMessage.MessageID DESC;";

	SQLite3DB::Statement st=m_db->Prepare(sql);

	st.Bind(0,messageid);
	if(boardid!=-1)
	{
		st.Bind(1,boardid);
	}
	st.Step();

	if(st.RowReturned())
	{
		int result;
		st.ResultInt(0,result);
		return Load(result,boardid);
	}
	else
	{
		return false;
	}
}

const bool Message::ParseNNTPMessage(const std::string &nntpmessage)
{

	Initialize();

	Poco::UUIDGenerator uuidgen;
	Poco::UUID uuid;
	CMimeMessage mime;
	mime.Load(nntpmessage.c_str(),nntpmessage.size());

	// get header info
	// date is always set to now regardless of what message has
	m_datetime=Poco::Timestamp();

	// messageuuid is always a unique id we generate regardless of message message-id
	try
	{
		uuid=uuidgen.createRandom();
		m_messageuuid=uuid.toString();
		StringFunctions::UpperCase(m_messageuuid,m_messageuuid);
	}
	catch(...)
	{
		m_log->fatal("Message::ParseNNTPMessage could not create UUID");
	}
	
	// get from
	if(mime.GetFieldValue("From"))
	{
		m_fromname=mime.GetFieldValue("From");
		// remove any path folding
		m_fromname=StringFunctions::Replace(m_fromname,"\r\n","");
		m_fromname=StringFunctions::Replace(m_fromname,"\t","");
		// strip off everything between () and <> and any whitespace
		std::string::size_type startpos=m_fromname.find("(");
		std::string::size_type endpos;
		if(startpos!=std::string::npos)
		{
			endpos=m_fromname.find(")",startpos);
			if(endpos!=std::string::npos)
			{
				m_fromname.erase(startpos,(endpos-startpos)+1);
			}
		}
		startpos=m_fromname.find("<");
		if(startpos!=std::string::npos)
		{
			endpos=m_fromname.find(">",startpos);
			if(endpos!=std::string::npos)
			{
				m_fromname.erase(startpos,(endpos-startpos)+1);
			}
		}
		m_fromname=StringFunctions::TrimWhitespace(m_fromname);

		// trim off " from beginning and end
		if(m_fromname.size()>0 && m_fromname[0]=='\"')
		{
			m_fromname.erase(0,1);
		}
		if(m_fromname.size()>0 && m_fromname[m_fromname.size()-1]=='\"')
		{
			m_fromname.erase(m_fromname.size()-1,1);
		}

		m_fromname=StringFunctions::TrimWhitespace(m_fromname);
	}
	else
	{
		m_fromname="Anonymous";
	}
	// get boards posted to
	if(mime.GetFieldValue("Newsgroups"))
	{
		std::string temp=mime.GetFieldValue("Newsgroups");
		// remove any path folding
		temp=StringFunctions::Replace(temp,"\r\n","");
		temp=StringFunctions::Replace(temp,"\t","");
		std::vector<std::string> parts;
		StringFunctions::SplitMultiple(temp,", \t",parts);
		for(std::vector<std::string>::iterator i=parts.begin(); i!=parts.end(); i++)
		{
			(*i)=StringFunctions::Replace((*i),"<","");
			(*i)=StringFunctions::Replace((*i),">","");
			(*i)=StringFunctions::TrimWhitespace((*i));
			if((*i)!="")
			{
				m_boards.push_back((*i));
			}
		}
	}
	// followup-to board - must be done after board vector populated
	if(mime.GetFieldValue("Followup-To"))
	{
		m_replyboardname=mime.GetFieldValue("Followup-To");
		// remove any path folding
		m_replyboardname=StringFunctions::Replace(m_replyboardname,"\r\n","");
		m_replyboardname=StringFunctions::Replace(m_replyboardname,"\t","");
		std::vector<std::string> parts;
		StringFunctions::Split(m_replyboardname,",",parts);
		if(parts.size()>1)
		{
			m_replyboardname=parts[0];
		}
	}
	else
	{
		if(m_boards.size()>0)
		{
			m_replyboardname=m_boards[0];
		}
	}
	// subject
	if(mime.GetFieldValue("Subject"))
	{
		m_subject=mime.GetFieldValue("Subject");
		// remove any path folding
		m_subject=StringFunctions::Replace(m_subject,"\r\n","");
		m_subject=StringFunctions::Replace(m_subject,"\t","");
#if DO_CHARSET_CONVERSION
		if(mime.GetFieldCharset("Subject"))
		{
			std::string charset=mime.GetFieldCharset("Subject");
			CharsetConverter ccv;
			if(charset!="" && charset!="UTF-8" && ccv.SetConversion(charset,"UTF-8"))
			{
				std::string output="";
				ccv.Convert(m_subject,output);
				m_subject=output;
			}
		}
#endif
	}
	else
	{
		m_subject="No Subject";
	}
	// references
	if(mime.GetFieldValue("References"))
	{
		std::string temp=mime.GetFieldValue("References");
		// remove any path folding
		temp=StringFunctions::Replace(temp,"\r\n","");
		temp=StringFunctions::Replace(temp,"\t"," ");
		std::vector<std::string> parts;
		int count=0;
		StringFunctions::SplitMultiple(temp,", \t",parts);
		for(std::vector<std::string>::reverse_iterator i=parts.rbegin(); i!=parts.rend(); i++)
		{
			if((*i).size()>2)
			{
				// get rid of < and > and any whitespace
				(*i)=StringFunctions::Replace((*i),"<","");
				(*i)=StringFunctions::Replace((*i),">","");
				(*i)=StringFunctions::TrimWhitespace((*i));
				/*
				// erase @ and everything after
				if((*i).find("@")!=std::string::npos)
				{
					(*i).erase((*i).find("@"));
				}
				*/
				// only erase after @ if message is old type with @freenetproject.org
				if((*i).find("@freenetproject.org")!=std::string::npos)
				{
					(*i).erase((*i).find("@"));
				}
				if((*i)!="")
				{
					m_inreplyto[count++]=(*i);
				}
			}
		}
	}

	CMimeBody::CBodyList mbl;
	mime.GetBodyPartList(mbl);

	// append all text parts of nntp message to body
	for(CMimeBody::CBodyList::iterator i=mbl.begin(); i!=mbl.end(); i++)
	{
		if((*i)->IsText() && (*i)->GetContent())
		{
			std::string bodypart=(char *)(*i)->GetContent();
#ifdef DO_CHARSET_CONVERSION
			std::string charset=(*i)->GetCharset();
			if(charset!="" && charset!="UTF-8")
			{
				CharsetConverter ccv;
				if(ccv.SetConversion(charset,"UTF-8"))
				{
					std::string output="";
					ccv.Convert(bodypart,output);
					bodypart=output;
				}
			}
#endif
			m_body+=bodypart;
		}
		// add a binary file attachment
		else if(((*i)->GetName()!="" || (*i)->GetFilename()!="") && (*i)->GetLength()>0 && (*i)->GetContent())
		{
			std::string filename="";
			std::string contenttype="";
			std::vector<unsigned char> data((*i)->GetContent(),(*i)->GetContent()+(*i)->GetContentLength());
			if((*i)->GetContentType())
			{
				contenttype=(*i)->GetContentType();
				// find first ; tab cr or lf and erase it and everything after it
				std::string::size_type endpos=contenttype.find_first_of(";\t\r\n ");
				if(endpos!=std::string::npos)
				{
					contenttype.erase(endpos);
				}
			}
			filename=(*i)->GetFilename();
			if(filename=="")
			{
				filename=(*i)->GetName();
			}
			m_fileattachments.push_back(fileattachment(filename,contenttype,data));
		}
	}

	return true;
}

const bool Message::StartFreenetInsert()
{

	MessageXML xml;
	int localidentityid=-1;

	StripAdministrationBoards();

	if(m_boards.size()>0)
	{

		xml.SetMessageID(m_messageuuid);
		xml.SetSubject(m_subject);
		xml.SetBody(m_body);
		xml.SetReplyBoard(m_replyboardname);
		
		for(std::vector<std::string>::iterator i=m_boards.begin(); i!=m_boards.end(); i++)
		{
			xml.AddBoard((*i));
		}
		
		for(std::map<long,std::string>::iterator j=m_inreplyto.begin(); j!=m_inreplyto.end(); j++)
		{
			xml.AddInReplyTo((*j).first,(*j).second);
		}

		localidentityid=FindLocalIdentityID(m_fromname);
		if(localidentityid==-1)
		{
			return false;
		}

		// add the message delay if there is one
		SQLite3DB::Statement st=m_db->Prepare("SELECT MinMessageDelay,MaxMessageDelay FROM tblLocalIdentity WHERE LocalIdentityID=?;");
		st.Bind(0,localidentityid);
		st.Step();
		if(st.RowReturned())
		{
			int min=0;
			int max=0;
			st.ResultInt(0,min);
			st.ResultInt(1,max);

			min<0 ? min=0 : false;
			max<0 ? max=0 : false;
			min>max ? min=max : false;

			if(min==max)
			{
				m_datetime+=Poco::Timespan(0,0,min,0,0);
			}
			else if(max>min)
			{
				int delay=(rand()%(max-min))+min;
				m_datetime+=Poco::Timespan(0,0,delay,0,0);
			}

		}
		st.Finalize();

		// set date in xml file AFTER we set the delay
		xml.SetDate(Poco::DateTimeFormatter::format(m_datetime,"%Y-%m-%d"));
		xml.SetTime(Poco::DateTimeFormatter::format(m_datetime,"%H:%M:%S"));

		st=m_db->Prepare("INSERT INTO tblMessageInserts(LocalIdentityID,MessageUUID,MessageXML,SendDate) VALUES(?,?,?,?);");
		st.Bind(0,localidentityid);
		st.Bind(1,m_messageuuid);
		st.Bind(2,xml.GetXML());
		st.Bind(3,Poco::DateTimeFormatter::format(m_datetime,"%Y-%m-%d %H:%M:%S"));
		st.Step();

		// insert file attachments into database
		st=m_db->Prepare("INSERT INTO tblFileInserts(MessageUUID,FileName,Size,MimeType,Data) VALUES(?,?,?,?,?);");
		for(std::vector<fileattachment>::iterator i=m_fileattachments.begin(); i!=m_fileattachments.end(); i++)
		{
			st.Bind(0,m_messageuuid);
			st.Bind(1,(*i).m_filename);
			st.Bind(2,(long)(*i).m_data.size());
			st.Bind(3,(*i).m_mimetype);
			st.Bind(4,&((*i).m_data[0]),(*i).m_data.size());
			st.Step();
			st.Reset();
		}

		HandleChangeTrust();

	}

	return true;

}

void Message::StripAdministrationBoards()
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT tblBoard.BoardID FROM tblBoard INNER JOIN tblAdministrationBoard ON tblBoard.BoardID=tblAdministrationBoard.BoardID WHERE BoardName=?;");
	for(std::vector<std::string>::iterator i=m_boards.begin(); i!=m_boards.end(); )
	{
		st.Bind(0,(*i));
		st.Step();
		if(st.RowReturned())
		{
			if(m_replyboardname==(*i))
			{
				m_replyboardname="";
			}
			i=m_boards.erase(i);
		}
		else
		{
			i++;
		}
		st.Reset();
	}
	if(m_replyboardname=="" && m_boards.begin()!=m_boards.end())
	{
		m_replyboardname=(*m_boards.begin());
	}
}
