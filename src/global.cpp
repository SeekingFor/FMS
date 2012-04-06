#include "../include/global.h"
#include "../include/stringfunctions.h"

Poco::SingletonHolder<StringTranslation> Translation;

std::string CreateShortIdentityName(const std::string &name, const std::string &publickey)
{
	std::string result="";
	std::vector<std::string> keyparts;

	StringFunctions::SplitMultiple(publickey,"@,",keyparts);

	result+=name;
	if(keyparts.size()>1 && keyparts[1].size()>8)
	{
		result+="@"+keyparts[1].substr(0,4)+"...";
	}

	return result;
}

void UpdateMissingAuthorID(SQLite3DB::DB *db, const int identityid, const std::string &name, const std::string &publickey)
{
	SQLite3DB::Statement st=db->Prepare("UPDATE tblMessage SET IdentityID=? WHERE IdentityID IS NULL AND FromName=?;");
	st.Bind(0,identityid);
	st.Bind(1,name+publickey.substr(3,44));
	st.Step();
}

namespace global
{
	std::string basepath("");
	bool volatile shutdown=false;
	long volatile httplistenport=8080;
}
