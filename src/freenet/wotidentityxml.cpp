#include "../../include/freenet/wotidentityxml.h"
#include "../../include/stringfunctions.h"

#include <limits>

WOTIdentityXML::WOTIdentityXML()
{
	Initialize();
}

std::string WOTIdentityXML::GetXML()
{
	return std::string("");
}

void WOTIdentityXML::Initialize()
{
	m_name="";
	m_contexts.clear();
	m_properties.clear();
	m_trustlist.clear();
}

const bool WOTIdentityXML::ParseXML(const std::string &xml)
{
	bool parsed=false;
	Poco::XML::DOMParser dp;

	dp.setEntityResolver(0);

	Initialize();

	try
	{
		Poco::AutoPtr<Poco::XML::Document> doc=dp.parseString(FixCDATA(xml));
		Poco::XML::Element *root=XMLGetFirstChild(doc,"WebOfTrust");
		Poco::XML::Element *txt=NULL;

		if(root)
		{
			root=XMLGetFirstChild(root,"Identity");
		}

		if(root)
		{
			m_name=root->getAttribute("Name");

			txt=XMLGetFirstChild(root,"Context");
			while(txt)
			{
				std::string cname=txt->getAttribute("Name");

				if(cname!="")
				{
					m_contexts.push_back(cname);
				}

				txt=XMLGetNextSibling(txt,"Context");
			}

			txt=XMLGetFirstChild(root,"Property");
			while(txt)
			{
				std::string pname=txt->getAttribute("Name");
				std::string pvalue=txt->getAttribute("Value");

				if(pname!="" && pvalue!="")
				{
					m_properties.push_back(std::pair<std::string,std::string>(pname,pvalue));
				}

				txt=XMLGetNextSibling(txt,"Property");
			}

			txt=XMLGetFirstChild(root,"TrustList");
			if(txt)
			{
				txt=XMLGetFirstChild(txt,"Trust");
				while(txt)
				{
					std::string identity=txt->getAttribute("Identity");
					std::string comment=txt->getAttribute("Comment");
					int value=(std::numeric_limits<int>::min)();
					std::string valuestr=txt->getAttribute("Value");
					if(valuestr!="")
					{
						StringFunctions::Convert(valuestr,value);
					}

					if(identity!="")
					{
						m_trustlist.push_back(trust(identity,comment,value));
					}

					txt=XMLGetNextSibling(txt,"Trust");
				}
			}

		}

		parsed=true;

	}
	catch(...)
	{
	}

	return parsed;

}
