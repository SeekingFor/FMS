#include "../../../include/http/pages/forumcreatepostpage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/message.h"
#include "../../../include/unicode/unicodeformatter.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string ForumCreatePostPage::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	int page=0;
	std::string content="";
	std::string boardidstr="";
	std::string currentpagestr="";
	std::string threadidstr="";
	std::string replytomessageidstr="";
	std::string error="";
	std::string boardname="";
	std::string subject="";
	std::string body="";
	std::string localidentityidstr="";

	if(queryvars.find("boardid")!=queryvars.end())
	{
		boardidstr=(*queryvars.find("boardid")).second;
	}
	if(queryvars.find("currentpage")!=queryvars.end())
	{
		currentpagestr=(*queryvars.find("currentpage")).second;
	}
	if(queryvars.find("threadid")!=queryvars.end())
	{
		threadidstr=(*queryvars.find("threadid")).second;
	}
	if(queryvars.find("replytomessageid")!=queryvars.end())
	{
		replytomessageidstr=(*queryvars.find("replytomessageid")).second;
	}

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="send")
	{
		if(queryvars.find("localidentityid")!=queryvars.end() && (*queryvars.find("localidentityid")).second!="")
		{
			localidentityidstr=(*queryvars.find("localidentityid")).second;
		}
		else
		{
			error="You must select a local identity as the sender<br />";
		}

		if(queryvars.find("subject")!=queryvars.end() && (*queryvars.find("subject")).second!="")
		{
			subject=(*queryvars.find("subject")).second;
		}
		else
		{
			error+="You must enter a subject<br />";
		}

		if(queryvars.find("body")!=queryvars.end() && (*queryvars.find("body")).second!="")
		{
			body=(*queryvars.find("body")).second;
			body=StringFunctions::Replace(body,"\r\n","\n");
			UnicodeFormatter::LineWrap(body,80,">",body);
		}
		else
		{
			error+="You must enter a message body</br />";
		}

		if(error=="")
		{
			Message mess;
			
			long localidentityid=-1;
			long boardid=-1;
			std::string references="";

			StringFunctions::Convert(localidentityidstr,localidentityid);
			StringFunctions::Convert(boardidstr,boardid);

			if(replytomessageidstr!="")
			{
				SQLite3DB::Statement st=m_db->Prepare("SELECT MessageUUID FROM tblMessage WHERE MessageID=?;");
				st.Bind(0,replytomessageidstr);
				st.Step();
				if(st.RowReturned())
				{
					st.ResultText(0,references);
				}
			}

			if(mess.Create(localidentityid,boardid,subject,body,references))
			{
				if(mess.PostedToAdministrationBoard()==true)
				{
					mess.HandleAdministrationMessage();
				}
				if(mess.StartFreenetInsert())
				{
					page=1;
				}
			}
			else
			{
				error="Could not create message";
			}
		}
	}
	else
	{
		if(replytomessageidstr!="")
		{
			SQLite3DB::Statement replyst=m_db->Prepare("SELECT Subject, Body FROM tblMessage WHERE MessageID=?;");
			replyst.Bind(0,replytomessageidstr);
			replyst.Step();
			if(replyst.RowReturned())
			{
				replyst.ResultText(0,subject);
				replyst.ResultText(1,body);

				if(subject.size()<3 || (subject.substr(0,3)!="re:" && subject.substr(0,3)!="Re:"))
				{
					subject="Re: "+subject;
				}

				if(body.size()>0)
				{
					if(body[0]=='>')
					{
						body=">"+body;
					}
					else
					{
						body="> "+body;
					}
					std::string::size_type pos=body.find("\n");
					while(pos!=std::string::npos)
					{
						if(pos+1<body.size() && body[pos+1]=='>')
						{
							body.insert(pos+1,">");
						}
						else
						{
							body.insert(pos+1,"> ");
						}
						pos=body.find("\n",pos+2);
					}
					body+="\n";
				}

			}
		}
	}

	SQLite3DB::Statement boardnamest=m_db->Prepare("SELECT BoardName FROM tblBoard WHERE BoardID=?;");
	boardnamest.Bind(0,boardidstr);
	boardnamest.Step();
	if(boardnamest.RowReturned())
	{
		boardnamest.ResultText(0,boardname);
	}

	content+=CreateForumHeader();

	content+="<table class=\"forumheader\">";
	content+="<tr>";
	content+="<td>Forum : <a href=\"forumthreads.htm?boardid="+boardidstr+"&currentpage="+currentpagestr+"\">"+SanitizeOutput(boardname)+"</a></td>";
	content+="</tr>";
	content+="</table>\r\n";

	if(error!="")
	{
		content+="<div class=\"error\">"+error+"</div>\r\n";
	}

	if(page==0)
	{
		content+="<form name=\"frmcreatemessage\" method=\"post\" action=\"forumcreatepost.htm\">";
		content+="<input type=\"hidden\" name=\"boardid\" value=\""+boardidstr+"\">";
		content+="<input type=\"hidden\" name=\"currentpage\" value=\""+currentpagestr+"\">";
		content+="<input type=\"hidden\" name=\"threadid\" value=\""+threadidstr+"\">";
		content+="<input type=\"hidden\" name=\"replytomessageid\" value=\""+replytomessageidstr+"\">";
		content+="<input type=\"hidden\" name=\"formaction\" value=\"send\">";
		content+="<table class=\"createpost\">";
		content+="<tr><td class=\"identity\">From</td><td>"+LocalIdentityDropDown("localidentityid",localidentityidstr)+"</td></tr>";
		content+="<tr><td class=\"subject\">Subject</td><td><input type=\"text\" name=\"subject\" maxlength=\"60\" size=\"60\" value=\""+SanitizeOutput(subject)+"\"></td></tr>";
		content+="<tr><td class=\"body\">Message</td><td><textarea name=\"body\" cols=\"80\" rows=\"30\">"+SanitizeTextAreaOutput(body)+"</textarea></td></tr>";
		content+="<tr><td colspan=\"2\" class=\"send\"><input type=\"submit\" value=\"Send\"></td></tr>";
		content+="</table>\r\n";
		content+="</form>";
	}
	else if(page==1)
	{
		content+="You have sent your message.  It will show up in the thread after it has been successfully inserted and retrieved by FMS.";
	}

	return StringFunctions::Replace(m_template,"[CONTENT]",content);
}

const std::string ForumCreatePostPage::LocalIdentityDropDown(const std::string &name, const std::string &selectedid)
{
	std::string html="<select name=\""+name+"\">";
	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID, Name FROM tblLocalIdentity ORDER BY Name COLLATE NOCASE ASC;");

	if(selectedid=="")
	{
		html+="<option value=\"\" selected></option>";
	}

	st.Step();
	while(st.RowReturned())
	{
		std::string id="";
		std::string name="";
		st.ResultText(0,id);
		st.ResultText(1,name);
		html+="<option value=\""+id+"\"";
		if(id==selectedid)
		{
			html+=" selected";
		}
		html+=">"+FixFromName(name)+"</option>";
		st.Step();
	}
	html+="</select>";
	return html;
}
