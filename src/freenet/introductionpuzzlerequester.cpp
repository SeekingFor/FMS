#include "../../include/freenet/introductionpuzzlerequester.h"
#include "../../include/freenet/introductionpuzzlexml.h"
#include "../../include/option.h"
#include "../../include/stringfunctions.h"
#include "../../include/bitmapvalidator.h"
#include "../../include/base64.h"

#include <Poco/DateTime.h>
#include <Poco/Timespan.h>
#include <Poco/Timestamp.h>
#include <Poco/DateTimeFormatter.h>

#ifdef XMEM
	#include <xmem.h>
#endif

std::string IntroductionPuzzleRequester::m_validuuidchars="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~@_-";

IntroductionPuzzleRequester::IntroductionPuzzleRequester(SQLite3DB::DB *db):IIndexRequester<long>(db)
{
	Initialize();
}

IntroductionPuzzleRequester::IntroductionPuzzleRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexRequester<long>(db,fcp)
{
	Initialize();
}

const bool IntroductionPuzzleRequester::HandleAllData(FCPv2::Message &message)
{
	Poco::DateTime now;
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long datalength;
	std::vector<char> data;
	IntroductionPuzzleXML xml;
	long identityid;
	long index;
	bool validmessage=true;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(message["DataLength"],datalength);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[2],index);

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

		// check if last part of UUID matches first part of public key of identity who inserted it
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
			StringFunctions::SplitMultiple(xml.GetUUID(),"@",uuidparts);

			if(uuidparts.size()>1 && keyparts.size()>1 && xml.GetUUID().find_first_not_of(m_validuuidchars)==std::string::npos)
			{
				keypart=StringFunctions::Replace(StringFunctions::Replace(keyparts[1],"-",""),"~","");
				if(keypart!=uuidparts[1])
				{
					m_log->error("IntroductionPuzzleRequester::HandleAllData UUID in IntroductionPuzzle doesn't match public key of identity : "+message["Identifier"]);
					validmessage=false;
				}
			}
			else
			{
				m_log->error("IntroductionPuzzleRequester::HandleAllData Error with identity's public key or UUID : "+message["Identifier"]);
				validmessage=false;
			}

		}
		else
		{
			m_log->error("IntroductionPuzzleRequester::HandleAllData Error couldn't find identity : "+message["Identifier"]);
			validmessage=false;
		}

		// we can only validate bitmaps for now
		BitmapValidator val;
		val.SetMax(200,200);
		std::vector<unsigned char> puzzledata;
		Base64::Decode(xml.GetPuzzleData(),puzzledata);
		if(xml.GetMimeType()!="image/bmp" || val.Validate(puzzledata)==false)
		{
			m_log->error("IntroductionPuzzleRequester::HandleAllData received bad mime type and/or data for "+message["Identifier"]);
			validmessage=false;
		}

		st=m_db->Prepare("INSERT INTO tblIntroductionPuzzleRequests(IdentityID,Day,RequestIndex,Found,UUID,Type,MimeType,PuzzleData) VALUES(?,?,?,?,?,?,?,?);");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		if(validmessage)
		{
			st.Bind(3,"true");
			st.Bind(4,xml.GetUUID());
			st.Bind(5,xml.GetType());
			st.Bind(6,xml.GetMimeType());
			st.Bind(7,xml.GetPuzzleData());
		}
		else
		{
			st.Bind(3,"false");
			st.Bind(4);
			st.Bind(5);
			st.Bind(6);
			st.Bind(7);
		}
		st.Step();
		st.Finalize();

		m_log->debug("IntroductionPuzzleRequester::HandleAllData parsed IntroductionPuzzle XML file : "+message["Identifier"]);
	}
	else
	{
		// bad data - mark index
		st=m_db->Prepare("INSERT INTO tblIntroductionPuzzleRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->error("IntroductionPuzzleRequester::HandleAllData error parsing IntroductionPuzzle XML file : "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;

}

const bool IntroductionPuzzleRequester::HandleGetFailed(FCPv2::Message &message)
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
		st=m_db->Prepare("INSERT INTO tblIntroductionPuzzleRequests(IdentityID,Day,RequestIndex,Found) VALUES(?,?,?,'false');");
		st.Bind(0,identityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
		st.Finalize();

		m_log->error("IntroductionPuzzleRequester::HandleGetFailed fatal error requesting "+message["Identifier"]);
	}

	// remove this identityid from request list
	RemoveFromRequestList(identityid);

	return true;

}

void IntroductionPuzzleRequester::Initialize()
{
	m_fcpuniquename="IntroductionPuzzleRequester";
	m_maxrequests=0;
	Option option(m_db);
	option.GetInt("MaxIntroductionPuzzleRequests",m_maxrequests);
	if(m_maxrequests<1)
	{
		m_maxrequests=1;
		m_log->error("Option MaxIntroductionPuzzleRequests is currently set at less than 1.  It must be 1 or greater.");
	}
	if(m_maxrequests>100)
	{
		m_log->warning("Option MaxIntroductionPuzzleRequests is currently set at more than 100.  This value might be incorrectly configured.");
	}
}

void IntroductionPuzzleRequester::PopulateIDList()
{
	Poco::DateTime now;
	int id;
	std::string limitnum="30";

	// if we don't have an identity that we haven't seen yet, then set limit to 5
	SQLite3DB::Statement st=m_db->Prepare("SELECT tblLocalIdentity.LocalIdentityID FROM tblLocalIdentity LEFT JOIN tblIdentity ON tblLocalIdentity.PublicKey=tblIdentity.PublicKey WHERE tblIdentity.IdentityID IS NULL;");
	st.Step();
	if(!st.RowReturned())
	{
		limitnum="5";
	}
	st.Finalize();

	// select identities that aren't single use, are publishing a trust list, and have been seen today ( order by trust DESC and limit to limitnum )
	st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublishTrustList='true' AND PublicKey IS NOT NULL AND PublicKey <> '' AND SingleUse='false' AND (LocalTrustListTrust IS NULL OR LocalTrustListTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalTrustListTrust')) AND LastSeen>='"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"' AND FailureCount<=(SELECT OptionValue FROM tblOption WHERE Option='MaxFailureCount') ORDER BY LocalMessageTrust DESC LIMIT 0,"+limitnum+";");
	st.Step();

	m_ids.clear();

	while(st.RowReturned())
	{
		st.ResultInt(0,id);
		m_ids[id]=false;
		st.Step();
	}
}

void IntroductionPuzzleRequester::StartRequest(const long &identityid)
{
	Poco::DateTime now;
	FCPv2::Message message;
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

		now=Poco::Timestamp();

		SQLite3DB::Statement st2=m_db->Prepare("SELECT MAX(RequestIndex) FROM tblIntroductionPuzzleRequests WHERE Day=? AND IdentityID=?;");
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
		message["URI"]=publickey+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|IntroductionPuzzle|"+indexstr+".xml";
		message["Identifier"]=m_fcpuniquename+"|"+identityidstr+"|"+indexstr+"|"+message["URI"];
		message["ReturnType"]="direct";
		message["MaxSize"]="1000000";		// 1 MB

		m_fcp->Send(message);
		
		m_requesting.push_back(identityid);
	}

	m_ids[identityid]=true;
}
