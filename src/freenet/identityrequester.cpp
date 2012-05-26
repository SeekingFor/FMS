#include "../../include/freenet/identityrequester.h"
#include "../../include/freenet/identityxml.h"
#include "../../include/freenet/identitypublickeycache.h"
#include "../../include/stringfunctions.h"
#include "../../include/option.h"
#include "../../include/unicode/unicodestring.h"
#include "../../include/global.h"

#include <Poco/DateTime.h>
#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>
#include <Poco/DateTimeFormatter.h>

#ifdef XMEM
	#include <xmem.h>
#endif

IdentityRequester::IdentityRequester(SQLite3DB::DB *db):IIndexRequester<std::pair<long,long> >(db)
{

}

IdentityRequester::IdentityRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexRequester<std::pair<long,long> >(db,fcp)
{

}

const std::pair<long,long> IdentityRequester::GetIDFromIdentifier(const std::string &identifier)
{
	long identityid;
	long identityorder;

	std::vector<std::string> idparts;
	StringFunctions::Split(identifier,"|",idparts);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[3],identityorder);

	return std::pair<long,long>(identityorder,identityid);
}

const bool IdentityRequester::HandleAllData(FCPv2::Message &message)
{
	Poco::DateTime now;
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long datalength;
	std::vector<char> data;
	IdentityXML xml;
	long identityid;
	long index;
	UnicodeString name;
	long identityorder;

	now=Poco::Timestamp();
	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(message["DataLength"],datalength);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[2],index);
	StringFunctions::Convert(idparts[3],identityorder);

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
		std::string prevname("");
		std::string publickey("");
		st=m_db->Prepare("SELECT Name, PublicKey FROM tblIdentity WHERE IdentityID=?;");
		st.Bind(0,identityid);
		st.Step();
		if(st.RowReturned())
		{
			st.ResultText(0,prevname);
			st.ResultText(1,publickey);
		}
		st.Finalize();

		st=m_db->Prepare("UPDATE tblIdentity SET Name=?, SingleUse=?, LastSeen=?, PublishTrustList=?, PublishBoardList=?, FreesiteEdition=?, Signature=?, IsFMS=1 WHERE IdentityID=?");
		
		name=xml.GetName();
		name.Trim(MAX_IDENTITY_NAME_LENGTH);

		st.Bind(0,StringFunctions::RemoveControlChars(name.NarrowString()));
		if(xml.GetSingleUse()==true)
		{
			st.Bind(1,"true");
		}
		else
		{
			st.Bind(1,"false");
		}
		st.Bind(2,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
		if(xml.GetPublishTrustList()==true)
		{
			st.Bind(3,"true");
		}
		else
		{
			st.Bind(3,"false");
		}
		if(xml.GetPublishBoardList()==true)
		{
			st.Bind(4,"true");
		}
		else
		{
			st.Bind(4,"false");
		}
		if(xml.GetFreesiteEdition()>=0)
		{
			st.Bind(5,xml.GetFreesiteEdition());
		}
		else
		{
			st.Bind(5);
		}
		if(xml.GetSignature()!="")
		{
			st.Bind(6,xml.GetSignature());
		}
		else
		{
			st.Bind(6);
		}
		st.Bind(7,identityid);
		st.Step();
		st.Finalize();

		if(name.NarrowString()!=prevname)
		{
			UpdateMissingAuthorID(m_db,identityid,name.NarrowString(),publickey);
		}

		st=m_db->Prepare("INSERT INTO tblIdentityRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'true');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[5]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->debug(m_fcpuniquename+"::HandleAllData parsed Identity XML file : "+message["Identifier"]);
	}
	else
	{
		// bad data - mark index
		st=m_db->Prepare("INSERT INTO tblIdentityRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[5]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->error(m_fcpuniquename+"::HandleAllData error parsing Identity XML file : "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(std::pair<long,long>(identityorder,identityid));

	return true;

}

const bool IdentityRequester::HandleGetFailed(FCPv2::Message &message)
{
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long identityid;
	long index;
	long identityorder;

	if(message["Identifier"].find(".xml")!=std::string::npos)
	{

		StringFunctions::Split(message["Identifier"],"|",idparts);
		StringFunctions::Convert(idparts[1],identityid);
		StringFunctions::Convert(idparts[2],index);
		StringFunctions::Convert(idparts[3],identityorder);

		// if this is a fatal error - insert index into database so we won't try to download this index again
		if(message["Fatal"]=="true")
		{
			if(message["Code"]!="25")
			{
				st=m_db->Prepare("INSERT INTO tblIdentityRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
				st.Bind(0,identityid);
				st.Bind(1,idparts[5]);
				st.Bind(2,index);
				st.Step();
				st.Finalize();
			}

			m_log->error(m_fcpuniquename+"::HandleGetFailed fatal error requesting "+message["Identifier"]);
		}

		// remove this identityid from request list
		RemoveFromRequestList(std::pair<long,long>(identityorder,identityid));

	}
	else
	{
		if(message["RedirectURI"]!="")
		{
			FCPv2::Message mess("ClientGet");
			mess["URI"]=StringFunctions::UriDecode(message["RedirectURI"]);
			mess["Identifier"]=message["Identifier"];
			mess["PriorityClass"]=m_defaultrequestpriorityclassstr;
			mess["ReturnType"]="direct";
			mess["MaxSize"]="10000";			// 10 KB

			m_fcp->Send(mess);

			m_log->debug(m_fcpuniquename+"::HandleGetFailed started redirect request for "+mess["URI"]);
		}
		else
		{
			m_log->trace(m_fcpuniquename+"::HandleGetFailed for "+message["Identifier"]+" message = "+message.GetFCPString());
		}
	}

	return true;

}

void IdentityRequester::StartRequest(const std::pair<long,long> &inputpair)
{
	const long identityorder=inputpair.first;
	const long identityid=inputpair.second;
	Poco::DateTime now;
	FCPv2::Message message;
	std::string publickey;
	int index;
	std::string indexstr;
	std::string identityidstr;
	std::string identityorderstr;
	IdentityPublicKeyCache pkcache(m_db);

	if(pkcache.PublicKey(identityid,publickey))
	{
		now=Poco::Timestamp();

		SQLite3DB::Statement st2=m_db->Prepare("SELECT MAX(RequestIndex) FROM tblIdentityRequests WHERE Day=? AND IdentityID=?;");
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
		StringFunctions::Convert(identityorder,identityorderstr);

		message.SetName("ClientGet");
		message["URI"]=publickey+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|Identity|"+indexstr+".xml";
		message["Identifier"]=m_fcpuniquename+"|"+identityidstr+"|"+indexstr+"|"+identityorderstr+"|"+message["URI"];
		message["PriorityClass"]=m_defaultrequestpriorityclassstr;
		message["ReturnType"]="direct";
		message["MaxSize"]="10000";			// 10 KB

		m_fcp->Send(message);

		m_log->trace(m_fcpuniquename+"::StartRequest started request for "+message["Identifier"]);

		StartedRequest(inputpair,message["Identifier"]);

		// if this is from the UnknownIdentityRequester, also request the editioned USK
		if(m_fcpuniquename=="UnknownIdentityRequester")
		{
			message.Clear();
			message.SetName("ClientGet");
			message["URI"]="USK"+publickey.substr(3)+m_messagebase+"|IdentityRedirect/0/";
			message["Identifier"]=m_fcpuniquename+"|"+identityidstr+"|"+indexstr+"|"+identityorderstr+"|_|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|"+message["URI"];
			message["PriorityClass"]=m_defaultrequestpriorityclassstr;
			message["ReturnType"]="direct";
			message["MaxSize"]="10000";

			m_fcp->Send(message);

			m_log->trace(m_fcpuniquename+"::StartRequest started request for "+message["Identifier"]);
		}

	}

	m_ids[inputpair].m_requested=true;

}
