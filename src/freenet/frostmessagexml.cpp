#include "../../include/freenet/frostmessagexml.h"

const std::string FrostMessageXML::FrostMakeFileName(const std::string &input) const
{
	std::string output;
	std::string::size_type strpos;
	std::string invalidchars="/\\?*<>\":|#&";

	output=input;

	std::transform(input.begin(),input.end(),output.begin(),tolower);

	if(output.size()>0 && output[0]=='.')
	{
		output[0]='_';
	}

	strpos=output.find_first_of(invalidchars);
	while(strpos!=std::string::npos)
	{
		output.at(strpos)='_';
		strpos=output.find_first_of(invalidchars,strpos);
	}

	return output;
}

const std::string FrostMessageXML::GetSignableContentV2() const
{
	char separator='|';
	std::string temp;
	long i;

	temp=GetFrostDate()+separator+GetFrostTime()+separator;

	temp+=GetFrostBoard()+separator;
	temp+=GetFrostAuthor()+separator;
	temp+=GetFrostMessageID()+separator;
	if(GetFrostInReplyTo()!="")
	{
		temp+=GetFrostInReplyTo()+separator;
	}
	// recipient goes here - private messages ??

	// add id info even if it is -1
	{
		std::string tempval="";
		StringFunctions::Convert(GetFrostIDLinePos(),tempval);
		temp+=tempval+separator;
		StringFunctions::Convert(GetFrostIDLineLen(),tempval);
		temp+=tempval+separator;
	}


	temp+=GetFrostSubject()+separator+GetBody()+separator;

	std::vector<FrostMessageXML::frostboardattachment> boards=GetFrostBoardAttachments();
	for(i=0; i<boards.size(); i++)
	{
		temp+=FrostMakeFileName(boards[i].m_name)+separator;
		if(boards[i].m_publickey!="")
		{
			temp+=boards[i].m_publickey+separator;
		}
		if(boards[i].m_privatekey!="")
		{
			temp+=boards[i].m_privatekey+separator;
		}
	}

	std::vector<FrostMessageXML::frostfileattachment> files=GetFrostFileAttachments();
	for(i=0; i<files.size(); i++)
	{
		temp+=files[i].m_name+separator;
		temp+=files[i].m_key+separator;
	}

	return temp;
}

std::string FrostMessageXML::GetXML()
{
	return std::string("");
}

void FrostMessageXML::Initialize()
{
	MessageXML::Initialize();
	m_frostidlinepos=0;
	m_frostidlinelen=0;
	m_frostdate="";
	m_frosttime="";
	m_frostauthor="";
	m_frostsubject="";
	m_frostmessageid="";
	m_frostboard="";
	m_frostpublickey="";
	m_frostsignature="";
	m_frostsignaturev2="";
}

const bool FrostMessageXML::ParseXML(const std::string &xml)
{
	/*
		FrostMessage
			MessageId	- CDATA
			InReplyTo	- CDATA
			IdLinePos	- long
			IdLenLen	- long
			From		- CDATA
			Subject		- CDATA
			Date		- YYYY.MM.DD
			Time		- HH:MM:SSGMT
			Body		- CDATA
			Board		- CDATA
			pubKey		- CDATA
			Signature	- CDATA
			AttachmentList
				Attachment type="file"
					File
						name	- CDATA
						size	- long
						key		- key without filename
	*/

	bool parsed=false;
	Poco::XML::DOMParser dp;

	dp.setEntityResolver(0);

	Initialize();

	try
	{
		Poco::AutoPtr<Poco::XML::Document> doc=dp.parseString(FixCDATA(xml));
		Poco::XML::Element *root=XMLGetFirstChild(doc,"FrostMessage");
		Poco::XML::Element *txt=0;

		txt=XMLGetFirstChild(root,"Date");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_frostdate=txt->firstChild()->getNodeValue();
				m_date=SanitizeSingleString(m_frostdate);
				m_date=StringFunctions::Replace(m_date,".","-");
				if(m_date.size()<10)
				{
					std::vector<std::string> dateparts;
					StringFunctions::Split(m_date,"-",dateparts);
					if(dateparts.size()==3)
					{
						m_date=dateparts[0]+"-";
						if(dateparts[1].size()==1)
						{
							m_date+="0";
						}
						m_date+=dateparts[1]+"-";
						if(dateparts[2].size()==1)
						{
							m_date+="0";
						}
						m_date+=dateparts[2];
					}
				}
			}
		}
		txt=XMLGetFirstChild(root,"Time");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_frosttime=txt->firstChild()->getNodeValue();
				m_time=SanitizeSingleString(m_frosttime);
				m_time=StringFunctions::Replace(m_time,"GMT","");
			}
		}
		txt=XMLGetFirstChild(root,"From");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_frostauthor=txt->firstChild()->getNodeValue();
			}
		}
		txt=XMLGetFirstChild(root,"Subject");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_frostsubject=txt->firstChild()->getNodeValue();
				m_subject=SanitizeSingleString(m_frostsubject);
			}
		}
		txt=XMLGetFirstChild(root,"MessageId");
		if(txt)
		{
			if(txt->firstChild())
			{
				// we have to append an @frost (or anything really) to the message ID, otherwise someone could insert valid FMS UUIDs here
				m_frostmessageid=txt->firstChild()->getNodeValue();
				m_messageid=SanitizeSingleString(m_frostmessageid)+"@frost";
			}
		}
		txt=XMLGetFirstChild(root,"Board");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_frostboard=txt->firstChild()->getNodeValue();
				std::string boardname=SanitizeSingleString(m_frostboard);
				StringFunctions::LowerCase(boardname,boardname);
				if(boardname.size()>40)
				{
					boardname.erase(40);
				}
				m_replyboard=boardname;
				m_boards.push_back(boardname);
			}
		}
		txt=XMLGetFirstChild(root,"Body");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_body=txt->firstChild()->getNodeValue();
			}
		}
		txt=XMLGetFirstChild(root,"pubKey");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_frostpublickey=txt->firstChild()->getNodeValue();
			}
		}
		txt=XMLGetFirstChild(root,"Signature");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_frostsignature=txt->firstChild()->getNodeValue();
			}
		}
		txt=XMLGetFirstChild(root,"SignatureV2");
		if(txt)
		{
			if(txt->firstChild())
			{
				m_frostsignaturev2=txt->firstChild()->getNodeValue();
			}
		}
		txt=XMLGetFirstChild(root,"IdLinePos");
		if(txt)
		{
			if(txt->firstChild())
			{
				std::string temp=txt->firstChild()->getNodeValue();
				StringFunctions::Convert(temp,m_frostidlinepos);
			}
		}
		txt=XMLGetFirstChild(root,"IdLineLen");
		if(txt)
		{
			if(txt->firstChild())
			{
				std::string temp=txt->firstChild()->getNodeValue();
				StringFunctions::Convert(temp,m_frostidlinelen);
			}
		}
		Poco::XML::Element *inreplyto=XMLGetFirstChild(root,"InReplyTo");
		if(inreplyto)
		{
			if(inreplyto->firstChild())
			{
				int order=0;
				m_frostinreplyto=inreplyto->firstChild()->getNodeValue();
				std::vector<std::string> inreplytoids;
				StringFunctions::Split(m_frostinreplyto,",",inreplytoids);
				for(std::vector<std::string>::reverse_iterator i=inreplytoids.rbegin(); i!=inreplytoids.rend(); i++)
				{
					m_inreplyto[order++]=(*i)+"@frost";
				}
			}
		}
		Poco::XML::Element *attachments=XMLGetFirstChild(root,"AttachmentList");
		if(attachments)
		{
			Poco::XML::Element *file=XMLGetFirstChild(attachments,"Attachment");
			while(file)
			{
				if(file->getAttribute("type")=="file")
				{
					Poco::XML::Element *fileel=XMLGetFirstChild(file,"File");
					if(fileel)
					{
						Poco::XML::Element *keyel=XMLGetFirstChild(fileel,"key");
						Poco::XML::Element *sizeel=XMLGetFirstChild(fileel,"size");
						Poco::XML::Element *nameel=XMLGetFirstChild(fileel,"name");

						if(keyel && keyel->firstChild() && sizeel && sizeel->firstChild() && nameel && nameel->firstChild())
						{
							int size=-1;
							std::string key="";
							std::string name="";
							
							StringFunctions::Convert(sizeel->firstChild()->getNodeValue(),size);
							key=keyel->firstChild()->getNodeValue();
							name=nameel->firstChild()->getNodeValue();

							if(size!=-1 && key!="")
							{
								m_fileattachments.push_back(fileattachment(key,size));
							}

							m_frostfileattachments.push_back(frostfileattachment(key,size,name));
						}
					}
				}
				else if(file->getAttribute("type")=="board")
				{
					Poco::XML::Element *nameel=XMLGetFirstChild(file,"Name");
					Poco::XML::Element *descel=XMLGetFirstChild(file,"description");
					Poco::XML::Element *pubel=XMLGetFirstChild(file,"pubKey");
					Poco::XML::Element *privel=XMLGetFirstChild(file,"privKey");
					std::string name="";
					std::string desc="";
					std::string pubkey="";
					std::string privkey="";

					if(nameel && nameel->firstChild())
					{
						name=nameel->firstChild()->getNodeValue();
					}
					if(descel && descel->firstChild())
					{
						desc=descel->firstChild()->getNodeValue();
					}
					if(pubel && pubel->firstChild())
					{
						pubkey=pubel->firstChild()->getNodeValue();
					}
					if(privel && privel->firstChild())
					{
						privkey=privel->firstChild()->getNodeValue();
					}

					m_frostboardattachments.push_back(frostboardattachment(name,desc,pubkey,privkey));

				}

				file=XMLGetNextSibling(file,"Attachment");

			}
		}

		parsed=true;

	}
	catch(...)
	{
	}

	return parsed;

}
