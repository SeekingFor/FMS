#include "../../include/http/ipagehandler.h"
#include "../../include/stringfunctions.h"
#include "../../include/http/multipartparser.h"

#include <Poco/Net/HTMLForm.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/UUID.h>
#include <Poco/DateTime.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/Timespan.h>

#include <cstring>

#ifdef XMEM
	#include <xmem.h>
#endif

void IPageHandler::CreateArgArray(const std::map<std::string,std::string> &vars, const std::string &basename, std::vector<std::string> &args)
{
	for(std::map<std::string,std::string>::const_iterator i=vars.begin(); i!=vars.end(); i++)
	{
		if((*i).first.find(basename)==0 && (*i).first.find("[")!=std::string::npos && (*i).first.find("]")!=std::string::npos)
		{
			int index=0;
			std::string indexstr;
			std::string::size_type startpos;
			std::string::size_type endpos;
			startpos=(*i).first.find("[");
			endpos=(*i).first.find("]");

			indexstr=(*i).first.substr(startpos+1,(endpos-startpos)-1);
			StringFunctions::Convert(indexstr,index);

			while(args.size()<index+1)
			{
				args.push_back("");
			}
			args[index]=(*i).second;
		}
	}
}

const std::string IPageHandler::CreateFormPassword()
{
	Poco::DateTime date;
	Poco::UUIDGenerator uuidgen;
	Poco::UUID uuid;
	try
	{
		uuid=uuidgen.createRandom();
	}
	catch(...)
	{
	}

	SQLite3DB::Statement st=m_db->Prepare("INSERT INTO tmpFormPassword(Date,Password) VALUES(?,?);");
	st.Bind(0,Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S"));
	st.Bind(1,uuid.toString());
	st.Step();

	return "<input type=\"hidden\" name=\"formpassword\" value=\""+uuid.toString()+"\">";

}

const std::string IPageHandler::CreateTrueFalseDropDown(const std::string &name, const std::string &selected)
{
	std::string rval="";

	rval+="<select name=\""+name+"\">";
	rval+="<option value=\"true\"";
	if(selected=="true")
	{
		rval+=" SELECTED";
	}
	rval+=">"+m_trans->Get("web.option.true")+"</option>";
	rval+="<option value=\"false\"";
	if(selected=="false")
	{
		rval+=" SELECTED";
	}
	rval+=">"+m_trans->Get("web.option.false")+"</option>";
	rval+="</select>";

	return rval;
}

void IPageHandler::CreateQueryVarMap(Poco::Net::HTTPServerRequest &request, std::map<std::string,std::string> &vars)
{
	for(Poco::Net::HTTPServerRequest::ConstIterator i=request.begin(); i!=request.end(); i++)
	{
		vars[(*i).first]=(*i).second;
	}

	// handle HTMLForm and multiparts
	MultiPartParser mpp;
	Poco::Net::HTMLForm form(request,request.stream(),mpp);
	for(Poco::Net::HTMLForm::ConstIterator i=form.begin(); i!=form.end(); i++)
	{
		vars[(*i).first]=(*i).second;
	}

	// for a POST method, the HTMLForm won't grab vars off the query string so we
	// temporarily set the method to GET and parse with the HTMLForm again
	if(request.getMethod()=="POST")
	{
		request.setMethod("GET");
		Poco::Net::HTMLForm form1(request,request.stream(),mpp);
		for(Poco::Net::HTMLForm::ConstIterator i=form1.begin(); i!=form1.end(); i++)
		{
			vars[(*i).first]=(*i).second;
		}
		request.setMethod("POST");
	}

	// get any multiparts
	std::map<std::string,std::string> mpvars=mpp.GetVars();
	for(std::map<std::string,std::string>::iterator i=mpvars.begin(); i!=mpvars.end(); i++)
	{
		vars[(*i).first]=(*i).second;
	}

}

const std::string IPageHandler::GenerateNavigationLinks()
{
	std::string links="<div class=\"box\">\n";
	links+="	<div class=\"header\">"+m_trans->Get("web.navlink.links")+"</div>\n";
	links+="	<ul>\n";
	links+="		<li><a href=\"index.htm\">"+m_trans->Get("web.navlink.home")+"</a></li>\n";
	links+="		<li><a href=\"options.htm\">"+m_trans->Get("web.navlink.options")+"</a></li>\n";
	links+="		<li><a href=\"createidentity.htm\">"+m_trans->Get("web.navlink.createidentity")+"</a></li>\n";
	links+="		<li><a href=\"localidentities.htm\">"+m_trans->Get("web.navlink.localidentities")+"</a></li>\n";
	links+="		<li><a href=\"announceidentity.htm\">"+m_trans->Get("web.navlink.announceidentity")+"</a></li>\n";
	links+="		<li><a href=\"addpeer.htm\">"+m_trans->Get("web.navlink.addpeer")+"</a></li>\n";
	links+="		<li><a href=\"peermaintenance.htm\">"+m_trans->Get("web.navlink.peermaintenance")+"</a></li>\n";
	links+="		<li><a href=\"peertrust.htm\">"+m_trans->Get("web.navlink.peertrust")+"</a></li>\n";
	links+="		<li><a href=\"boards.htm\">"+m_trans->Get("web.navlink.boardmaintenance")+"</a></li>\n";
	links+="		<li><a href=\"controlboard.htm\">"+m_trans->Get("web.navlink.controlboards")+"</a></li>\n";
	links+="		<li><a href=\"insertedfiles.htm\">"+m_trans->Get("web.navlink.insertedfiles")+"</a></li>\n";
	links+="		<li><a href=\"forummain.htm\">"+m_trans->Get("web.navlink.browseforums")+"</a></li>\n";
	links+="	</ul>\n";
	links+="</div>\n";

	return links;

}

const std::string IPageHandler::GeneratePage(const std::string &method, const std::map<std::string,std::string> &queryvars)
{
	return StringFunctions::Replace(StringFunctions::Replace(m_template,"[NAVLINKS]",GenerateNavigationLinks()),"[CONTENT]",GenerateContent(method,queryvars));
}

void IPageHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
	m_log->trace("IPageHandler::handleRequest from "+request.clientAddress().toString());

	std::map<std::string,std::string> vars;

	CreateQueryVarMap(request,vars);

	if(request.getVersion()==Poco::Net::HTTPRequest::HTTP_1_1)
	{
		response.setChunkedTransferEncoding(true);
	}
	response.setContentType("text/html");

	std::ostream &ostr = response.send();
	ostr << GeneratePage(request.getMethod(),vars);

}

const std::string IPageHandler::SanitizeOutput(const std::string &input)
{
	// must do & first because all other elements have & in them!
	std::string output=StringFunctions::Replace(input,"&","&amp;");
	output=StringFunctions::Replace(output,"<","&lt;");
	output=StringFunctions::Replace(output,">","&gt;");
	output=StringFunctions::Replace(output,"\"","&quot;");
	output=StringFunctions::Replace(output," ","&nbsp;");
	return output;
}

const std::string IPageHandler::SanitizeTextAreaOutput(const std::string &input)
{
	// must do & first because all other elements have & in them!
	std::string output=StringFunctions::Replace(input,"&","&amp;");
	output=StringFunctions::Replace(output,"<","&lt;");
	output=StringFunctions::Replace(output,">","&gt;");
	output=StringFunctions::Replace(output,"\"","&quot;");
	return output;
}

const bool IPageHandler::ValidateFormPassword(const std::map<std::string,std::string> &vars)
{
	Poco::DateTime date;
	date-=Poco::Timespan(0,1,0,0,0);

	SQLite3DB::Statement st=m_db->Prepare("DELETE FROM tmpFormPassword WHERE Date<?;");
	st.Bind(0,Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S"));
	st.Step();

	std::map<std::string,std::string>::const_iterator i=vars.find("formpassword");
	if(i!=vars.end())
	{
		st=m_db->Prepare("SELECT COUNT(*) FROM tmpFormPassword WHERE Password=?;");
		st.Bind(0,(*i).second);
		st.Step();
		if(st.RowReturned())
		{
			if(st.ResultNull(0)==false)
			{
				int rval=0;
				st.ResultInt(0,rval);
				if(rval>0)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

const bool IPageHandler::WillHandleURI(const std::string &uri)
{
	if(uri.find(m_pagename)!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
