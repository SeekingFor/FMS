#include "../include/threadbuilder.h"
#include "../include/messagethread.h"

#include "../include/dbsetup.h"
#include "../include/stringfunctions.h"
#include "../include/option.h"

const bool ThreadBuilder::Build(const long messageid, const long boardid, const bool bydate)
{
	int count=0;
	int threadid=-1;
	MessageThread mt(m_db);
	std::vector<MessageThread::threadnode> m_threadmessages;
	std::string logmessage("");	// temp var to help track down exactly when corruption occurrs
	std::string ll("");
	Option option(m_db);

	option.Get("LogLevel",ll);

	mt.Load(messageid,boardid,bydate);
	m_threadmessages=mt.GetNodes();

	m_db->Execute("BEGIN;");

	// find threadid of this mesage if it already exists in a thread
	SQLite3DB::Statement st=m_db->Prepare("SELECT tblThread.ThreadID FROM tblThread INNER JOIN tblThreadPost ON tblThread.ThreadID=tblThreadPost.ThreadID WHERE tblThread.BoardID=? AND tblThreadPost.MessageID=?;");
	st.Bind(0,boardid);
	st.Bind(1,messageid);

	if(ll=="8")
	{
		std::string temp1("");
		std::string temp2("");
		StringFunctions::Convert(boardid,temp1);
		StringFunctions::Convert(messageid,temp2);

		logmessage+="initial bound boardid=" + temp1 + " messageid=" + temp2 + " | ";
	}

	st.Step();
	if(st.RowReturned())
	{
		st.ResultInt(0,threadid);

		if(ll=="8")
		{
			std::string temp1("");
			StringFunctions::Convert(threadid,temp1);

			logmessage+="result threadid=" + temp1 + " | ";
		}
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

			if(ll=="8")
			{
				std::string temp1("");
				std::string temp2("");
				StringFunctions::Convert(boardid,temp1);
				StringFunctions::Convert((*i).m_messageid,temp2);

				logmessage+="find bound boardid=" + temp1 + " messageid=" + temp2 + " | ";
			}

			if(st.RowReturned())
			{
				st.ResultInt(0,threadid);
				if(ll=="8")
				{
					std::string temp1("");
					StringFunctions::Convert(threadid,temp1);

					logmessage+="find result threadid=" + temp1 + " | ";
				}
			}
			else
			{
				if(ll=="8")
				{
					std::string temp1("");
					StringFunctions::Convert(threadid,temp1);

					logmessage+="find not found | ";
				}
			}

			st.Reset();

		}
		st.Finalize();

		// thread doesn't exist - create it
		if(threadid==-1)
		{
			st=m_db->Prepare("INSERT INTO tblThread(BoardID) VALUES(?);");
			st.Bind(0,boardid);
			st.Step(true);
			threadid=st.GetLastInsertRowID();

			if(ll=="8")
			{
				std::string temp1("");
				std::string temp2("");
				StringFunctions::Convert(boardid,temp1);
				StringFunctions::Convert(threadid,temp2);

				logmessage+="insert thread bind boardid=" + temp1 + " result threadid=" + temp2 + " | ";
			}

		}
	}

	if(m_threadmessages.size()>0)
	{
		SQLite3DB::Statement st2=m_db->Prepare("UPDATE tblThread SET FirstMessageID=?, LastMessageID=? WHERE ThreadID=?;");
		st2.Bind(0,m_threadmessages[0].m_messageid);
		st2.Bind(1,m_threadmessages[m_threadmessages.size()-1].m_messageid);
		st2.Bind(2,threadid);
		st2.Step();

		if(ll=="8")
		{
			std::string temp1("");
			std::string temp2("");
			std::string temp3("");
			StringFunctions::Convert(m_threadmessages[0].m_messageid,temp1);
			StringFunctions::Convert(m_threadmessages[m_threadmessages.size()-1].m_messageid,temp2);
			StringFunctions::Convert(threadid,temp3);

			logmessage+="update bind fmessageid=" + temp1 + " lmessageid=" + temp2 + " threadid=" + temp3 + " | ";
		}

		SQLite3DB::Statement deleteotherst=m_db->Prepare("DELETE FROM tblThread WHERE ThreadID IN (SELECT tblThread.ThreadID FROM tblThreadPost INNER JOIN tblThread ON tblThreadPost.ThreadID=tblThread.ThreadID WHERE tblThread.ThreadID<>? AND tblThread.BoardID=? AND tblThreadPost.MessageID=?);");

		count=0;
		SQLite3DB::Statement st4=m_db->Prepare("INSERT OR REPLACE INTO tblThreadPost(ThreadID,MessageID,PostOrder) VALUES(?,?,?);");
		for(std::vector<MessageThread::threadnode>::const_iterator i=m_threadmessages.begin(); i!=m_threadmessages.end(); i++, count++)
		{
			deleteotherst.Bind(0,threadid);
			deleteotherst.Bind(1,boardid);
			deleteotherst.Bind(2,(*i).m_messageid);
			deleteotherst.Step();
			deleteotherst.Reset();

			if(ll=="8")
			{
				std::string temp1("");
				std::string temp2("");
				StringFunctions::Convert(boardid,temp1);
				StringFunctions::Convert((*i).m_messageid,temp2);

				logmessage+="deleteother bind boardid=" + temp1 + " messageid=" + temp2 + " | ";
			}

			st4.Bind(0,threadid);
			st4.Bind(1,(*i).m_messageid);
			st4.Bind(2,count);
			st4.Step();
			st4.Reset();

			if(ll=="8")
			{
				std::string temp1("");
				std::string temp2("");
				std::string temp3("");
				StringFunctions::Convert(threadid,temp1);
				StringFunctions::Convert((*i).m_messageid,temp2);
				StringFunctions::Convert(count,temp3);

				logmessage+="insertthreadpost bind threadid=" + temp1 + " messageid=" + temp2 + " count=" + temp3 + " | ";
			}

		}
	}
	else
	{
		SQLite3DB::Statement st2=m_db->Prepare("DELETE FROM tblThread WHERE ThreadID=?;");
		st2.Bind(0,threadid);
		st2.Step();

		if(ll=="8")
		{
			std::string temp1("");
			StringFunctions::Convert(threadid,temp1);

			logmessage+="delete thread bind threadid=" + temp1 + " | ";
		}

		m_log->trace("ThreadBuilder::Build deleted thread");
	}

	st.Finalize();

	m_db->Execute("COMMIT;");

	if(ll=="8")
	{
		m_log->trace(logmessage);
	}

	return true;

}
