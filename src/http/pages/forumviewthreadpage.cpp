#include "../../../include/http/pages/forumviewthreadpage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/unicode/unicodeformatter.h"
#include "../../../include/option.h"

#ifdef XMEM
	#include <xmem.h>
#endif

ForumTemplateViewThreadPage::ForumTemplateViewThreadPage(SQLite3DB::DB *db, const HTMLTemplateHandler &templatehandler):ForumTemplatePage(db,templatehandler,"forumviewthread.htm"),m_emot("images/smilies/")
{
	m_localtrustoverrides=false;
	m_minlocalmessagetrust=0;
	m_minpeermessagetrust=0;

	Option option(db);
	option.GetBool("ForumDetectLinks",m_detectlinks);
	option.GetBool("ForumShowSmilies",m_showsmilies);
	option.GetBool("LocalTrustOverridesPeerTrust",m_localtrustoverrides);
	option.GetInt("MinLocalMessageTrust",m_minlocalmessagetrust);
	option.GetInt("MinPeerMessageTrust",m_minpeermessagetrust);
	m_htmlrenderer.SetDetectLinks(m_detectlinks);
	m_pagetitle=GetBasePageTitle();
}

const std::string ForumTemplateViewThreadPage::FixUUIDAnchor(const std::string &uuid)
{
	return StringFunctions::Replace(StringFunctions::Replace(uuid,"\"","_"),">","_");
}

const std::string ForumTemplateViewThreadPage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
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
	std::vector<std::string> skipspace;
	SQLite3DB::Statement fileattachmentst=m_db->Prepare("SELECT Key, Size FROM tblMessageFileAttachment WHERE MessageID=?;");
	SQLite3DB::Statement truststpeeronly=m_db->Prepare("SELECT PeerMessageTrust, PeerTrustListTrust FROM tblIdentity WHERE IdentityID=?;");
	SQLite3DB::Statement truststboth=m_db->Prepare("SELECT tblIdentityTrust.LocalMessageTrust, tblIdentity.PeerMessageTrust, tblIdentityTrust.LocalTrustListTrust, tblIdentity.PeerTrustListTrust FROM tblIdentity LEFT JOIN tblIdentityTrust ON tblIdentity.IdentityID=tblIdentityTrust.IdentityID WHERE tblIdentity.IdentityID=? AND tblIdentityTrust.LocalIdentityID=?;");

	skipspace.push_back(" ");

	if(queryvars.find("messageuuid")!=queryvars.end())
	{
		SQLite3DB::Statement st=m_db->Prepare("SELECT tblMessage.MessageID, tblThreadPost.ThreadID, tblThread.BoardID FROM tblMessage LEFT JOIN tblThreadPost ON tblMessage.MessageID=tblThreadPost.MessageID LEFT JOIN tblThread ON tblThreadPost.ThreadID=tblThread.ThreadID WHERE MessageUUID=?;");
		st.Bind(0,(*queryvars.find("messageuuid")).second.GetData());
		st.Step();
		if(st.RowReturned())
		{
			int messageid=0;
			int threadid=0;
			int boardid=0;

			st.ResultInt(2,boardid);
			m_viewstate.SetBoardID(boardid);

			if(st.ResultNull(1)==false)
			{
				st.ResultInt(1,threadid);
				m_viewstate.SetThreadID(threadid);
			}
			else
			{
				m_viewstate.SetThreadID(0);
			}
		}
	}
	if(queryvars.find("threadid")!=queryvars.end())
	{
		int temp=0;
		threadidstr=(*queryvars.find("threadid")).second.GetData();
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
		pagestr=(*queryvars.find("page")).second.GetData();
	}
	else
	{
		int temp=0;
		temp=m_viewstate.GetPage();
		StringFunctions::Convert(temp,pagestr);
	}
	if(queryvars.find("boardid")!=queryvars.end())
	{
		boardidstr=(*queryvars.find("boardid")).second.GetData();
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
	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second.GetData().find("trust")!=std::string::npos && queryvars.find("identityid")!=queryvars.end() && ValidateFormPassword(queryvars))
	{
		SQLite3DB::Statement currenttrustst=m_db->Prepare("SELECT IFNULL(LocalMessageTrust,-1), IFNULL(LocalTrustListTrust,-1) FROM tblIdentityTrust WHERE IdentityID=? AND LocalIdentityID=?;");
		SQLite3DB::Statement updatetrustst=m_db->Prepare("UPDATE tblIdentityTrust SET LocalMessageTrust=?, LocalTrustListTrust=? WHERE IdentityID=? AND LocalIdentityID=?;");

		currenttrustst.Bind(0,(*queryvars.find("identityid")).second.GetData());
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
			updatetrustst.Bind(2,(*queryvars.find("identityid")).second.GetData());
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
	SQLite3DB::Statement st=m_db->Prepare("SELECT tblMessage.MessageID, tblMessage.IdentityID, tblMessage.FromName, tblMessage.Subject, tblMessage.MessageDate || ' ' || tblMessage.MessageTime, tblMessage.Body, tblIdentity.PublicKey || (SELECT OptionValue FROM tblOption WHERE Option='MessageBase') || '|' || tblMessage.InsertDate || '|Message-' || tblMessage.MessageIndex, tblMessage.MessageUUID FROM tblMessage INNER JOIN tblThreadPost ON tblMessage.MessageID=tblThreadPost.MessageID LEFT JOIN tblIdentity ON tblMessage.IdentityID=tblIdentity.IdentityID WHERE tblThreadPost.ThreadID=? ORDER BY tblThreadPost.PostOrder;");
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
		std::string postlink="";
		std::string messageuuid="";
		bool allowreply=true;
		
		st.ResultInt(0,messageid);
		st.ResultText(0,messageidstr);
		st.ResultText(1,identityidstr);
		st.ResultText(2,fromname);
		st.ResultText(3,subject);
		st.ResultText(4,datetime);
		st.ResultText(5,body);
		st.ResultText(6,postlink);
		st.ResultText(7,messageuuid);

		if(postcount==0)
		{
			if(subject!="")
			{
				m_pagetitle+=" - "+SanitizeOutput(boardname,skipspace)+" - "+SanitizeOutput(subject,skipspace);
			}
			breadcrumblinks.push_back(std::pair<std::string,std::string>(m_pagename+"?viewstate="+m_viewstate.GetViewStateID()+"&threadid="+threadidstr+"&boardid="+boardidstr+"&page="+pagestr,SanitizeOutput(subject,skipspace)));
		}

		postvars["THREADPOSTANCHOR"]="<a name=\""+messageidstr+"\"></a><a name=\""+FixUUIDAnchor(messageuuid)+"\"></a>";

		if(identityidstr=="")
		{
			postvars["CONDITIONALTRUSTTABLE"]="";
		}
		else
		{
			int lmt=0;
			int pmt=0;
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
					truststpeeronly.ResultInt(0,pmt);
					if(peermessagetrust!="" && pmt<m_minpeermessagetrust)
					{
						allowreply=false;
					}

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
					truststboth.ResultInt(0,lmt);
					truststboth.ResultInt(1,pmt);
					if(localmessagetrust=="")
					{
						lmt=100;
					}
					if(peermessagetrust=="")
					{
						pmt=100;
					}
					if(((m_localtrustoverrides==false || localmessagetrust=="") && (pmt<m_minpeermessagetrust || lmt<m_minlocalmessagetrust)) || (m_localtrustoverrides==true && (lmt<m_minlocalmessagetrust)))
					{
						allowreply=false;
					}
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
			int size=0;
			std::string sizestr("");
			std::string keyname("");
			std::string::size_type slashpos=std::string::npos;
				
			fileattachmentst.ResultText(0,key);
			fileattachmentst.ResultInt(0,size);
			fileattachmentst.ResultText(1,sizestr);

			if(size<0)
			{
				sizestr="?";
			}

			keyname=key;
			slashpos=keyname.find("/");
			if(slashpos!=std::string::npos && slashpos<keyname.size())
			{
				keyname=keyname.substr(slashpos+1);
			}

			attachmentvars["THREADPOSTATTACHMENTLINK"]="<a href=\"[FPROXYPROTOCOL]://[FPROXYHOST]:[FPROXYPORT]/"+StringFunctions::UriEncode(key)+"\"><img src=\"images/attach.png\" border=\"0\" style=\"vertical-align:bottom;\"> "+SanitizeOutput(keyname)+"</a>";
			attachmentvars["THREADPOSTATTACHMENTSIZE"]=sizestr+" bytes";

			m_templatehandler.PerformReplacements(threadpostattachment,attachmentvars,thisattachment);
			postattachments+=thisattachment;
			fileattachmentst.Step();
		}
		fileattachmentst.Reset();
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
		postvars["THREADPOSTTITLE"]=SanitizeOutput(subject,skipspace);
		if(identityidstr!="" && postlink!="")
		{
			postvars["THREADPOSTLINK"]="<a href=\"[FPROXYPROTOCOL]://[FPROXYHOST]:[FPROXYPORT]/"+StringFunctions::UriEncode(postlink)+"?type=text/plain\"><img src=\"images/link.png\" border=\"0\" title=\""+m_trans->Get("web.page.forumviewthread.permalink")+"\"></a>";
		}
		else
		{
			postvars["THREADPOSTLINK"]="";
		}
		postvars["THREADPOSTLINK"]+="&nbsp;<a href=\""+m_pagename+"?messageuuid="+FixUUIDAnchor(messageuuid)+"#"+FixUUIDAnchor(messageuuid)+"\"><img src=\"images/link.png\" border=\"0\" title=\""+m_trans->Get("web.page.forumviewthread.shareablelink")+"\"></a>";

		postvars["THREADPOSTDATE"]=datetime;

		if(allowreply==true)
		{
			postvars["THREADPOSTREPLYLINK"]="<a href=\"forumcreatepost.htm?viewstate="+m_viewstate.GetViewStateID()+"&replytomessageid="+messageidstr+"&threadid="+threadidstr+"&boardid="+boardidstr+"&page="+pagestr+"\"><img src=\"images/mail_reply.png\" border=\"0\" style=\"vertical-align:bottom;\">"+m_trans->Get("web.page.forumviewthread.reply")+"</a>";
		}
		else
		{
			postvars["THREADPOSTREPLYLINK"]="";
		}

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

const std::string ForumTemplateViewThreadPage::GetPageTitle(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
{
	return m_pagetitle;
}

const std::string ForumTemplateViewThreadPage::FixBody(const std::string &body)
{
	std::string output=body;

	output=StringFunctions::Replace(output,"\r\n","\n");
	output=StringFunctions::Replace(output,"&","&amp;");
	output=StringFunctions::Replace(output,"<","&lt;");
	output=StringFunctions::Replace(output,">","&gt;");
	output=StringFunctions::Replace(output,"[","&#91;");
	output=StringFunctions::Replace(output,"]","&#93;");

	output=m_htmlrenderer.Render(output);

	/*
	if(m_detectlinks==true)
	{
		output=KeyFinderHTMLRenderer::Render(output,"[FPROXYPROTOCOL]","[FPROXYHOST]","[FPROXYPORT]");
	}
	*/
	if(m_showsmilies==true)
	{
		output=m_emot.Replace(output);
	}

	output=StringFunctions::Replace(output,"\n","<br />");

	return output;
}
