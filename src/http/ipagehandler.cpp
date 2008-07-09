#include "../../include/http/ipagehandler.h"
#include "../../include/stringfunctions.h"
#include "../../include/http/multipartparser.h"

#include <Poco/Net/HTMLForm.h>

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

const std::string IPageHandler::CreateTrueFalseDropDown(const std::string &name, const std::string &selected)
{
	std::string rval="";

	rval+="<select name=\""+name+"\">";
	rval+="<option value=\"true\"";
	if(selected=="true")
	{
		rval+=" SELECTED";
	}
	rval+=">true</option>";
	rval+="<option value=\"false\"";
	if(selected=="false")
	{
		rval+=" SELECTED";
	}
	rval+=">false</option>";
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

void IPageHandler::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
	m_log->trace("IPageHandler::handleRequest from "+request.clientAddress().toString());

	std::map<std::string,std::string> vars;

	CreateQueryVarMap(request,vars);

	response.setChunkedTransferEncoding(true);
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
