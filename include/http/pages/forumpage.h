#ifndef _forumpage_
#define _forumpage_

#include "../ipagehandler.h"
#include "../../unicode/unicodestring.h"

class ForumPage:public IPageHandler
{
public:
	ForumPage(SQLite3DB::DB *db, const std::string &templatestr, const std::string &pagename):IPageHandler(db,templatestr,pagename)	{}

	virtual IPageHandler *New()=0;	// returns a new instance of the object

protected:
	const std::string FixFromName(const std::string &fromname)
	{
		UnicodeString tempname(fromname);
		if(tempname.CharacterCount()>30)
		{
			tempname.Trim(27);
			tempname+="...";
		}
		return SanitizeOutput(tempname.NarrowString());
	}

	const std::string FixSubject(const std::string &subject)
	{
		UnicodeString tempsubject(subject);
		if(tempsubject.CharacterCount()>30)
		{
			tempsubject.Trim(27);
			tempsubject+="...";
		}
		return SanitizeOutput(tempsubject.NarrowString());
	}

	const std::string CreateForumHeader()
	{
		std::string content="<table class=\"header\">\r\n";
		content+="<tr><td><a href=\"index.htm\">"+m_trans->Get("web.navlink.home")+"</a> | <a href=\"forummain.htm\">"+m_trans->Get("web.navlink.browseforums")+"</a></td></tr>\r\n";
		content+="</table>\r\n";
		return content;
	}

private:
	virtual const std::string GenerateContent(const std::string &method, const std::map<std::string,std::string> &queryvars)=0;

};

#include "../htmltemplatehandler.h"
#include "../forumviewstate.h"
#include "../../option.h"

class ForumTemplatePage:public IPageHandler
{
public:
	ForumTemplatePage(SQLite3DB::DB *db, const HTMLTemplateHandler &templatehandler, const std::string &pagename):IPageHandler(db,"",pagename),m_templatehandler(templatehandler),m_viewstate(db)	{}

private:
	virtual const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)=0;
	virtual const std::string GeneratePage(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
	{
		Option option(m_db);
		std::map<std::string,std::string> sections;
		std::string output("");

		if(queryvars.find("viewstate")!=queryvars.end() && (*queryvars.find("viewstate")).second!="")
		{
			if(m_viewstate.LoadViewState((*queryvars.find("viewstate")).second.GetData())==false)
			{
				m_viewstate.CreateViewState();
			}
		}
		else
		{
			m_viewstate.CreateViewState();
		}

		if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="login" && queryvars.find("localidentityid")!=queryvars.end() && (*queryvars.find("localidentityid")).second!="" && ValidateFormPassword(queryvars))
		{
			SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID FROM tblLocalIdentity WHERE LocalIdentityID=?;");
			st.Bind(0,(*queryvars.find("localidentityid")).second.GetData());
			st.Step();
			if(st.RowReturned())
			{
				int localidentityid=0;
				st.ResultInt(0,localidentityid);
				m_viewstate.SetLocalIdentityID(localidentityid);
			}
		}
		if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="logout" && ValidateFormPassword(queryvars))
		{
			m_viewstate.SetLocalIdentityID(0);
		}

		if(m_viewstate.GetLocalIdentityID()==0)
		{
			sections["FORUMLOGINHEADER"]="<form method=\"post\" action=\""+m_pagename+"\">";
			sections["FORUMLOGINHEADER"]+="<input type=\"hidden\" name=\"formaction\" value=\"login\">";
			sections["FORUMLOGINHEADER"]+=CreateFormPassword();
			sections["FORUMLOGINHEADER"]+="<input type=\"hidden\" name=\"viewstate\" value=\""+m_viewstate.GetViewStateID()+"\">";
			sections["FORUMLOGINHEADER"]+=CreateLocalIdentityDropDown("localidentityid","");
			sections["FORUMLOGINHEADER"]+="<input type=\"submit\" class=\"loginbutton\" value=\""+m_trans->Get("web.page.forum.signin")+"\">";
			sections["FORUMLOGINHEADER"]+="</form>";
		}
		else
		{
			std::string name("");
			std::string publickey("");
			SQLite3DB::Statement st=m_db->Prepare("SELECT Name, PublicKey FROM tblLocalIdentity WHERE LocalIdentityID=?;");
			st.Bind(0,m_viewstate.GetLocalIdentityID());
			st.Step();
			if(st.RowReturned())
			{
				st.ResultText(0,name);
				st.ResultText(1,publickey);
				name+=publickey.substr(3);
				name=FixAuthorName(name);
			}

			sections["FORUMLOGINHEADER"]="<form method=\"post\">";
			sections["FORUMLOGINHEADER"]+="<input type=\"hidden\" name=\"formaction\" value=\"logout\">";
			sections["FORUMLOGINHEADER"]+=CreateFormPassword();
			sections["FORUMLOGINHEADER"]+="<input type=\"hidden\" name=\"viewstate\" value=\""+m_viewstate.GetViewStateID()+"\">";
			sections["FORUMLOGINHEADER"]+=SanitizeOutput(name);
			sections["FORUMLOGINHEADER"]+="<input type=\"submit\" class=\"logoutbutton\" value=\""+m_trans->Get("web.page.forum.signout")+"\">";
			sections["FORUMLOGINHEADER"]+="</form>";			
		}

		m_templatehandler.GetSection("FORUMPAGE",output);


		sections["PAGETITLE"]="FMS Forum";
		sections["FORUMCONTENT"]=GenerateContent(method,queryvars);

		// these need to come after GenerateContent
		sections["FORUMSEARCH"]="";
		sections["FORUMSEARCHBOXACTION"]="forumsearch.htm";
		sections["FORUMSEARCHBOXEXTRAFIELDS"]="<input type=\"hidden\" name=\"viewstate\" value=\""+m_viewstate.GetViewStateID()+"\">"+CreateFormPassword()+CreateForumSearchBoxExtraFields();

		sections["FORMPASSWORD"]=CreateFormPassword();
		sections["VIEWSTATEQS"]="viewstate="+m_viewstate.GetViewStateID();
		option.Get("FCPHost",sections["FCPHOST"]);
		option.Get("FProxyPort",sections["FPROXYPORT"]);
		option.Get("FProxyProtocol",sections["FPROXYPROTOCOL"]);
		option.Get("FProxyHost",sections["FPROXYHOST"]);

		m_templatehandler.PerformReplacements(output,sections,output);
		m_templatehandler.PerformTranslations(output,*m_trans,output);

		return output;
	}

protected:
	void CreateBreadcrumbLinks(const std::vector<std::pair<std::string,std::string> > &links, std::string &result)
	{
		result="";
		std::string breadcrumb("");
		m_templatehandler.GetSection("LOCATIONBREADCRUMB",breadcrumb);
		for(std::vector<std::pair<std::string,std::string> >::const_iterator i=links.begin(); i!=links.end(); i++)
		{
			std::string temp("");
			std::map<std::string,std::string> vars;
			vars["BREADCRUMBLINK"]=(*i).first;
			vars["BREADCRUMBNAME"]=(*i).second;
			m_templatehandler.PerformReplacements(breadcrumb,vars,temp);
			result+=temp;
		}
	}

	const std::string FixAuthorName(const std::string &author)
	{
		UnicodeString tempname(author);
		if(tempname.CharacterCount()>30)
		{
			tempname.Trim(27);
			tempname+="...";
		}
		return SanitizeOutput(tempname.NarrowString());
	}

	const std::string FixSubject(const std::string &subject)
	{
		std::vector<std::string> skip;
		UnicodeString tempsubject(subject);

		skip.push_back(" ");

		if(tempsubject.CharacterCount()>30)
		{
			tempsubject.Trim(27);
			tempsubject+="...";
		}
		return SanitizeOutput(tempsubject.NarrowString());
	}

	const std::string CreateLocalIdentityDropDown(const std::string &name, const std::string &selectedid)
	{
		std::string result("<select name=\""+name+"\">");
		bool found=false;
		SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID, Name, PublicKey FROM tblLocalIdentity WHERE PublicKey IS NOT NULL AND PrivateKey IS NOT NULL AND Active='true' ORDER BY Name COLLATE NOCASE;");
		st.Step();
		while(st.RowReturned())
		{
			std::string idstr("");
			std::string name("");
			std::string publickey("");

			st.ResultText(0,idstr);
			st.ResultText(1,name);
			st.ResultText(2,publickey);

			if(publickey.size()>4)
			{
				name+=publickey.substr(3);
			}
			name=FixAuthorName(name);

			result+="<option value=\""+idstr+"\"";
			if(selectedid==idstr)
			{
				found=true;
				result+=" selected";
			}
			result+=">"+name+"</option>";

			st.Step();
		}
		if(found==false)
		{
			result+="<option value=\"\" selected></option>";
		}
		result+="</select>";

		return result;
	}

	virtual const std::string CreateForumSearchBoxExtraFields() const
	{
		return std::string("");
	}

	const HTMLTemplateHandler &m_templatehandler;
	ForumViewState m_viewstate;
};

#endif	// _forumpage_
