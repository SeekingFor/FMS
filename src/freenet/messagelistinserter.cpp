#include "../../include/freenet/messagelistinserter.h"
#include "../../include/freenet/messagexml.h"
#include "../../include/freenet/messagelistxml.h"

#include <Poco/DateTime.h>
#include <Poco/Timespan.h>
#include <Poco/DateTimeFormatter.h>

#include <sstream>

#ifdef XMEM
	#include <xmem.h>
#endif

MessageListInserter::MessageListInserter(SQLite3DB::DB *db):IIndexInserter<long>(db)
{
	Initialize();
}

MessageListInserter::MessageListInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexInserter<long>(db,fcp)
{
	Initialize();
}

void MessageListInserter::CheckForNeededInsert()
{

	// more than 15 minutes trying to insert - restart
	if(m_inserting.size()>0 && (m_laststartedinsert+Poco::Timespan(0,0,15,0,0)<=Poco::DateTime()))
	{
		m_log->error("MessageListInserter::CheckForNeededInsert more than 15 minutes have passed without success/failure.  Clearing inserts.");
		m_lastinsertedxml[m_inserting[0]]="";
		m_inserting.clear();
	}

	// only do 1 insert at a time
	if(m_inserting.size()==0)
	{
		std::string sql;
		Poco::DateTime now;
		Poco::DateTime previous;
		bool startedinsert=false;

		// reset the last inserted xml doc to nothing if the day has changed
		if(m_lastchecked.day()!=now.day())
		{
			m_lastinsertedxml.clear();
		}

		previous-=Poco::Timespan(m_daysbackward,0,0,0,0);

		// query for identities that have messages in the past X days and (we haven't inserted lists for in the past 30 minutes OR identity has a record in tmpMessageListInsert)
		sql="SELECT tblLocalIdentity.LocalIdentityID ";
		sql+="FROM tblLocalIdentity INNER JOIN tblMessageInserts ON tblLocalIdentity.LocalIdentityID=tblMessageInserts.LocalIdentityID ";
		sql+="WHERE tblLocalIdentity.Active='true' AND tblMessageInserts.Day>=? AND ((tblLocalIdentity.LastInsertedMessageList<=? OR tblLocalIdentity.LastInsertedMessageList IS NULL OR tblLocalIdentity.LastInsertedMessageList='') OR tblLocalIdentity.LocalIdentityID IN (SELECT LocalIdentityID FROM tmpMessageListInsert)) AND tblLocalIdentity.PrivateKey IS NOT NULL AND tblLocalIdentity.PrivateKey <> '' ";
		sql+="GROUP BY tblLocalIdentity.LocalIdentityID ";
		sql+="ORDER BY tblLocalIdentity.LastInsertedMessageList;";

		SQLite3DB::Statement st=m_db->Prepare(sql);
		st.Bind(0,Poco::DateTimeFormatter::format(previous,"%Y-%m-%d"));
		st.Bind(1,Poco::DateTimeFormatter::format((now-Poco::Timespan(0,0,30,0,0)),"%Y-%m-%d %H:%M:%S"));
		st.Step();

		while(st.RowReturned() && startedinsert==false)
		{
			int localidentityid;
			st.ResultInt(0,localidentityid);
			startedinsert=StartInsert(localidentityid);
			st.Step();
		}
	}

}

const bool MessageListInserter::HandlePutFailed(FCPv2::Message &message)
{
	std::vector<std::string> idparts;
	//std::vector<std::string> uriparts;
	long localidentityid=0;
	long index=0;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	//StringFunctions::Split(message["URI"],"/",uriparts);

	StringFunctions::Convert(idparts[1],localidentityid);
	//StringFunctions::Convert(uriparts[2],index); - URI won't be set for USKs

	if(message["Fatal"]=="true" || message["Code"]=="9")
	{
		SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblMessageListInserts(LocalIdentityID,Day,InsertIndex,Inserted) VALUES(?,?,?,'false');");
		st.Bind(0,localidentityid);
		st.Bind(1,idparts[2]);
		st.Bind(2,index);
		st.Step();
	}

	RemoveFromInsertList(localidentityid);

	// reset the last inserted xml doc to nothing so we will try to insert this one again
	m_lastinsertedxml[localidentityid]="";

	m_log->trace("MessageListInserter::HandlePutFailed insert failed for "+message["Identifier"]);

	return true;

}

const bool MessageListInserter::HandlePutSuccessful(FCPv2::Message &message)
{
	Poco::DateTime now;
	std::vector<std::string> idparts;
	std::vector<std::string> uriparts;
	long localidentityid;
	long index;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Split(message["URI"],"/",uriparts);

	StringFunctions::Convert(idparts[1],localidentityid);
	StringFunctions::Convert(uriparts[2],index);

	SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblMessageListInserts(LocalIdentityID,Day,InsertIndex,Inserted) VALUES(?,?,?,'true');");
	st.Bind(0,localidentityid);
	st.Bind(1,idparts[2]);
	st.Bind(2,index);
	st.Step();

	now=Poco::Timestamp();
	st=m_db->Prepare("UPDATE tblLocalIdentity SET LastInsertedMessageList=? WHERE LocalIdentityID=?;");
	st.Bind(0,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
	st.Bind(1,localidentityid);
	st.Step();

	// delete only a single record from tmpMessageListInsert
	st=m_db->Prepare("SELECT MessageListInsertID FROM tmpMessageListInsert WHERE LocalIdentityID=?;");
	st.Bind(0,localidentityid);
	st.Step();
	if(st.RowReturned())
	{
		int id=-1;
		st.ResultInt(0,id);

		st=m_db->Prepare("DELETE FROM tmpMessageListInsert WHERE MessageListInsertID=?;");
		st.Bind(0,id);
		st.Step();
	}

	RemoveFromInsertList(localidentityid);

	m_log->debug("MessageListInserter::HandlePutSuccessful successfully inserted MessageList.");

	return true;
}

void MessageListInserter::Initialize()
{
	std::string tempval("");
	m_fcpuniquename="MessageListInserter";
	m_daysbackward=0;
	Option option(m_db);

	option.Get("MessageListDaysBackward",tempval);
	StringFunctions::Convert(tempval,m_daysbackward);
}

const bool MessageListInserter::StartInsert(const long &localidentityid)
{
	FCPv2::Message message;
	Poco::DateTime date;
	Poco::DateTime now;
	std::string privatekey;
	std::string localidentityidstr;
	MessageListXML mlxml;
	MessageXML messxml;
	std::string xmlstr;
	std::string xmlsizestr;
	int index;
	std::string indexstr;
	int messagecount=0;

	date-=Poco::Timespan(m_daysbackward,0,0,0,0);
	StringFunctions::Convert(localidentityid,localidentityidstr);

	m_db->Execute("BEGIN;");

	SQLite3DB::Statement st=m_db->Prepare("SELECT PrivateKey FROM tblLocalIdentity WHERE LocalIdentityID=?;");
	st.Bind(0,localidentityid);
	st.Step();
	if(st.RowReturned())
	{
		st.ResultText(0,privatekey);
	}

	st=m_db->Prepare("SELECT Day, InsertIndex, MessageXML FROM tblMessageInserts INNER JOIN tblLocalIdentity ON tblMessageInserts.LocalIdentityID=tblLocalIdentity.LocalIdentityID WHERE tblLocalIdentity.LocalIdentityID=? AND Day>=? AND tblMessageInserts.MessageUUID IS NOT NULL;");
	st.Bind(0,localidentityid);
	st.Bind(1,Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
	st.Step();

	while(st.RowReturned())
	{
		std::string day="";
		int index=-1;
		std::string xmlstr="";
		std::vector<std::string> boards;

		st.ResultText(0,day);
		st.ResultInt(1,index);
		st.ResultText(2,xmlstr);

		messxml.ParseXML(xmlstr);

		mlxml.AddMessage(day,index,messxml.GetBoards());

		st.Step();
		messagecount++;
	}
	st.Finalize();

	m_db->Execute("COMMIT;");
	m_db->Execute("BEGIN;");

	// Add any other messages from this local identity that were inserted by another client
	st=m_db->Prepare("SELECT MessageID, InsertDate, MessageIndex \
					FROM tblMessage INNER JOIN tblIdentity ON tblMessage.IdentityID=tblIdentity.IdentityID \
					INNER JOIN tblLocalIdentity ON tblIdentity.PublicKey=tblLocalIdentity.PublicKey\
					WHERE tblLocalIdentity.LocalIdentityID=? \
					AND tblMessage.InsertDate>? \
					AND tblMessage.MessageUUID NOT IN \
					(SELECT MessageUUID FROM tblMessageInserts WHERE Inserted='true' AND Day>?);");
	st.Bind(0,localidentityid);
	st.Bind(1,Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
	st.Bind(2,Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
	st.Step();

	SQLite3DB::Statement st2=m_db->Prepare("SELECT BoardName FROM tblBoard INNER JOIN tblMessageBoard ON tblBoard.BoardID=tblMessageBoard.BoardID WHERE tblMessageBoard.MessageID=?;");

	while(st.RowReturned())
	{
		std::string day="";
		int index=-1;
		int messageid=-1;
		std::vector<std::string> boardlist;

		st.ResultInt(0,messageid);
		st.ResultText(1,day);
		st.ResultInt(2,index);
		
		st2.Bind(0,messageid);
		st2.Step();
		while(st2.RowReturned())
		{
			std::string boardname="";
			st2.ResultText(0,boardname);
			StringFunctions::LowerCase(boardname,boardname);
			boardlist.push_back(boardname);
			st2.Step();
		}
		st2.Reset();

		mlxml.AddMessage(day,index,boardlist);

		st.Step();
	}
	st.Finalize();

	m_db->Execute("COMMIT;");
	m_db->Execute("BEGIN;");

	if(messagecount<600)
	{
		std::ostringstream limitstr;
		limitstr << (600-messagecount);

		st=m_db->Prepare("SELECT MessageDate, MessageIndex, PublicKey, MessageID, InsertDate FROM tblMessage INNER JOIN tblIdentity ON tblMessage.IdentityID=tblIdentity.IdentityID WHERE MessageIndex IS NOT NULL ORDER BY MessageDate DESC, MessageTime DESC LIMIT "+limitstr.str()+";");
		st.Step();

		while(st.RowReturned())
		{
			std::string day="";
			int index=0;
			std::string publickey="";
			std::vector<std::string> boardlist;
			int messageid=0;
			std::string insertdate="";
			
			st.ResultText(0,day);
			st.ResultInt(1,index);
			st.ResultText(2,publickey);
			st.ResultInt(3,messageid);
			st.ResultText(4,insertdate);

			st2.Bind(0,messageid);
			st2.Step();
			while(st2.RowReturned())
			{
				std::string boardname="";
				st2.ResultText(0,boardname);
				StringFunctions::LowerCase(boardname,boardname);
				boardlist.push_back(boardname);
				st2.Step();
			}
			st2.Reset();

			// TODO - remove insertdate empty check sometime after 0.3.32 release and get rid of using day
			if(insertdate!="")
			{
				mlxml.AddExternalMessage(publickey,insertdate,index,boardlist);
			}
			else
			{
				mlxml.AddExternalMessage(publickey,day,index,boardlist);
			}

			st.Step();
		}
	}

	// get last inserted messagelist index for this day
	// apparently the list might be inserted without the node notifying us, so we need to look at the message list requests as well for the identity
	index=0;
	st=m_db->Prepare("SELECT IFNULL(MAX(ListIndex)+1,0) \
					FROM \
					(\
					SELECT MAX(InsertIndex) AS ListIndex FROM tblMessageListInserts WHERE LocalIdentityID=? AND Day=?\
					UNION\
					SELECT MAX(RequestIndex) AS ListIndex FROM tblMessageListRequests INNER JOIN tblIdentity ON tblMessageListRequests.IdentityID=tblIdentity.IdentityID INNER JOIN tblLocalIdentity ON tblLocalIdentity.PublicKey=tblIdentity.PublicKey WHERE tblLocalIdentity.LocalIdentityID=? AND tblMessageListRequests.Day=?\
					);");
	st.Bind(0,localidentityid);
	st.Bind(1,Poco::DateTimeFormatter::format(now,"%Y-%m-%d"));
	st.Bind(2,localidentityid);
	st.Bind(3,Poco::DateTimeFormatter::format(now,"%Y-%m-%d"));
	st.Step();
	if(st.ResultNull(0)==false)
	{
		st.ResultInt(0,index);
	}
	StringFunctions::Convert(index,indexstr);

	m_db->Execute("COMMIT;");

	xmlstr=mlxml.GetXML();

	// only insert if the last message this identity inserted is different than this message
	if(m_lastinsertedxml[localidentityid]!=xmlstr)
	{
		StringFunctions::Convert(xmlstr.size(),xmlsizestr);

		/*
		message.SetName("ClientPut");
		message["URI"]=privatekey+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|MessageList|"+indexstr+".xml";
		message["Identifier"]=m_fcpuniquename+"|"+localidentityidstr+"|"+indexstr+"|"+message["URI"];
		message["UploadFrom"]="direct";
		message["DataLength"]=xmlsizestr;
		m_fcp->Send(message);
		m_fcp->Send(std::vector<char>(xmlstr.begin(),xmlstr.end()));
		message.Clear();
		*/

		message.SetName("ClientPutComplexDir");
		// don't insert into edition 0 because 1208 has major issues with this
		message["URI"]="USK"+privatekey.substr(3)+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y.%m.%d")+"|MessageList/"+indexstr+"/";
		message["Identifier"]=m_fcpuniquename+"|"+localidentityidstr+"|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|"+message["URI"];
		message["PriorityClass"]=m_defaultinsertpriorityclassstr;
		message["DefaultName"]="MessageList.xml";
		message["Files.0.Name"]="MessageList.xml";
		message["Files.0.UploadFrom"]="direct";
		message["Files.0.DataLength"]=xmlsizestr;
		m_fcp->Send(message);
		m_fcp->Send(std::vector<char>(xmlstr.begin(),xmlstr.end()));

		m_inserting.push_back(localidentityid);
		m_lastinsertedxml[localidentityid]=xmlstr;

		m_laststartedinsert=Poco::DateTime();

		m_log->trace("MessageListInserter::StartInsert started insert of "+message["Identifier"]);

		return true;
	}
	else
	{

		// xml was the same one that we inserted 30 minutes ago, reset date so we don't continue checking every minute
		st=m_db->Prepare("UPDATE tblLocalIdentity SET LastInsertedMessageList=? WHERE LocalIdentityID=?;");
		st.Bind(0,Poco::DateTimeFormatter::format(Poco::DateTime(),"%Y-%m-%d %H:%M:%S"));
		st.Bind(1,localidentityid);
		st.Step();

		return false;
	}

}
