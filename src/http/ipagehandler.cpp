#include "../../include/http/ipagehandler.h"
#include "../../include/http/httpdefs.h"
#include "../../include/stringfunctions.h"

#ifdef XMEM
	#include <xmem.h>
#endif

void IPageHandler::CreateArgArray(const std::map<std::string,std::string> &vars, const std::string &basename, std::vector<std::string> &args)
{
	for(std::map<std::string,std::string>::const_iterator i=vars.begin(); i!=vars.end(); i++)
	{
		if((*i).first.find(basename)==0 && (*i).first.find("[")!=std::string::npos && (*i).first.find("]")!=std::string::npos)
		{
			int index;
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

const bool IPageHandler::Handle(shttpd_arg *arg)
{
	const char *uri=shttpd_get_env(arg,"REQUEST_URI");
	const char *method=shttpd_get_env(arg,"REQUEST_METHOD");
	std::string methodstr="";
	if(method)
	{
		methodstr=method;
	}

	if(uri && WillHandleURI(std::string(uri)))
	{
		httpstate *mystate=(httpstate *)arg->state;
		// this is a new request - create a new arg object
		if(arg->state==NULL)
		{
			arg->state=new httpstate;
			memset(arg->state,0,sizeof(httpstate));
			mystate=(httpstate *)arg->state;

			// if post then create input buffer
			if(methodstr=="POST")
			{
				const char *lenstr=shttpd_get_header(arg,"Content-Length");
				if(lenstr)
				{
					long len;
					StringFunctions::Convert(std::string(lenstr),len);
					mystate->m_indata=new char[len+1];
					mystate->m_indata[len]=NULL;
					mystate->m_indatalen=len;
					mystate->m_indatapos=0;
				}
			}
		}

		// we have more POST data to get
		if(arg->in.len>0)
		{
			int pos=0;
			while(pos<arg->in.len)
			{
				mystate->m_indata[mystate->m_indatapos++]=arg->in.buf[pos++];
			}
			arg->in.num_bytes=arg->in.len;
		}

		// we have all POST data (or it was 0 to begin with) - generate the page
		if(mystate->m_indatalen==mystate->m_indatapos && mystate->m_outdata==NULL)
		{
			//TODO parse POST data and any QUERY_STRING before generating page
			std::map<std::string,std::string> args;
			std::vector<std::string> argparts;
			
			if(mystate->m_indata)
			{
				StringFunctions::Split(mystate->m_indata,"&",argparts);
			}
			if(shttpd_get_env(arg,"QUERY_STRING"))
			{
				StringFunctions::Split(shttpd_get_env(arg,"QUERY_STRING"),"&",argparts);
			}
			for(std::vector<std::string>::iterator argi=argparts.begin(); argi!=argparts.end(); argi++)
			{
				std::vector<std::string> parts;
				StringFunctions::Split((*argi),"=",parts);
				if(parts.size()>0)
				{
					// replace + with space before UriDecoding
					parts[0]=StringFunctions::Replace(parts[0],"+"," ");
					args[StringFunctions::UriDecode(parts[0])];
					if(parts.size()>1)
					{
						// replace + with space before UriDecoding
						parts[1]=StringFunctions::Replace(parts[1],"+"," ");
						args[StringFunctions::UriDecode(parts[0])]=StringFunctions::UriDecode(parts[1]);
					}
				}
			}

			std::string page=GeneratePage(methodstr,args);
			mystate->m_outdata=new char[page.size()];
			memcpy(mystate->m_outdata,page.c_str(),page.size());
			mystate->m_outdatalen=page.size();
			mystate->m_outdatapos=0;
		}

		// if we have output data, push next block of data onto out buffer
		if(mystate->m_outdata && mystate->m_outdatapos<mystate->m_outdatalen)
		{
			int pos=0;
			while(mystate->m_outdatapos<mystate->m_outdatalen && pos<arg->out.len)
			{
				arg->out.buf[pos++]=mystate->m_outdata[mystate->m_outdatapos++];
			}
			arg->out.num_bytes=pos;
		}

		// if we have no more output data to send - delete the data pointers and set end of output flag
		if(mystate->m_outdata && mystate->m_outdatapos==mystate->m_outdatalen)
		{
			if(mystate->m_indata)
			{
				delete [] mystate->m_indata;
			}
			if(mystate->m_outdata)
			{
				delete [] mystate->m_outdata;
			}
			delete mystate;
			arg->state=NULL;

			arg->flags|=SHTTPD_END_OF_OUTPUT;
		}

		return true;
	}
	else
	{
		return false;
	}
}
