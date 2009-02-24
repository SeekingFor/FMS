#include "../../include/freenet/messagelistinserter.h"
#include "../../include/freenet/messagexml.h"
#include "../../include/freenet/messagelistxml.h"

#include <Poco/DateTime.h>
#include <Poco/Timespan.h>
#include <Poco/DateTimeFormatter.h>

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

	// more than 10 minutes trying to insert - restart
	if(m_inserting.size()>0 && (m_laststartedinsert+Poco::Timespan(0,0,10,0,0)<=Poco::DateTime()))
	{
		m_log->error("MessageListInserter::CheckForNeededInsert more than 10 minutes have passed without success/failure.  Clearing inserts.");
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
		sql+="WHERE tblMessageInserts.Day>=? AND ((tblLocalIdentity.LastInsertedMessageList<=? OR tblLocalIdentity.LastInsertedMessageList IS NULL OR tblLocalIdentity.LastInsertedMessageList='') OR tblLocalIdentity.LocalIdentityID IN (SELECT LocalIdentityID FROM tmpMessageListInsert)) ";
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
	long localidentityid;
	long index;

	StringFunctions::Split(message["Identifier"],"|",idparts);

	// non USK
	if(idparts[0]==m_fcpuniquename)
	{
		StringFunctions::Convert(idparts[1],localidentityid);
		StringFunctions::Convert(idparts[2],index);

		if(message["Fatal"]=="true" || message["Code"]=="9")
		{
			SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblMessageListInserts(LocalIdentityID,Day,InsertIndex,Inserted) VALUES(?,?,?,'false');");
			st.Bind(0,localidentityid);
			st.Bind(1,idparts[4]);
			st.Bind(2,index);
			st.Step();
		}

		RemoveFromInsertList(localidentityid);

		// reset the last inserted xml doc to nothing so we will try to insert this one again
		m_lastinsertedxml[localidentityid]="";
	}
	else
	{
		m_log->debug("MessageListInserter::HandlePutFailed "+message["Identifier"]);
	}

	return true;

}

const bool MessageListInserter::HandlePutSuccessful(FCPv2::Message &message)
{
	Poco::DateTime now;
	std::vector<std::string> idparts;
	long localidentityid;
	long index;

	StringFunctions::Split(message["Identifier"],"|",idparts);

	// non USK
	if(idparts[0]==m_fcpuniquename)
	{
		StringFunctions::Convert(idparts[1],localidentityid);
		StringFunctions::Convert(idparts[2],index);

		SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblMessageListInserts(LocalIdentityID,Day,InsertIndex,Inserted) VALUES(?,?,?,'true');");
		st.Bind(0,localidentityid);
		st.Bind(1,idparts[4]);
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
	}
	else
	{
		m_log->debug("MessageListInserter::HandlePutSuccessful inserted USK MessageList "+message["Identifier"]);
	}

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

	date-=Poco::Timespan(m_daysbackward,0,0,0,0);
	StringFunctions::Convert(localidentityid,localidentityidstr);

	SQLite3DB::Statement st=m_db->Prepare("SELECT Day, InsertIndex, MessageXML, PrivateKey FROM tblMessageInserts INNER JOIN tblLocalIdentity ON tblMessageInserts.LocalIdentityID=tblLocalIdentity.LocalIdentityID WHERE tblLocalIdentity.LocalIdentityID=? AND Day>=? AND tblMessageInserts.MessageUUID IS NOT NULL;");
	st.Bind(0,localidentityid);
	st.Bind(1,Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
	st.Step();

	while(st.RowReturned())
	{
		std::string day;
		int index;
		std::string xmlstr;
		std::vector<std::string> boards;

		st.ResultText(0,day);
		st.ResultInt(1,index);
		st.ResultText(2,xmlstr);
		st.ResultText(3,privatekey);

		messxml.ParseXML(xmlstr);

		mlxml.AddMessage(day,index,messxml.GetBoards());

		st.Step();
	}
	st.Finalize();


	st=m_db->Prepare("SELECT MessageDate, MessageIndex, PublicKey, MessageID, InsertDate FROM tblMessage INNER JOIN tblIdentity ON tblMessage.IdentityID=tblIdentity.IdentityID WHERE MessageIndex IS NOT NULL ORDER BY MessageDate DESC, MessageTime DESC LIMIT 175;");
	SQLite3DB::Statement st2=m_db->Prepare("SELECT BoardName FROM tblBoard INNER JOIN tblMessageBoard ON tblBoard.BoardID=tblMessageBoard.BoardID WHERE tblMessageBoard.MessageID=?;");
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

	// get last inserted messagelist index for this day
	index=0;
	st=m_db->Prepare("SELECT MAX(InsertIndex) FROM tblMessageListInserts WHERE LocalIdentityID=? AND Day=?;");
	st.Bind(0,localidentityid);
	st.Bind(1,Poco::DateTimeFormatter::format(now,"%Y-%m-%d"));
	st.Step();
	if(st.ResultNull(0)==false)
	{
		st.ResultInt(0,index);
		index++;
	}
	StringFunctions::Convert(index,indexstr);

	xmlstr=mlxml.GetXML();

	// only insert if the last message this identity inserted is different than this message
	if(m_lastinsertedxml[localidentityid]!=xmlstr)
	{
		std::string targeturi="";
		StringFunctions::Convert(xmlstr.size(),xmlsizestr);

		message.SetName("ClientPut");
		message["URI"]=privatekey+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|MessageList|"+indexstr+".xml";
		message["Identifier"]=m_fcpuniquename+"|"+localidentityidstr+"|"+indexstr+"|"+message["URI"];
		message["UploadFrom"]="direct";
		message["DataLength"]=xmlsizestr;
		m_fcp->Send(message);
		m_fcp->Send(std::vector<char>(xmlstr.begin(),xmlstr.end()));

		message.Clear();
		message.SetName("ClientPutComplexDir");
		message["URI"]="USK"+privatekey.substr(3)+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y.%m.%d")+"|MessageList/0/";
		message["Identifier"]=m_fcpuniquename+"USK|"+message["URI"];
		message["DefaultName"]="MessageList.xml";
		message["Files.0.Name"]="MessageList.xml";
		message["Files.0.UploadFrom"]="direct";
		message["Files.0.DataLength"]=xmlsizestr;
		m_fcp->Send(message);
		m_fcp->Send(std::vector<char>(xmlstr.begin(),xmlstr.end()));

		m_inserting.push_back(localidentityid);
		m_lastinsertedxml[localidentityid]=xmlstr;

		m_laststartedinsert=Poco::DateTime();

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
