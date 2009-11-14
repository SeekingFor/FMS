#include "../../../include/http/pages/forumviewthreadpage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/unicode/unicodeformatter.h"
#include "../../../include/option.h"
#include "../../../include/quoter.h"
#include "../../../include/keyfinder.h"

#ifdef XMEM
	#include <xmem.h>
#endif
/*
ForumViewThreadPage::ForumViewThreadPage(SQLite3DB::DB *db,const std::string &templatestr):ForumPage(db,templatestr,"forumviewthread.htm")
{
	Option option(db);

	option.Get("FCPHost",m_fcphost);
	option.Get("FProxyPort",m_fproxyport);
}

const std::string ForumViewThreadPage::FixBody(const std::string &body)
{
	std::string output=body;

	output=StringFunctions::Replace(output,"\r\n","\n");

	UnicodeFormatter::LineWrap(output,80,"",output);

	output=StringFunctions::Replace(output,"<","&lt;");
	output=StringFunctions::Replace(output,">","&gt;");
	output=StringFunctions::Replace(output,"\n","<br />");
	return output;
}

const std::string ForumViewThreadPage::GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	std::string content="";
	std::string threadidstr="";
	std::string boardidstr="";
	std::string currentpagestr="";
	std::string boardname="";
	std::string firstunreadidstr="";

	if(queryvars.find("threadid")!=queryvars.end())
	{
		threadidstr=(*queryvars.find("threadid")).second;
	}
	if(queryvars.find("currentpage")!=queryvars.end())
	{
		currentpagestr=(*queryvars.find("currentpage")).second;
	}
	if(queryvars.find("boardid")!=queryvars.end())
	{
		boardidstr=(*queryvars.find("boardid")).second;
	}

	content+=CreateForumHeader();

	SQLite3DB::Statement firstunreadst=m_db->Prepare("SELECT tblMessage.MessageID FROM tblThreadPost INNER JOIN tblMessage ON tblThreadPost.MessageID=tblMessage.MessageID WHERE ThreadID=? AND tblMessage.Read=0;");
	firstunreadst.Bind(0,threadidstr);
	firstunreadst.Step();
	if(firstunreadst.RowReturned())
	{
		firstunreadst.ResultText(0,firstunreadidstr);
	}

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="markunread")
	{
		SQLite3DB::Statement updateread=m_db->Prepare("UPDATE tblMessage SET Read=0 WHERE tblMessage.MessageID IN (SELECT MessageID FROM tblThreadPost WHERE ThreadID=?);");
		updateread.Bind(0,threadidstr);
		updateread.Step();
	}
	else
	{
		SQLite3DB::Statement updateread=m_db->Prepare("UPDATE tblMessage SET Read=1 WHERE tblMessage.MessageID IN (SELECT MessageID FROM tblThreadPost WHERE ThreadID=?);");
		updateread.Bind(0,threadidstr);
		updateread.Step();
	}

	SQLite3DB::Statement trustst=m_db->Prepare("SELECT LocalMessageTrust, LocalTrustListTrust, PeerMessageTrust, PeerTrustListTrust, Name FROM tblIdentity WHERE IdentityID=?;");

	SQLite3DB::Statement boardnamest=m_db->Prepare("SELECT tblBoard.BoardName FROM tblBoard INNER JOIN tblThread ON tblBoard.BoardID=tblThread.BoardID WHERE tblThread.ThreadID=?;");
	boardnamest.Bind(0,threadidstr);
	boardnamest.Step();

	if(boardnamest.RowReturned())
	{
		boardnamest.ResultText(0,boardname);
	}

	content+="<table class=\"forumheader\">";
	content+="<tr>";
	content+="<td> "+m_trans->Get("web.page.forumviewthread.forum")+" <a href=\"forumthreads.htm?boardid="+boardidstr+"&currentpage="+currentpagestr+"\">"+SanitizeOutput(boardname)+"</a></td>";
	if(firstunreadidstr!="")
	{
		content+="<td>";
		content+="<a href=\"#"+firstunreadidstr+"\">"+m_trans->Get("web.page.forumviewthread.firstunread")+"</a>";
		content+="</td>";
	}
	content+="<td>";
	content+="<a href=\""+m_pagename+"?formaction=markunread&threadid="+threadidstr+"&boardid="+boardidstr+"&currentpage="+currentpagestr+"\">"+m_trans->Get("web.page.forumviewthread.markunread")+"</a>";
	content+="</td>";
	content+="</tr>";
	content+="</table>\r\n";

	SQLite3DB::Statement fileattachmentst=m_db->Prepare("SELECT Key, Size FROM tblMessageFileAttachment WHERE MessageID=?;");

	SQLite3DB::Statement st=m_db->Prepare("SELECT tblMessage.MessageID, tblMessage.IdentityID, tblMessage.FromName, tblMessage.Subject, tblMessage.MessageDate || ' ' || tblMessage.MessageTime, tblMessage.Body FROM tblMessage INNER JOIN tblThreadPost ON tblMessage.MessageID=tblThreadPost.MessageID WHERE tblThreadPost.ThreadID=? ORDER BY tblThreadPost.PostOrder;");
	st.Bind(0,threadidstr);

	content+="<table class=\"thread\">";
	st.Step();
	while(st.RowReturned())
	{
		int messageid(0);
		std::string messageidstr="";
		std::string identityidstr="";
		std::string fromname="";
		std::string subject="";
		std::string datetime="";
		std::string body="";
		
		st.ResultInt(0,messageid);
		st.ResultText(0,messageidstr);
		st.ResultText(1,identityidstr);
		st.ResultText(2,fromname);
		st.ResultText(3,subject);
		st.ResultText(4,datetime);
		st.ResultText(5,body);

		content+="<tr>";
		content+="<td rowspan=\"2\" class=\"from\">";
		content+="<a name=\""+messageidstr+"\"></a>";
		content+="<a href=\"peerdetails.htm?identityid="+identityidstr+"\">"+FixFromName(fromname)+"</a><br />";

		trustst.Bind(0,identityidstr);
		trustst.Step();
		if(trustst.RowReturned())
		{
			std::string localmessagetrust="";
			std::string localtrustlisttrust="";
			std::string peermessagetrust="";
			std::string peertrustlisttrust="";
			std::string name="";

			trustst.ResultText(0,localmessagetrust);
			trustst.ResultText(1,localtrustlisttrust);
			trustst.ResultText(2,peermessagetrust);
			trustst.ResultText(3,peertrustlisttrust);
			trustst.ResultText(4,name);

			content+="<table class=\"trust\">";
			content+="<tr>";
			content+="<td colspan=\"3\" style=\"text-align:center;\"><a href=\"peertrust.htm?namesearch="+StringFunctions::UriEncode(name)+"\">"+m_trans->Get("web.page.forumviewthread.trust")+"</a></td>";
			content+="</tr>";
			content+="<tr>";
			content+="<td></td><td>"+m_trans->Get("web.page.forumviewthread.local")+"</td><td>"+m_trans->Get("web.page.forumviewthread.peer")+"</td>";
			content+="</tr>";
			content+="<tr>";
			content+="<td>"+m_trans->Get("web.page.forumviewthread.message")+"</td><td>"+localmessagetrust+"</td><td>"+peermessagetrust+"</td>";
			content+="</tr>";
			content+="<tr>";
			content+="<td>"+m_trans->Get("web.page.forumviewthread.trustlist")+"</td><td>"+localtrustlisttrust+"</td><td>"+peertrustlisttrust+"</td>";
			content+="</tr>";
			content+="</table>";
		}

		content+="</td>";
		content+="<td class=\"subject\">";
		content+=SanitizeOutput(subject)+" "+m_trans->Get("web.page.forumviewthread.on")+" "+datetime;
		content+="</td>";
		content+="<td><a href=\"forumcreatepost.htm?replytomessageid="+messageidstr+"&threadid="+threadidstr+"&boardid="+boardidstr+"&currentpage="+currentpagestr+"\">"+m_trans->Get("web.page.forumviewthread.reply")+"</a></td>";
		content+="</tr>\r\n";
		content+="<tr>";
		content+="<td class=\"body\" colspan=\"2\">";
		content+=FixBody(body);

		fileattachmentst.Bind(0,messageid);
		fileattachmentst.Step();
		if(fileattachmentst.RowReturned())
		{
			content+="<div class=\"fileattachments\">";
			while(fileattachmentst.RowReturned())
			{
				std::string key("");
				std::string sizestr("");
				
				fileattachmentst.ResultText(0,key);
				fileattachmentst.ResultText(1,sizestr);

				content+="<div class=\"attachment\">";
				content+="<a href=\"http://"+SanitizeOutput(m_fcphost)+":"+SanitizeOutput(m_fproxyport)+"/"+SanitizeOutput(key)+"\">"+SanitizeOutput(key)+"</a>";
				content+="<br />";
				content+=sizestr+" bytes";
				content+="</div>";

				fileattachmentst.Step();
			}
			content+="</div>";
		}
		fileattachmentst.Reset();

		content+="</td>";
		content+="</tr>";
		trustst.Reset();

		st.Step();
	}
	content+="</table>";

	return content;
}
*/

ForumTemplateViewThreadPage::ForumTemplateViewThreadPage(SQLite3DB::DB *db, const HTMLTemplateHandler &templatehandler):ForumTemplatePage(db,templatehandler,"forumviewthread.htm"),m_emot("images/smilies/")
{
	Option option(db);
	option.GetBool("ForumDetectLinks",m_detectlinks);
	option.GetBool("ForumShowSmilies",m_showsmilies);
}

const std::string ForumTemplateViewThreadPage::GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	int postcount=0;
	std::string maincontent("");
	std::string result("");
	std::map<std::string,std::string> vars;
	std::vector<std::pair<std::string,std::string> > breadcrumblinks;
	std::string threadidstr("");
	std::string boardidstr("");
	std::string pagestr("");
	std::string boardname("");
	std::string firstunreadidstr("");
	std::string threadpostrowodd("");
	std::string threadpostroweven("");
	std::string postrows("");
	std::string threadpostattachment("");
	std::string postattachments("");
	std::string trusttable("");
	SQLite3DB::Statement fileattachmentst=m_db->Prepare("SELECT Key, Size FROM tblMessageFileAttachment WHERE MessageID=?;");
	SQLite3DB::Statement truststpeeronly=m_db->Prepare("SELECT PeerMessageTrust, PeerTrustListTrust FROM tblIdentity WHERE IdentityID=?;");
	SQLite3DB::Statement truststboth=m_db->Prepare("SELECT tblIdentityTrust.LocalMessageTrust, tblIdentity.PeerMessageTrust, tblIdentityTrust.LocalTrustListTrust, tblIdentity.PeerTrustListTrust FROM tblIdentity LEFT JOIN tblIdentityTrust ON tblIdentity.IdentityID=tblIdentityTrust.IdentityID WHERE tblIdentity.IdentityID=? AND tblIdentityTrust.LocalIdentityID=?;");

	if(queryvars.find("threadid")!=queryvars.end())
	{
		int temp=0;
		threadidstr=(*queryvars.find("threadid")).second;
		StringFunctions::Convert(threadidstr,temp);
		m_viewstate.SetThreadID(temp);
	}
	else
	{
		int temp=0;
		temp=m_viewstate.GetThreadID();
		StringFunctions::Convert(temp,threadidstr);
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

	// first unread select must come before marking read messages
	SQLite3DB::Statement firstunreadst=m_db->Prepare("SELECT tblMessage.MessageID FROM tblThreadPost INNER JOIN tblMessage ON tblThreadPost.MessageID=tblMessage.MessageID WHERE ThreadID=? AND tblMessage.Read=0;");
	firstunreadst.Bind(0,threadidstr);
	firstunreadst.Step();
	if(firstunreadst.RowReturned())
	{
		firstunreadst.ResultText(0,firstunreadidstr);
	}

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="markunread")
	{
		SQLite3DB::Statement updateread=m_db->Prepare("UPDATE tblMessage SET Read=0 WHERE tblMessage.MessageID IN (SELECT MessageID FROM tblThreadPost WHERE ThreadID=?);");
		updateread.Bind(0,threadidstr);
		updateread.Step();
	}
	else
	{
		SQLite3DB::Statement updateread=m_db->Prepare("UPDATE tblMessage SET Read=1 WHERE tblMessage.MessageID IN (SELECT MessageID FROM tblThreadPost WHERE ThreadID=?);");
		updateread.Bind(0,threadidstr);
		updateread.Step();
	}

	// add/remove trust
	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second.find("trust")!=std::string::npos && queryvars.find("identityid")!=queryvars.end() && ValidateFormPassword(queryvars))
	{
		SQLite3DB::Statement currenttrustst=m_db->Prepare("SELECT IFNULL(LocalMessageTrust,-1), IFNULL(LocalTrustListTrust,-1) FROM tblIdentityTrust WHERE IdentityID=? AND LocalIdentityID=?;");
		SQLite3DB::Statement updatetrustst=m_db->Prepare("UPDATE tblIdentityTrust SET LocalMessageTrust=?, LocalTrustListTrust=? WHERE IdentityID=? AND LocalIdentityID=?;");

		currenttrustst.Bind(0,(*queryvars.find("identityid")).second);
		currenttrustst.Bind(1,m_viewstate.GetLocalIdentityID());
		currenttrustst.Step();
		if(currenttrustst.RowReturned())
		{
			int localmessagetrust=-1;
			int localtrustlisttrust=-1;

			currenttrustst.ResultInt(0,localmessagetrust);
			currenttrustst.ResultInt(1,localtrustlisttrust);

			if((*queryvars.find("formaction")).second=="addmessagetrust")
			{
				if(localmessagetrust==-1)
				{
					localmessagetrust=50;
				}
				localmessagetrust=(std::min)(localmessagetrust+10,100);
			}
			else if((*queryvars.find("formaction")).second=="removemessagetrust")
			{
				if(localmessagetrust==-1)
				{
					localmessagetrust=50;
				}
				localmessagetrust=(std::max)(localmessagetrust-10,0);
			}
			else if((*queryvars.find("formaction")).second=="addtrustlisttrust")
			{
				if(localtrustlisttrust==-1)
				{
					localtrustlisttrust=50;
				}
				localtrustlisttrust=(std::min)(localtrustlisttrust+10,100);
			}
			else if((*queryvars.find("formaction")).second=="removetrustlisttrust")
			{
				if(localtrustlisttrust==-1)
				{
					localtrustlisttrust=50;
				}
				localtrustlisttrust=(std::max)(localtrustlisttrust-10,0);
			}

			if(localmessagetrust!=-1)
			{
				updatetrustst.Bind(0,localmessagetrust);
			}
			else
			{
				updatetrustst.Bind(0);
			}
			if(localtrustlisttrust!=-1)
			{
				updatetrustst.Bind(1,localtrustlisttrust);
			}
			else
			{
				updatetrustst.Bind(1);
			}
			updatetrustst.Bind(2,(*queryvars.find("identityid")).second);
			updatetrustst.Bind(3,m_viewstate.GetLocalIdentityID());
			updatetrustst.Step();

		}
	}

	SQLite3DB::Statement boardnamest=m_db->Prepare("SELECT tblBoard.BoardName FROM tblBoard INNER JOIN tblThread ON tblBoard.BoardID=tblThread.BoardID WHERE tblThread.ThreadID=?;");
	boardnamest.Bind(0,threadidstr);
	boardnamest.Step();
	if(boardnamest.RowReturned())
	{
		boardnamest.ResultText(0,boardname);
	}


	breadcrumblinks.push_back(std::pair<std::string,std::string>("forummain.htm?viewstate="+m_viewstate.GetViewStateID(),SanitizeOutput(m_trans->Get("web.navlink.browseforums"))));
	breadcrumblinks.push_back(std::pair<std::string,std::string>("forumthreads.htm?viewstate="+m_viewstate.GetViewStateID()+"&boardid="+boardidstr+"&page="+pagestr,SanitizeOutput(boardname)));

	if(firstunreadidstr!="")
	{
		vars["FIRSTUNREADPOSTLINK"]="<a href=\"#"+firstunreadidstr+"\"><img src=\"images/mail_get.png\" border=\"0\" style=\"vertical-align:bottom;\">"+m_trans->Get("web.page.forumviewthread.firstunread")+"</a>";
	}
	else
	{
		vars["FIRSTUNREADPOSTLINK"]="";
	}

	m_templatehandler.GetSection("THREADPOSTATTACHMENT",threadpostattachment);
	m_templatehandler.GetSection("TRUSTTABLE",trusttable);

	vars["MARKUNREADLINK"]="<a href=\""+m_pagename+"?viewstate="+m_viewstate.GetViewStateID()+"&formaction=markunread&threadid="+threadidstr+"&boardid="+boardidstr+"&page="+pagestr+"\"><img src=\"images/mail_generic.png\" border=\"0\" style=\"vertical-align:bottom;\">"+m_trans->Get("web.page.forumviewthread.markunread")+"</a>";

	// thread posts
	m_templatehandler.GetSection("THREADPOSTROWODD",threadpostrowodd);
	m_templatehandler.GetSection("THREADPOSTROWEVEN",threadpostroweven);
	SQLite3DB::Statement st=m_db->Prepare("SELECT tblMessage.MessageID, tblMessage.IdentityID, tblMessage.FromName, tblMessage.Subject, tblMessage.MessageDate || ' ' || tblMessage.MessageTime, tblMessage.Body FROM tblMessage INNER JOIN tblThreadPost ON tblMessage.MessageID=tblThreadPost.MessageID WHERE tblThreadPost.ThreadID=? ORDER BY tblThreadPost.PostOrder;");
	st.Bind(0,threadidstr);
	st.Step();
	while(st.RowReturned())
	{
		std::map<std::string,std::string> postvars;
		std::string postrow("");
		int messageid(0);
		std::string messageidstr="";
		std::string identityidstr="";
		std::string fromname="";
		std::string subject="";
		std::string datetime="";
		std::string body="";
		
		st.ResultInt(0,messageid);
		st.ResultText(0,messageidstr);
		st.ResultText(1,identityidstr);
		st.ResultText(2,fromname);
		st.ResultText(3,subject);
		st.ResultText(4,datetime);
		st.ResultText(5,body);

		if(postcount==0)
		{
			breadcrumblinks.push_back(std::pair<std::string,std::string>(m_pagename+"?viewstate="+m_viewstate.GetViewStateID()+"&threadid="+threadidstr+"&boardid="+boardidstr+"&page="+pagestr,SanitizeOutput(subject)));
		}

		postvars["THREADPOSTANCHOR"]="<a name=\""+messageidstr+"\"></a>";

		if(identityidstr=="")
		{
			postvars["CONDITIONALTRUSTTABLE"]="";
		}
		else
		{
			std::string localmessagetrust("");
			std::string localtrustlisttrust("");
			std::string peermessagetrust("");
			std::string peertrustlisttrust("");

			if(m_viewstate.GetLocalIdentityID()==0)
			{
				truststpeeronly.Bind(0,identityidstr);
				truststpeeronly.Step();
				if(truststpeeronly.RowReturned())
				{
					truststpeeronly.ResultText(0,peermessagetrust);
					truststpeeronly.ResultText(1,peertrustlisttrust);
				}
				truststpeeronly.Reset();
			}
			else
			{
				truststboth.Bind(0,identityidstr);
				truststboth.Bind(1,m_viewstate.GetLocalIdentityID());
				truststboth.Step();
				if(truststboth.RowReturned())
				{
					truststboth.ResultText(0,localmessagetrust);
					truststboth.ResultText(1,peermessagetrust);
					truststboth.ResultText(2,localtrustlisttrust);
					truststboth.ResultText(3,peertrustlisttrust);
				}
				truststboth.Reset();
			}

			postvars["CONDITIONALTRUSTTABLE"]=trusttable;
			postvars["LOCALMESSAGETRUST"]=localmessagetrust;
			postvars["PEERMESSAGETRUST"]=peermessagetrust;
			postvars["LOCALTRUSTLISTTRUST"]=localtrustlisttrust;
			postvars["PEERTRUSTLISTTRUST"]=peertrustlisttrust;

			if(m_viewstate.GetLocalIdentityID()!=0)
			{
				postvars["LOCALMESSAGETRUST"]+="<a href=\""+m_pagename+"?viewstate="+m_viewstate.GetViewStateID()+"&"+CreateLinkFormPassword()+"&threadid="+threadidstr+"&boardid="+boardidstr+"&page="+pagestr+"&formaction=addmessagetrust&identityid="+identityidstr+"#"+messageidstr+"\"><img src=\"images/circleplus.png\" border=\"0\" style=\"vertical-align:bottom;\"></a>";
				postvars["LOCALMESSAGETRUST"]+="<a href=\""+m_pagename+"?viewstate="+m_viewstate.GetViewStateID()+"&"+CreateLinkFormPassword()+"&threadid="+threadidstr+"&boardid="+boardidstr+"&page="+pagestr+"&formaction=removemessagetrust&identityid="+identityidstr+"#"+messageidstr+"\"><img src=\"images/circleminus.png\" border=\"0\" style=\"vertical-align:bottom;\"></a>";
				postvars["LOCALTRUSTLISTTRUST"]+="<a href=\""+m_pagename+"?viewstate="+m_viewstate.GetViewStateID()+"&"+CreateLinkFormPassword()+"&threadid="+threadidstr+"&boardid="+boardidstr+"&page="+pagestr+"&formaction=addtrustlisttrust&identityid="+identityidstr+"#"+messageidstr+"\"><img src=\"images/circleplus.png\" border=\"0\" style=\"vertical-align:bottom;\"></a>";
				postvars["LOCALTRUSTLISTTRUST"]+="<a href=\""+m_pagename+"?viewstate="+m_viewstate.GetViewStateID()+"&"+CreateLinkFormPassword()+"&threadid="+threadidstr+"&boardid="+boardidstr+"&page="+pagestr+"&formaction=removetrustlisttrust&identityid="+identityidstr+"#"+messageidstr+"\"><img src=\"images/circleminus.png\" border=\"0\" style=\"vertical-align:bottom;\"></a>";
			}

		}

		postvars["THREADPOSTBODY"]=FixBody(body);

		postattachments="";
		fileattachmentst.Bind(0,messageid);
		fileattachmentst.Step();
		while(fileattachmentst.RowReturned())
		{
			std::string thisattachment("");
			std::map<std::string,std::string> attachmentvars;
			std::string key("");
			std::string sizestr("");
			std::string keyname("");
			std::string::size_type slashpos=std::string::npos;
				
			fileattachmentst.ResultText(0,key);
			fileattachmentst.ResultText(1,sizestr);

			keyname=key;
			slashpos=keyname.find("/");
			if(slashpos!=std::string::npos && slashpos<keyname.size())
			{
				keyname=keyname.substr(slashpos+1);
			}

			attachmentvars["THREADPOSTATTACHMENTLINK"]="<a href=\"http://[FCPHOST]:[FPROXYPORT]/"+StringFunctions::UriEncode(key)+"\"><img src=\"images/attach.png\" border=\"0\" style=\"vertical-align:bottom;\">"+SanitizeOutput(keyname)+"</a>";
			attachmentvars["THREADPOSTATTACHMENTSIZE"]=sizestr+" bytes";

			m_templatehandler.PerformReplacements(threadpostattachment,attachmentvars,thisattachment);
			postattachments+=thisattachment;
			fileattachmentst.Step();
		}
		if(postattachments!="")
		{
			postattachments="<div class=\"postattachments\">"+postattachments+"</div>";
		}
		postvars["THREADPOSTATTACHMENTS"]=postattachments;

		if(identityidstr!="")
		{
			postvars["THREADPOSTAUTHORNAME"]="<a href=\"peerdetails.htm?identityid="+identityidstr+"\">"+FixAuthorName(fromname)+"</a>";
		}
		else
		{
			postvars["THREADPOSTAUTHORNAME"]=FixAuthorName(fromname);
		}
		postvars["THREADPOSTTITLE"]=SanitizeOutput(subject);
		postvars["THREADPOSTDATE"]=datetime;
		postvars["THREADPOSTREPLYLINK"]="<a href=\"forumcreatepost.htm?viewstate="+m_viewstate.GetViewStateID()+"&replytomessageid="+messageidstr+"&threadid="+threadidstr+"&boardid="+boardidstr+"&page="+pagestr+"\"><img src=\"images/mail_reply.png\" border=\"0\" style=\"vertical-align:bottom;\">"+m_trans->Get("web.page.forumviewthread.reply")+"</a>";

		postcount++;

		if(postcount%2==1)
		{
			m_templatehandler.PerformReplacements(threadpostrowodd,postvars,postrow);
		}
		else
		{
			m_templatehandler.PerformReplacements(threadpostroweven,postvars,postrow);
		}
		postrows+=postrow;

		st.Step();
	}

	vars["THREADPOSTROWS"]=postrows;

	CreateBreadcrumbLinks(breadcrumblinks,result);
	vars["LOCATIONBREADCRUMBS"]=result;

	m_templatehandler.GetSection("FORUMVIEWTHREADCONTENT",maincontent);
	m_templatehandler.PerformReplacements(maincontent,vars,result);

	return result;
}

const std::string ForumTemplateViewThreadPage::FixBody(const std::string &body)
{
	std::string output=body;

	output=StringFunctions::Replace(output,"\r\n","\n");
	output=StringFunctions::Replace(output,"&","&amp;");
	output=StringFunctions::Replace(output,"<","&lt;");
	output=StringFunctions::Replace(output,">","&gt;");

	output=QuoterHTMLRenderer::Render(output);

	if(m_detectlinks==true)
	{
		output=KeyFinderHTMLRenderer::Render(output,"[FCPHOST]","[FPROXYPORT]");
	}
	if(m_showsmilies==true)
	{
		output=m_emot.Replace(output);
	}

	output=StringFunctions::Replace(output,"\n","<br />");

	return output;
}
