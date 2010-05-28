#include "../../../include/http/pages/localidentitiespage.h"
#include "../../../include/stringfunctions.h"
#include "../../../include/http/identityexportxml.h"
#include "../../../include/global.h"

#ifdef XMEM
	#include <xmem.h>
#endif

const std::string LocalIdentitiesPage::GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars)
{
	int count;
	std::string countstr;
	std::string content="";

	content+="<h2>"+m_trans->Get("web.page.localidentities.title")+"</h2>";

	content+="<table><tr><th>"+m_trans->Get("web.page.localidentities.exportidentities")+"</th><th>"+m_trans->Get("web.page.localidentities.importidentities")+"</th></tr>";
	content+="<tr><td>";
	content+="<form name=\"frmexport\" method=\"POST\">";
	content+=CreateFormPassword();
	content+="<input type=\"hidden\" name=\"formaction\" value=\"export\">";
	content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.localidentities.exportidentities")+"\">";
	content+="</form>";
	content+="</td><td>";
	content+="<form name=\"frmimport\" method=\"POST\" enctype=\"multipart/form-data\">";
	content+=CreateFormPassword();
	content+="<input type=\"hidden\" name=\"formaction\" value=\"import\">";
	content+="<input type=\"file\" name=\"file\">";
	content+="<input type=\"submit\" value=\""+m_trans->Get("web.page.localidentities.importidentities")+"\">";
	content+="</form>";
	content+="</td></tr></table>";

	content+="<hr>";

	content+="<table class=\"small90\"><tr><th>"+m_trans->Get("web.page.localidentities.name")+"</th><th>"+m_trans->Get("web.page.localidentities.singleuse")+"</th><th>"+m_trans->Get("web.page.localidentities.publishtrustlist")+"</th><th>"+m_trans->Get("web.page.localidentities.publishboardlist")+"</th><th>"+m_trans->Get("web.page.localidentities.publishfreesite")+"</th><th title="+SanitizeOutput(m_trans->Get("web.page.localidentities.active.description"))+">"+m_trans->Get("web.page.localidentities.active")+"</th><th>"+m_trans->Get("web.page.localidentities.minmessagedelay")+"</th><th>"+m_trans->Get("web.page.localidentities.maxmessagedelay")+"</th><th>"+m_trans->Get("web.page.localidentities.announced")+"</th></tr>";

	SQLite3DB::Statement st=m_db->Prepare("SELECT LocalIdentityID,tblLocalIdentity.Name,tblLocalIdentity.PublicKey,tbLLocalIdentity.PublishTrustList,tblLocalIdentity.SingleUse,tblLocalIdentity.PublishBoardList,tblIdentity.IdentityID,tblLocalIdentity.PublishFreesite,tblLocalIdentity.MinMessageDelay,tblLocalIdentity.MaxMessageDelay,tblLocalIdentity.Active FROM tblLocalIdentity LEFT JOIN tblIdentity ON tblLocalIdentity.PublicKey=tblIdentity.PublicKey ORDER BY tblLocalIdentity.Name;");
	st.Step();
	SQLite3DB::Statement st2=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE PublicKey=?;");

	SQLite3DB::Statement trustst=m_db->Prepare("SELECT COUNT(*) FROM tblPeerTrust LEFT JOIN tblIdentity ON tblPeerTrust.TargetIdentityID=tblIdentity.IdentityID WHERE tblIdentity.PublicKey=? GROUP BY tblPeerTrust.TargetIdentityID;");

	count=0;
	while(st.RowReturned())
	{
		StringFunctions::Convert(count,countstr);
		std::string id="";
		std::string name="";
		std::string publickey="";
		std::string publishtrustlist="";
		std::string singleuse="";
		std::string publishboardlist="";
		std::string publishfreesite="";
		std::string minmessagedelay="0";
		std::string maxmessagedelay="0";
		std::string identityidstr="";
		std::string active="";

		st.ResultText(0,id);
		st.ResultText(1,name);
		st.ResultText(2,publickey);
		st.ResultText(3,publishtrustlist);
		st.ResultText(4,singleuse);
		st.ResultText(5,publishboardlist);
		st.ResultText(7,publishfreesite);
		st.ResultText(8,minmessagedelay);
		st.ResultText(9,maxmessagedelay);
		st.ResultText(10,active);

		st2.Bind(0,publickey);
		st2.Step();
		if(st2.RowReturned())
		{
			st2.ResultText(0,identityidstr);
		}
		st2.Reset();

		content+="<tr>";
		content+="<td title=\""+publickey+"\"><form name=\"frmupdate"+countstr+"\" method=\"POST\"><input type=\"hidden\" name=\"formaction\" value=\"update\">"+CreateFormPassword()+"<input type=\"hidden\" name=\"chkidentityid["+countstr+"]\" value=\""+id+"\">";
		if(identityidstr!="")
		{
			content+="<a href=\"peerdetails.htm?identityid="+identityidstr+"\">";
		}
		content+=SanitizeOutput(CreateShortIdentityName(name,publickey));
		if(identityidstr!="")
		{
			content+="</a>";
		}
		content+="</td>";
		content+="<td>"+CreateTrueFalseDropDown("singleuse["+countstr+"]",singleuse)+"</td>";
		content+="<td>"+CreateTrueFalseDropDown("publishtrustlist["+countstr+"]",publishtrustlist)+"</td>";
		content+="<td>"+CreateTrueFalseDropDown("publishboardlist["+countstr+"]",publishboardlist)+"</td>";
		content+="<td>"+CreateTrueFalseDropDown("publishfreesite["+countstr+"]",publishfreesite)+"</td>";
		content+="<td>"+CreateTrueFalseDropDown("active["+countstr+"]",active)+"</td>";
		content+="<td><input type=\"text\" size=\"2\" name=\"mindelay["+countstr+"]\" value=\""+minmessagedelay+"\"></td>";
		content+="<td><input type=\"text\" size=\"2\" name=\"maxdelay["+countstr+"]\" value=\""+maxmessagedelay+"\"></td>";
		
		trustst.Bind(0,publickey);
		trustst.Step();
		if(trustst.RowReturned())
		{
			std::string numlists="";
			trustst.ResultText(0,numlists);
			content+="<td>"+m_trans->Get("web.page.localidentities.yes")+" ("+numlists+")</td>";
		}
		else
		{
			content+="<td>"+m_trans->Get("web.page.localidentities.no")+"</td>";
		}
		trustst.Reset();

		content+="<td><input type=\"submit\" value=\""+m_trans->Get("web.page.localidentities.update")+"\"></form></td>";
		content+="<td><form name=\"frmdel"+countstr+"\" method=\"POST\" action=\"confirm.htm\">"+CreateFormPassword()+"<input type=\"hidden\" name=\"formaction\" value=\"delete\"><input type=\"hidden\" name=\"chkidentityid["+countstr+"]\" value=\""+id+"\"><input type=\"hidden\" name=\"targetpage\" value=\"localidentities.htm\"><input type=\"hidden\" name=\"confirmdescription\" value=\""+m_trans->Get("web.page.localidentities.confirmdelete")+" "+SanitizeOutput(CreateShortIdentityName(name,publickey))+"?\"><input type=\"submit\" value=\""+m_trans->Get("web.page.localidentities.delete")+"\"></form></td>";
		content+="</tr>";
		content+="<tr><td></td><td colspan=\"7\" class=\"smaller\">"+publickey+"</td></tr>";
		st.Step();
		count++;
	}

	content+="</table>";
	content+="<p class=\"paragraph\">"+m_trans->Get("web.page.localidentities.announceddescription")+"</p>";
	content+="<p class=\"paragraph\">"+m_trans->Get("web.page.localidentities.singleusedescription")+"</p>";
	content+="<p class=\"paragraph\">"+m_trans->Get("web.page.localidentities.delaydescription")+"</p>";
	content+="<p class=\"paragraph\">"+m_trans->Get("web.page.localidentities.active.description")+"</p>";

	return content;
}

void LocalIdentitiesPage::HandleDelete(const std::map<std::string,QueryVar> &queryvars)
{
	int id=0;
	std::vector<std::string> ids;
	CreateArgArray(queryvars,"chkidentityid",ids);

	SQLite3DB::Statement del=m_db->Prepare("DELETE FROM tblLocalIdentity WHERE LocalIdentityID=?;");
	for(int i=0; i<ids.size(); i++)
	{
		if(ids[i]!="")
		{
			StringFunctions::Convert(ids[i],id);
			del.Bind(0,id);
			del.Step();
			del.Reset();
		}
	}
}

const std::string LocalIdentitiesPage::HandleExport()
{
	IdentityExportXML xml;
	SQLite3DB::Statement exp=m_db->Prepare("SELECT Name,PublicKey,PrivateKey,SingleUse,PublishTrustList,PublishBoardList,PublishFreesite FROM tblLocalIdentity WHERE PublicKey IS NOT NULL AND PrivateKey IS NOT NULL;");
	exp.Step();
	while(exp.RowReturned())
	{
		std::string name="";
		std::string publickey="";
		std::string privatekey="";
		std::string tempval="";
		bool singleuse=false;
		bool publishtrustlist=false;
		bool publishboardlist=false;
		bool publishfreesite=false;

		exp.ResultText(0,name);
		exp.ResultText(1,publickey);
		exp.ResultText(2,privatekey);
		exp.ResultText(3,tempval);
		if(tempval=="true")
		{
			singleuse=true;
		}
		exp.ResultText(4,tempval);
		if(tempval=="true")
		{
			publishtrustlist=true;
		}
		exp.ResultText(5,tempval);
		if(tempval=="true")
		{
			publishboardlist=true;
		}
		exp.ResultText(6,tempval);
		if(tempval=="true")
		{
			publishfreesite=true;
		}

		xml.AddIdentity(name,publickey,privatekey,singleuse,publishtrustlist,publishboardlist,publishfreesite);

		exp.Step();
	}
	return xml.GetXML();
}

void LocalIdentitiesPage::HandleImport(const std::map<std::string,QueryVar> &queryvars)
{
	if(queryvars.find("file")!=queryvars.end())
	{
		IdentityExportXML xml;
		if(xml.ParseXML((*queryvars.find("file")).second.GetData()))
		{
			SQLite3DB::Statement imp=m_db->Prepare("INSERT INTO tblLocalIdentity(Name,PublicKey,PrivateKey,SingleUse,PublishTrustList,PublishBoardList,PublishFreesite) VALUES(?,?,?,?,?,?,?);");
			for(int i=0; i<xml.GetCount(); i++)
			{
				std::string tempval="false";
				imp.Bind(0,xml.GetName(i));
				imp.Bind(1,xml.GetPublicKey(i));
				imp.Bind(2,xml.GetPrivateKey(i));
				if(xml.GetSingleUse(i))
				{
					tempval="true";
				}
				else
				{
					tempval="false";
				}
				imp.Bind(3,tempval);
				if(xml.GetPublishTrustList(i))
				{
					tempval="true";
				}
				else
				{
					tempval="false";
				}
				imp.Bind(4,tempval);
				if(xml.GetPublishBoardList(i))
				{
					tempval="true";
				}
				else
				{
					tempval="false";
				}
				imp.Bind(5,tempval);
				if(xml.GetPublishFreesite(i))
				{
					tempval="true";
				}
				else
				{
					tempval="false";
				}
				imp.Bind(6,tempval);
				imp.Step();
				imp.Reset();
			}
		}
	}
}

void LocalIdentitiesPage::handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response)
{
	m_log->trace("LocalIdentitiesPages::handleRequest from "+request.clientAddress().toString());

	std::map<std::string,QueryVar> vars;

	CreateQueryVarMap(request,vars);

	std::string formaction="";
	if(vars.find("formaction")!=vars.end() && ValidateFormPassword(vars))
	{
		formaction=(*vars.find("formaction")).second.GetData();
		if(formaction=="update")
		{
			HandleUpdate(vars);
		}
		else if(formaction=="delete")
		{
			HandleDelete(vars);
		}
		else if(formaction=="export")
		{
			response.setChunkedTransferEncoding(true);
			response.setContentType("application/xml");
			response.set("Content-Disposition","attachment; filename=identities.xml");
			std::ostream &out=response.send();
			out << HandleExport();
			return;
		}
		else if(formaction=="import")
		{
			HandleImport(vars);
		}
	}

	response.setChunkedTransferEncoding(true);
	response.setContentType("text/html");

	std::ostream &ostr = response.send();
	ostr << GeneratePage(request.getMethod(),vars);
}

void LocalIdentitiesPage::HandleUpdate(const std::map<std::string,QueryVar> &queryvars)
{
	int id;
	std::vector<std::string> ids;
	std::vector<std::string> singleuse;
	std::vector<std::string> publishtrustlist;
	std::vector<std::string> publishboardlist;
	std::vector<std::string> publishfreesite;
	std::vector<std::string> mindelay;
	std::vector<std::string> maxdelay;
	std::vector<std::string> active;

	CreateArgArray(queryvars,"chkidentityid",ids);
	CreateArgArray(queryvars,"singleuse",singleuse);
	CreateArgArray(queryvars,"publishtrustlist",publishtrustlist);
	CreateArgArray(queryvars,"publishboardlist",publishboardlist);
	CreateArgArray(queryvars,"publishfreesite",publishfreesite);
	CreateArgArray(queryvars,"mindelay",mindelay);
	CreateArgArray(queryvars,"maxdelay",maxdelay);
	CreateArgArray(queryvars,"active",active);

	SQLite3DB::Statement update=m_db->Prepare("UPDATE tblLocalIdentity SET SingleUse=?, PublishTrustList=?, PublishBoardList=?, PublishFreesite=?, MinMessageDelay=?, MaxMessageDelay=?, Active=? WHERE LocalIdentityID=?;");
	for(int i=0; i<ids.size(); i++)
	{
		if(ids[i]!="")
		{
			int minmessagedelay=0;
			int maxmessagedelay=0;
			StringFunctions::Convert(ids[i],id);
			StringFunctions::Convert(mindelay[i],minmessagedelay);
			StringFunctions::Convert(maxdelay[i],maxmessagedelay);
			update.Bind(0,singleuse[i]);
			update.Bind(1,publishtrustlist[i]);
			update.Bind(2,publishboardlist[i]);
			update.Bind(3,publishfreesite[i]);
			update.Bind(4,minmessagedelay);
			update.Bind(5,maxmessagedelay);
			update.Bind(6,active[i]);
			update.Bind(7,id);
			update.Step();
			update.Reset();
		}
	}
}

const bool LocalIdentitiesPage::WillHandleURI(const std::string &uri)
{
	if(uri.find("localidentities.")!=std::string::npos)
	{
		return true;
	}
	else
	{
		return false;
	}
}
