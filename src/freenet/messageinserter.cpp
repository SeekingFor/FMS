#include "../../include/freenet/messageinserter.h"
#include "../../include/freenet/messagexml.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/Timestamp.h>

MessageInserter::MessageInserter(SQLite3DB::DB *db):IIndexInserter<std::string>(db)
{
	Initialize();
}

MessageInserter::MessageInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexInserter<std::string>(db,fcp)
{
	Initialize();
}

void MessageInserter::CheckForNeededInsert()
{
	Poco::DateTime now;
	bool didinsert=false;
	// only do 1 insert at a time
	if(m_inserting.size()==0)
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT MessageUUID FROM tblMessageInserts INNER JOIN tblLocalIdentity ON tblMessageInserts.LocalIdentityID=tblLocalIdentity.LocalIdentityID WHERE tblLocalIdentity.Active='true' AND tblLocalIdentity.PrivateKey IS NOT NULL AND tblLocalIdentity.PrivateKey <> '' AND tblMessageInserts.Inserted='false' AND tblMessageInserts.SendDate<=?;");
		st.Bind(0,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
		st.Step();

		while(st.RowReturned() && m_inserting.size()==0)
		{
			std::string messageuuid="";
			st.ResultText(0,messageuuid);

			// make sure there are no uninserted files attached to this message
			SQLite3DB::Statement st2=m_db->Prepare("SELECT FileInsertID FROM tblFileInserts WHERE Key IS NULL AND MessageUUID=?;");
			st2.Bind(0,messageuuid);
			st2.Step();

			if(st2.RowReturned()==false)
			{
				StartInsert(messageuuid);
			}

			st.Step();
		}
	}
}

const bool MessageInserter::HandlePutFailed(FCPv2::Message &message)
{
	int index;
	int localidentityid;
	std::vector<std::string> idparts;

	// do check to make sure this is the non-editioned SSK - we ignore failure/success for editioned SSK for now
	if(message["Identifier"].find(".xml")!=std::string::npos)
	{

		StringFunctions::Split(message["Identifier"],"|",idparts);
		StringFunctions::Convert(idparts[2],localidentityid);
		StringFunctions::Convert(idparts[3],index);

		// fatal put - or data exists - insert bogus index into database so we'll try to insert this message again
		if(message["Fatal"]=="true" || message["Code"]=="9")
		{
			SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblMessageInserts(LocalIdentityID,Day,InsertIndex,Inserted) VALUES(?,?,?,'true');");
			st.Bind(0,localidentityid);
			st.Bind(1,idparts[6]);
			st.Bind(2,index);
			st.Step();
		}

		m_log->trace("MessageInserter::HandlePutFailed error code "+message["Code"]+" fatal="+message["Fatal"]);

		RemoveFromInsertList(idparts[1]);

	}
	else
	{
		m_log->trace("MessageInserter::HandlePutFailed for editioned SSK error code "+message["Code"]+ " id "+message["Identifier"]);
	}

	return true;
}

const bool MessageInserter::HandlePutSuccessful(FCPv2::Message &message)
{
	MessageXML xml;
	Poco::DateTime date;
	int localidentityid;
	int index;
	std::vector<std::string> idparts;

	// do check to make sure this is the non-editioned SSK - we ignore failure/success for editioned SSK for now
	if(message["Identifier"].find(".xml")!=std::string::npos)
	{

		StringFunctions::Split(message["Identifier"],"|",idparts);
		StringFunctions::Convert(idparts[3],index);
		StringFunctions::Convert(idparts[2],localidentityid);

		SQLite3DB::Statement st=m_db->Prepare("UPDATE tblMessageInserts SET Day=?, InsertIndex=?, Inserted='true' WHERE MessageUUID=?;");
		st.Bind(0,idparts[6]);
		st.Bind(1,index);
		st.Bind(2,idparts[1]);
		st.Step();

		// insert record into temp table so MessageList will be inserted ASAP
		date=Poco::Timestamp();
		st=m_db->Prepare("INSERT INTO tmpMessageListInsert(LocalIdentityID,Date) VALUES(?,?);");
		st.Bind(0,localidentityid);
		st.Bind(1,Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
		st.Step();

		// update the messageuuid to the real messageuuid
		st=m_db->Prepare("SELECT MessageXML FROM tblMessageInserts WHERE MessageUUID=?;");
		st.Bind(0,idparts[1]);
		st.Step();
		if(st.RowReturned())
		{
			std::string xmldata="";
			st.ResultText(0,xmldata);
			xml.ParseXML(xmldata);
			xml.SetMessageID(idparts[4]);

			SQLite3DB::Statement st2=m_db->Prepare("UPDATE tblMessageInserts SET MessageUUID=?, MessageXML=? WHERE MessageUUID=?;");
			st2.Bind(0,idparts[4]);
			st2.Bind(1,xml.GetXML());
			st2.Bind(2,idparts[1]);
			st2.Step();

			//update file insert MessageUUID as well
			st2=m_db->Prepare("UPDATE tblFileInserts SET MessageUUID=? WHERE MessageUUID=?;");
			st2.Bind(0,idparts[4]);
			st2.Bind(1,idparts[1]);
			st2.Step();
		}

		RemoveFromInsertList(idparts[1]);

		m_log->debug("MessageInserter::HandlePutSuccessful successfully inserted message "+message["Identifier"]);

	}
	else
	{
		m_log->debug("MessageInserter::HandlePutSuccessful for editioned SSK "+message["Identifier"]);
	}

	return true;
}

void MessageInserter::Initialize()
{
	m_fcpuniquename="MessageInserter";
}

const bool MessageInserter::StartInsert(const std::string &messageuuid)
{
	MessageXML xmlfile;
	Poco::DateTime now;
	SQLite3DB::Statement st=m_db->Prepare("SELECT MessageXML,PrivateKey,tblLocalIdentity.LocalIdentityID,PublicKey FROM tblMessageInserts INNER JOIN tblLocalIdentity ON tblMessageInserts.LocalIdentityID=tblLocalIdentity.LocalIdentityID WHERE MessageUUID=?;");
	st.Bind(0,messageuuid);
	st.Step();

	if(st.RowReturned())
	{
		int localidentityid;
		std::string idstr;
		std::string xml;
		std::string xmlsizestr;
		std::string privatekey;
		std::string publickey;
		FCPv2::Message message;
		std::string indexstr;
		int index=0;
		
		st.ResultText(0,xml);
		st.ResultText(1,privatekey);
		st.ResultInt(2,localidentityid);
		st.ResultText(3,publickey);
		StringFunctions::Convert(localidentityid,idstr);

		st=m_db->Prepare("SELECT MAX(InsertIndex) FROM tblMessageInserts WHERE Day=? AND LocalIdentityID=?;");
		st.Bind(0,Poco::DateTimeFormatter::format(now,"%Y-%m-%d"));
		st.Bind(1,localidentityid);
		st.Step();

		if(st.ResultNull(0)==false)
		{
			st.ResultInt(0,index);
			index++;
		}
		StringFunctions::Convert(index,indexstr);

		xmlfile.ParseXML(xml);

		// add file attachments to xml - must do this before we change UUID
		st=m_db->Prepare("SELECT Key, Size FROM tblFileInserts WHERE MessageUUID=?;");
		st.Bind(0,xmlfile.GetMessageID());
		st.Step();
		while(st.RowReturned())
		{
			std::string key="";
			int size;
			
			st.ResultText(0,key);
			st.ResultInt(1,size);

			xmlfile.AddFileAttachment(key,size);

			st.Step();
		}

		// recreate messageuuid in xml - UUID of message will not match entry in MessageInserts table until we successfully insert it
		// see HandlePutSuccessful
		// if we don't already have an @sskpart - add it
		if(xmlfile.GetMessageID().find('@')==std::string::npos)
		{
			// remove - and ~ from publickey part
			std::string publickeypart=StringFunctions::Replace(StringFunctions::Replace(publickey.substr(4,43),"-",""),"~","");
			xmlfile.SetMessageID(xmlfile.GetMessageID()+"@"+publickeypart);
		}
		xml=xmlfile.GetXML();

		StringFunctions::Convert(xml.size(),xmlsizestr);

		message.SetName("ClientPut");
		message["URI"]=privatekey+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|Message|"+indexstr+".xml";
		message["Identifier"]=m_fcpuniquename+"|"+messageuuid+"|"+idstr+"|"+indexstr+"|"+xmlfile.GetMessageID()+"|"+message["URI"];
		message["PriorityClass"]=m_defaultinsertpriorityclassstr;
		message["UploadFrom"]="direct";
		message["DataLength"]=xmlsizestr;
		message["Metadata.ContentType"]="";
		message["ExtraInsertsSingleBlock"]="2";
		m_fcp->Send(message);
		m_fcp->Send(std::vector<char>(xml.begin(),xml.end()));

		// test insert as editioned SSK
		message.Clear();
		message.SetName("ClientPut");
		message["URI"]=privatekey+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|Message-"+indexstr;
		message["Identifier"]=m_fcpuniquename+"|"+message["URI"];
		message["PriorityClass"]=m_defaultinsertpriorityclassstr;
		message["UploadFrom"]="direct";
		message["DataLength"]=xmlsizestr;
		message["Metadata.ContentType"]="";
		message["ExtraInsertsSingleBlock"]="2";
		m_fcp->Send(message);
		m_fcp->Send(std::vector<char>(xml.begin(),xml.end()));

		m_inserting.push_back(messageuuid);

		m_log->debug("MessageInserter::StartInsert started message insert "+message["URI"]);
	
		return true;
	}
	else
	{
		return false;
	}

}
