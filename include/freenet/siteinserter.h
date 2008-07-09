#ifndef _site_inserter_
#define _site_inserter_

#include "iindexinserter.h"

class SiteInserter:public IIndexInserter<long>
{
public:
	SiteInserter();
	SiteInserter(FCPv2 *fcp);

private:
	void Initialize();
	const bool HandlePutSuccessful(FCPMessage &message);
	const bool HandlePutFailed(FCPMessage &message);
	const bool StartInsert(const long &localidentityid);
	void CheckForNeededInsert();
	const std::string SanitizeOutput(const std::string &input);
	void GeneratePages(const long localidentityid, std::string &uskkey, std::map<std::string,std::string> &pages);
	std::string GenerateLinks(const bool publishtrustlist, const bool publishboardlist);
	std::string GenerateIndex(const std::string &htmltemplate, const long localidentityid, const std::string &name);
	std::string GenerateTrustList(const std::string &htmltemplate, const long localidentityid, const std::string &name);
	const std::string GetClassString(const std::string &trustlevel);

};

#endif	// _site_inserter_
