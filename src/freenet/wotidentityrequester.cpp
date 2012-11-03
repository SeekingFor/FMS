#include "../../include/freenet/wotidentityrequester.h"
#include "../../include/freenet/wotidentityxml.h"
#include "../../include/freenet/identitypublickeycache.h"
#include "../../include/freenet/freenetkeys.h"
#include "../../include/option.h"
#include "../../include/unicode/unicodestring.h"

#include <Poco/DateTime.h>
#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>
#include <Poco/DateTimeFormatter.h>

WOTIdentityRequester::WOTIdentityRequester(SQLite3DB::DB *db):IIndexRequester<std::pair<long,long> >(db)
{
	Initialize();
}

WOTIdentityRequester::WOTIdentityRequester(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexRequester<std::pair<long,long> >(db,fcp)
{
	Initialize();
}

const std::pair<long,long> WOTIdentityRequester::GetIDFromIdentifier(const std::string &identifier)
{
	long identityid;
	long identityorder;

	std::vector<std::string> idparts;
	StringFunctions::Split(identifier,"|",idparts);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[3],identityorder);

	return std::pair<long,long>(identityorder,identityid);
}

const bool WOTIdentityRequester::HandleAllData(FCPv2::Message &message)
{
	std::vector<std::string> idparts;
	std::vector<char> data;
	std::vector<char>::size_type datalength=0;
	WOTIdentityXML xml;
	long identityid;
	long index;
	long identityorder;
	SQLite3DB::Transaction trans(m_db);
	std::string idstr("");
	std::string indexstr("");
	Poco::DateTime now;
	int insertcount=0;
	int dayinsertcount=0;
	int previnsertcount=0;
	std::string publisherid("");
	FreenetUSKKey usk;
	FreenetSSKKey ssk;

	trans.Begin(SQLite3DB::Transaction::TRANS_IMMEDIATE);

	m_log->debug(m_fcpuniquename+"::HandleAllData for "+message["Identifier"]);

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(message["DataLength"],datalength);
	StringFunctions::Convert(idparts[1],identityid);
	StringFunctions::Convert(idparts[2],index);
	StringFunctions::Convert(idparts[3],identityorder);

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

	if(data.size()>0 && xml.ParseXML(std::string(data.begin(),data.end()))==true)
	{
		UnicodeString name;
		std::string puzzlecount("");
		bool savenewidentities=false;

		std::string prevname("");
		std::string publickey("");
		SQLite3DB::Statement st=m_db->Prepare("SELECT Name, PublicKey FROM tblIdentity WHERE IdentityID=?;");
		st.Bind(0,identityid);
		st.Step();
		if(st.RowReturned())
		{
			st.ResultText(0,prevname);
			st.ResultText(1,publickey);
		}
		st.Finalize();

		// see if we will save new identities found on the trust list, and get name and public key
		// allow adding new ids if trust is higher than min trust, or trust is null
		st=m_db->Prepare("SELECT IFNULL(LocalTrustListTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalTrustListTrust'),1), Name, PublicKey FROM tblIdentity WHERE IdentityID=?;");
		st.Bind(0,identityid);
		trans.Step(st);
		if(st.RowReturned())
		{
			int result=0;
			std::string publishername("");
			std::string publisherpublickey("");
			st.ResultInt(0,result);
			if(result==1)
			{
				savenewidentities=true;
			}
			st.ResultText(1,publishername);
			st.ResultText(2,publisherpublickey);
			publisherid=publishername;
			if(publisherpublickey.size()>50)
			{
				publisherid+=publisherpublickey.substr(3,44);
			}
		}

		// get count of identities added in last 24 hours
		st=m_db->Prepare("SELECT COUNT(*) FROM tblIdentity WHERE DateAdded>=? AND IsWOT=1;");
		now-=Poco::Timespan(1,0,0,0,0);
		st.Bind(0,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
		trans.Step(st);
		if(st.RowReturned())
		{
			if(st.ResultNull(0)==false)
			{
				st.ResultInt(0,dayinsertcount);
			}
		}
		else
		{
			m_log->error(m_fcpuniquename+"::HandleAllData couldn't get count of identities added in last 24 hours");
		}

		// get count of identities added more than 24 hours ago - if 0 then we will accept more than 100 identities now
		st=m_db->Prepare("SELECT COUNT(*) FROM tblIdentity WHERE DateAdded<? AND IsWOT=1;");
		st.Bind(0,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
		trans.Step(st);
		if(st.RowReturned())
		{
			if(st.ResultNull(0)==false)
			{
				st.ResultInt(0,previnsertcount);
			}
		}
		else
		{
			m_log->error(m_fcpuniquename+"::HandleAllData couldn't get count of identities added more than 24 hours ago");
		}

		SQLite3DB::Statement dst=m_db->Prepare("DELETE FROM tblWOTIdentityProperty WHERE IdentityID=?;");
		dst.Bind(0,idstr);
		trans.Step(dst);

		SQLite3DB::Statement pst=m_db->Prepare("INSERT INTO tblWOTIdentityProperty(IdentityID,Property,Value) VALUES(?,?,?);");

		for(std::vector<std::pair<std::string,std::string> >::const_iterator i=xml.GetProperties().begin(); i!=xml.GetProperties().end(); i++)
		{
			pst.Bind(0,idstr);
			pst.Bind(1,(*i).first);
			pst.Bind(2,(*i).second);
			trans.Step(pst);
			trans.Reset(pst);
		}

		dst=m_db->Prepare("DELETE FROM tblWOTIdentityContext WHERE IdentityID=?;");
		dst.Bind(0,idstr);
		trans.Step(dst);

		SQLite3DB::Statement cst=m_db->Prepare("INSERT INTO tblWOTIdentityContext(IdentityID,Context) VALUES(?,?);");

		for(std::vector<std::string>::const_iterator i=xml.GetContexts().begin(); i!=xml.GetContexts().end(); i++)
		{
			cst.Bind(0,idstr);
			cst.Bind(1,(*i));
			trans.Step(cst);
			trans.Reset(cst);
		}

		name=xml.GetName();
		name.Trim(MAX_IDENTITY_NAME_LENGTH);

		st=m_db->Prepare("UPDATE tblIdentity SET Name=?, WOTLastSeen=datetime('now'), WOTLastIndex=?, WOTLastRequest=datetime('now') WHERE IdentityID=?;");
		st.Bind(0,StringFunctions::RemoveControlChars(name.NarrowString()));
		st.Bind(1,indexstr);
		st.Bind(2,idstr);
		if(trans.Step(st)==false)
		{
			m_log->debug(m_fcpuniquename+"::HandleAllData couldn't update record "+idstr+" "+message["Identifier"]);
		}

		if(name.NarrowString()!=prevname)
		{
			UpdateMissingAuthorID(m_db,identityid,name.NarrowString(),publickey);
		}

		if(savenewidentities==true)
		{
			SQLite3DB::Statement findst=m_db->Prepare("SELECT IdentityID, IsWOT FROM tblIdentity WHERE PublicKey=?;");
			SQLite3DB::Statement idupdst=m_db->Prepare("UPDATE tblIdentity SET IsWOT=1 WHERE IdentityID=?;");
			st=m_db->Prepare("INSERT INTO tblIdentity(PublicKey,DateAdded,IsWOT,AddedMethod) VALUES(?,datetime('now'),1,?);");
			for(std::vector<WOTIdentityXML::trust>::const_iterator i=xml.GetTrustList().begin(); i!=xml.GetTrustList().end(); i++)
			{
				if(usk.TryParse((*i).m_identity)==true)
				{
					ssk=usk;

					findst.Bind(0,ssk.GetBaseKey());
					trans.Step(findst);
					if(findst.RowReturned()==false)
					{
						// allow up to 10 new identities per downloaded trust list, where total inserted in the last 24 hours may not exceed 1/10 the total number of identities we know about or 500 (whichever is smaller, minimum 10)
						// 24 hour limit is lifted if the database didn't contain any identities inserted more than 24 hours ago (new db) - 100 new identities per trust list allowed in this case
						if((insertcount<100 && previnsertcount==0) || (insertcount<10 && dayinsertcount<((std::min)(((std::max)(previnsertcount/10,10)),500))))
						{
							st.Bind(0,ssk.GetBaseKey());
							st.Bind(1,"WOT trust list of "+publisherid);
							trans.Step(st);
							trans.Reset(st);
						}
						insertcount++;
						dayinsertcount++;
					}
					else
					{
						int id=0;
						int oldiswot=0;
						int iswot=1;
						findst.ResultInt(0,id);
						findst.ResultInt(1,oldiswot);
						// only change from 0 to 1
						if(oldiswot==0 && iswot==1)
						{
							idupdst.Bind(0,id);
							trans.Step(idupdst);
							trans.Reset(idupdst);
						}
					}
					trans.Reset(findst);
				}
				else
				{
					m_log->error(m_fcpuniquename+"::HandleAllData invalid USK "+(*i).m_identity+" in "+message["Identifier"]);
				}
			}
		}

	}
	else
	{
		m_log->error(m_fcpuniquename+"::HandleAllData trouble with "+message["Identifier"]);
	}

	trans.Commit();

	// remove this identityid from request list
	RemoveFromRequestList(std::pair<long,long>(identityorder,identityid));

	return true;
}

const bool WOTIdentityRequester::HandleGetFailed(FCPv2::Message &message)
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
				if(newedition>0)
				{
					newedition--;
				}
				SQLite3DB::Statement st=m_db->Prepare("UPDATE tblIdentity SET WOTLastIndex=? WHERE IdentityID=?;");
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
		SQLite3DB::Statement failst=m_db->Prepare("UPDATE tblIdentity SET FailureCount=FailureCount+1, WOTLastRequest=datetime('now') WHERE IdentityID=?;");
		failst.Bind(0,identityid);
		failst.Step();

		m_log->error(m_fcpuniquename+"::HandleGetFailed handled code "+message["Code"]+" for "+message["Identifier"]);
	}

	return true;

}

void WOTIdentityRequester::Initialize()
{
	Option option(m_db);
	int previnsertcount=0;
	bool savewot=false;

	// get count of identities added more than 24 hours ago - if 0 then we will accept more than 100 identities now
	Poco::DateTime onedayago;
	onedayago-=Poco::Timespan(1,0,0,0,0);
	SQLite3DB::Statement st=m_db->Prepare("SELECT COUNT(*) FROM tblIdentity WHERE DateAdded<?;");
	st.Bind(0,Poco::DateTimeFormatter::format(onedayago,"%Y-%m-%d %H:%M:%S"));
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
		m_log->error("WOTIdentityRequester::Initialize couldn't get count of identities added more than 24 hours ago");
	}

	m_fcpuniquename="WOTIdentityRequester";


	option.GetBool("WOTDownloadIdentities",savewot);
	if(savewot==false)
	{
		m_maxrequests=0;
	}
	else
	{
		option.GetInt("WOTMaxIdentityRequests",m_maxrequests);

		if(m_maxrequests>100)
		{
			m_log->warning("Option WOTMaxIdentityRequests is currently set at more than 100.  This value might be incorrectly configured.");
		}
		if(previnsertcount==0)
		{
			m_maxrequests=(std::max)(m_maxrequests,10);
		}
		else
		{
			std::string previnsertcountstr;
			StringFunctions::Convert(previnsertcount,previnsertcountstr);
			m_log->trace("WOTIdentityRequester::Initialize previnsertcount is "+previnsertcountstr);
		}

	}

}

void WOTIdentityRequester::PopulateIDList()
{
	if(m_maxrequests>0)
	{
		SQLite3DB::Transaction trans(m_db);

		// only selects, deferred OK
		trans.Begin();

		int count=0;
		std::string countstr("");
		SQLite3DB::Statement st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE (WOTLastRequest IS NULL OR WOTLastRequest<datetime('now','-24 hours')) AND IsWOT=1;");
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
		m_log->trace("WOTIdentityRequester::PopulateIDList populated "+countstr+" ids");
	}

}

void WOTIdentityRequester::StartRequest(const std::pair<long,long> &inputpair)
{
	const long identityorder=inputpair.first;
	const long identityid=inputpair.second;
	std::string publickey("");
	int index=0;
	std::string indexstr("");
	std::string identityidstr("");
	std::string identityorderstr("");
	IdentityPublicKeyCache pkcache(m_db);
	FCPv2::Message message;

	if(pkcache.PublicKey(identityid,publickey) && publickey.size()>4)
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT IFNULL(WOTLastIndex+1,0) FROM tblIdentity WHERE tblIdentity.IdentityID=?;");
		st.Bind(0,identityid);
		st.Step();
		if(st.RowReturned())
		{
			st.ResultInt(0,index);
			StringFunctions::Convert(index,indexstr);
			StringFunctions::Convert(identityid,identityidstr);
			StringFunctions::Convert(identityorder,identityorderstr);

			message.SetName("ClientGet");
			message["URI"]="USK"+publickey.substr(3)+"WebOfTrust/"+indexstr;
			message["Identifier"]=m_fcpuniquename+"|"+identityidstr+"|"+indexstr+"|"+identityorderstr+"|"+message["URI"];
			message["ReturnType"]="direct";
			message["MaxSize"]="10000000";
			m_fcp->Send(message);

			StartedRequest(inputpair,message["Identifier"]);

			m_log->trace(m_fcpuniquename+"::StartRequest started request for "+message["Identifier"]);
		}
	}

	m_ids[inputpair].m_requested=true;

}
