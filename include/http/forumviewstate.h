#ifndef _forumviewstate_
#define _forumviewstate_

#include "../idatabase.h"

class ForumViewState:public IDatabase
{
public:
	ForumViewState(SQLite3DB::DB *db);

	const std::string CreateViewState();
	const bool LoadViewState(const std::string &viewstateid);

	void SetLocalIdentityID(const int localidentityid);
	const int GetLocalIdentityID() const					{ return m_localidentityid; }

	const std::string GetViewStateID() const				{ return m_viewstateid; }

private:
	std::string m_viewstateid;
	int m_localidentityid;
};

#endif	// _forumviewstate_
