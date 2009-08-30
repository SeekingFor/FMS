#include "../../include/freenet/siteinserter.h"
#include "../../include/global.h"

#include <Poco/DateTime.h>
#include <Poco/Timespan.h>
#include <Poco/DateTimeFormatter.h>
#include <cstdio>

#ifdef XMEM
	#include <xmem.h>
#endif

SiteInserter::SiteInserter(SQLite3DB::DB *db):IIndexInserter<long>(db)
{
	Initialize();
}

SiteInserter::SiteInserter(SQLite3DB::DB *db, FCPv2::Connection *fcp):IIndexInserter<long>(db,fcp)
{
	Initialize();
}

void SiteInserter::CheckForNeededInsert()
{
	// only do 1 insert at a time
	if(m_inserting.size()==0)
	{
		Poco::DateTime date;
		date.assign(date.year(),date.month(),date.day(),0,0,0);

		SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID FROM tblLocalIdentity WHERE PublishFreesite='true' AND (LastInsertedFreesite IS NULL OR LastInsertedFreesite<?);");
		st.Bind(0,Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));

		st.Step();
		if(st.RowReturned())
		{
			int localidentityid=0;
			st.ResultInt(0,localidentityid);
			StartInsert(localidentityid);
		}
	}
}

std::string SiteInserter::GenerateIndex(const std::string &htmltemplate, const long localidentityid, const std::string &name)
{
	std::string content="";

	SQLite3DB::Statement boardst=m_db->Prepare("SELECT tblBoard.BoardName FROM tblBoard INNER JOIN tblMessageBoard ON tblBoard.BoardID=tblMessageBoard.BoardID WHERE tblMessageBoard.MessageID=? ORDER BY tblBoard.BoardName COLLATE NOCASE;");
	SQLite3DB::Statement st=m_db->Prepare("SELECT tblMessage.Body, tblMessage.Subject, tblMessage.MessageID FROM tblMessage INNER JOIN tblIdentity ON tblMessage.IdentityID=tblIdentity.IdentityID INNER JOIN tblLocalIdentity ON tblIdentity.PublicKey=tblLocalIdentity.PublicKey WHERE tblLocalIdentity.LocalIdentityID=? ORDER BY tblMessage.MessageDate DESC, tblMessage.MessageTime DESC LIMIT 0,10;");
	st.Bind(0,localidentityid);
	st.Step();

	while(st.RowReturned())
	{
		std::string post="";
		std::string subject="";
		std::string boards="";
		int messageid=0;

		st.ResultText(0,post);
		st.ResultText(1,subject);
		st.ResultInt(2,messageid);

		boardst.Bind(0,messageid);
		boardst.Step();
		while(boardst.RowReturned())
		{
			std::string board="";
			boardst.ResultText(0,board);
			if(boards!="")
			{
				boards+=",";
			}
			boards+=board;
			boardst.Step();
		}
		boardst.Reset();

		content+="<div class=\"post\">";
		content+="<div class=\"postboards\">";
		content+=SanitizeOutput(boards);
		content+="</div>";
		content+="<div class=\"postsubject\">";
		content+=SanitizeOutput(subject);
		content+="</div>";
		content+="<div class=\"postbody\">";
		content+=SanitizeOutput(post);
		//post=SanitizeOutput(post);
		//StringFunctions::Replace(post,"\r\n","<br>");
		//content+=post;
		content+="</div>";
		content+="</div>";

		st.Step();
	}

	return StringFunctions::Replace(htmltemplate,"[CONTENT]",content);

}

std::string SiteInserter::GenerateLinks(const bool publishtrustlist, const bool publishboardlist)
{
	std::string links="";
	links+="<ul>";
	links+="<li><a href=\"index.htm\">Home</a></li>";
	if(publishtrustlist)
	{
		links+="<li><a href=\"trustlist.htm\">Trust List</a></li>";
	}
	if(publishboardlist)
	{
//		links+="<li><a href=\"boardlist.htm\">Board List</a></li>";
	}
	links+="</ul>";
	return links;
}

void SiteInserter::GeneratePages(const long localidentityid, std::string &uskkey, std::map<std::string,std::string> &pages)
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT Name, PrivateKey, PublishTrustList, PublishBoardList, FreesiteEdition FROM tblLocalIdentity WHERE LocalIdentityID=?;");
	st.Bind(0,localidentityid);
	st.Step();

	if(st.RowReturned())
	{
		std::string htmltemplate="";
		std::string filename="";
		std::string name="";
		std::string key="";
		std::string publishtrustliststr="";
		std::string publishboardliststr="";
		bool publishtrustlist=false;
		bool publishboardlist=false;
		std::string editionstr="";

		st.ResultText(0,name);
		st.ResultText(1,key);
		st.ResultText(2,publishtrustliststr);
		st.ResultText(3,publishboardliststr);
		st.ResultText(4,editionstr);

		publishtrustliststr=="true" ? publishtrustlist=true : publishtrustlist=false;
		publishboardliststr=="true" ? publishboardlist=true : publishboardlist=false;
		// no edition exists - start at 0
		if(editionstr=="")
		{
			editionstr="0";
		}
		// previous edition exists - add 1
		else
		{
			int edition=0;
			StringFunctions::Convert(editionstr,edition);
			edition++;
			StringFunctions::Convert(edition,editionstr);
		}

		// make SSK a USK
		if(key.find("SSK@")==0)
		{
			key.erase(0,3);
			key="USK"+key;
		}
		key+=m_messagebase+"/"+editionstr+"/";
		uskkey=key;

		filename=name+"-template.htm";
		FILE *infile=fopen(filename.c_str(),"rb");
		if(!infile)
		{
			infile=fopen("site-template.htm","rb");
		}
		if(infile)
		{
			fseek(infile,0,SEEK_END);
			long len=ftell(infile);
			fseek(infile,0,SEEK_SET);

			std::vector<unsigned char> data;
			data.resize(len);
			fread(&data[0],1,data.size(),infile);
			fclose(infile);

			htmltemplate.append(data.begin(),data.end());

			htmltemplate=StringFunctions::Replace(htmltemplate,"[LINKS]",GenerateLinks(publishtrustlist,publishboardlist));
			htmltemplate=StringFunctions::Replace(htmltemplate,"[IDENTITYNAME]",SanitizeOutput(name));

			pages["index.htm"]=GenerateIndex(htmltemplate,localidentityid,name);
			if(publishtrustlist)
			{
				pages["trustlist.htm"]=GenerateTrustList(htmltemplate,localidentityid,name);
			}
			if(publishboardlist)
			{
//				pages["boardlist.htm"]=GenerateBoardList(htmltemplate,localidentityid,name);
			}

		}
		else
		{
			m_log->error("SiteInserter::GeneratePages unable to open "+filename+" or site-template.htm.");
		}

		// get extra files that the user wants to add to the Freesite
		filename=name+"-files.txt";
		infile=fopen(filename.c_str(),"rb");
		if(infile)
		{
			std::vector<std::string> files;

			fseek(infile,0,SEEK_END);
			long len=ftell(infile);
			fseek(infile,0,SEEK_SET);

			std::vector<unsigned char> data;
			data.resize(len);
			fread(&data[0],1,data.size(),infile);
			fclose(infile);

			// split on \r and \n - on systems with \r\n line endings there will be blank entries, but we'll just skip those
			std::string filecontent(data.begin(),data.end());
			StringFunctions::SplitMultiple(filecontent,"\r\n",files);

			for(std::vector<std::string>::iterator i=files.begin(); i!=files.end(); i++)
			{
				if((*i)!="" && (*i).find("index.htm")==std::string::npos && (*i).find("trustlist.htm")==std::string::npos && (*i).find("files.htm")==std::string::npos)
				{
					filename=(*i);
					infile=fopen(filename.c_str(),"rb");
					if(infile)
					{
						fseek(infile,0,SEEK_END);
						len=ftell(infile);
						fseek(infile,0,SEEK_SET);

						data.resize(len);
						fread(&data[0],1,data.size(),infile);
						fclose(infile);

						filecontent="";
						filecontent.append(data.begin(),data.end());

						// strip off path from filename
						while(filename.find_first_of("/")!=std::string::npos)
						{
							filename.erase(0,filename.find_first_of("/")+1);
						}

						if(filecontent.size()>0)
						{
							pages[filename]=filecontent;
						}

					}
					else
					{
						m_log->error("SiteInserter::GeneratePages could not include user file "+(*i));
					}
				}
			}

		}

	}
}

std::string SiteInserter::GenerateTrustList(const std::string &htmltemplate, const long localidentityid, const std::string &name)
{
	std::string content="";
	Poco::DateTime date;

	date-=Poco::Timespan(20,0,0,0,0);
	SQLite3DB::Statement st=m_db->Prepare("SELECT Name,PublicKey,tblIdentityTrust.LocalMessageTrust,tblIdentityTrust.LocalTrustListTrust,tblIdentity.IdentityID,tblIdentityTrust.MessageTrustComment,tblIdentityTrust.TrustListTrustComment,tblIdentity.FreesiteEdition FROM tblIdentity LEFT JOIN (SELECT IdentityID,LocalMessageTrust,LocalTrustListTrust,MessageTrustComment,TrustListTrustComment FROM tblIdentityTrust WHERE LocalIdentityID=?) AS 'tblIdentityTrust' ON tblIdentity.IdentityID=tblIdentityTrust.IdentityID WHERE PublicKey IS NOT NULL AND LastSeen IS NOT NULL AND LastSeen>=? ORDER BY Name COLLATE NOCASE;");
	st.Bind(0,localidentityid);
	st.Bind(1,Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S"));
	st.Step();

	content+="<table class=\"trustlist\">";
	content+="<tr class=\"title\"><th colspan=\"5\">";
	content+="Trust List of "+SanitizeOutput(name);
	content+="</th></tr>";
	content+="<tr class=\"headings\"><th></th><th>Message Trust</th><th>Message Comment</th><th>Trust List Trust</th><th>Trust Comment</th></tr>";
	while(st.RowReturned())
	{
		std::string idname="";
		std::string thisid="";
		std::string messagetrustcomment="";
		std::string trustlisttrustcomment="";
		std::string messagetrust="";
		std::string trustlisttrust="";
		std::string publickey="";
		std::string uskkey="";
		std::string freesiteedition="";

		st.ResultText(0,idname);
		st.ResultText(1,publickey);
		st.ResultText(2,messagetrust);
		st.ResultText(3,trustlisttrust);
		st.ResultText(4,thisid);
		st.ResultText(5,messagetrustcomment);
		st.ResultText(6,trustlisttrustcomment);
		st.ResultText(7,freesiteedition);

		if(freesiteedition!="")
		{
			if(publickey.find("SSK@")==0)
			{
				uskkey=publickey;
				uskkey.erase(0,3);
				uskkey="USK"+uskkey;
				uskkey+=m_messagebase+"/"+freesiteedition+"/";
			}
		}

		content+="<tr>";
		if(freesiteedition!="")
		{
			content+="<td><div><a href=\""+uskkey+"\">"+SanitizeOutput(CreateShortIdentityName(idname,publickey))+"</a></div></td>";
		}
		else
		{
			content+="<td><div>"+SanitizeOutput(CreateShortIdentityName(idname,publickey))+"</div></td>";
		}
		content+="<td "+GetClassString(messagetrust)+">"+messagetrust+"</td>";
		content+="<td>"+SanitizeOutput(messagetrustcomment)+"</td>";
		content+="<td "+GetClassString(trustlisttrust)+">"+trustlisttrust+"</td>";
		content+="<td>"+SanitizeOutput(trustlisttrustcomment)+"</td>";
		content+="</tr>\r\n";

		st.Step();
	}
	content+="</table>";

	return StringFunctions::Replace(htmltemplate,"[CONTENT]",content);

}

const std::string SiteInserter::GetClassString(const std::string &trustlevel)
{
	int tempint=0;
	std::string tempstr;

	StringFunctions::Convert(trustlevel,tempint);
	tempint/=10;
	StringFunctions::Convert(tempint,tempstr);

	if(trustlevel!="")
	{
		return "class=\"trust"+tempstr+"\"";
	}
	else
	{
		return "";
	}
}

const bool SiteInserter::HandlePutFailed(FCPv2::Message &message)
{
	std::vector<std::string> idparts;
	long localidentityid;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[1],localidentityid);

	RemoveFromInsertList(localidentityid);

	m_log->error("SiteInserter::HandlePutFailed failed to insert Freesite, Freenet error code : "+message["Code"]);

	return true;
}

const bool SiteInserter::HandlePutSuccessful(FCPv2::Message &message)
{
	std::vector<std::string> idparts;
	std::vector<std::string> uriparts;
	long localidentityid;
	int edition=-1;
	Poco::DateTime now;

	StringFunctions::Split(message["Identifier"],"|",idparts);
	StringFunctions::Convert(idparts[1],localidentityid);

	// edition is very last part of uri
	StringFunctions::Split(message["URI"],"/",uriparts);
	if(uriparts.size()>0)
	{
		StringFunctions::Convert(uriparts[uriparts.size()-1],edition);
	}

	SQLite3DB::Statement st=m_db->Prepare("UPDATE tblLocalIdentity SET LastInsertedFreesite=?, FreesiteEdition=? WHERE LocalIdentityID=?;");
	st.Bind(0,Poco::DateTimeFormatter::format(now,"%Y-%m-%d %H:%M:%S"));
	st.Bind(1,edition);
	st.Bind(2,localidentityid);
	st.Step();

	m_log->information("SiteInserter::HandlePutSuccessful successfully inserted Freesite.");

	RemoveFromInsertList(localidentityid);

	return true;
}

void SiteInserter::Initialize()
{
	m_fcpuniquename="SiteInserter";
}

const std::string SiteInserter::SanitizeOutput(const std::string &input)
{
	// must do & first because all other elements have & in them!
	std::string output=StringFunctions::Replace(input,"&","&amp;");
	output=StringFunctions::Replace(output,"<","&lt;");
	output=StringFunctions::Replace(output,">","&gt;");
	output=StringFunctions::Replace(output,"\"","&quot;");
	output=StringFunctions::Replace(output," ","&nbsp;");
	return output;
}

const bool SiteInserter::StartInsert(const long &localidentityid)
{
	FCPv2::Message message;
	std::string localidentityidstr="";
	std::string sizestr="";
	std::string uskkey="";
	std::map<std::string,std::string> pages;
	int filenum=0;

	StringFunctions::Convert(localidentityid,localidentityidstr);

	GeneratePages(localidentityid,uskkey,pages);

	message.SetName("ClientPutComplexDir");
	message["URI"]=uskkey;
	message["Identifier"]=m_fcpuniquename+"|"+localidentityidstr+"|"+message["URI"];
	message["DefaultName"]="index.htm";
	message["PriorityClass"]=m_defaultinsertpriorityclassstr;

	// add each page to the message
	for(std::map<std::string,std::string>::iterator pagei=pages.begin(); pagei!=pages.end(); pagei++)
	{
		std::string filenumstr;
		StringFunctions::Convert(filenum,filenumstr);

		sizestr="0";
		StringFunctions::Convert((*pagei).second.size(),sizestr);

		message["Files."+filenumstr+".Name"]=(*pagei).first;
		message["Files."+filenumstr+".UploadFrom"]="direct";
		message["Files."+filenumstr+".DataLength"]=sizestr;

		filenum++;
	}

	m_fcp->Send(message);

	// send data of each page
	for(std::map<std::string,std::string>::iterator pagei=pages.begin(); pagei!=pages.end(); pagei++)
	{
		m_fcp->Send(std::vector<char>((*pagei).second.begin(),(*pagei).second.end()));
	}

	m_inserting.push_back(localidentityid);

	return true;

}
