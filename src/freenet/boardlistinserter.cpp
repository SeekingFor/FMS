#include "../../include/freenet/boardlistinserter.h"
#include "../../include/freenet/boardlistxml.h"

#include <Poco/DateTime.h>
#include <Poco/Timespan.h>
#include <Poco/DateTimeFormatter.h>

#ifdef XMEM
	#include <xmem.h>
#endif

BoardListInserter::BoardListInserter(SQLite3DB::DB *db):IIndexInserter<long>(db)
{
	Initialize();
}

BoardListInserter::BoardListInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexInserter<long>(db,fcp)
{
	Initialize();
}

void BoardListInserter::CheckForNeededInsert()
{
	// only do 1 insert at a time
	if(m_inserting.size()==0)
	{
		Poco::DateTime today;
		Poco::DateTime daysback;

		// 20 days back
		daysback-=Poco::Timespan(20,0,0,0,0);

		// get identities who posted messages to boards in the past 20 days
		SQLite3DB::Statement st=m_db->Prepare("SELECT tblLocalIdentity.LocalIdentityID FROM tblLocalIdentity INNER JOIN tblMessageInserts ON tblLocalIdentity.LocalIdentityID=tblMessageInserts.LocalIdentityID WHERE tblLocalIdentity.Active='true' AND tblLocalIdentity.PublishBoardList='true' AND (tblLocalIdentity.LastInsertedBoardList<? OR tblLocalIdentity.LastInsertedBoardList IS NULL) AND tblMessageInserts.Day>=? GROUP BY tblLocalIdentity.LocalIdentityID;");
		st.Bind(0,Poco::DateTimeFormatter::format(today,"%Y-%m-%d"));
		st.Bind(1,Poco::DateTimeFormatter::format(daysback,"%Y-%m-%d"));
		st.Step();

		if(st.RowReturned())
		{
			int localidentityid;
			st.ResultInt(0,localidentityid);
			StartInsert(localidentityid);
		}
	}
}

const bool BoardListInserter::HandlePutFailed(FCPv2::Message &message)
{
	std::vector<std::string> idparts;
	long localidentityid;
	long index;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[1],localidentityid);
	StringFunctions::Convert(idparts[2],index);

	if(message["Fatal"]=="true" || message["Code"]=="9")
	{
		SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblBoardListInserts(LocalIdentityID,Day,InsertIndex,Inserted) VALUES(?,?,?,'false');");
		st.Bind(0,localidentityid);
		st.Bind(1,idparts[4]);
		st.Bind(2,index);
		st.Step();
	}

	RemoveFromInsertList(localidentityid);

	return true;
}

const bool BoardListInserter::HandlePutSuccessful(FCPv2::Message &message)
{
	Poco::DateTime now;
	std::vector<std::string> idparts;
	long localidentityid;
	long index;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[1],localidentityid);
	StringFunctions::Convert(idparts[2],index);

	SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tblBoardListInserts(LocalIdentityID,Day,InsertIndex,Inserted) VALUES(?,?,?,'true');");
	st.Bind(0,localidentityid);
	st.Bind(1,idparts[4]);
	st.Bind(2,index);
	st.Step();

	st=m_db->Prepare("UPDATE tblLocalIdentity SET LastInsertedBoardList=? WHERE LocalIdentityID=?;");
	st.Bind(0,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
	st.Bind(1,localidentityid);
	st.Step();

	RemoveFromInsertList(localidentityid);

	m_log->debug("BoardListInserter::HandlePutSuccessful successfully inserted BoardList.");

	return true;
}

void BoardListInserter::Initialize()
{
	m_fcpuniquename="BoardListInserter";
}

const bool BoardListInserter::StartInsert(const long &localidentityid)
{
	Poco::DateTime daysback;
	Poco::DateTime now;
	BoardListXML xml;
	FCPv2::Message message;
	std::string data;
	std::string datasizestr;
	std::string privatekey="";
	int index;
	std::string indexstr="";
	std::string localidentityidstr;

	// 20 days back
	daysback-=Poco::Timespan(20,0,0,0,0);

	// get boards
	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardName,BoardDescription FROM tblBoard INNER JOIN tblMessageBoard ON tblBoard.BoardID=tblMessageBoard.BoardID INNER JOIN tblMessage ON tblMessageBoard.MessageID=tblMessage.MessageID INNER JOIN tblMessageInserts ON tblMessage.MessageUUID=tblMessageInserts.MessageUUID WHERE tblMessageInserts.LocalIdentityID=? AND tblMessageInserts.Day>=? GROUP BY tblBoard.BoardID ORDER BY tblBoard.BoardName;");
	st.Bind(0,localidentityid);
	st.Bind(1,Poco::DateTimeFormatter::format(daysback,"%Y-%m-%d"));
	st.Step();

	while(st.RowReturned())
	{
		std::string name="";
		std::string description="";

		st.ResultText(0,name);
		st.ResultText(1,description);

		xml.AddBoard(name,description);

		st.Step();
	}

	// get public key
	st=m_db->Prepare("SELECT PrivateKey FROM tblLocalIdentity WHERE LocalIdentityID=?;");
	st.Bind(0,localidentityid);
	st.Step();
	if(st.RowReturned())
	{
		st.ResultText(0,privatekey);
	}

	// get last index
	index=0;
	st=m_db->Prepare("SELECT MAX(InsertIndex) FROM tblBoardListInserts WHERE LocalIdentityID=? AND Day=?;");
	st.Bind(0,localidentityid);
	st.Bind(1,Poco::DateTimeFormatter::format(now,"%Y-%m-%d"));
	st.Step();
	if(st.RowReturned())
	{
		if(st.ResultNull(0)==false)
		{
			st.ResultInt(0,index);
			index++;
		}
	}
	StringFunctions::Convert(index,indexstr);

	data=xml.GetXML();
	StringFunctions::Convert(data.size(),datasizestr);
	StringFunctions::Convert(localidentityid,localidentityidstr);

	message.SetName("ClientPut");
	message["URI"]=privatekey+m_messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|BoardList|"+indexstr+".xml";
	message["Identifier"]=m_fcpuniquename+"|"+localidentityidstr+"|"+indexstr+"|"+message["URI"];
	message["PriorityClass"]=m_defaultinsertpriorityclassstr;
	message["UploadFrom"]="direct";
	message["DataLength"]=datasizestr;
	message["Metadata.ContentType"]="";
	m_fcp->Send(message);
	m_fcp->Send(std::vector<char>(data.begin(),data.end()));

	m_inserting.push_back(localidentityid);

	return true;

}
