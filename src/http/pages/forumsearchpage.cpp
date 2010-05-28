#include "../../../include/http/pages/forumsearchpage.h"
#include "../../../include/stringfunctions.h"

#include <Poco/DateTime.h>
#include <Poco/DateTimeParser.h>
#include <Poco/DateTimeFormatter.h>

const std::string ForumSearchPage::CreateForumDropDown(const std::string &name, const int selectedforumid) const
{
	std::string result("<select name=\""+name+"\">");
	result+="<option value=\"\"></option>";
	SQLite3DB::Statement st=m_db->Prepare("SELECT BoardID, BoardName FROM tblBoard WHERE Forum='true' ORDER BY BoardName COLLATE NOCASE;");
	
	st.Step();
	while(st.RowReturned())
	{
		int boardid=-1;
		std::string boardidstr("");
		std::string boardname("");

		st.ResultInt(0,boardid);
		st.ResultText(0,boardidstr);
		st.ResultText(1,boardname);

		result+="<option value=\""+boardidstr+"\"";
		if(boardid==selectedforumid)
		{
			result+=" selected";
		}
		result+=">"+SanitizeOutput(boardname)+"</option>";

		st.Step();
	}

	result+="</select>";

	return result;
}

const std::string ForumSearchPage::CreateSortByDropDown(const std::string &name, const std::string &selecteditem) const
{
	std::string result("<select name=\""+name+"\">");

	result+="<option value=\"date\"";
	if(selecteditem=="date")
	{
		result+=" selected";
	}
	result+=">"+m_trans->Get("web.page.forumsearch.sortby.date")+"</option>";

	result+="<option value=\"board\"";
	if(selecteditem=="board")
	{
		result+=" selected";
	}
	result+=">"+m_trans->Get("web.page.forumsearch.sortby.board")+"</option>";

	result+="<option value=\"subject\"";
	if(selecteditem=="subject")
	{
		result+=" selected";
	}
	result+=">"+m_trans->Get("web.page.forumsearch.sortby.subject")+"</option>";


	result+="<option value=\"author\"";
	if(selecteditem=="author")
	{
		result+=" selected";
	}
	result+=">"+m_trans->Get("web.page.forumsearch.sortby.author")+"</option>";

	result+="</select>";
	return result;
}

const std::string ForumSearchPage::CreateSortOrderDropDown(const std::string &name, const std::string &selecteditem) const
{
	std::string result("<select name=\""+name+"\">");

	result+="<option value=\"ASC\"";
	if(selecteditem=="ASC")
	{
		result+=" selected";
	}
	result+=">"+m_trans->Get("web.page.forumsearch.sortorder.ascending")+"</option>";

	result+="<option value=\"DESC\"";
	if(selecteditem=="DESC")
	{
		result+=" selected";
	}
	result+=">"+m_trans->Get("web.page.forumsearch.sortorder.descending")+"</option>";

	result+="</select>";
	return result;
}

const std::string ForumSearchPage::CreateSQLCriteriaClause(const std::vector<searchitem> &searchitems, const std::string &fieldname) const
{
	std::string sql("");
	int lastgroup=0;
	bool firstgroupitem=false;
	
	for(std::vector<searchitem>::const_iterator i=searchitems.begin(); i!=searchitems.end(); i++)
	{
		if(lastgroup!=(*i).m_group)
		{
			if((*i).m_group>1)
			{
				sql+=") OR ";
			}
			sql+="(";
			lastgroup=(*i).m_group;
			firstgroupitem=true;
		}

		if(firstgroupitem==true)
		{
			firstgroupitem=false;
		}
		else
		{
			sql+=" AND ";
		}
		
		sql+=fieldname+" ";
		if((*i).m_include==false)
		{
			sql+="NOT ";
		}
		sql+="LIKE '%' || ? || '%'";
	}

	if(searchitems.size()>0)
	{
		sql+=")";
	}

	return sql;

}

const std::string ForumSearchPage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
{
	struct searchfields
	{
		searchfields():m_querystring(""),m_subject(""),m_author(""),m_boardid(-1),m_threadid(-1),m_hasstartdate(false),m_hasenddate(false),m_sortby("Date"),m_sortorder("DESC")	{}
		std::string m_querystring;
		std::string m_subject;
		std::string m_author;
		int m_boardid;
		int m_threadid;
		bool m_hasstartdate;
		Poco::DateTime m_startdate;
		bool m_hasenddate;
		Poco::DateTime m_enddate;
		std::string m_sortby;
		std::string m_sortorder;
	};

	std::string result("");
	std::string searchresultrows("");
	std::vector<std::pair<std::string,std::string> > breadcrumblinks;
	std::map<std::string,std::string> vars;
	searchfields sf;

	breadcrumblinks.push_back(std::pair<std::string,std::string>("forummain.htm?viewstate="+m_viewstate.GetViewStateID(),SanitizeOutput(m_trans->Get("web.navlink.browseforums"))));
	breadcrumblinks.push_back(std::pair<std::string,std::string>("forumsearch.htm?viewstate="+m_viewstate.GetViewStateID(),SanitizeOutput(m_trans->Get("web.navlink.search"))));
	CreateBreadcrumbLinks(breadcrumblinks,result);
	vars["LOCATIONBREADCRUMBS"]=result;

	if(queryvars.find("formaction")!=queryvars.end() && (*queryvars.find("formaction")).second=="search" && ValidateFormPassword(queryvars)==true)
	{
		std::string sqlstring("");
		int lastgroup=0;
		bool firstgroupitem=true;
		std::vector<searchitem> searchitems;
		bool startedcriteria=false;

		if(queryvars.find("searchquery")!=queryvars.end())
		{
			sf.m_querystring=(*queryvars.find("searchquery")).second.GetData();
		}
		if(queryvars.find("subject")!=queryvars.end())
		{
			sf.m_subject=(*queryvars.find("subject")).second.GetData();
		}
		if(queryvars.find("author")!=queryvars.end())
		{
			sf.m_author=(*queryvars.find("author")).second.GetData();
		}
		if(queryvars.find("boardid")!=queryvars.end())
		{
			std::string temp((*queryvars.find("boardid")).second.GetData());
			StringFunctions::Convert(temp,sf.m_boardid);
		}
		if(queryvars.find("threadid")!=queryvars.end())
		{
			std::string temp((*queryvars.find("threadid")).second.GetData());
			StringFunctions::Convert(temp,sf.m_threadid);
		}
		if(queryvars.find("startdate")!=queryvars.end())
		{
			int tz=0;
			std::string temp((*queryvars.find("startdate")).second.GetData());
			if(Poco::DateTimeParser::tryParse(temp,sf.m_startdate,tz)==true)
			{
				sf.m_hasstartdate=true;
			}
		}
		if(queryvars.find("enddate")!=queryvars.end())
		{
			int tz=0;
			std::string temp((*queryvars.find("enddate")).second.GetData());
			if(Poco::DateTimeParser::tryParse(temp,sf.m_enddate,tz)==true)
			{
				sf.m_hasenddate=true;
			}
		}
		if(queryvars.find("sortby")!=queryvars.end())
		{
			sf.m_sortby=((*queryvars.find("sortby")).second.GetData());
		}
		else
		{
			sf.m_sortby="date";
		}
		if(queryvars.find("sortorder")!=queryvars.end())
		{
			sf.m_sortorder=(*queryvars.find("sortorder")).second.GetData();
		}
		else
		{
			sf.m_sortorder="DESC";
		}

		sqlstring="SELECT tblMessage.MessageID, tblThread.ThreadID, tblBoard.BoardID, tblMessage.FromName, MessageDate || ' ' || MessageTime, Subject, tblBoard.BoardName, Body, tblMessage.IdentityID FROM tblMessage INNER JOIN tblThreadPost ON tblMessage.MessageID=tblThreadPost.MessageID INNER JOIN tblThread ON tblThreadPost.ThreadID=tblThread.ThreadID INNER JOIN tblBoard ON tblThread.BoardID=tblBoard.BoardID WHERE ";

		if(sf.m_subject!="")
		{
			if(startedcriteria==true)
			{
				sqlstring+="AND ";
			}
			sqlstring+="tblMessage.Subject LIKE '%' || ? || '%' ";
			startedcriteria=true;
		}

		if(sf.m_author!="")
		{
			if(startedcriteria==true)
			{
				sqlstring+="AND ";
			}
			sqlstring+="tblMessage.FromName LIKE '%' || ? || '%' ";
			startedcriteria=true;
		}

		if(sf.m_boardid>0)
		{
			if(startedcriteria==true)
			{
				sqlstring+="AND ";
			}
			sqlstring+="tblThread.BoardID=? ";
			startedcriteria=true;
		}

		if(sf.m_threadid>0)
		{
			if(startedcriteria==true)
			{
				sqlstring+="AND ";
			}
			sqlstring+="tblThread.ThreadID=? ";
			startedcriteria=true;
		}

		if(sf.m_hasstartdate==true)
		{
			if(startedcriteria==true)
			{
				sqlstring+="AND ";
			}
			sqlstring+="tblMessage.MessageDate>=? ";
			startedcriteria=true;
		}

		if(sf.m_hasenddate==true)
		{
			if(startedcriteria==true)
			{
				sqlstring+="AND ";
			}
			sqlstring+="tblMessage.MessageDate<=? ";
			startedcriteria=true;
		}

		if(sf.m_querystring!="")
		{
			if(startedcriteria==true)
			{
				sqlstring+="AND ";
			}
			SeparateSearchItems(sf.m_querystring,searchitems);
			sqlstring+="("+CreateSQLCriteriaClause(searchitems,"Body")+") ";
			startedcriteria=true;
		}

		if(sf.m_sortby=="author")
		{
			sqlstring+="ORDER BY tblMessage.FromName COLLATE NOCASE ";
		}
		else if(sf.m_sortby=="board")
		{
			sqlstring+="ORDER BY tblBoard.BoardName COLLATE NOCASE ";
		}
		else if(sf.m_sortby=="subject")
		{
			sqlstring+="ORDER BY tblMessage.Subject COLLATE NOCASE ";
		}
		else	// date
		{
			sqlstring+="ORDER BY tblMessage.MessageDate || tblMessage.MessageTime ";
		}

		if(sf.m_sortorder=="ASC")
		{
			sqlstring+="ASC";
		}
		else
		{
			sqlstring+="DESC";
		}

		sqlstring+=" LIMIT 0,51";

		int currentparam=0;
		SQLite3DB::Statement st=m_db->Prepare(sqlstring);

		//subject,author,boardid,threadid,startdate,enddate,querystring
		if(sf.m_subject!="")
		{
			st.Bind(currentparam++,sf.m_subject);
		}
		if(sf.m_author!="")
		{
			st.Bind(currentparam++,sf.m_author);
		}
		if(sf.m_boardid>0)
		{
			st.Bind(currentparam++,sf.m_boardid);
		}
		if(sf.m_threadid>0)
		{
			st.Bind(currentparam++,sf.m_threadid);
		}
		if(sf.m_hasstartdate==true)
		{
			st.Bind(currentparam++,Poco::DateTimeFormatter::format(sf.m_startdate,"%Y-%m-%d"));
		}
		if(sf.m_hasenddate==true)
		{
			st.Bind(currentparam++,Poco::DateTimeFormatter::format(sf.m_enddate,"%Y-%m-%d"));
		}
		for(std::vector<searchitem>::iterator i=searchitems.begin(); i!=searchitems.end(); i++)
		{
			st.Bind(currentparam++,(*i).m_phrase);
		}

		st.Step();

		if(st.RowReturned())
		{

		}
		else
		{
			
		}

		int currentrow=0;
		std::string evenrowtemplate("");
		std::string oddrowtemplate("");
		m_templatehandler.GetSection("SEARCHRESULTEVEN",evenrowtemplate);
		m_templatehandler.GetSection("SEARCHRESULTODD",oddrowtemplate);

		while(currentrow<50 && st.RowReturned())
		{
			int messageid=0;
			std::string messageidstr("");
			int threadid=0;
			std::string threadidstr("");
			int boardid=0;
			std::string boardidstr("");
			std::string author("");
			std::string subject("");
			std::string board("");
			std::string datetime("");
			std::string temp("");
			std::string identityidstr("");
			std::map<std::string,std::string> rowvars;

			st.ResultInt(0,messageid);
			st.ResultText(0,messageidstr);
			st.ResultInt(1,threadid);
			st.ResultText(1,threadidstr);
			st.ResultInt(2,boardid);
			st.ResultText(2,boardidstr);
			st.ResultText(3,author);
			st.ResultText(4,datetime);
			st.ResultText(5,subject);
			st.ResultText(6,board);
			st.ResultText(8,identityidstr);

			if(identityidstr!="")
			{
				rowvars["AUTHOR"]="<a href=\"peerdetails.htm?identityid="+identityidstr+"\">"+FixAuthorName(author)+"</a>";
			}
			else
			{
				rowvars["AUTHOR"]=FixAuthorName(author);
			}
			if(boardidstr!="" && threadidstr!="" && messageidstr!="")
			{
				rowvars["SUBJECT"]="<a href=\"forumviewthread.htm?viewstate="+m_viewstate.GetViewStateID()+"&boardid="+boardidstr+"&threadid="+threadidstr+"#"+messageidstr+"\">"+FixSubject(subject)+"</a>";
			}
			else
			{
				rowvars["SUBJECT"]=FixSubject(subject);
			}
			rowvars["DATE"]=SanitizeOutput(datetime);
			if(boardidstr!="")
			{
				rowvars["BOARD"]="<a href=\"forumthreads.htm?viewstate="+m_viewstate.GetViewStateID()+"&boardid="+boardidstr+"\">"+SanitizeOutput(board)+"</a>";
			}
			else
			{
				rowvars["BOARD"]=SanitizeOutput(board);
			}

			if(currentrow%2==0)
			{
				m_templatehandler.PerformReplacements(evenrowtemplate,rowvars,temp);
			}
			else
			{
				m_templatehandler.PerformReplacements(oddrowtemplate,rowvars,temp);
			}

			searchresultrows+=temp;

			st.Step();
			currentrow++;
		}

		if(st.RowReturned())
		{
			std::string temp("");
			m_templatehandler.GetSection("SEARCHRESULTLIMITED",temp);
			searchresultrows+=temp;
		}

	}

	std::string searchpagetemplate("");
	std::vector<std::string> ignored;
	ignored.push_back("FORUMSEARCHRESULTS");
	m_templatehandler.GetSection("FORUMSEARCHCONTENT",searchpagetemplate,ignored);

	vars["FORUMSEARCHEXTRAFIELDS"]=CreateFormPassword();
	vars["FORUMSEARCHQUERY"]="<input type=\"text\" name=\"searchquery\" value=\""+SanitizeTextAreaOutput(sf.m_querystring)+"\" style=\"width:250px;\">";
	vars["FORUMSEARCHSUBJECT"]="<input type=\"text\" name=\"subject\" value=\""+SanitizeTextAreaOutput(sf.m_subject)+"\">";
	vars["FORUMSEARCHBOARDID"]=CreateForumDropDown("boardid",sf.m_boardid);
	vars["FORUMSEARCHAUTHOR"]="<input type=\"text\" name=\"author\" value=\""+SanitizeTextAreaOutput(sf.m_author)+"\">";
	std::string tempdatestr("");
	if(sf.m_hasstartdate==true)
	{
		tempdatestr=Poco::DateTimeFormatter::format(sf.m_startdate,"%Y-%m-%d");
	}
	vars["FORUMSEARCHSTARTDATE"]="<input type=\"text\" name=\"startdate\" value=\""+tempdatestr+"\" size=\"10\">";
	tempdatestr="";
	if(sf.m_hasenddate==true)
	{
		tempdatestr=Poco::DateTimeFormatter::format(sf.m_enddate,"%Y-%m-%d");
	}
	vars["FORUMSEARCHENDDATE"]="<input type=\"text\" name=\"enddate\" value=\""+tempdatestr+"\" size=\"10\">";

	vars["FORUMSEARCHSORTBY"]=CreateSortByDropDown("sortby",sf.m_sortby);
	vars["FORUMSEARCHSORTORDER"]=CreateSortOrderDropDown("sortorder",sf.m_sortorder);

	if(searchresultrows!="")
	{
		vars["SEARCHRESULTROWS"]=searchresultrows;
	}
	else
	{
		vars["FORUMSEARCHRESULTS"]="";
	}

	std::string searchresultpage("");
	m_templatehandler.PerformReplacements(searchpagetemplate,vars,searchresultpage);

	return searchresultpage;
}

void ForumSearchPage::SeparateSearchItems(const std::string &querystring, std::vector<searchitem> &searchitems)
{
	std::vector<std::string> searchwords;
	int currentgroup=1;
	bool include=true;
	bool inphrase=false;
	std::string currentphrase("");

	StringFunctions::Split(querystring," ",searchwords);

	for(std::vector<std::string>::iterator i=searchwords.begin(); i!=searchwords.end(); i++)
	{
		std::string currentword((*i));
		if((currentword=="or" || currentword=="OR") && inphrase==false)
		{
			currentgroup++;
			continue;
		}
		if((currentword=="and" || currentword=="AND") && inphrase==false)
		{
			include=true;
			continue;
		}
		if((currentword=="not" || currentword=="NOT") && inphrase==false)
		{
			include=false;
			continue;
		}
		if(currentword.size()>0 && currentword[0]=='-' && inphrase==false)
		{
			currentword.erase(0,1);
			include=false;
		}
		else if(currentword.size()>0 && currentword[0]=='+' && inphrase==false)
		{
			currentword.erase(0,1);
			include=true;
		}
		if(currentword.size()>0 && currentword[0]=='"' && inphrase==false)
		{
			currentword.erase(0,1);
			inphrase=true;
		}
		if(currentword.size()>0 && currentword[currentword.size()-1]=='"' && inphrase==true)
		{
			currentword.erase(currentword.size()-1,1);
			inphrase=false;
		}

		if(currentphrase!="")
		{
			currentphrase+=" "+currentword;
		}
		else
		{
			currentphrase=currentword;
		}

		if(inphrase==false || i==(searchwords.end()-1))
		{
			searchitem thisitem;
			thisitem.m_phrase=currentphrase;
			thisitem.m_group=currentgroup;
			thisitem.m_include=include;

			searchitems.push_back(thisitem);

			currentphrase="";
			include=true;
		}

	}

}
