#ifndef _localidentitiespage_
#define _localidentitiespage_

#include "../ipagehandler.h"

class LocalIdentitiesPage:public IPageHandler
{
public:
	LocalIdentitiesPage(SQLite3DB::DB *db, const std::string &templatestr):IPageHandler(db,templatestr,"localidentities.htm")	{}

	IPageHandler *New()	{ return new LocalIdentitiesPage(m_db,m_template); }

	void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response);

private:
	const bool WillHandleURI(const std::string &uri);
	const std::string GenerateContent(const std::string &method, const std::map<std::string,QueryVar> &queryvars);

	const std::string CreatePuzzleTypeDropDown(const std::string &name, const std::string &selected);

	void HandleUpdate(const std::map<std::string,QueryVar> &queryvars);
	void HandleDelete(const std::map<std::string,QueryVar> &queryvars);
	void HandleImport(const std::map<std::string,QueryVar> &queryvars);
	const std::string HandleExport();
	void ForceInsertion(std::string field, std::string idstr);
	std::string GenerateInsertionTR(std::string label, std::string inserting, std::string date, std::string formaction, std::string id);
	std::string GenerateComposedInsertionTR(std::string label, std::string tableName, std::string date, std::string formaction, std::string id);

};

#endif	// _localidentitiespage_
