#include "../include/threadbuilder.h"
#include "../include/messagethread.h"

#include "../include/dbsetup.h"
#include "../include/stringfunctions.h"

const bool ThreadBuilder::Build(const long messageid, const long boardid, const bool bydate)
{
	int count=0;
	int threadid=-1;
	MessageThread mt(m_db);
	std::vector<MessageThread::threadnode> m_threadmessages;

	mt.Load(messageid,boardid,bydate);
	m_threadmessages=mt.GetNodes();

	m_db->Execute("BEGIN;");

	// find threadid of this mesage if it already exists in a thread
	SQLite3DB::Statement st=m_db->Prepare("SELECT tblThread.ThreadID FROM tblThread INNER JOIN tblThreadPost ON tblThread.ThreadID=tblThreadPost.ThreadID WHERE tblThread.BoardID=? AND tblThreadPost.MessageID=?;");
	st.Bind(0,boardid);
	st.Bind(1,messageid);

	st.Step();
	if(st.RowReturned())
	{
		st.ResultInt(0,threadid);
	}
	else
	{
		st.Reset();
		// message doesn't exist in a thread, try to find a message in the thread that is already in a thread
		for(std::vector<MessageThread::threadnode>::const_iterator i=m_threadmessages.begin(); i!=m_threadmessages.end() && threadid==-1; i++)
		{
			st.Bind(0,boardid);
			st.Bind(1,(*i).m_messageid);
			st.Step();

			if(st.RowReturned())
			{
				st.ResultInt(0,threadid);
			}

			st.Reset();

		}

		// thread doesn't exist - create it
		if(threadid==-1)
		{
			st=m_db->Prepare("INSERT INTO tblThread(BoardID) VALUES(?);");
			st.Bind(0,boardid);
			st.Step(true);
			threadid=st.GetLastInsertRowID();
		}
	}

	if(m_threadmessages.size()>0)
	{
		SQLite3DB::Statement st2=m_db->Prepare("UPDATE tblThread SET FirstMessageID=?, LastMessageID=? WHERE ThreadID=?;");
		st2.Bind(0,m_threadmessages[0].m_messageid);
		st2.Bind(1,m_threadmessages[m_threadmessages.size()-1].m_messageid);
		st2.Bind(2,threadid);
		st2.Step();

		SQLite3DB::Statement st3=m_db->Prepare("DELETE FROM tblThreadPost WHERE ThreadID=?;");
		st3.Bind(0,threadid);
		st3.Step();

		SQLite3DB::Statement deleteotherst=m_db->Prepare("DELETE FROM tblThread WHERE ThreadID IN (SELECT tblThread.ThreadID FROM tblThreadPost INNER JOIN tblThread ON tblThreadPost.ThreadID=tblThread.ThreadID WHERE tblThread.BoardID=? AND tblThreadPost.MessageID=?);");

		count=0;
		SQLite3DB::Statement st4=m_db->Prepare("INSERT INTO tblThreadPost(ThreadID,MessageID,PostOrder) VALUES(?,?,?);");
		for(std::vector<MessageThread::threadnode>::const_iterator i=m_threadmessages.begin(); i!=m_threadmessages.end(); i++, count++)
		{
			deleteotherst.Bind(0,boardid);
			deleteotherst.Bind(1,(*i).m_messageid);
			deleteotherst.Step();
			deleteotherst.Reset();

			st4.Bind(0,threadid);
			st4.Bind(1,(*i).m_messageid);
			st4.Bind(2,count);
			st4.Step();
			st4.Reset();
		}
	}
	else
	{
		SQLite3DB::Statement st2=m_db->Prepare("DELETE FROM tblThread WHERE ThreadID=?;");
		st2.Bind(0,threadid);
		st2.Step();

		m_log->trace("ThreadBuilder::Build deleted thread");
	}

	m_db->Execute("COMMIT;");

	return true;

}
