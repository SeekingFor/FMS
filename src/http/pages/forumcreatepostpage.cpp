#include "../../../include/http/pages/forumcreatepostpage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/message.h"
#include "../../../include/unicode/unicodeformatter.h"

#ifdef XMEM
	#include <xmem.h>
#endif

void ForumTemplateCreatePostPage::ClearFileAttachments(const std::string &viewstateid)
{
	SQLite3DB::Statement st=m_db->Prepare("DELETE FROM tmpFileAttachment WHERE ForumViewStateID=?;");
	st.Bind(0,viewstateid);
	st.Step();

	st=m_db->Prepare("DELETE FROM tmpFileAttachment WHERE DateAdded<strftime('%Y-%m-%d %H:%M:%S','now','-6 hours');");
	st.Step();
}

const std::string ForumTemplateCreatePostPage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
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
	std::vector<fileattachment> fileattachments;
	SQLite3DB::Statement boardnamest=m_db->Prepare("SELECT BoardName FROM tblBoard WHERE BoardID=?;");
	SQLite3DB::Statement threadsubjectst=m_db->Prepare("SELECT Subject FROM tblMessage INNER JOIN tblThread ON tblMessage.MessageID=tblThread.FirstMessageID INNER JOIN tblThreadPost ON tblThread.ThreadID=tblThreadPost.ThreadID WHERE tblThreadPost.MessageID=?;");

	if(queryvars.find("boardid")!=queryvars.end())
	{
		boardidstr=(*queryvars.find("boardid")).second.GetData();
		int temp=0;
		if(StringFunctions::Convert(boardidstr,temp)==true)
		{
			m_viewstate.SetBoardID(temp);
		}
	}
	else
	{
		int temp=0;
		temp=m_viewstate.GetBoardID();
		StringFunctions::Convert(temp,boardidstr);
	}
	if(queryvars.find("page")!=queryvars.end())
	{
		pagestr=(*queryvars.find("page")).second.GetData();
	}
	else
	{
		int temp=0;
		temp=m_viewstate.GetPage();
		StringFunctions::Convert(temp,pagestr);
	}
	if(queryvars.find("threadid")!=queryvars.end())
	{
		threadidstr=(*queryvars.find("threadid")).second.GetData();
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
		replytomessageidstr=(*queryvars.find("replytomessageid")).second.GetData();
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
		int action=0;

		LoadFileAttachments(m_viewstate.GetViewStateID(),fileattachments);

		if(queryvars.find("attachbutton")!=queryvars.end())
		{
			action=1;
			fileattachment attach;
			bool validattachment=false;

			if(queryvars.find("uploadfile")!=queryvars.end() && (*queryvars.find("uploadfile")).second!="")
			{
				attach.m_data=(*queryvars.find("uploadfile")).second.GetData();
				attach.m_datasize=attach.m_data.size();
				attach.m_filename=(*queryvars.find("uploadfile")).second.GetFileName();
				attach.m_contenttype=(*queryvars.find("uploadfile")).second.GetContentType();
				validattachment=true;
			}
			else if(queryvars.find("freenetkey")!=queryvars.end() && (*queryvars.find("freenetkey")).second!="")
			{
				/* TODO - make sure key is valid */
				attach.m_freenetkey=(*queryvars.find("freenetkey")).second.GetData();
				validattachment=true;
			}

			if(validattachment==true)
			{
				SQLite3DB::Statement filest=m_db->Prepare("INSERT INTO tmpFileAttachment(DateUploaded,ForumViewStateID,FileName,Data,DataLength,ContentType,FreenetKey) VALUES(strftime('%Y-%m-%d %H:%M:%S','now'),?,?,?,?,?,?);");

				filest.Bind(0,m_viewstate.GetViewStateID());
				if(attach.m_filename!="")
				{
					filest.Bind(1,attach.m_filename);
					std::vector<char> attachdata(attach.m_data.begin(),attach.m_data.end());
					filest.Bind(2,&attachdata[0],attachdata.size());
					filest.Bind(3,static_cast<long>(attachdata.size()));
					filest.Bind(4,attach.m_contenttype);
					filest.Bind(5);
				}
				else
				{
					filest.Bind(1);
					filest.Bind(2);
					filest.Bind(3);
					filest.Bind(4);
					filest.Bind(5,attach.m_freenetkey);
				}
				filest.Step(true);

				attach.m_id=filest.GetLastInsertRowID();

				fileattachments.push_back(attach);
			}

		}

		if(queryvars.find("localidentityid")!=queryvars.end() && (*queryvars.find("localidentityid")).second!="")
		{
			localidentityidstr=(*queryvars.find("localidentityid")).second.GetData();
		}
		else
		{
			if(action==0)
			{
				error=m_trans->Get("web.page.forumcreatepost.error.localidentity")+"<br />";
			}
		}

		if(queryvars.find("subject")!=queryvars.end() && (*queryvars.find("subject")).second!="")
		{
			subject=(*queryvars.find("subject")).second.GetData();
		}
		else
		{
			if(action==0)
			{
				error+=m_trans->Get("web.page.forumcreatepost.error.subject")+"<br />";
			}
		}

		if(queryvars.find("body")!=queryvars.end() && (*queryvars.find("body")).second!="")
		{
			body=(*queryvars.find("body")).second.GetData();
		}
		else
		{
			if(action==0)
			{
				error+=m_trans->Get("web.page.forumcreatepost.error.body")+"</br />";
			}
		}

		if(error=="" && action==0)
		{
			Message mess(m_db);
			
			long localidentityid=-1;
			long boardid=-1;
			std::vector<std::string> references;
			std::string reference="";

			StringFunctions::Convert(localidentityidstr,localidentityid);
			StringFunctions::Convert(boardidstr,boardid);

			body=StringFunctions::Replace(body,"\r\n","\n");
			//UnicodeFormatter::LineWrap(body,80,">",body);

			if(replytomessageidstr!="")
			{
				SQLite3DB::Statement st=m_db->Prepare("SELECT MessageUUID FROM tblMessage WHERE MessageID=?;");
				st.Bind(0,replytomessageidstr);
				st.Step();
				if(st.RowReturned())
				{
					st.ResultText(0,reference);
					if(reference!="")
					{
						references.push_back(reference);
					}

					st=m_db->Prepare("SELECT ReplyToMessageUUID FROM tblMessageReplyTo WHERE MessageID=? ORDER BY ReplyOrder ASC;");
					st.Bind(0,replytomessageidstr);
					st.Step();
					while(st.RowReturned())
					{
						reference="";
						st.ResultText(0,reference);
						if(reference!="")
						{
							references.push_back(reference);
						}
						st.Step();
					}
				}
			}

			if(boardid>0 && mess.Create(localidentityid,boardid,subject,body,references))
			{

				// add all attachments
				SQLite3DB::Statement fst=m_db->Prepare("SELECT FileName, Data, DataLength, ContentType FROM tmpFileAttachment WHERE FileAttachmentID=?;");
				for(std::vector<fileattachment>::const_iterator fi=fileattachments.begin(); fi!=fileattachments.end(); fi++)
				{
					fst.Bind(0,(*fi).m_id);
					fst.Step();
					if(fst.RowReturned())
					{
						std::string filename("");
						std::string contenttype("");
						int datalength=0;

						fst.ResultText(0,filename);
						fst.ResultText(3,contenttype);
						fst.ResultInt(2,datalength);
						std::vector<unsigned char> filedata(datalength,0);
						fst.ResultBlob(1,&filedata[0],datalength);

						mess.AddInsertFileAttachment(filename,contenttype,filedata);
					}
					fst.Reset();
				}

				if(mess.PostedToAdministrationBoard()==true)
				{
					mess.HandleAdministrationMessage();
				}
				if(mess.PrepareFreenetInsert() && mess.GetMessageXML(true).size()<=Message::MaxMessageXMLSize() && mess.StartFreenetInsert())
				{
					m_viewstate.UnsetReplyToMessageID();
					page=1;
				}
				if(mess.GetMessageXML(true).size()>Message::MaxMessageXMLSize())
				{
					error=m_trans->Get("web.page.forumcreatepost.error.toobig");
				}
			}
			else
			{
				error=m_trans->Get("web.page.forumcreatepost.error.message");
				if(boardid<=0)
				{
					m_log->debug("ForumTemplateCreatePostPage::GenerateContent error with boardid.  boardidstr="+boardidstr+" threadidstr="+threadidstr+" replytomessageidstr="+replytomessageidstr);
				}
			}
		}
	}
	else
	{

		ClearFileAttachments(m_viewstate.GetViewStateID());

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
					std::string::size_type pos=body.find('\n');
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
						pos=body.find('\n',pos+2);
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
	if(replytomessageidstr!="0")
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
	// must do this after sanitize, because sanitize will convert & to &amp;
	vars["MESSAGETEXTAREA"]=StringFunctions::Replace(vars["MESSAGETEXTAREA"],"[","&#91;");
	vars["MESSAGETEXTAREA"]=StringFunctions::Replace(vars["MESSAGETEXTAREA"],"]","&#93;");

	vars["STARTFORM"]="<form name=\"frmcreatemessage\" method=\"post\" action=\""+m_pagename+"\" enctype=\"multipart/form-data\">";
	vars["STARTFORM"]+="<input type=\"hidden\" name=\"boardid\" value=\""+boardidstr+"\">";
	vars["STARTFORM"]+="<input type=\"hidden\" name=\"page\" value=\""+pagestr+"\">";
	vars["STARTFORM"]+="<input type=\"hidden\" name=\"threadid\" value=\""+threadidstr+"\">";
	vars["STARTFORM"]+="<input type=\"hidden\" name=\"replytomessageid\" value=\""+replytomessageidstr+"\">";
	vars["STARTFORM"]+="<input type=\"hidden\" name=\"formaction\" value=\"send\">";
	vars["STARTFORM"]+="<input type=\"hidden\" name=\"viewstate\" value=\""+m_viewstate.GetViewStateID()+"\">";
	vars["SENDBUTTON"]="<input type=\"submit\" value=\""+m_trans->Get("web.page.forumcreatepost.send")+"\">";
	vars["STARTFORM"]+=CreateFormPassword();
	vars["ENDFORM"]="</form>";

	/* TODO - add existing attachments */
	std::string currentattachments="";
	std::string attachmentrow;
	m_templatehandler.GetSection("NEWPOSTATTACHMENT",attachmentrow);
	for(std::vector<fileattachment>::const_iterator i=fileattachments.begin(); i!=fileattachments.end(); i++)
	{
		std::map<std::string,std::string> avars;
		std::string res("");

		if((*i).m_filename!="")
		{
			std::string sizestr("");
			avars["ATTACHMENTNAME"]=(*i).m_filename;

			StringFunctions::Convert((*i).m_datasize,sizestr);
			avars["ATTACHMENTSIZE"]=sizestr;
		}
		else
		{
			avars["ATTACHMENTNAME"]=(*i).m_freenetkey;
			avars["ATTACHMENTSIZE"]=m_trans->Get("web.page.forumcreatepost.unknownsize");
		}

		m_templatehandler.PerformReplacements(attachmentrow,avars,res);
		currentattachments+=res;
	}

	vars["NEWPOSTATTACHMENTS"]=currentattachments;

	vars["ATTACHFORM"]=m_trans->Get("web.page.forumcreatepost.upload")+" ";
	vars["ATTACHFORM"]+="<input type=\"file\" name=\"uploadfile\">";
	/*
	vars["ATTACHFORM"]+=" "+m_trans->Get("web.page.forumcreatepost.orfreenetkey")+" ";
	vars["ATTACHFORM"]+="<input type=\"text\" name=\"freenetkey\" size=\"80\">";
	*/
	vars["ATTACHFORM"]+=CreateFormPassword();
	vars["ATTACHFORM"]+="<input type=\"submit\" name=\"attachbutton\" value=\""+m_trans->Get("web.page.forumcreatepost.attach")+"\">";

	if(page==0)
	{
		m_templatehandler.GetSection("FORUMCREATEPOSTCONTENT",maincontent);
	}
	else if(page==1)
	{
		m_templatehandler.GetSection("FORUMCREATEPOSTSUCCESSFULSEND",maincontent);
	}
	m_templatehandler.PerformReplacements(maincontent,vars,result);

	vars["FORUMSEARCH"]="";

	return result;
}

void ForumTemplateCreatePostPage::LoadFileAttachments(const std::string &viewstateid, std::vector<fileattachment> &fileattachments)
{
	SQLite3DB::Statement st=m_db->Prepare("SELECT FileAttachmentID, FileName, DataLength, ContentType, FreenetKey FROM tmpFileAttachment WHERE ForumViewStateID=?;");
	st.Bind(0,viewstateid);
	st.Step();
	while(st.RowReturned())
	{
		fileattachment att;
		st.ResultInt(0,att.m_id);

		if(st.ResultNull(1)==false && st.ResultNull(2)==false)
		{
			st.ResultText(1,att.m_filename);
			st.ResultInt(2,att.m_datasize);
			st.ResultText(3,att.m_contenttype);
		}
		else
		{
			st.ResultText(4,att.m_freenetkey);
		}

		fileattachments.push_back(att);
		st.Step();
	}
}
