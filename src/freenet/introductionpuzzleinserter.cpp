#include "../../include/freenet/introductionpuzzleinserter.h"
#include "../../include/freenet/introductionpuzzlexml.h"
#include "../../include/stringfunctions.h"
#include "../../include/option.h"
#include "../../include/freenet/captcha/simplecaptcha.h"
#include "../../include/freenet/captcha/unlikecaptcha1.h"
#ifdef ALTERNATE_CAPTCHA
#include "../../include/freenet/captcha/alternatecaptcha1.h"
#include "../../include/freenet/captcha/alternatecaptcha2.h"
#endif
#ifdef AUDIO_CAPTCHA
#include "../../include/freenet/captcha/audiocaptcha1.h"
#endif
#include "../../include/base64.h"

#include <Poco/DateTimeFormatter.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/UUID.h>

#ifdef XMEM
	#include <xmem.h>
#endif

IntroductionPuzzleInserter::IntroductionPuzzleInserter(SQLite3DB::DB *db):IIndexInserter<long>(db)
{
	Initialize();
}

IntroductionPuzzleInserter::IntroductionPuzzleInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexInserter<long>(db,fcp)
{
	Initialize();
}

void IntroductionPuzzleInserter::CheckForNeededInsert()
{
	// only do 1 insert at a time
	if(m_inserting.size()==0)
	{
		// select all local ids that aren't single use and that aren't currently inserting a puzzle and are publishing a trust list
		SQLite3DB::Recordset rs=m_db->Query("SELECT LocalIdentityID FROM tblLocalIdentity WHERE tblLocalIdentity.Active='true' AND PublishTrustList='true' AND SingleUse='false' AND PrivateKey IS NOT NULL AND PrivateKey <> '' ORDER BY LastInsertedPuzzle;");
		
		while(!rs.AtEnd())
		{
			int localidentityid=0;
			std::string localidentityidstr="";
			Poco::DateTime now;
			float minutesbetweeninserts=0;
			minutesbetweeninserts=1440.0/(float)m_maxpuzzleinserts;
			Poco::DateTime lastinsert=now;
			lastinsert-=Poco::Timespan(0,0,minutesbetweeninserts,0,0);

			if(rs.GetField(0))
			{
				localidentityidstr=rs.GetField(0);
			}

			// if this identity has any non-solved puzzles for today, we don't need to insert a new puzzle
			SQLite3DB::Recordset rs2=m_db->Query("SELECT UUID FROM tblIntroductionPuzzleInserts WHERE Day='"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"' AND FoundSolution='false' AND LocalIdentityID="+localidentityidstr+";");

			// identity doesn't have any non-solved puzzles for today - start a new insert
			if(rs2.Empty()==true)
			{
				// make sure we are on the next day or the appropriate amount of time has elapsed since the last insert
				if(m_lastinserted.find(rs.GetInt(0))==m_lastinserted.end() || m_lastinserted[rs.GetInt(0)]<=lastinsert || m_lastinserted[rs.GetInt(0)].day()!=now.day())
				{
					StartInsert(rs.GetInt(0));
					m_lastinserted[rs.GetInt(0)]=now;
				}
				else
				{
					m_log->trace("IntroductionPuzzleInserter::CheckForNeededInsert waiting to insert puzzle for "+localidentityidstr);
				}
			}

			rs.Next();
		}
	}
}

void IntroductionPuzzleInserter::GenerateCaptcha(std::string &encodeddata, std::string &solution, const std::string &requestedcaptchatype, std::string &captchatype, std::string &mimetype)
{
	ICaptcha *cap=0;

#ifdef AUDIO_CAPTCHA
	if(requestedcaptchatype=="audio")
	{
		cap=new AudioCaptcha1();
		if(cap->Generate()==false)
		{
			delete cap;
			cap=0;
		}
	}
#endif
	if(cap==0 && requestedcaptchatype=="unlikeimage")
	{
		cap=new UnlikeCaptcha1("unlikeimages");
		if(cap->Generate()==false)
		{
			delete cap;
			cap=0;
		}
	}
	// image captcha is always basic fallback type
	// first the alternate type, then the basic type
	// which should always be available
#ifdef ALTERNATE_CAPTCHA
	if(cap==0)
	{
		if(rand()%2==0)
		{
			cap=new AlternateCaptcha1();
		}
		else
		{
			cap=new AlternateCaptcha2();
		}
		if(cap->Generate()==false)
		{
			delete cap;
			cap=0;
		}
	}
#endif
	if(cap==0)
	{
		cap=new SimpleCaptcha();
		if(cap->Generate()==false)
		{
			delete cap;
			cap=0;
		}
	}

	if(cap!=0)
	{
		std::vector<unsigned char> puzzle;
		std::vector<unsigned char> puzzlesolution;

		encodeddata.clear();
		solution.clear();

		cap->GetPuzzle(puzzle);
		cap->GetSolution(puzzlesolution);

		Base64::Encode(puzzle,encodeddata);
		solution.insert(solution.begin(),puzzlesolution.begin(),puzzlesolution.end());

		captchatype=cap->GetCaptchaType();
		mimetype=cap->GetMimeType();

		delete cap;
	}

}

const bool IntroductionPuzzleInserter::HandlePutFailed(FCPv2::Message &message)
{
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long localidentityid;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[1],localidentityid);

	// non USK
	if(idparts[0]==m_fcpuniquename)
	{
		// if fatal error or collision - mark index
		if(message["Fatal"]=="true" || message["Code"]=="9")
		{
			m_db->Execute("UPDATE tblIntroductionPuzzleInserts SET Day='"+idparts[5]+"', InsertIndex="+idparts[2]+", FoundSolution='true' WHERE UUID='"+idparts[3]+"';");
		}

		RemoveFromInsertList(localidentityid);

		m_log->debug("IntroductionPuzzleInserter::HandlePutFailed failed to insert puzzle "+idparts[3]);
	}

	return true;
}

const bool IntroductionPuzzleInserter::HandlePutSuccessful(FCPv2::Message &message)
{
	Poco::DateTime now;
	SQLite3DB::Statement st;
	std::vector<std::string> idparts;
	long localidentityid;
	long insertindex;

	StringFunctions::Split(message["Identifier"],"|",idparts);

	// non USK
	if(idparts[0]==m_fcpuniquename)
	{
		StringFunctions::Convert(idparts[1],localidentityid);
		StringFunctions::Convert(idparts[2],insertindex);

		st=m_db->Prepare("UPDATE tblIntroductionPuzzleInserts SET Day=?, InsertIndex=? WHERE UUID=?;");
		st.Bind(0,idparts[5]);
		st.Bind(1,insertindex);
		st.Bind(2,idparts[3]);
		st.Step();
		st.Finalize();

		st=m_db->Prepare("UPDATE tblLocalIdentity SET LastInsertedPuzzle=? WHERE LocalIdentityID=?;");
		st.Bind(0,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
		st.Bind(1,localidentityid);
		st.Step();
		st.Finalize();

		RemoveFromInsertList(localidentityid);

		m_log->debug("IntroductionPuzzleInserter::HandlePutSuccessful inserted puzzle "+idparts[3]);
	}

	return true;
}

void IntroductionPuzzleInserter::Initialize()
{
	m_fcpuniquename="IntroductionPuzzleInserter";
	m_maxpuzzleinserts=50;
}

const bool IntroductionPuzzleInserter::StartInsert(const long &localidentityid)
{
	Poco::DateTime now;
	std::string idstring="";
	long index=0;
	std::string indexstr="";
	Poco::UUIDGenerator uuidgen;
	Poco::UUID uuid;
	std::string messagebase="";
	IntroductionPuzzleXML xml;
	std::string encodedpuzzle="";
	std::string solutionstring="";
	FCPv2::Message message;
	std::string xmldata="";
	std::string xmldatasizestr="";
	std::string privatekey="";
	std::string publickey="";
	std::string keypart="";
	std::string captchatype="";
	std::string mimetype="";
	std::string requestedtype="";

	StringFunctions::Convert(localidentityid,idstring);
	SQLite3DB::Recordset rs=m_db->Query("SELECT MAX(InsertIndex) FROM tblIntroductionPuzzleInserts WHERE Day='"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"' AND LocalIdentityID="+idstring+";");

	if(rs.Empty() || rs.GetField(0)==NULL)
	{
		index=0;
	}
	else
	{
		index=rs.GetInt(0)+1;
	}
	StringFunctions::Convert(index,indexstr);

	if(index<m_maxpuzzleinserts)
	{
		SQLite3DB::Recordset rs2=m_db->Query("SELECT PrivateKey,PublicKey,IntroductionPuzzleType FROM tblLocalIdentity WHERE LocalIdentityID="+idstring+";");
		if(rs2.Empty()==false && rs2.GetField(0)!=NULL)
		{
			privatekey=rs2.GetField(0);
			if(rs2.GetField(1))
			{
				publickey=rs2.GetField(1);
			}
			if(publickey.size()>=50)
			{
				// remove - and ~
				keypart=StringFunctions::Replace(StringFunctions::Replace(publickey.substr(4,43),"-",""),"~","");
			}
		}
		if(rs2.GetField(2)!=NULL)
		{
			requestedtype=rs2.GetField(2);
		}

		Option option(m_db);
		option.Get("MessageBase",messagebase);

		GenerateCaptcha(encodedpuzzle,solutionstring,requestedtype,captchatype,mimetype);
		if(encodedpuzzle.size()==0)
		{
			m_log->fatal("IntroductionPuzzleInserter::StartInsert could not create introduction puzzle");
			return false;
		}

		try
		{
			uuid=uuidgen.createRandom();
		}
		catch(...)
		{
			m_log->fatal("IntroductionPuzzleInserter::StartInsert could not create UUID");
		}

		xml.SetType(captchatype);
		std::string uuidstr=uuid.toString();
		StringFunctions::UpperCase(uuidstr,uuidstr);
		xml.SetUUID(uuidstr+"@"+keypart);
		xml.SetPuzzleData(encodedpuzzle);
		xml.SetMimeType(mimetype);

		xmldata=xml.GetXML();
		StringFunctions::Convert(xmldata.size(),xmldatasizestr);

		message.SetName("ClientPut");
		message["URI"]=privatekey+messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y-%m-%d")+"|IntroductionPuzzle|"+indexstr+".xml";
		message["Identifier"]=m_fcpuniquename+"|"+idstring+"|"+indexstr+"|"+xml.GetUUID()+"|"+message["URI"];
		message["PriorityClass"]=m_defaultinsertpriorityclassstr;
		message["UploadFrom"]="direct";
		message["DataLength"]=xmldatasizestr;
		message["Metadata.ContentType"]="";
		m_fcp->Send(message);
		m_fcp->Send(std::vector<char>(xmldata.begin(),xmldata.end()));

		// insert to USK - not used, but don't remove code yet
		/*
		message.Clear();
		message.SetName("ClientPutComplexDir");
		message["URI"]="USK"+privatekey.substr(3)+messagebase+"|"+Poco::DateTimeFormatter::format(now,"%Y.%m.%d")+"|IntroductionPuzzle/0/";
		message["Identifier"]=m_fcpuniquename+"USK|"+message["URI"];
		message["DefaultName"]="IntroductionPuzzle.xml";
		message["Files.0.Name"]="IntroductionPuzzle.xml";
		message["Files.0.UplaodFrom"]="direct";
		message["Files.0.DataLength"]=xmldatasizestr;
		m_fcp->Send(message);
		m_fcp->Send(std::vector<char>(xmldata.begin(),xmldata.end()));
		*/

		m_db->Execute("INSERT INTO tblIntroductionPuzzleInserts(UUID,Type,MimeType,LocalIdentityID,PuzzleData,PuzzleSolution) VALUES('"+xml.GetUUID()+"','"+captchatype+"','"+mimetype+"',"+idstring+",'"+encodedpuzzle+"','"+solutionstring+"');");

		m_inserting.push_back(localidentityid);

		m_log->debug("IntroductionPuzzleInserter::StartInsert started insert for id "+idstring);
	}
	else
	{
		m_log->warning("IntroductionPuzzleInserter::StartInsert already inserted max puzzles for "+idstring);
	}

	return true;

}
