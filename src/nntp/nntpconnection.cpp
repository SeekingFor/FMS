#include "../../include/nntp/nntpconnection.h"
#include "../../include/nntp/uwildmat.h"
#include "../../include/stringfunctions.h"
#include "../../include/boardlist.h"
#include "../../include/message.h"
#include "../../include/messagelist.h"
#include "../../include/option.h"
#include "../../include/nntp/extensiontrust.h"
#include "../../include/threadwrapper/cancelablethread.h"

#include <algorithm>
#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/Timestamp.h>

#ifdef XMEM
	#include <xmem.h>
#endif

NNTPConnection::NNTPConnection(SOCKET sock)
{
	std::string tempval;

	m_socket=sock;
	m_tempbuffer.resize(32768);
	
	m_status.m_isposting=false;
	m_status.m_allowpost=false;
	m_status.m_boardid=-1;
	m_status.m_messageid=-1;
	m_status.m_mode=MODE_NONE;
	m_status.m_authenticated=false;

	Option::Instance()->Get("NNTPAllowPost",tempval);
	if(tempval=="true")
	{
		m_status.m_allowpost=true;
	}

}

NNTPConnection::~NNTPConnection()
{

}

void NNTPConnection::Disconnect()
{
	if(m_socket!=INVALID_SOCKET)
	{
	#ifdef _WIN32
		closesocket(m_socket);
	#else
		close(m_socket);
	#endif
		m_socket=INVALID_SOCKET;
	}
}

std::vector<char>::iterator NNTPConnection::Find(std::vector<char> &buffer, const std::string &val)
{
	return std::search(buffer.begin(),buffer.end(),val.begin(),val.end());
}

const bool NNTPConnection::HandleArticleCommand(const NNTPCommand &command)
{

	SendArticleParts(command);
	
	return true;
}

const bool NNTPConnection::HandleAuthInfoCommand(const NNTPCommand &command)
{
	if(command.m_arguments.size()<2)
	{
		SendBufferedLine("501 Syntax error");
	}
	else if(m_status.m_authenticated==true)
	{
		SendBufferedLine("502 Command unavailable");		// not available when already authenticated
	}
	else
	{
		std::string arg=command.m_arguments[0];
		StringFunctions::UpperCase(arg,arg);
		std::string name="";
		// get remaining args as part of the name since a name might have a space and the args are split on spaces
		for(std::vector<std::string>::const_iterator i=command.m_arguments.begin()+1; i!=command.m_arguments.end(); i++)
		{
			// we split on the space, so add it back
			if(i!=command.m_arguments.begin()+1)
			{
				name+=" ";
			}	
			name+=(*i);
		}
		if(arg=="USER")
		{
			LocalIdentity localid;
			if(localid.Load(name))
			{
				m_status.m_authuser=localid;
				m_status.m_authenticated=true;
				SendBufferedLine("281 Authentication accepted");
			}
			else
			{
				SendBufferedLine("481 Authentication failed");
			}
		}
		else if(arg=="PASS")
		{
			SendBufferedLine("482 Authentication commands issued out of sequence");	// only require username
		}
		else
		{
			SendBufferedLine("501 Syntax error");
		}
	}

	return true;
}

const bool NNTPConnection::HandleBodyCommand(const NNTPCommand &command)
{
	SendArticleParts(command);

	return true;
}

const bool NNTPConnection::HandleCapabilitiesCommand(const NNTPCommand &command)
{
	
	SendBufferedLine("101 Capability list :");
	SendBufferedLine("VERSION 2");
	if(m_status.m_authenticated==false)		// RFC 4643 2.2 0 - don't advertise MODE-READER after authentication
	{
		SendBufferedLine("MODE-READER");
	}
	SendBufferedLine("READER");
	SendBufferedLine("LIST OVERVIEW.FMT");
	SendBufferedLine("OVER MSGID");
	if(m_status.m_allowpost==true)
	{
		SendBufferedLine("POST");
	}
	if(m_status.m_authenticated==false)
	{
		SendBufferedLine("AUTHINFO USER");
	}
	SendBufferedLine("XFMSTRUST");
	SendBufferedLine(".");
	
	return true;
}

const bool NNTPConnection::HandleCommand(const NNTPCommand &command)
{
	if(command.m_command=="QUIT")
	{
		return HandleQuitCommand(command);
	}
	if(command.m_command=="MODE")
	{
		return HandleModeCommand(command);
	}
	if(command.m_command=="CAPABILITIES")
	{
		return HandleCapabilitiesCommand(command);
	}
	if(command.m_command=="HELP")
	{
		return HandleHelpCommand(command);
	}
	if(command.m_command=="DATE")
	{
		return HandleDateCommand(command);
	}
	if(command.m_command=="LIST")
	{
		return HandleListCommand(command);
	}
	if(command.m_command=="GROUP")
	{
		return HandleGroupCommand(command);
	}
	if(command.m_command=="LISTGROUP")
	{
		return HandleListGroupCommand(command);
	}
	if(command.m_command=="LAST")
	{
		return HandleLastCommand(command);
	}
	if(command.m_command=="NEXT")
	{
		return HandleNextCommand(command);
	}
	if(command.m_command=="ARTICLE")
	{
		return HandleArticleCommand(command);
	}
	if(command.m_command=="HEAD")
	{
		return HandleHeadCommand(command);
	}
	if(command.m_command=="BODY")
	{
		return HandleBodyCommand(command);
	}
	if(command.m_command=="STAT")
	{
		return HandleStatCommand(command);
	}
	if(command.m_command=="NEWGROUPS")
	{
		return HandleNewGroupsCommand(command);
	}
	if(command.m_command=="POST")
	{
		return HandlePostCommand(command);
	}
	if(command.m_command=="OVER" || command.m_command=="XOVER")
	{
		return HandleOverCommand(command);
	}
	if(command.m_command=="AUTHINFO")
	{
		return HandleAuthInfoCommand(command);
	}
	if(command.m_command=="XGETTRUST")
	{
		return HandleGetTrustCommand(command);
	}
	if(command.m_command=="XSETTRUST")
	{
		return HandleSetTrustCommand(command);
	}
	if(command.m_command=="XGETTRUSTLIST")
	{
		return HandleGetTrustListCommand(command);
	}

	return false;
}

const bool NNTPConnection::HandleDateCommand(const NNTPCommand &command)
{
	Poco::DateTime now;
	SendBufferedLine("111 "+Poco::DateTimeFormatter::format(now,"%Y%m%d%H%M%S"));
	return true;
}

const bool NNTPConnection::HandleGetTrustCommand(const NNTPCommand &command)
{
	if(command.m_arguments.size()>=2)
	{
		std::string type=command.m_arguments[0];
		StringFunctions::UpperCase(type,type);
		if(type=="MESSAGE" || type=="TRUSTLIST" || type=="PEERMESSAGE" || type=="PEERTRUSTLIST")
		{
			if(m_status.m_authenticated)
			{
				bool found=false;
				int trust=-1;
				std::string nntpname="";
				for(int i=1; i<command.m_arguments.size(); i++)
				{
					if(i!=1)
					{
						nntpname+=" ";
					}
					nntpname+=command.m_arguments[i];
				}

				TrustExtension tr(m_status.m_authuser.GetID());

				if(type=="MESSAGE")
				{
					if(tr.GetMessageTrust(nntpname,trust))
					{
						found=true;
					}
				}
				else if(type=="TRUSTLIST")
				{
					if(tr.GetTrustListTrust(nntpname,trust))
					{
						found=true;
					}
				}
				else if(type=="PEERMESSAGE")
				{
					if(tr.GetPeerMessageTrust(nntpname,trust))
					{
						found=true;
					}
				}
				else if(type=="PEERTRUSTLIST")
				{
					if(tr.GetPeerTrustListTrust(nntpname,trust))
					{
						found=true;
					}
				}

				if(trust>=0 && found)
				{
					std::string truststr="";
					StringFunctions::Convert(trust,truststr);
					SendBufferedLine("280 "+truststr);
				}
				else if(found)
				{
					SendBufferedLine("281 null");
				}
				else
				{
					SendBufferedLine("480 Identity not found");
				}

			}
			else
			{
				SendBufferedLine("480 User not authenticated");
			}
		}
		else
		{
			SendBufferedLine("501 Syntax error");
		}
	}
	else
	{
		SendBufferedLine("501 Syntax error");
	}
	return true;
}	

const bool NNTPConnection::HandleGetTrustListCommand(const NNTPCommand &command)
{
	if(m_status.m_authenticated)
	{
		TrustExtension tr(m_status.m_authuser.GetID());
		std::map<std::string,TrustExtension::trust> trustlist;
		if(tr.GetTrustList(trustlist))
		{
			SendBufferedLine("280 Trust list follows");
			for(std::map<std::string,TrustExtension::trust>::iterator i=trustlist.begin(); i!=trustlist.end(); i++)
			{
				std::ostringstream tempstr;
				tempstr << (*i).first << "\t";
				if((*i).second.m_localmessagetrust>-1)
				{
					tempstr << (*i).second.m_localmessagetrust;
				} 
				else
				{
					tempstr << "null";
				}
				tempstr << "\t";
				if((*i).second.m_localtrustlisttrust>-1)
				{
					tempstr << (*i).second.m_localtrustlisttrust;
				}
				else
				{
					tempstr << "null";
				}
				tempstr << "\t";
				if((*i).second.m_peermessagetrust>-1)
				{
					tempstr << (*i).second.m_peermessagetrust;
				}
				else
				{
					tempstr << "null";
				}
				tempstr << "\t";
				if((*i).second.m_peertrustlisttrust>-1)
				{
					tempstr << (*i).second.m_peertrustlisttrust;
				}
				else
				{
					tempstr << "null";
				}
				tempstr << "\t";
				tempstr << (*i).second.m_messagetrustcomment;
				tempstr << "\t";
				tempstr << (*i).second.m_trustlisttrustcomment;

				SendBufferedLine(tempstr.str());
			}
			SendBufferedLine(".");
		}
		else
		{
			SendBufferedLine("501 Syntax error");
		}
	}
	else
	{
		SendBufferedLine("480 User not authenticated");
	}
	return true;
}

const bool NNTPConnection::HandleGroupCommand(const NNTPCommand &command)
{
	if(command.m_arguments.size()==1)
	{
		Board board;
		if(board.Load(command.m_arguments[0])==true)
		{
			std::ostringstream tempstr;

			tempstr << "211 " << board.GetMessageCount() << " " << board.GetLowMessageID() << " " << board.GetHighMessageID() << " " << board.GetBoardName();

			SendBufferedLine(tempstr.str());

			// set the current boardid to this one
			m_status.m_boardid=board.GetBoardID();
			//set the first message id, -1 if there are no messages
			board.GetLowMessageID()!=0 ? m_status.m_messageid=board.GetLowMessageID() : m_status.m_messageid=-1;

		}
		else
		{
			SendBufferedLine("411 No such newsgroup");
		}
	}
	else
	{
		SendBufferedLine("501 Syntax error");
		m_log->debug("NNTPConnection::HandleGroupCommand syntax error");
	}

	return true;
}

const bool NNTPConnection::HandleHeadCommand(const NNTPCommand &command)
{
	
	SendArticleParts(command);

	return true;
}

const bool NNTPConnection::HandleHelpCommand(const NNTPCommand &command)
{
	SendBufferedLine("100 Help text follows");
	SendBufferedLine("There is no help text");
	SendBufferedLine(".");

	return true;
}

const bool NNTPConnection::HandleLastCommand(const NNTPCommand &command)
{
	if(m_status.m_boardid!=-1)
	{
		if(m_status.m_messageid!=-1)
		{
			Message mess;

			if(mess.LoadPrevious(m_status.m_messageid,m_status.m_boardid))
			{
				std::ostringstream tempstr;

				m_status.m_messageid=mess.GetMessageID();

				tempstr << "223 " << mess.GetMessageID() << " " << mess.GetNNTPArticleID();

				SendBufferedLine(tempstr.str());

			}
			else
			{
				SendBufferedLine("422 No previous article in this group");
			}
		}
		else
		{
			SendBufferedLine("420 Current article number is invalid");
		}
	}
	else
	{
		SendBufferedLine("412 No newsgroup selected");
	}

	return true;
}

const bool NNTPConnection::HandleListCommand(const NNTPCommand &command)
{

	int type=1;	// default LIST type is active
	std::string arg1="";
	std::string arg2="";

	// type of LIST
	if(command.m_arguments.size()>0)
	{
		StringFunctions::UpperCase(command.m_arguments[0],arg1);
		if(arg1=="ACTIVE")
		{
			type=1;
		}
		else if(arg1=="NEWSGROUPS")
		{
			type=2;
		}
		else if(arg1=="OVERVIEW.FMT")
		{
			type=3;
		}
		else
		{
			type=0;
		}
	}
	// wildmat
	if(command.m_arguments.size()>1)
	{
		arg2=command.m_arguments[1];
	}

	// LIST ACTIVE [wildmat]
	if(type==1)
	{
		bool show;
		std::ostringstream tempstr;
		BoardList bl;
		bl.Load();

		SendBufferedLine("215 list of newsgroups follows");

		for(BoardList::iterator i=bl.begin(); i!=bl.end(); i++)
		{
			show=true;
			tempstr.str("");

			// check wilmat match
			if(arg2!="")
			{
				show=uwildmat((*i).GetBoardName().c_str(),arg2.c_str());
			}

			if(show==true && (*i).GetSaveReceivedMessages()==true)
			{
				tempstr << (*i).GetBoardName() << " " << (*i).GetHighMessageID() << " " << (*i).GetLowMessageID() << " " << (m_status.m_allowpost ? "y" : "n");
				SendBufferedLine(tempstr.str());
			}
		}

		SendBufferedLine(".");

	}
	// LIST NEWSGROUPS
	else if(type==2)
	{
		bool show;
		std::ostringstream tempstr;
		BoardList bl;
		bl.Load();

		SendBufferedLine("215 list of newsgroups follows");

		for(BoardList::iterator i=bl.begin(); i!=bl.end(); i++)
		{
			show=true;
			tempstr.str("");

			// check wilmat match
			if(arg2!="")
			{
				show=uwildmat((*i).GetBoardName().c_str(),arg2.c_str());
			}

			if(show==true && (*i).GetSaveReceivedMessages()==true)
			{
				tempstr << (*i).GetBoardName() << "\t" << (*i).GetBoardDescription();
				SendBufferedLine(tempstr.str());
			}
		}

		SendBufferedLine(".");

	}
	// LIST OVERVIEW.FMT
	else if(type==3)
	{
		SendBufferedLine("215 Order of fields in overview database.");
		SendBufferedLine("Subject:");
		SendBufferedLine("From:");
		SendBufferedLine("Date:");
		SendBufferedLine("Message-ID:");
		SendBufferedLine("References:");
		SendBufferedLine(":bytes");
		SendBufferedLine(":lines");
		SendBufferedLine(".");
	}
	else
	{
		// unknown arg
		SendBufferedLine("501 Syntax error");
		m_log->debug("NNTPConnection::HandleListCommand unhandled LIST variant");
	}

	return true;
}

const bool NNTPConnection::HandleListGroupCommand(const NNTPCommand &command)
{

	std::ostringstream tempstr;
	Board board;
	bool validgroup=false;
	int lownum=-1;
	int highnum=-1;

	// no args and invalid boardid
	if(command.m_arguments.size()==0 && m_status.m_boardid==-1)
	{
		SendBufferedLine("412 No newsgroup selected");
	}
	else if(command.m_arguments.size()==0)
	{
		validgroup=board.Load(m_status.m_boardid);
	}
	else if(command.m_arguments.size()==1)
	{
		validgroup=board.Load(command.m_arguments[0]);
		if(validgroup)
		{
			lownum=board.GetLowMessageID();
			highnum=board.GetHighMessageID();
		}
		else
		{
			SendBufferedLine("411 No such newsgroup");
		}
	}
	else if(command.m_arguments.size()==2)
	{
		validgroup=board.Load(command.m_arguments[0]);
		std::vector<std::string> rangeparts;
		StringFunctions::Split(command.m_arguments[1],"-",rangeparts);

		if(rangeparts.size()>0)
		{
			StringFunctions::Convert(rangeparts[0],lownum);
		}
		if(rangeparts.size()>1)
		{
			StringFunctions::Convert(rangeparts[1],highnum);
		}

	}
	else
	{
		// unknown arg
		SendBufferedLine("501 Syntax error");
		m_log->debug("NNTPConnection::HandleListGroupCommand unknown arguments");
	}

	if(validgroup)
	{

		// set boardid and messageid
		m_status.m_boardid=board.GetBoardID();
		board.GetLowMessageID()!=0 ? m_status.m_messageid=board.GetLowMessageID() : m_status.m_messageid=-1;

		if(lownum==-1)
		{
			lownum=board.GetLowMessageID();
		}
		if(highnum==-1)
		{
			highnum=board.GetHighMessageID();
		}

		tempstr << "211 " << board.GetMessageCount() << " " << board.GetLowMessageID() << " " << board.GetHighMessageID() << " " << board.GetBoardName();
		SendBufferedLine(tempstr.str());

		MessageList ml;
		ml.LoadRange(lownum,highnum,board.GetBoardID());

		for(std::vector<Message>::iterator i=ml.begin(); i!=ml.end(); i++)
		{
			tempstr.str("");
			tempstr << (*i).GetMessageID();

			SendBufferedLine(tempstr.str());
		}

		// end of multi-line response
		SendBufferedLine(".");

	}

	return true;
}

const bool NNTPConnection::HandleModeCommand(const NNTPCommand &command)
{
	if(command.m_arguments.size()>0)
	{
		std::string arg=command.m_arguments[0];
		StringFunctions::UpperCase(arg,arg);
		if(arg=="READER")
		{
			m_status.m_mode=MODE_READER;
			if(m_status.m_allowpost==true)
			{
				SendBufferedLine("200 Posting allowed");
			}
			else
			{
				SendBufferedLine("201 Posting prohibited");
			}
			
			m_log->debug("NNTPConnection::HandleModeCommand set mode to reader");
		}
		else
		{
			SendBufferedLine("501 Syntax error");
			m_log->debug("NNTPConnection::HandleModeCommand unknown MODE argument : "+arg);
		}
	}
	else
	{
		SendBufferedLine("501 Syntax error");
		m_log->debug("NNTPConnection::HandleModeCommand no argument supplied for MODE");	
	}

	return true;
}

const bool NNTPConnection::HandleNewGroupsCommand(const NNTPCommand &command)
{
	if(command.m_arguments.size()>=2)
	{
		Poco::DateTime date;
		int tempyear=0;
		int tempmonth=0;
		int tempday=0;
		if(command.m_arguments[0].size()==8)
		{
			StringFunctions::Convert(command.m_arguments[0].substr(0,4),tempyear);
			StringFunctions::Convert(command.m_arguments[0].substr(4,2),tempmonth);
			StringFunctions::Convert(command.m_arguments[0].substr(6,2),tempday);
			try
			{
				date.assign(tempyear,tempmonth,tempday,date.hour(),date.minute(),date.second());
			}
			catch(...)
			{
				m_log->fatal("NNTPConnection::HandleNewGroupsCommand error assigning date");
			}
		}
		else
		{
			/*
			from RFC 3977
			If the first two digits of the year are not specified
			(this is supported only for backward compatibility), the year is to
			be taken from the current century if yy is smaller than or equal to
			the current year, and the previous century otherwise.
			*/
			int century;
			Poco::DateTime now;
			century=now.year()-(now.year()%100);

			StringFunctions::Convert(command.m_arguments[0].substr(0,2),tempyear);
			tempyear<=now.year()-century ? tempyear+=century : tempyear+=(century-100);
			
			//tempint > 50 ? tempint+=1900 : tempint+=2000;
			
			StringFunctions::Convert(command.m_arguments[0].substr(2,2),tempmonth);
			StringFunctions::Convert(command.m_arguments[0].substr(4,2),tempday);
			try
			{
				date.assign(tempyear,tempmonth,tempday);
			}
			catch(...)
			{
				m_log->fatal("NNTPConnection::HandleNewGroupsCommand error assigning date");
			}
		}

		BoardList bl;

		bl.LoadNew(Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S"));

		SendBufferedLine("231 List of new newsgroups follows");

		for(BoardList::iterator i=bl.begin(); i!=bl.end(); i++)
		{
			if((*i).GetSaveReceivedMessages()==true)
			{
				std::ostringstream tempstr;
				tempstr << (*i).GetBoardName() << " " << (*i).GetHighMessageID() << " " << (*i).GetLowMessageID() << " " << m_status.m_allowpost ? "y" : "n";
				SendBufferedLine(tempstr.str());
			}
		}

		SendBufferedLine(".");

	}
	else
	{
		SendBufferedLine("501 Syntax error");
		m_log->debug("NNTPConnection::HandleNewGroupsCommand syntax error");
	}

	return true;

}

const bool NNTPConnection::HandleNextCommand(const NNTPCommand &command)
{
	if(m_status.m_boardid!=-1)
	{
		if(m_status.m_messageid!=-1)
		{
			Message mess;

			if(mess.LoadNext(m_status.m_messageid,m_status.m_boardid))
			{
				std::ostringstream tempstr;

				m_status.m_messageid=mess.GetMessageID();

				tempstr << "223 " << mess.GetMessageID() << " " << mess.GetNNTPArticleID();

				SendBufferedLine(tempstr.str());

			}
			else
			{
				SendBufferedLine("421 No next article in this group");
			}
		}
		else
		{
			SendBufferedLine("420 Current article number is invalid");
		}
	}
	else
	{
		SendBufferedLine("412 No newsgroup selected");
	}

	return true;

}

const bool NNTPConnection::HandleOverCommand(const NNTPCommand &command)
{
	long lowmessageid,highmessageid;
	std::string messageuuid="";

	lowmessageid=highmessageid=-2;

	if(command.m_arguments.size()==0)
	{
		lowmessageid=m_status.m_messageid;
		highmessageid=m_status.m_messageid;
	}
	else
	{
		// Message-ID
		if(command.m_arguments.size()>0 && command.m_arguments[0].find("<")==0 && command.m_arguments[0].find(">")>0)
		{
			messageuuid=command.m_arguments[0];
			messageuuid=StringFunctions::Replace(messageuuid,"<","");
			messageuuid=StringFunctions::Replace(messageuuid,">","");
			/*
			// get rid of @ and everything after
			if(messageuuid.find("@")!=std::string::npos)
			{
				messageuuid.erase(messageuuid.find("@"));
			}
			*/
		}
		// single article or range
		else
		{
			// range
			if(command.m_arguments[0].find("-")!=std::string::npos)
			{
				std::vector<std::string> rangeparts;
				StringFunctions::Split(command.m_arguments[0],"-",rangeparts);
				// no upper bound
				if(rangeparts.size()>0)
				{
					StringFunctions::Convert(rangeparts[0],lowmessageid);
					highmessageid=-1;
				}
				//upper bound
				else if(rangeparts.size()>1)
				{
					StringFunctions::Convert(rangeparts[1],highmessageid);
				}
			}
			// single
			else
			{
				StringFunctions::Convert(command.m_arguments[0],lowmessageid);
			}
		}
	}

	if(messageuuid!="")
	{
		Message mess;
		if(mess.Load(messageuuid))
		{
			SendBufferedLine("224 Overview information follows");
			SendArticleOverInfo(mess);
			SendBufferedLine(".");
		}
		else
		{
			SendBufferedLine("423 No such article");
		}
	}
	else
	{
		Board bd;
		if(m_status.m_boardid!=-1 && bd.Load(m_status.m_boardid))
		{
			// single message
			if(highmessageid==-2)
			{
				Message mess;
				if(mess.Load(lowmessageid,m_status.m_boardid))
				{
					SendBufferedLine("224 Overview information follows");
					SendArticleOverInfo(mess);
					SendBufferedLine(".");
				}
				else
				{
					SendBufferedLine("423 No such article in this group");
				}
			}
			// range with no upper bound
			else if(highmessageid==-1)
			{
				MessageList ml;
				ml.LoadRange(lowmessageid,bd.GetHighMessageID(),m_status.m_boardid);
				if(ml.size()>0)
				{
					SendBufferedLine("224 Overview information follows");
					for(MessageList::iterator i=ml.begin(); i!=ml.end(); i++)
					{
						SendArticleOverInfo((*i));
					}
					SendBufferedLine(".");
				}
				else
				{
					SendBufferedLine("423 Empty range");
				}
			}
			// range with upper and lower bound
			else if(highmessageid>=lowmessageid)
			{
				MessageList ml;
				ml.LoadRange(lowmessageid,highmessageid,m_status.m_boardid);
				if(ml.size()>0)
				{
					SendBufferedLine("224 Overview information follows");
					for(MessageList::iterator i=ml.begin(); i!=ml.end(); i++)
					{
						SendArticleOverInfo((*i));
					}
					SendBufferedLine(".");
				}
				else
				{
					SendBufferedLine("423 Empty range");
				}
			}
			// invalid range
			else
			{
				SendBufferedLine("423 Empty range");
			}
		}
		else
		{
			SendBufferedLine("423 No newsgroup selected");
		}
	}

	return true;

}

const bool NNTPConnection::HandlePostCommand(const NNTPCommand &command)
{
	if(m_status.m_allowpost==true)
	{
		SendBufferedLine("340 Send article to be posted");
		m_status.m_isposting=true;
	}
	else
	{
		SendBufferedLine("440 Posting not permitted");
	}

	return true;
}

void NNTPConnection::HandlePostedMessage(const std::string &message)
{
	Message mess;

	if(mess.ParseNNTPMessage(message))
	{
		// if we authenticated, set the username to the authenticated user
		if(m_status.m_authenticated)
		{
			mess.SetFromName(m_status.m_authuser.GetName());
		}
		// handle a messages posted to an adminboard
		if(mess.PostedToAdministrationBoard()==true)
		{
			mess.HandleAdministrationMessage();
		}
		if(mess.StartFreenetInsert())
		{
			SendBufferedLine("240 Article received OK");
		}
		else
		{
			SendBufferedLine("441 Posting failed.  Make sure the identity you are sending with exists!");
		}
	}
	else
	{
		SendBufferedLine("441 Posting failed");
	}
}

void NNTPConnection::HandleReceivedData()
{
	if(m_status.m_isposting==false)
	{
		// get end of command line
		std::vector<char>::iterator endpos=Find(m_receivebuffer,"\r\n");
		
		// we got a command
		if(endpos!=m_receivebuffer.end())
		{
			NNTPCommand command;
			std::string commandline(m_receivebuffer.begin(),endpos);

			// remove command from receive buffer
			m_receivebuffer.erase(m_receivebuffer.begin(),endpos+2);

			// remove any leading/trailing whitespace
			commandline=StringFunctions::TrimWhitespace(commandline);

			// split out command and arguments separated by space or tab
			StringFunctions::SplitMultiple(commandline," \t",command.m_arguments);

			// command is first element in argument vector
			command.m_command=command.m_arguments[0];
			// erase command from argument vector and make it upper case
			command.m_arguments.erase(command.m_arguments.begin());
			StringFunctions::UpperCase(command.m_command,command.m_command);

			if(HandleCommand(command)==true)
			{
				
			}
			else
			{
				SendBufferedLine("500 Unknown command");

				m_log->debug("NNTPConnection::HandleReceivedData received unhandled NNTP command : "+commandline);
			}

		}

	}
	else
	{
		// check for end of post
		std::vector<char>::iterator endpos=Find(m_receivebuffer,"\r\n.\r\n");

		if(endpos!=m_receivebuffer.end())
		{
			// get the message
			std::string message(m_receivebuffer.begin(),endpos);
			// remove from receive buffer
			m_receivebuffer.erase(m_receivebuffer.begin(),endpos+5);

			// get rid of dot stuffing ( 2 dots on start of a line - used to prevent premature message end in NNTP)
			message=StringFunctions::Replace(message,"\r\n..","\r\n.");

			HandlePostedMessage(message);

			// message was received, so posting is completed
			m_status.m_isposting=false;

		}
	}
}

const bool NNTPConnection::HandleSetTrustCommand(const NNTPCommand &command)
{
	if(command.m_arguments.size()>=3)
	{
		std::string type=command.m_arguments[0];
		StringFunctions::UpperCase(type,type);
		if(type=="MESSAGE" || type=="TRUSTLIST" || type=="MESSAGECOMMENT" || type=="TRUSTLISTCOMMENT")
		{
			if(m_status.m_authenticated)
			{
				bool found=false;
				bool valid=false;
				int trust=-1;
				std::string comment="";
				std::string nntpname="";

				if(type=="MESSAGE" || type=="TRUSTLIST")
				{
					for(int i=1; i<command.m_arguments.size()-1; i++)
					{
						if(i!=1)
						{
							nntpname+=" ";
						}
						nntpname+=command.m_arguments[i];
					}

					if(command.m_arguments[command.m_arguments.size()-1]!="null")
					{
						StringFunctions::Convert(command.m_arguments[command.m_arguments.size()-1],trust);
					}

					if(trust>=-1 && trust<=100)
					{
						valid=true;
					}
				}
				else
				{
					int startpos=-1;
					// get nntpname
					for(int i=1; i<command.m_arguments.size() && startpos==-1; i++)
					{
						if(command.m_arguments[i].size()>0 && command.m_arguments[i][0]!='\"')
						{
							if(i!=1)
							{
								nntpname+=" ";
							}
							nntpname+=command.m_arguments[i];
						}
						else
						{
							startpos=i;
						}
					}

					// get comment
					for(int i=startpos; i<command.m_arguments.size(); i++)
					{
						if(i!=startpos)
						{
							comment+=" ";
						}
						comment+=command.m_arguments[i];
					}
					// strip " from comment beginning and end
					if(comment.size()>0 && comment[0]=='\"')
					{
						comment.erase(0,1);
					}
					if(comment.size()>0 && comment[comment.size()-1]=='\"')
					{
						comment.erase(comment.size()-1);
					}

					valid=true;
				}

				TrustExtension tr(m_status.m_authuser.GetID());

				if(type=="MESSAGE")
				{
					if(tr.SetMessageTrust(nntpname,trust))
					{
						found=true;
					}
				}
				if(type=="TRUSTLIST")
				{
					if(tr.SetTrustListTrust(nntpname,trust))
					{
						found=true;
					}
				}
				if(type=="MESSAGECOMMENT")
				{
					if(tr.SetMessageTrustComment(nntpname,comment))
					{
						found=true;
					}
				}
				if(type=="TRUSTLISTCOMMENT")
				{
					if(tr.SetTrustListTrustComment(nntpname,comment))
					{
						found=true;
					}
				}

				if(found && valid)
				{
					SendBufferedLine("280 Trust Set");
				}
				else if(found==false)
				{
					SendBufferedLine("480 Identity not found");
				}
				else
				{
					SendBufferedLine("501 Syntax error");
				}

			}
			else
			{
				SendBufferedLine("480 User not authenticated");
			}
		}
		else
		{
			SendBufferedLine("501 Syntax error");
		}
	}
	else
	{
		SendBufferedLine("501 Syntax error");
	}
	return true;
}

const bool NNTPConnection::HandleStatCommand(const NNTPCommand &command)
{
	SendArticleParts(command);

	return true;
}

const bool NNTPConnection::HandleQuitCommand(const NNTPCommand &command)
{
	SendBufferedLine("205 Connection Closing");
	SocketSend();
	Disconnect();
	m_log->information("NNTPConnection::HandleQuitCommand client closed connection");
	return true;
}

void NNTPConnection::run()
{
	struct timeval tv;
	fd_set writefs,readfs;
	int rval;

	// seed random number generater for this thread
	srand(time(NULL));
	
	if(m_status.m_allowpost==true)
	{
		SendBufferedLine("200 Service available, posting allowed");
	}
	else
	{
		SendBufferedLine("201 Service available, posting prohibited");
	}

	do
	{
		FD_ZERO(&readfs);
		FD_ZERO(&writefs);
		
		FD_SET(m_socket,&readfs);
		if(m_sendbuffer.size()>0)
		{
			FD_SET(m_socket,&writefs);
		}
		
		tv.tv_sec=1;
		tv.tv_usec=0;
		
		rval=select(m_socket+1,&readfs,&writefs,0,&tv);
		
		if(rval>0)
		{
			if(FD_ISSET(m_socket,&readfs))
			{
				SocketReceive();
				HandleReceivedData();
			}
			if(m_socket!=INVALID_SOCKET && FD_ISSET(m_socket,&writefs))
			{
				SocketSend();
			}
		}
		else if(rval==SOCKET_ERROR)
		{
			m_log->error("NNTPConnection::run select returned -1 : "+GetSocketErrorMessage());	
		}

	}while(!Disconnected() && !IsCancelled());

	Disconnect();

}

void NNTPConnection::SendArticleOverInfo(Message &message)
{
	std::string tempval;
	std::string line;
	std::map<long,std::string> references;

	StringFunctions::Convert(message.GetMessageID(),tempval);
	line=tempval+"\t";
	line+=message.GetSubject()+"\t";
	line+=message.GetFromName()+"\t";
	line+=Poco::DateTimeFormatter::format(message.GetDateTime(),"%w, %d %b %y %H:%M:%S -0000")+"\t";
	line+=message.GetNNTPArticleID()+"\t";
	references=message.GetInReplyTo();
	if(references.size()>0)
	{
		for(std::map<long,std::string>::reverse_iterator i=references.rbegin(); i!=references.rend(); i++)
		{
			if(i!=references.rbegin())
			{
				line+=" ";
			}
			line+="<"+(*i).second+">"; //+"@freenetproject.org>";
		}
		line+="\t";
	}
	else
	{
		line+="\t";
	}
	line+="\t";

	SendBufferedLine(line);
}

void NNTPConnection::SendArticleParts(const NNTPConnection::NNTPCommand &command)
{
	bool sendheaders,sendbody;
	std::string successcode;

	if(command.m_command=="ARTICLE")
	{
		sendheaders=true;
		sendbody=true;
		successcode="220";
	}
	else if(command.m_command=="HEAD")
	{
		sendheaders=true;
		sendbody=false;
		successcode="221";
	}
	else if(command.m_command=="BODY")
	{
		sendheaders=false;
		sendbody=true;
		successcode="222";
	}
	else if(command.m_command=="STAT")
	{
		sendheaders=false;
		sendbody=false;
		successcode="223";
	}

	Message message;
	int messageid=m_status.m_messageid;
	std::string articleid="";
	int type=0;	// default to current messageid, 1=messageid, 2=articleid

	if(command.m_arguments.size()==1 && command.m_arguments[0].size()>0)
	{
		if(command.m_arguments[0].find("<")==std::string::npos)
		{
			StringFunctions::Convert(command.m_arguments[0],messageid);
			message.Load(messageid,m_status.m_boardid);
			m_status.m_messageid=message.GetMessageID();
			type=1;
		}
		else
		{
			articleid=command.m_arguments[0];
			//strip off < and > and everthing after @
			if(articleid.size()>0 && articleid[0]=='<')
			{
				articleid.erase(0,1);
			}
			if(articleid.size()>0 && articleid[articleid.size()-1]=='>')
			{
				articleid.erase(articleid.size()-1);
			}
			/*
			if(articleid.size()>0 && articleid.find('@')!=std::string::npos)
			{
				articleid.erase(articleid.find('@'));
			}
			*/
			message.Load(articleid);
			type=2;
		}
	}
	else
	{
		message.Load(m_status.m_messageid,m_status.m_boardid);
	}

	switch(type)
	{
	case 0:
		if(m_status.m_boardid!=-1)
		{
			if(m_status.m_messageid!=-1)
			{
				std::ostringstream tempstr;
				std::string article;
				if(sendheaders&&sendbody)
				{
					article=message.GetNNTPHeaders()+"\r\n"+message.GetNNTPBody();
				}
				else if(sendheaders && !sendbody)
				{
					article=message.GetNNTPHeaders();
					// strip off final \r\n from headers
					if(article.rfind("\r\n")==article.size()-2)
					{
						article.erase(article.size()-2);
					}
				}
				else
				{
					article=message.GetNNTPBody();
				}
				// dot stuff article
				article=StringFunctions::Replace(article,"\r\n.","\r\n..");

				tempstr << successcode << " " << message.GetMessageID() << " " << message.GetNNTPArticleID();

				SendBufferedLine(tempstr.str());
				if(sendheaders || sendbody)
				{
					SendBufferedLine(article);
					SendBufferedLine(".");
				}

			}
			else
			{
				SendBufferedLine("420 Current article number is invalid");
			}
		}
		else
		{
			SendBufferedLine("412 No newsgroup selected");
		}
		break;
	case 1:
		if(m_status.m_boardid!=-1)
		{
			if(message.GetMessageID()!=-1)
			{
				std::ostringstream tempstr;
				std::string article;
				if(sendheaders&&sendbody)
				{
					article=message.GetNNTPHeaders()+"\r\n"+message.GetNNTPBody();
				}
				else if(sendheaders && !sendbody)
				{
					article=message.GetNNTPHeaders();
					// strip off final \r\n from headers
					if(article.rfind("\r\n")==article.size()-2)
					{
						article.erase(article.size()-2);
					}
				}
				else
				{
					article=message.GetNNTPBody();
				}
				// dot stuff article
				article=StringFunctions::Replace(article,"\r\n.","\r\n..");

				tempstr << successcode << " " << message.GetMessageID() << " " << message.GetNNTPArticleID();

				SendBufferedLine(tempstr.str());
				if(sendheaders || sendbody)
				{
					SendBufferedLine(article);
					SendBufferedLine(".");
				}
			}
			else
			{
				SendBufferedLine("423 No article with that number");
			}
		}
		else
		{
			SendBufferedLine("412 No newsgroup selected");
		}
		break;
	case 2:
		if(message.GetMessageID()!=-1)
		{
			std::string article;
			if(sendheaders&&sendbody)
			{
				article=message.GetNNTPHeaders()+"\r\n"+message.GetNNTPBody();
			}
			else if(sendheaders && !sendbody)
			{
				article=message.GetNNTPHeaders();
				// strip off final \r\n from headers
				if(article.rfind("\r\n")==article.size()-2)
				{
					article.erase(article.size()-2);
				}
			}
			else
			{
				article=message.GetNNTPBody();
			}
			// dot stuff article
			article=StringFunctions::Replace(article,"\r\n.","\r\n..");

			SendBufferedLine(successcode+" 0 "+message.GetNNTPArticleID());
			if(sendheaders || sendbody)
			{
				SendBufferedLine(article);
				SendBufferedLine(".");
			}
		}
		else
		{
			SendBufferedLine("430 No article with that message-id");
		}
		break;
	}

}

void NNTPConnection::SendBuffered(const std::string &data)
{
	m_sendbuffer.insert(m_sendbuffer.end(),data.begin(),data.end());
}

void NNTPConnection::SocketReceive()
{
	int rval=recv(m_socket,&m_tempbuffer[0],m_tempbuffer.size(),0);
	if(rval>0)
	{
		m_receivebuffer.insert(m_receivebuffer.end(),m_tempbuffer.begin(),m_tempbuffer.begin()+rval);
	}
	else if(rval==0)
	{
		Disconnect();
		m_log->information("NNTPConnection::SocketReceive remote host closed connection");
	}
	else if(rval==-1)
	{
		std::string errnostr;
		StringFunctions::Convert(GetSocketErrorNumber(),errnostr);
		// error on receive - close the connection
		Disconnect();
		m_log->error("NNTPConnection::SocketReceive recv returned -1 : "+errnostr+" - "+GetSocketErrorMessage());
	}
}

void NNTPConnection::SocketSend()
{
	if(m_sendbuffer.size()>0 && m_socket!=INVALID_SOCKET)
	{
		int rval=send(m_socket,&m_sendbuffer[0],m_sendbuffer.size(),0);
		if(rval>0)
		{	
			m_sendbuffer.erase(m_sendbuffer.begin(),m_sendbuffer.begin()+rval);
		}
		else if(rval==-1)
		{
			std::string errnostr;
			StringFunctions::Convert(GetSocketErrorNumber(),errnostr);
			m_log->error("NNTPConnection::SocketSend returned -1 : "+errnostr+" - "+GetSocketErrorMessage());
		}
	}
}
