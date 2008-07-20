#include "../../include/freenet/trustlistrequester.h"
#include "../../include/option.h"
#include "../../include/stringfunctions.h"
#include "../../include/freenet/trustlistxml.h"

#include <Poco/DateTimeFormatter.h>
#include <Poco/Timespan.h>

#ifdef XMEM
	#include <xmem.h>
#endif

TrustListRequester::TrustListRequester()
{
	Initialize();
}

TrustListRequester::TrustListRequester(FCPv2 *fcp):IIndexRequester<long>(fcp)
{
	Initialize();
}

const bool TrustListRequester::HandleAllData(FCPMessage &message)
{
	Poco::DateTime now;
	SQLite3DB::Statement st;
	SQLite3DB::Statement trustst;
	std::vector<std::string> idparts;
	long datalength;
	std::vector<char> data;
	TrustListXML xml;
	long identityid;
	long index;
	int insertcount=0;
	int dayinsertcount=0;
	int previnsertcount=0;

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

	// get count of identities added in last 24 hours
	st=m_db->Prepare("SELECT COUNT(*) FROM tblIdentity WHERE DateAdded>=?;");
	now-=Poco::Timespan(1,0,0,0,0);
	st.Bind(0,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
	st.Step();
	if(st.RowReturned())
	{
		if(st.ResultNull(0)==false)
		{
			st.ResultInt(0,dayinsertcount);
		}
	}
	else
	{
		m_log->error("TrustListRequester::HandleAllData couldn't get count of identities added in last 24 hours");
	}

	// get count of identities added more than 24 hours ago - if 0 then we will accept more than 100 identities now
	st=m_db->Prepare("SELECT COUNT(*) FROM tblIdentity WHERE DateAdded<?;");
	st.Bind(0,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
	st.Step();
	if(st.RowReturned())
	{
		if(st.ResultNull(0)==false)
		{
			st.ResultInt(0,previnsertcount);
		}
	}
	else
	{
		m_log->error("TrustListRequester::HandleAllData couldn't get count of identities added more than 24 hours ago");
	}

	now=Poco::DateTime();

	// parse file into xml and update the database
	if(xml.ParseXML(std::string(data.begin(),data.end()))==true)
	{
		// find the identity name and public key of the identity publishing the trust list
		std::string publisherid="";
		st=m_db->Prepare("SELECT Name,PublicKey FROM tblIdentity WHERE IdentityID=?;");
		st.Bind(0,identityid);
		st.Step();
		if(st.RowReturned())
		{
			std::string publishername="";
			std::string publisherpublickey="";
			st.ResultText(0,publishername);
			st.ResultText(1,publisherpublickey);
			publisherid=publishername;
			if(publisherpublickey.size()>4)
			{
				publisherid+=publisherpublickey.substr(3,44);
			}
		}
		st.Finalize();

		// drop all existing peer trust from this identity - we will rebuild it when we go through each trust in the xml file
		st=m_db->Prepare("DELETE FROM tblPeerTrust WHERE IdentityID=?;");
		st.Bind(0,identityid);
		st.Step();
		st.Finalize();

		st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey=?;");
		trustst=m_db->Prepare("INSERT INTO tblPeerTrust(IdentityID,TargetIdentityID,MessageTrust,TrustListTrust,MessageTrustComment,TrustListTrustComment) VALUES(?,?,?,?,?,?);");
		
		SQLite3DB::Statement idinsert=m_db->Prepare("INSERT INTO tblIdentity(PublicKey,DateAdded,AddedMethod) VALUES(?,?,?);");
		
		// loop through all trust entries in xml and add to database if we don't already know them
		for(long i=0; i<xml.TrustCount(); i++)
		{
			int id=-1;
			std::string identity;
			std::string messagetrustcomment="";
			std::string trustlisttrustcomment="";
			identity=xml.GetIdentity(i);

			st.Bind(0,identity);
			st.Step();
			if(st.RowReturned()==false)
			{
				if(insertcount<50 && (dayinsertcount<100 || previnsertcount==0))
				{
					idinsert.Bind(0,identity);
					idinsert.Bind(1,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
					idinsert.Bind(2,"trust list of "+publisherid);
					idinsert.Step(true);
					id=idinsert.GetLastInsertRowID();
					idinsert.Reset();
				}
				insertcount++;
				dayinsertcount++;
			}
			else
			{
				st.ResultInt(0,id);
			}
			st.Reset();

			//insert trust for this identity
			if(id!=-1)
			{
				trustst.Bind(0,identityid);
				trustst.Bind(1,id);
				if(xml.GetMessageTrust(i)==-1)
				{
					trustst.Bind(2);
				}
				else
				{
					trustst.Bind(2,xml.GetMessageTrust(i));
				}
				if(xml.GetTrustListTrust(i)==-1)
				{
					trustst.Bind(3);
				}
				else
				{
					trustst.Bind(3,xml.GetTrustListTrust(i));
				}
				messagetrustcomment=xml.GetMessageTrustComment(i);
				trustlisttrustcomment=xml.GetTrustListTrustComment(i);
				// limit comments to 50 characters each
				if(messagetrustcomment.size()>50)
				{
					messagetrustcomment.erase(50);
				}
				if(trustlisttrustcomment.size()>50)
				{
					trustlisttrustcomment.erase(50);
				}
				trustst.Bind(4,messagetrustcomment);
				trustst.Bind(5,trustlisttrustcomment);
				trustst.Step();
				trustst.Reset();
			}
		}

		trustst.Finalize();
		st.Finalize();

		if(insertcount>=50)
		{
			m_log->warning("TrustListRequester::HandleAllData TrustList contained more than 50 new identities : "+message["Identifier"]);
		}
		if(dayinsertcount>=100 && previnsertcount>0)
		{
			m_log->warning("TrustListRequester::HandleAllData TrustList would have inserted more than 100 new identities in the last 24 hours : "+message["Identifier"]);
		}

		st=m_db->Prepare("INSERT INTO tblTrustListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'true');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->debug("TrustListRequester::HandleAllData parsed TrustList XML file : "+message["Identifier"]);
	}
	else
	{
		// bad data - mark index
		st=m_db->Prepare("INSERT INTO tblTrustListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->error("TrustListRequester::HandleAllData error parsing TrustList XML file : "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;

}

const bool TrustListRequester::HandleGetFailed(FCPMessage &message)
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
		st=m_db->Prepare("INSERT INTO tblTrustListRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->error("TrustListRequester::HandleGetFailed fatal error requesting "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;

}

void TrustListRequester::Initialize()
{
	std::string tempval="";
	m_fcpuniquename="TrustListRequester";
	Option::Instance()->Get("MaxIdentityRequests",tempval);
	StringFunctions::Convert(tempval,m_maxrequests);
	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->error("Option MaxTrustListRequests is currently set at "+tempval+".  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->warning("Option MaxTrustListRequests is currently set at "+tempval+".  This value might be incorrectly configured.");
	}
	m_tempdate=Poco::Timestamp();
}

void TrustListRequester::PopulateIDList()
{
	Poco::DateTime date;
	int id;
	std::string sql;

	// select identities we want to query (we've seen them today and they are publishing trust list) - sort by their trust level (descending) with secondary sort on how long ago we saw them (ascending)
	sql="SELECT IdentityID FROM tblIdentity ";
	sql+="WHERE Name IS NOT NULL AND Name <> '' AND PublicKey IS NOT NULL AND PublicKey <> '' AND LastSeen>='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"' AND PublishTrustList='true' AND LocalTrustListTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalTrustListTrust') AND ( PeerTrustListTrust IS NULL OR PeerTrustListTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerTrustListTrust') )";
	sql+="ORDER BY LocalTrustListTrust DESC, LastSeen;";

	SQLite3DB::Statement st=m_db->Prepare(sql);
	st.Step();

	m_ids.clear();

	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[id]=false;
		st.Step();
	}
}

void TrustListRequester::StartRequest(const long &identityid)
{
	Poco::DateTime now;
	FCPMessage message;
	std::string publickey;
	int index;
	std::string indexstr;
	std::string identityidstr;

	SQLite3DB::Statement st=m_db->Prepare("SELECT PublicKey FROM tblIdentity WHERE IdentityID=?;");
	st.Bind(0,identityid);
	st.Step();

	if(st.RowReturned())
	{
		st.ResultText(0,publickey);

		SQLite3DB::Statement st2=m_db->Prepare("SELECT MAX(RequestIndex) FROM tblTrustListRequests WHERE Day=? AND IdentityID=?;");
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
		message["URI"]=publickey+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|TrustList|"+indexstr+".xml";
		message["Identifier"]=m_fcpuniquename+"|"+identityidstr+"|"+indexstr+"|"+message["URI"];
		message["ReturnType"]="direct";
		message["MaxSize"]="1000000";			// 1 MB

		m_fcp->SendMessage(message);

		m_requesting.push_back(identityid);
	}
	st.Finalize();

	m_ids[identityid]=true;

}
