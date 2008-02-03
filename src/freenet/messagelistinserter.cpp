#include "../../include/freenet/messagelistinserter.h"
#include "../../include/freenet/messagexml.h"
#include "../../include/freenet/messagelistxml.h"

#ifdef XMEM
	#include <xmem.h>
#endif

MessageListInserter::MessageListInserter()
{
	Initialize();
}

MessageListInserter::MessageListInserter(FCPv2 *fcp):IIndexInserter<long>(fcp)
{
	Initialize();
}

void MessageListInserter::CheckForNeededInsert()
{
	// only do 1 insert at a time
	if(m_inserting.size()==0)
	{
		std::string sql;
		DateTime now;
		DateTime previous;

		now.SetToGMTime();
		previous.SetToGMTime();

		previous.Add(0,0,0,-m_daysbackward);

		// query for identities that have messages in the past X days and (we haven't inserted lists for in the past 30 minutes OR identity has a record in tmpMessageListInsert)
		sql="SELECT tblLocalIdentity.LocalIdentityID ";
		sql+="FROM tblLocalIdentity INNER JOIN tblMessageInserts ON tblLocalIdentity.LocalIdentityID=tblMessageInserts.LocalIdentityID ";
		sql+="WHERE tblMessageInserts.Day>=? AND ((tblLocalIdentity.LastInsertedMessageList<=? OR tblLocalIdentity.LastInsertedMessageList IS NULL OR tblLocalIdentity.LastInsertedMessageList='') OR tblLocalIdentity.LocalIdentityID IN (SELECT LocalIdentityID FROM tmpMessageListInsert)) ";
		sql+=";";

		SQLite3DB::Statement st=m_db->Prepare(sql);
		st.Bind(0,previous.Format("%Y-%m-%d"));
		st.Bind(1,(now-(1.0/48.0)).Format("%Y-%m-%d %H:%M:%S"));
		st.Step();

		if(st.RowReturned())
		{
			int localidentityid;
			st.ResultInt(0,localidentityid);
			StartInsert(localidentityid);
		}
	}

}

const bool MessageListInserter::HandlePutFailed(FCPMessage &message)
{
	std::vector<std::string> idparts;
	long localidentityid;
	long index;

	StringFunctions::Split(message["Identifier"],"|",idparts);
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

	return true;

}

const bool MessageListInserter::HandlePutSuccessful(FCPMessage &message)
{
	DateTime now;
	std::vector<std::string> idparts;
	long localidentityid;
	long index;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[1],localidentityid);
	StringFunctions::Convert(idparts[2],index);

	SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblMessageListInserts(LocalIdentityID,Day,InsertIndex,Inserted) VALUES(?,?,?,'true');");
	st.Bind(0,localidentityid);
	st.Bind(1,idparts[4]);
	st.Bind(2,index);
	st.Step();

	now.SetToGMTime();
	st=m_db->Prepare("UPDATE tblLocalIdentity SET LastInsertedMessageList=? WHERE LocalIdentityID=?;");
	st.Bind(0,now.Format("%Y-%m-%d %H:%M:%S"));
	st.Bind(1,localidentityid);
	st.Step();

	// delete any record from tmpMessageListInsert
	st=m_db->Prepare("DELETE FROM tmpMessageListInsert WHERE LocalIdentityID=?;");
	st.Bind(0,localidentityid);
	st.Step();

	RemoveFromInsertList(localidentityid);

	m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"MessageListInserter::HandlePutSuccessful successfully inserted MessageList.");

	return true;
}

void MessageListInserter::Initialize()
{
	std::string tempval;

	m_fcpuniquename="MessageListInserter";
	m_daysbackward=0;
	Option::Instance()->Get("MessageListDaysBackward",tempval);
	StringFunctions::Convert(tempval,m_daysbackward);
}

void MessageListInserter::StartInsert(const long &localidentityid)
{
	FCPMessage message;
	DateTime date;
	DateTime now;
	std::string privatekey;
	std::string localidentityidstr;
	MessageListXML mlxml;
	MessageXML messxml;
	std::string xmlstr;
	std::string xmlsizestr;
	int index;
	std::string indexstr;

	now.SetToGMTime();
	date.SetToGMTime();
	date.Add(0,0,0,-m_daysbackward);
	StringFunctions::Convert(localidentityid,localidentityidstr);

	SQLite3DB::Statement st=m_db->Prepare("SELECT Day, InsertIndex, MessageXML, PrivateKey FROM tblMessageInserts INNER JOIN tblLocalIdentity ON tblMessageInserts.LocalIdentityID=tblLocalIdentity.LocalIdentityID WHERE tblLocalIdentity.LocalIdentityID=? AND Day>=?;");
	st.Bind(0,localidentityid);
	st.Bind(1,date.Format("%Y-%m-%d"));
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

	// get last inserted messagelist index for this day
	index=0;
	st=m_db->Prepare("SELECT MAX(InsertIndex) FROM tblMessageListInserts WHERE LocalIdentityID=? AND Day=?;");
	st.Bind(0,localidentityid);
	st.Bind(1,now.Format("%Y-%m-%d"));
	st.Step();
	if(st.ResultNull(0)==false)
	{
		st.ResultInt(0,index);
		index++;
	}
	StringFunctions::Convert(index,indexstr);

	// actually insert message
	xmlstr=mlxml.GetXML();
	StringFunctions::Convert(xmlstr.size(),xmlsizestr);

	message.SetName("ClientPut");
	message["URI"]=privatekey+m_messagebase+"|"+now.Format("%Y-%m-%d")+"|MessageList|"+indexstr+".xml";
	message["Identifier"]=m_fcpuniquename+"|"+localidentityidstr+"|"+indexstr+"|"+message["URI"];
	message["UploadFrom"]="direct";
	message["DataLength"]=xmlsizestr;
	m_fcp->SendMessage(message);
	m_fcp->SendRaw(xmlstr.c_str(),xmlstr.size());

	m_inserting.push_back(localidentityid);

}
