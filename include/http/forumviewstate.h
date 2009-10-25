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
	void UnsetLocalIdentityID();
	const int GetLocalIdentityID() const					{ return m_localidentityid; }

	void SetBoardID(const int boardid);
	void UnsetBoardID();
	const int GetBoardID() const							{ return m_boardid; }

	void SetPage(const int page);
	void UnsetPage();
	const int GetPage() const								{ return m_page; }

	void SetThreadID(const int threadid);
	void UnsetThreadID();
	const int GetThreadID() const							{ return m_threadid; }

	void SetMessageID(const int messageid);
	void UnsetMessageID();
	const int GetMessageID() const							{ return m_messageid; }

	void SetReplyToMessageID(const int replytomessageid);
	void UnsetReplyToMessageID();
	const int GetReplyToMessageID() const					{ return m_replytomessageid; }

	const std::string GetViewStateID() const				{ return m_viewstateid; }

private:
	std::string m_viewstateid;
	int m_localidentityid;
	int m_boardid;
	int m_page;
	int m_threadid;
	int m_messageid;
	int m_replytomessageid;
};

#endif	// _forumviewstate_
