#ifndef _forumpage_
#define _forumpage_

#include "../ipagehandler.h"

class ForumPage:public IPageHandler
{
public:
	ForumPage(SQLite3DB::DB *db, const std::string &templatestr, const std::string &pagename):IPageHandler(db,templatestr,pagename)	{}

	virtual IPageHandler *New()=0;	// returns a new instance of the object

protected:
	const std::string FixFromName(const std::string &fromname)
	{
		std::string tempname=fromname;
		if(tempname.size()>30)
		{
			tempname.erase(27);
			tempname+="...";
		}
		tempname=SanitizeOutput(tempname);
		return tempname;
	}

	const std::string FixSubject(const std::string &subject)
	{
		std::string tempsubject=subject;
		if(tempsubject.size()>30)
		{
			tempsubject.erase(27);
			tempsubject+="...";
		}
		tempsubject=SanitizeOutput(tempsubject);
		return tempsubject;
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

#endif	// _forumpage_
