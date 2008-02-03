#include "../../include/freenet/introductionpuzzleinserter.h"
#include "../../include/freenet/introductionpuzzlexml.h"
#include "../../include/stringfunctions.h"
#include "../../include/option.h"
#include "../../include/freenet/captcha/simplecaptcha.h"
#include "../../include/uuidgenerator.h"
#include "../../include/base64.h"

#ifdef XMEM
	#include <xmem.h>
#endif

IntroductionPuzzleInserter::IntroductionPuzzleInserter()
{
	Initialize();
}

IntroductionPuzzleInserter::IntroductionPuzzleInserter(FCPv2 *fcp):IFCPConnected(fcp)
{
	Initialize();
}

void IntroductionPuzzleInserter::CheckForNeededInsert()
{
	// select all local ids that aren't single use and that aren't currently inserting a puzzle
	SQLite3DB::Recordset rs=m_db->Query("SELECT LocalIdentityID FROM tblLocalIdentity WHERE SingleUse='false' AND InsertingPuzzle='false' AND PrivateKey IS NOT NULL AND PrivateKey <> '' ORDER BY LastInsertedPuzzle;");
	
	while(!rs.AtEnd())
	{
		std::string localidentityidstr;
		DateTime now;
		now.SetToGMTime();

		if(rs.GetField(0))
		{
			localidentityidstr=rs.GetField(0);
		}

		// if this identity has any non-solved puzzles for today, we don't need to insert a new puzzle
		SQLite3DB::Recordset rs2=m_db->Query("SELECT UUID FROM tblIntroductionPuzzleInserts WHERE Day='"+now.Format("%Y-%m-%d")+"' AND FoundSolution='false' AND LocalIdentityID="+localidentityidstr+";");

		// identity doesn't have any non-solved puzzles for today - start a new insert
		if(rs2.Empty()==true)
		{
			StartInsert(rs.GetInt(0));
		}

		rs.Next();
	}
}

void IntroductionPuzzleInserter::FCPConnected()
{
	m_db->Execute("UPDATE tblLocalIdentity SET InsertingPuzzle='false';");
}

void IntroductionPuzzleInserter::FCPDisconnected()
{
	
}

void IntroductionPuzzleInserter::GenerateCaptcha(std::string &encodeddata, std::string &solution)
{
	SimpleCaptcha captcha;
	std::vector<unsigned char> puzzle;
	std::vector<unsigned char> puzzlesolution;

	captcha.Generate();
	captcha.GetPuzzle(puzzle);
	captcha.GetSolution(puzzlesolution);

	encodeddata.clear();
	solution.clear();

	Base64::Encode(puzzle,encodeddata);
	solution.insert(solution.begin(),puzzlesolution.begin(),puzzlesolution.end());

}

const bool IntroductionPuzzleInserter::HandleMessage(FCPMessage &message)
{

	if(message["Identifier"].find("IntroductionPuzzleInserter")==0)
	{

		// ignore URIGenerated message
		if(message.GetName()=="URIGenerated")
		{
			return true;
		}

		if(message.GetName()=="PutSuccessful")
		{
			return HandlePutSuccessful(message);
		}

		if(message.GetName()=="PutFailed")
		{
			return HandlePutFailed(message);
		}

	}

	return false;
}

const bool IntroductionPuzzleInserter::HandlePutFailed(FCPMessage &message)
{
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long localidentityid;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[1],localidentityid);

	st=m_db->Prepare("UPDATE tblLocalIdentity SET InsertingPuzzle='false' WHERE LocalIdentityID=?;");
	st.Bind(0,localidentityid);
	st.Step();
	st.Finalize();

	// if fatal error or collision - mark index
	if(message["Fatal"]=="true" || message["Code"]=="9")
	{
		m_db->Execute("UPDATE tblIntroductionPuzzleInserts SET Day='"+idparts[5]+"', InsertIndex="+idparts[2]+", FoundSolution='true' WHERE UUID='"+idparts[3]+"';");
	}

	m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"IntroductionPuzzleInserter::HandlePutFailed failed to insert puzzle "+idparts[3]);

	return true;
}

const bool IntroductionPuzzleInserter::HandlePutSuccessful(FCPMessage &message)
{
	DateTime now;
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long localidentityid;
	long insertindex;

	now.SetToGMTime();
	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[1],localidentityid);
	StringFunctions::Convert(idparts[2],insertindex);

	st=m_db->Prepare("UPDATE tblIntroductionPuzzleInserts SET Day=?, InsertIndex=? WHERE UUID=?;");
	st.Bind(0,idparts[5]);
	st.Bind(1,insertindex);
	st.Bind(2,idparts[3]);
	st.Step();
	st.Finalize();

	st=m_db->Prepare("UPDATE tblLocalIdentity SET InsertingPuzzle='false', LastInsertedPuzzle=? WHERE LocalIdentityID=?;");
	st.Bind(0,now.Format("%Y-%m-%d %H:%M:%S"));
	st.Bind(1,localidentityid);
	st.Step();
	st.Finalize();

	m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"IntroductionPuzzleInserter::HandlePutSuccessful inserted puzzle "+idparts[3]);

	return true;
}

void IntroductionPuzzleInserter::Initialize()
{
	m_lastchecked.SetToGMTime();
}

void IntroductionPuzzleInserter::Process()
{

	DateTime now;

	now.SetToGMTime();

	if(m_lastchecked<(now-(1.0/1440.0)))
	{
		CheckForNeededInsert();
		m_lastchecked=now;
	}

}

void IntroductionPuzzleInserter::RegisterWithThread(FreenetMasterThread *thread)
{
	thread->RegisterFCPConnected(this);
	thread->RegisterFCPMessageHandler(this);
	thread->RegisterPeriodicProcessor(this);
}

void IntroductionPuzzleInserter::StartInsert(const long localidentityid)
{
	DateTime now;
	std::string idstring;
	long index=0;
	std::string indexstr;
	UUIDGenerator uuid;
	std::string messagebase;
	IntroductionPuzzleXML xml;
	std::string encodedpuzzle;
	std::string solutionstring;
	FCPMessage message;
	std::string xmldata;
	std::string xmldatasizestr;
	std::string privatekey;

	StringFunctions::Convert(localidentityid,idstring);
	now.SetToGMTime();
	SQLite3DB::Recordset rs=m_db->Query("SELECT MAX(InsertIndex) FROM tblIntroductionPuzzleInserts WHERE Day='"+now.Format("%Y-%m-%d")+"' AND LocalIdentityID="+idstring+";");

	if(rs.Empty() || rs.GetField(0)==NULL)
	{
		index=0;
	}
	else
	{
		index=rs.GetInt(0)+1;
	}
	StringFunctions::Convert(index,indexstr);

	SQLite3DB::Recordset rs2=m_db->Query("SELECT PrivateKey FROM tblLocalIdentity WHERE LocalIdentityID="+idstring+";");
	if(rs2.Empty()==false && rs2.GetField(0)!=NULL)
	{
		privatekey=rs2.GetField(0);
	}

	Option::Instance()->Get("MessageBase",messagebase);

	GenerateCaptcha(encodedpuzzle,solutionstring);

	xml.SetType("captcha");
	xml.SetUUID(uuid.Generate());
	xml.SetPuzzleData(encodedpuzzle);
	xml.SetMimeType("bitmap/image");

	xmldata=xml.GetXML();
	StringFunctions::Convert(xmldata.size(),xmldatasizestr);

	message.SetName("ClientPut");
	message["URI"]=privatekey+messagebase+"|"+now.Format("%Y-%m-%d")+"|IntroductionPuzzle|"+indexstr+".xml";
	message["Identifier"]="IntroductionPuzzleInserter|"+idstring+"|"+indexstr+"|"+xml.GetUUID()+"|"+message["URI"];
	message["UploadFrom"]="direct";
	message["DataLength"]=xmldatasizestr;
	m_fcp->SendMessage(message);
	m_fcp->SendRaw(xmldata.c_str(),xmldata.size());

	m_db->Execute("UPDATE tblLocalIdentity SET InsertingPuzzle='true' WHERE LocalIdentityID="+idstring+";");
	m_db->Execute("INSERT INTO tblIntroductionPuzzleInserts(UUID,Type,MimeType,LocalIdentityID,PuzzleData,PuzzleSolution) VALUES('"+xml.GetUUID()+"','captcha','image/bmp',"+idstring+",'"+encodedpuzzle+"','"+solutionstring+"');");

	m_log->WriteLog(LogFile::LOGLEVEL_DEBUG,"IntroductionPuzzleInserter::StartInsert started insert for id "+idstring);

}
