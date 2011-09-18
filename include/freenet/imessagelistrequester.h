#ifndef _imessagelistrequester_
#define _imessagelistrequester_

#include "iindexrequester.h"
#include "messagelistxml.h"

#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>
#include <Poco/Timestamp.h>

#include <map>
#include <set>
#include <vector>

template <class IDTYPE>
class IMessageListRequester:public IIndexRequester<IDTYPE>
{
public:
	IMessageListRequester(SQLite3DB::DB *db);
	IMessageListRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp);
	virtual ~IMessageListRequester()		{}

protected:

	long m_messagedownloadmaxdaysbackward;
	bool m_localtrustoverrides;

private:
	void InitializeIMessageListRequester();
	virtual void PopulateIDList()=0;
	virtual const IDTYPE GetIDFromIdentifier(const std::string &identifier)=0;
	virtual void StartRequest(const IDTYPE &id)=0;
	virtual void StartRedirectRequest(FCPv2::Message &message);
	virtual const bool HandleAllData(FCPv2::Message &message);
	virtual void PostHandleAllData(FCPv2::Message &message)		{}
	virtual const bool HandleGetFailed(FCPv2::Message &message);
	virtual void PostHandleGetFailed(FCPv2::Message &message)	{}

	void GetBoardList(std::map<std::string,bool> &boards, const bool forceload=false);
	const bool CheckDateNotFuture(const std::string &datestr) const;
	const bool CheckDateWithinMaxDays(const std::string &datestr) const;

	bool m_savetonewboards;

	std::map<std::string,bool> m_boardscache;
	Poco::DateTime m_boardscacheupdate;			// last time we updated the boards cache

	std::map<std::string,std::map<long,std::set<long> > > m_requestindexcache;	// date - identity id - index
};

template <class IDTYPE>
IMessageListRequester<IDTYPE>::IMessageListRequester(SQLite3DB::DB *db):IIndexRequester<IDTYPE>(db)
{
	InitializeIMessageListRequester();
}

template <class IDTYPE>
IMessageListRequester<IDTYPE>::IMessageListRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexRequester<IDTYPE>(db,fcp)
{
	InitializeIMessageListRequester();
}

template <class IDTYPE>
const bool IMessageListRequester<IDTYPE>::CheckDateNotFuture(const std::string &datestr) const
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

template <class IDTYPE>
const bool IMessageListRequester<IDTYPE>::CheckDateWithinMaxDays(const std::string &datestr) const
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

template <class IDTYPE>
void IMessageListRequester<IDTYPE>::GetBoardList(std::map<std::string,bool> &boards, const bool forceload)
{
	// only query database when forced, or an 30 minutes have passed since last query
	if(forceload==true || m_boardscacheupdate+Poco::Timespan(0,0,30,0,0)<=Poco::DateTime())
	{
		m_boardscache.clear();
		SQLite3DB::Statement st=IIndexRequester<IDTYPE>::m_db->Prepare("SELECT BoardName, SaveReceivedMessages FROM tblBoard;");
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

template <class IDTYPE>
const bool IMessageListRequester<IDTYPE>::HandleAllData(FCPv2::Message &message)
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
	IDTYPE messageid;
	SQLite3DB::Transaction trans(IIndexRequester<IDTYPE>::m_db);

	GetBoardList(boards);

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(message["DataLength"],datalength);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[2],index);

	fromidentityid=identityid;

	// wait for all data to be received from connection
	IIndexRequester<IDTYPE>::m_fcp->WaitForBytes(1000,datalength);

	// if we got disconnected- return immediately
	if(IIndexRequester<IDTYPE>::m_fcp->IsConnected()==false)
	{
		return false;
	}

	// receive the file
	IIndexRequester<IDTYPE>::m_fcp->Receive(data,datalength);

	// parse file into xml and update the database
	if(data.size()>0 && xml.ParseXML(std::string(data.begin(),data.end()))==true)
	{

		trans.Begin(SQLite3DB::Transaction::TRANS_IMMEDIATE);

		SQLite3DB::Statement spk=IIndexRequester<IDTYPE>::m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey=?;");
		SQLite3DB::Statement mst=IIndexRequester<IDTYPE>::m_db->Prepare("INSERT OR IGNORE INTO tblMessageRequests(IdentityID,Day,RequestIndex,FromMessageList,FromIdentityID) VALUES(?,?,?,'true',?);");
		SQLite3DB::Statement ust=IIndexRequester<IDTYPE>::m_db->Prepare("UPDATE tblMessageRequests SET FromIdentityID=? WHERE IdentityID=? AND Day=? AND RequestIndex=?;");

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
				IIndexRequester<IDTYPE>::m_log->error(IIndexRequester<IDTYPE>::m_fcpuniquename+"::HandleAllData date for message is in future! "+xml.GetDate(i));
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
				trans.Step(mst);
				trans.Reset(mst);

				// We need to update ID here, in case this index was already inserted from another
				// identity's message list.  This doesn't reset try count - maybe we should if the from
				// identity was another identity
				ust.Bind(0,identityid);
				ust.Bind(1,identityid);
				ust.Bind(2,xml.GetDate(i));
				ust.Bind(3,xml.GetIndex(i));
				trans.Step(ust);
				trans.Reset(ust);

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
					IIndexRequester<IDTYPE>::m_log->error(IIndexRequester<IDTYPE>::m_fcpuniquename+"::HandleAllData date for external message is in future! "+xml.GetExternalDate(i));
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
						trans.Step(spk);

						if(spk.RowReturned())
						{
							spk.ResultInt(0,thisidentityid);
							identityids[xml.GetExternalIdentity(i)]=thisidentityid;
						}

						trans.Reset(spk);
					}

					if(thisidentityid!=0 && m_requestindexcache[xml.GetExternalDate(i)][thisidentityid].find(xml.GetExternalIndex(i))==m_requestindexcache[xml.GetExternalDate(i)][thisidentityid].end())
					{
						mst.Bind(0,thisidentityid);
						mst.Bind(1,xml.GetExternalDate(i));
						mst.Bind(2,xml.GetExternalIndex(i));
						mst.Bind(3,fromidentityid);
						trans.Step(mst);
						trans.Reset(mst);

						m_requestindexcache[xml.GetExternalDate(i)][thisidentityid].insert(xml.GetExternalIndex(i));
					}
				}
				else
				{
					//m_log->trace("MessageListRequester::HandleAllData will not download external message posted to "+boardsstr+" from " + xml.GetExternalIdentity(i) + " on " + xml.GetExternalDate(i));
				}
			}
		}

		st=IIndexRequester<IDTYPE>::m_db->Prepare("INSERT OR IGNORE INTO tblMessageListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'true');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		trans.Step(st);
		trans.Finalize(st);

		trans.Finalize(spk);
		trans.Finalize(mst);
		trans.Finalize(ust);

		trans.Commit();

		if(trans.IsSuccessful()==false)
		{
			IIndexRequester<IDTYPE>::m_log->error(IIndexRequester<IDTYPE>::m_fcpuniquename+"::HandleAllData transaction failed with SQLite error:"+trans.GetLastErrorStr()+" SQL="+trans.GetErrorSQL());
		}

		IIndexRequester<IDTYPE>::m_log->debug(IIndexRequester<IDTYPE>::m_fcpuniquename+"::HandleAllData parsed MessageList XML file : "+message["Identifier"]);
	}
	else
	{
		// bad data - mark index
		st=IIndexRequester<IDTYPE>::m_db->Prepare("INSERT INTO tblMessageListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		IIndexRequester<IDTYPE>::m_log->error(IIndexRequester<IDTYPE>::m_fcpuniquename+"::HandleAllData error parsing MessageList XML file : "+message["Identifier"]);
	}

	// remove this identityid from request list
	messageid=GetIDFromIdentifier(message["Identifier"]);
	RemoveFromRequestList(messageid);

	// keep 2 days of request indexes in the cache
	while(m_requestindexcache.size()>2)
	{
		m_requestindexcache.erase(m_requestindexcache.begin());
	}

	PostHandleAllData(message);

	return true;

}

template <class IDTYPE>
const bool IMessageListRequester<IDTYPE>::HandleGetFailed(FCPv2::Message &message)
{
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long identityid;
	long index;
	IDTYPE messageid;

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
		if(message["Code"]!="25")
		{
			st=IIndexRequester<IDTYPE>::m_db->Prepare("INSERT INTO tblMessageListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
			st.Bind(0,identityid);
			st.Bind(1,idparts[4]);
			st.Bind(2,index);
			st.Step();
			st.Finalize();
		}

		IIndexRequester<IDTYPE>::m_log->error(IIndexRequester<IDTYPE>::m_fcpuniquename+"::HandleGetFailed fatal error code="+message["Code"]+" requesting "+message["Identifier"]);
	}

	// remove this identityid from request list
	messageid=GetIDFromIdentifier(message["Identifier"]);
	RemoveFromRequestList(messageid);

	PostHandleGetFailed(message);

	return true;
}

template <class IDTYPE>
void IMessageListRequester<IDTYPE>::InitializeIMessageListRequester()
{
	std::string tempval("");
	Option option(IIndexRequester<IDTYPE>::m_db);

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

template <class IDTYPE>
void IMessageListRequester<IDTYPE>::StartRedirectRequest(FCPv2::Message &message)
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
	newmessage["Identifier"]=IIndexRequester<IDTYPE>::m_fcpuniquename+"|"+identityidstr+"|"+indexstr+"|_|"+datestr+"|"+newmessage["URI"];
	newmessage["ReturnType"]="direct";
	newmessage["MaxSize"]="1000000";

	IIndexRequester<IDTYPE>::m_fcp->Send(newmessage);

	IIndexRequester<IDTYPE>::m_log->debug(IIndexRequester<IDTYPE>::m_fcpuniquename+"::StartRedirectRequest started redirect request of "+message["Identifier"]+" to "+newmessage["URI"]);

}

#endif	// _imessagelistrequester_
