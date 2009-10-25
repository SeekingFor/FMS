#include "../../../include/http/pages/forumcreatepostpage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/message.h"
#include "../../../include/unicode/unicodeformatter.h"

#ifdef XMEM
	#include <xmem.h>
#endif
/*
const std::string ForumCreatePostPage::GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars)
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

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="send" && ValidateFormPassword(queryvars))
	{
		if(queryvars.find("localidentityid")!=queryvars.end() && (*queryvars.find("localidentityid")).second!="")
		{
			localidentityidstr=(*queryvars.find("localidentityid")).second;
		}
		else
		{
			error=m_trans->Get("web.page.forumcreatepost.error.localidentity")+"<br />";
		}

		if(queryvars.find("subject")!=queryvars.end() && (*queryvars.find("subject")).second!="")
		{
			subject=(*queryvars.find("subject")).second;
		}
		else
		{
			error+=m_trans->Get("web.page.forumcreatepost.error.subject")+"<br />";
		}

		if(queryvars.find("body")!=queryvars.end() && (*queryvars.find("body")).second!="")
		{
			body=(*queryvars.find("body")).second;
			body=StringFunctions::Replace(body,"\r\n","\n");
			UnicodeFormatter::LineWrap(body,80,">",body);
		}
		else
		{
			error+=m_trans->Get("web.page.forumcreatepost.error.body")+"</br />";
		}

		if(error=="")
		{
			Message mess(m_db);
			
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
				error=m_trans->Get("web.page.forumcreatepost.error.message");
			}
		}
	}
	else
	{
		if(replytomessageidstr!="")
		{
			std::string fromname="";
			SQLite3DB::Statement replyst=m_db->Prepare("SELECT Subject, Body, FromName FROM tblMessage WHERE MessageID=?;");
			replyst.Bind(0,replytomessageidstr);
			replyst.Step();
			if(replyst.RowReturned())
			{
				replyst.ResultText(0,subject);
				replyst.ResultText(1,body);
				replyst.ResultText(2,fromname);

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
				body=fromname+" "+m_trans->Get("web.page.forumcreatepost.wrote")+"\n"+body;

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
	content+="<td>"+m_trans->Get("web.page.forumcreatepost.forum")+" <a href=\"forumthreads.htm?boardid="+boardidstr+"&currentpage="+currentpagestr+"\">"+SanitizeOutput(boardname)+"</a></td>";
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
		content+=CreateFormPassword();
		content+="<table class=\"createpost\">";
		content+="<tr><td class=\"identity\">"+m_trans->Get("web.page.forumcreatepost.from")+"</td><td>"+LocalIdentityDropDown("localidentityid",localidentityidstr)+"</td></tr>";
		content+="<tr><td class=\"subject\">"+m_trans->Get("web.page.forumcreatepost.subject")+"</td><td><input type=\"text\" name=\"subject\" maxlength=\"60\" size=\"60\" value=\""+SanitizeOutput(subject)+"\"></td></tr>";
		content+="<tr><td class=\"body\">"+m_trans->Get("web.page.forumcreatepost.message")+"</td><td><textarea name=\"body\" cols=\"80\" rows=\"30\">"+SanitizeTextAreaOutput(body)+"</textarea></td></tr>";
		content+="<tr><td colspan=\"2\" class=\"send\"><input type=\"submit\" value=\""+m_trans->Get("web.page.forumcreatepost.send")+"\"></td></tr>";
		content+="</table>\r\n";
		content+="</form>";
	}
	else if(page==1)
	{
		content+=m_trans->Get("web.page.forumcreatepost.successfulsend");
	}

	return content;
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
*/
const std::string ForumTemplateCreatePostPage::GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string maincontent("");
	std::string result("");
	std::vector<std::pair<std::string,std::string> > breadcrumblinks;
	std::map<std::string,std::string> vars;
	std::string boardidstr("");
	std::string pagestr("");
	std::string threadidstr("");
	std::string replytomessageidstr("");
	std::string localidentityidstr("");
	std::string subject("");
	std::string body("");
	std::string boardname("");
	std::string error("");
	int page=0;
	SQLite3DB::Statement boardnamest=m_db->Prepare("SELECT BoardName FROM tblBoard WHERE BoardID=?;");
	SQLite3DB::Statement threadsubjectst=m_db->Prepare("SELECT Subject FROM tblMessage INNER JOIN tblThread ON tblMessage.MessageID=tblThread.FirstMessageID INNER JOIN tblThreadPost ON tblThread.ThreadID=tblThreadPost.ThreadID WHERE tblThreadPost.MessageID=?;");

	if(queryvars.find("boardid")!=queryvars.end())
	{
		boardidstr=(*queryvars.find("boardid")).second;
	}
	else
	{
		int temp=0;
		temp=m_viewstate.GetBoardID();
		StringFunctions::Convert(temp,boardidstr);
	}
	if(queryvars.find("page")!=queryvars.end())
	{
		pagestr=(*queryvars.find("page")).second;
	}
	else
	{
		int temp=0;
		temp=m_viewstate.GetPage();
		StringFunctions::Convert(temp,pagestr);
	}
	if(queryvars.find("threadid")!=queryvars.end())
	{
		threadidstr=(*queryvars.find("threadid")).second;
	}
	else
	{
		int temp=0;
		temp=m_viewstate.GetThreadID();
		StringFunctions::Convert(temp,threadidstr);
	}
	if(queryvars.find("replytomessageid")!=queryvars.end())
	{
		int temp=0;
		replytomessageidstr=(*queryvars.find("replytomessageid")).second;
		StringFunctions::Convert(replytomessageidstr,temp);
		m_viewstate.SetReplyToMessageID(temp);
	}
	else
	{
		int temp=0;
		temp=m_viewstate.GetReplyToMessageID();
		StringFunctions::Convert(temp,replytomessageidstr);
	}

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="send" && ValidateFormPassword(queryvars))
	{
		if(queryvars.find("localidentityid")!=queryvars.end() && (*queryvars.find("localidentityid")).second!="")
		{
			localidentityidstr=(*queryvars.find("localidentityid")).second;
		}
		else
		{
			error=m_trans->Get("web.page.forumcreatepost.error.localidentity")+"<br />";
		}

		if(queryvars.find("subject")!=queryvars.end() && (*queryvars.find("subject")).second!="")
		{
			subject=(*queryvars.find("subject")).second;
		}
		else
		{
			error+=m_trans->Get("web.page.forumcreatepost.error.subject")+"<br />";
		}

		if(queryvars.find("body")!=queryvars.end() && (*queryvars.find("body")).second!="")
		{
			body=(*queryvars.find("body")).second;
			body=StringFunctions::Replace(body,"\r\n","\n");
			UnicodeFormatter::LineWrap(body,80,">",body);
		}
		else
		{
			error+=m_trans->Get("web.page.forumcreatepost.error.body")+"</br />";
		}

		if(error=="")
		{
			Message mess(m_db);
			
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
				error=m_trans->Get("web.page.forumcreatepost.error.message");
			}
		}
	}
	else
	{
		if(replytomessageidstr!="")
		{
			std::string fromname="";
			SQLite3DB::Statement replyst=m_db->Prepare("SELECT Subject, Body, FromName FROM tblMessage WHERE MessageID=?;");
			replyst.Bind(0,replytomessageidstr);
			replyst.Step();
			if(replyst.RowReturned())
			{
				replyst.ResultText(0,subject);
				replyst.ResultText(1,body);
				replyst.ResultText(2,fromname);

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
				body=fromname+" "+m_trans->Get("web.page.forumcreatepost.wrote")+"\n"+body;

			}
		}
	}
	
	boardnamest.Bind(0,boardidstr);
	boardnamest.Step();
	if(boardnamest.RowReturned())
	{
		boardnamest.ResultText(0,boardname);
	}

	breadcrumblinks.push_back(std::pair<std::string,std::string>("forummain.htm?viewstate="+m_viewstate.GetViewStateID(),SanitizeOutput(m_trans->Get("web.navlink.browseforums"))));
	breadcrumblinks.push_back(std::pair<std::string,std::string>("forumthreads.htm?viewstate="+m_viewstate.GetViewStateID()+"&boardid="+boardidstr+"&page="+pagestr,SanitizeOutput(boardname)));
	if(replytomessageidstr!="")
	{
		std::string subject("");
		threadsubjectst.Bind(0,replytomessageidstr);
		threadsubjectst.Step();
		if(threadsubjectst.RowReturned())
		{
			threadsubjectst.ResultText(0,subject);
		}
		breadcrumblinks.push_back(std::pair<std::string,std::string>("forumviewthread.htm?viewstate="+m_viewstate.GetViewStateID()+"&boardid="+boardidstr+"&page="+pagestr+"&threadid="+threadidstr,SanitizeOutput(subject)));
		breadcrumblinks.push_back(std::pair<std::string,std::string>("forumcreatepost.htm?viewstate="+m_viewstate.GetViewStateID()+"&boardid="+boardidstr+"&page="+pagestr+"&threadid="+threadidstr+"&replytomessageid="+replytomessageidstr,SanitizeOutput(m_trans->Get("web.page.forumviewthread.reply"))));
	}
	else
	{
		breadcrumblinks.push_back(std::pair<std::string,std::string>("forumcreatepost.htm?viewstate="+m_viewstate.GetViewStateID()+"&boardid="+boardidstr+"&page="+pagestr+"&threadid="+threadidstr,SanitizeOutput(m_trans->Get("web.page.forumthreads.newpost"))));
	}
	CreateBreadcrumbLinks(breadcrumblinks,result);
	vars["LOCATIONBREADCRUMBS"]=result;

	vars["ERROR"]=error;

	StringFunctions::Convert(m_viewstate.GetLocalIdentityID(),localidentityidstr);
	vars["AUTHORDROPDOWN"]=CreateLocalIdentityDropDown("localidentityid",localidentityidstr);
	vars["SUBJECTTEXTBOX"]="<input type=\"text\" name=\"subject\" maxlength=\"60\" size=\"60\" value=\""+SanitizeOutput(subject)+"\">";
	vars["MESSAGETEXTAREA"]="<textarea name=\"body\" cols=\"80\" rows=\"30\">"+SanitizeTextAreaOutput(body)+"</textarea>";

	vars["STARTFORM"]="<form name=\"frmcreatemessage\" method=\"post\" action=\""+m_pagename+"\">";
	vars["STARTFORM"]+="<input type=\"hidden\" name=\"boardid\" value=\""+boardidstr+"\">";
	vars["STARTFORM"]+="<input type=\"hidden\" name=\"page\" value=\""+pagestr+"\">";
	vars["STARTFORM"]+="<input type=\"hidden\" name=\"threadid\" value=\""+threadidstr+"\">";
	vars["STARTFORM"]+="<input type=\"hidden\" name=\"replytomessageid\" value=\""+replytomessageidstr+"\">";
	vars["STARTFORM"]+="<input type=\"hidden\" name=\"formaction\" value=\"send\">";
	vars["SENDBUTTON"]="<input type=\"submit\" value=\""+m_trans->Get("web.page.forumcreatepost.send")+"\">";
	vars["STARTFORM"]+=CreateFormPassword();
	vars["ENDFORM"]="</form>";

	if(page==0)
	{
		m_templatehandler.GetSection("FORUMCREATEPOSTCONTENT",maincontent);
	}
	else if(page==1)
	{
		m_templatehandler.GetSection("FORUMCREATEPOSTSUCCESSFULSEND",maincontent);
	}
	m_templatehandler.PerformReplacements(maincontent,vars,result);

	return result;
}
