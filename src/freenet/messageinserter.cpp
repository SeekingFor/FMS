#include "../../include/freenet/messageinserter.h"

MessageInserter::MessageInserter()
{
	Initialize();
}

MessageInserter::MessageInserter(FCPv2 *fcp):IIndexInserter(fcp)
{
	Initialize();
}

void MessageInserter::CheckForNeededInsert()
{
	// only do 1 insert at a time
	if(m_inserting.size()==0)
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT MessageUUID FROM tblMessageInserts INNER JOIN tblLocalIdentity ON tblMessageInserts.LocalIdentityID=tblLocalIdentity.LocalIdentityID WHERE tblLocalIdentity.PrivateKey IS NOT NULL AND tblLocalIdentity.PrivateKey <> '' AND tblMessageInserts.Inserted='false';");
		st.Step();

		if(st.RowReturned())
		{
			std::string messageuuid;
			st.ResultText(0,messageuuid);
			StartInsert(messageuuid);
		}
	}
}

const bool MessageInserter::HandlePutFailed(FCPMessage &message)
{
	int index;
	int localidentityid;
	std::vector<std::string> idparts;
	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[2],localidentityid);
	StringFunctions::Convert(idparts[3],index);

	// fatal put - or data exists - insert bogus index into database so we'll try to insert this message again
	if(message["Fatal"]=="true" || message["Code"]=="9")
	{
		SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblMessageInserts(LocalIdentityID,Day,InsertIndex,Inserted) VALUES(?,?,?,'true');");
		st.Bind(0,localidentityid);
		st.Bind(1,idparts[5]);
		st.Bind(2,index);
		st.Step();
	}

	RemoveFromInsertList(idparts[1]);

	return true;
}

const bool MessageInserter::HandlePutSuccessful(FCPMessage &message)
{
	int index;
	std::vector<std::string> idparts;
	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[3],index);

	SQLite3DB::Statement st=m_db->Prepare("UPDATE tblMessageInserts SET Day=?, InsertIndex=?, Inserted='true' WHERE MessageUUID=?;");
	st.Bind(0,idparts[5]);
	st.Bind(1,index);
	st.Bind(2,idparts[1]);
	st.Step();

	RemoveFromInsertList(idparts[1]);

	m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"MessageInserter::HandlePutSuccessful successfully inserted message "+message["Identifier"]);

	return true;
}

void MessageInserter::Initialize()
{
	m_fcpuniquename="MessageInserter";
}

void MessageInserter::StartInsert(const std::string &messageuuid)
{
	DateTime now;
	now.SetToGMTime();
	SQLite3DB::Statement st=m_db->Prepare("SELECT MessageXML,PrivateKey,tblLocalIdentity.LocalIdentityID FROM tblMessageInserts INNER JOIN tblLocalIdentity ON tblMessageInserts.LocalIdentityID=tblLocalIdentity.LocalIdentityID WHERE MessageUUID=?;");
	st.Bind(0,messageuuid);
	st.Step();

	if(st.RowReturned())
	{
		int localidentityid;
		std::string idstr;
		std::string xml;
		std::string xmlsizestr;
		std::string privatekey;
		FCPMessage message;
		std::string indexstr;
		int index=0;
		
		st.ResultText(0,xml);
		st.ResultText(1,privatekey);
		st.ResultInt(2,localidentityid);
		StringFunctions::Convert(xml.size(),xmlsizestr);
		StringFunctions::Convert(localidentityid,idstr);

		st=m_db->Prepare("SELECT MAX(InsertIndex) FROM tblMessageInserts WHERE Day=? AND LocalIdentityID=?;");
		st.Bind(0,now.Format("%Y-%m-%d"));
		st.Bind(1,localidentityid);
		st.Step();

		if(st.ResultNull(0)==false)
		{
			st.ResultInt(0,index);
			index++;
		}
		StringFunctions::Convert(index,indexstr);

		message.SetName("ClientPut");
		message["URI"]=privatekey+m_messagebase+"|"+now.Format("%Y-%m-%d")+"|Message|"+indexstr+".xml";
		message["Identifier"]=m_fcpuniquename+"|"+messageuuid+"|"+idstr+"|"+indexstr+"|"+message["URI"];
		message["UploadFrom"]="direct";
		message["DataLength"]=xmlsizestr;
		m_fcp->SendMessage(message);
		m_fcp->SendRaw(xml.c_str(),xml.size());

		m_inserting.push_back(messageuuid);

		m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"MessageInserter::StartInsert started message insert "+message["URI"]);
	}

}
