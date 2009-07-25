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

#endif	// _forumpage_
