#include "../../include/freenet/fileinserter.h"

#ifdef XMEM
	#include <xmem.h>
#endif

FileInserter::FileInserter(SQLite3DB::DB *db):IIndexInserter<long>(db)
{
	Initialize();
}

FileInserter::FileInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexInserter<long>(db,fcp)
{
	Initialize();
}

void FileInserter::CheckForNeededInsert()
{
	// only do 1 insert at a time
	if(m_inserting.size()==0)
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT FileInsertID FROM tblFileInserts WHERE Key IS NULL;");
		st.Step();
		if(st.RowReturned())
		{
			int id=-1;
			st.ResultInt(0,id);
			StartInsert(id);
		}
	}
}

const bool FileInserter::HandlePutFailed(FCPv2::Message &message)
{
	std::vector<std::string> idparts;
	long fileinsertid;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[1],fileinsertid);

	RemoveFromInsertList(fileinsertid);

	m_log->error("FileInserter::HandlePutFailed failed to insert "+message["Identifier"]);

	return true;

}

const bool FileInserter::HandlePutSuccessful(FCPv2::Message &message)
{
	std::vector<std::string> idparts;
	long fileinsertid;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[1],fileinsertid);

	SQLite3DB::Statement st=m_db->Prepare("UPDATE tblFileInserts SET Key=?, Data=NULL WHERE FileInsertID=?;");
	st.Bind(0,StringFunctions::UriDecode(message["URI"]));
	st.Bind(1,fileinsertid);
	st.Step();

	RemoveFromInsertList(fileinsertid);

	return true;
}

void FileInserter::Initialize()
{
	m_fcpuniquename="FileInserter";
}

const bool FileInserter::StartInsert(const long &fileinsertid)
{
	FCPv2::Message message;
	std::string fileinsertidstr="";
	std::string sizestr="";
	std::string filename="";
	std::string mimetype="";
	int datalen=-1;
	std::vector<char> data;
	std::string keytype="CHK@";
	Option option(m_db);

	StringFunctions::Convert(fileinsertid,fileinsertidstr);

	option.Get("AttachmentKeyType",keytype);
	SQLite3DB::Statement st=m_db->Prepare("SELECT FileName,Size,Data,MimeType FROM tblFileInserts WHERE FileInsertID=?;");
	st.Bind(0,fileinsertid);
	st.Step();

	st.ResultText(0,filename);
	st.ResultInt(1,datalen);
	data.resize(datalen,0);
	st.ResultBlob(2,&data[0],datalen);
	data.resize(datalen);
	st.ResultText(3,mimetype);

	StringFunctions::Convert(data.size(),sizestr);

	message.SetName("ClientPut");
	message["URI"]=keytype;

	message["TargetFilename"]=filename;
	if(mimetype!="")
	{
		message["Metadata.ContentType"]=mimetype;
	}
	message["Identifier"]=m_fcpuniquename+"|"+fileinsertidstr;
	message["UploadFrom"]="direct";
	message["DataLength"]=sizestr;
	message["PriorityClass"]=m_defaultinsertpriorityclassstr;
	m_fcp->Send(message);
	m_fcp->Send(data);

	m_inserting.push_back(fileinsertid);

	return true;
}
