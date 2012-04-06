#include "../include/dbmaintenancethread.h"
#include "../include/stringfunctions.h"
#include "../include/option.h"
#include "../include/threadbuilder.h"
#include "../include/dbsetup.h"
#include "../include/fmsapp.h"

#include <vector>

#include <Poco/Timestamp.h>
#include <Poco/Timespan.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/Thread.h>

DBMaintenanceThread::DBMaintenanceThread()
{
	// move last maintenance times back so they will all run soon
	m_last10minute=Poco::Timestamp();
	m_last30minute=Poco::Timestamp();
	m_last30minute-=Poco::Timespan(0,0,11,0,0);
	m_last1hour=Poco::Timestamp();
	m_last1hour-=Poco::Timespan(0,0,49,0,0);
	m_last6hour=Poco::Timestamp();
	m_last6hour-=Poco::Timespan(0,5,42,0,0);
	m_last1day=Poco::Timestamp();
	m_last1day-=Poco::Timespan(0,23,51,0,0);
}


void DBMaintenanceThread::Do10MinuteMaintenance()
{
	// commented out for now
	m_log->debug("PeriodicDBMaintenance::Do10MinuteMaintenance");
}

void DBMaintenanceThread::Do30MinuteMaintenance()
{
	std::vector<int> m_boardlist;
	std::vector<std::pair<long,long> > m_unthreadedmessages;
	Option option(m_db);
	std::string ll("");
	option.Get("LogLevel",ll);

	m_log->debug("PeriodicDBMaintenance::Do30MinuteMaintenance start");

	ThreadBuilder tb(m_db);
	SQLite3DB::Statement boardst=m_db->Prepare("SELECT BoardID FROM tblBoard WHERE Forum='true';");
	// select messages for a board that aren't in a thread
	// This query was causing the db to be locked and a journal file created.
	// build a list of boards and messageids and then use that instead of keeping the query in use
	
	/*SQLite3DB::Statement selectst=m_db->Prepare("SELECT tblMessage.MessageID FROM tblMessage \
												INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID \
												LEFT JOIN (SELECT tblThread.BoardID, tblThreadPost.MessageID FROM tblThread INNER JOIN tblThreadPost ON tblThread.ThreadID=tblThreadPost.ThreadID WHERE tblThread.BoardID=?) AS temp1 ON tblMessage.MessageID=temp1.MessageID \
												WHERE tblMessageBoard.BoardID=? AND temp1.BoardID IS NULL;");*/
	// This query is about 50x faster than the above (roughly tested)
	SQLite3DB::Statement selectst=m_db->Prepare("SELECT tblMessageBoard.MessageID FROM tblMessageBoard \
												WHERE tblMessageBoard.BoardID=? AND \
												tblMessageBoard.MessageID NOT IN (SELECT MessageID FROM tblThreadPost INNER JOIN tblThread ON tblThreadPost.ThreadID=tblThread.ThreadID WHERE tblThread.BoardID=?);");

	SQLite3DB::Statement latestmessagest=m_db->Prepare("UPDATE tblBoard SET LatestMessageID=(SELECT tblMessage.MessageID FROM tblMessage INNER JOIN tblMessageBoard ON tblMessage.MessageID=tblMessageBoard.MessageID WHERE tblMessageBoard.BoardID=tblBoard.BoardID ORDER BY tblMessage.MessageDate DESC, tblMessage.MessageTime DESC LIMIT 0,1) WHERE tblBoard.BoardID=?;");

	boardst.Step();
	while(boardst.RowReturned())
	{
		int boardid=-1;
		boardst.ResultInt(0,boardid);
		boardst.Step();

		selectst.Bind(0,boardid);
		selectst.Bind(1,boardid);
		selectst.Step();

		while(selectst.RowReturned())
		{
			int messageid=-1;

			selectst.ResultInt(0,messageid);

			m_unthreadedmessages.push_back(std::pair<long,long>(boardid,messageid));
			m_boardlist.push_back(boardid);

			selectst.Step();
		}
		selectst.Reset();
	}
	selectst.Finalize();
	boardst.Finalize();

	for(std::vector<std::pair<long,long> >::iterator i=m_unthreadedmessages.begin(); i!=m_unthreadedmessages.end(); i++)
	{
		tb.Build((*i).second,(*i).first,true);
	}

	// get latest message ids for the updated boards
	for(std::vector<int>::iterator i=m_boardlist.begin(); i!=m_boardlist.end(); i++)
	{
		latestmessagest.Bind(0,(*i));
		latestmessagest.Step();
		latestmessagest.Reset();
	}

	// now rebuild threads where the message has been deleted
	SQLite3DB::Statement st=m_db->Prepare("SELECT tblThreadPost.MessageID, tblThread.BoardID FROM tblThreadPost INNER JOIN tblThread ON tblThreadPost.ThreadID=tblThread.ThreadID LEFT JOIN tblMessage ON tblThreadPost.MessageID=tblMessage.MessageID WHERE tblMessage.MessageID IS NULL;");
	st.Step();
	while(st.RowReturned())
	{
		int messageid=-1;
		int boardid=-1;

		st.ResultInt(0,messageid);
		st.ResultInt(1,boardid);

		tb.Build(messageid,boardid,true);

		st.Step();
	}

	// delete threads that have no messages
	m_db->Execute("DELETE FROM tblThread WHERE ThreadID IN (SELECT tblThread.ThreadID FROM tblThread LEFT JOIN tblThreadPost ON tblThread.ThreadID=tblThreadPost.ThreadID WHERE tblThreadPost.ThreadID IS NULL);");

	m_log->debug("PeriodicDBMaintenance::Do30MinuteMaintenance end");
}

void DBMaintenanceThread::Do1HourMaintenance()
{
	SQLite3DB::Transaction trans(m_db);
	int idcount=0;

	m_log->debug("PeriodicDBMaintenance::Do1HourMaintenance start");

	// get count of local identities
	SQLite3DB::Statement st=m_db->Prepare("SELECT COUNT(*) FROM tblLocalIdentity;");
	st.Step();
	if(st.RowReturned())
	{
		st.ResultInt(0,idcount);
	}

	// only recalculate trust if a local identity exists
	if(idcount>0)
	{

		trans.Begin(SQLite3DB::Transaction::TRANS_IMMEDIATE);
		// recalculate all trust levels - this is CPU instensive
		// do 1 identity at a time as doing it with 1 UPDATE statement locks that database for the duration
		SQLite3DB::Statement st=m_db->Prepare("SELECT TargetIdentityID,PeerMessageTrust,PeerTrustListTrust FROM vwCalculatedPeerTrust;");
		SQLite3DB::Statement upd=m_db->Prepare("UPDATE tblIdentity SET PeerMessageTrust=?, PeerTrustListTrust=? WHERE IdentityID=?");
		trans.Step(st);
		while(st.RowReturned())
		{
			int identityid=0;
			int trust=0;
			
			st.ResultInt(0,identityid);

			upd.Bind(0,identityid);
			if(st.ResultNull(1)==false)
			{
				trust=0;
				st.ResultInt(1,trust);
				upd.Bind(0,trust);
			}
			else
			{
				upd.Bind(0);
			}
			if(st.ResultNull(2)==false)
			{
				trust=0;
				st.ResultInt(2,trust);
				upd.Bind(1,trust);
			}
			else
			{
				upd.Bind(1);
			}
			upd.Bind(2,identityid);
			trans.Step(upd);
			trans.Reset(upd);

			trans.Step(st);
		}

		// set null peer trust for identities without a calculated trust
		st=m_db->Prepare("SELECT IdentityID FROM tblIdentity WHERE IdentityID NOT IN (SELECT TargetIdentityID FROM vwCalculatedPeerTrust);");
		upd=m_db->Prepare("UPDATE tblIdentity SET PeerMessageTrust=NULL, PeerTrustListTrust=NULL WHERE IdentityID=?;");
		trans.Step(st);
		while(st.RowReturned())
		{
			int identityid=0;
			st.ResultInt(0,identityid);
			upd.Bind(0,identityid);
			trans.Step(upd);
			trans.Reset(upd);
			trans.Step(st);
		}

		trans.Finalize(st);
		trans.Finalize(upd);

		// insert all identities not in trust list already
		m_db->Execute("INSERT INTO tblIdentityTrust(LocalIdentityID,IdentityID) SELECT LocalIdentityID,IdentityID FROM tblLocalIdentity,tblIdentity WHERE LocalIdentityID || '_' || IdentityID NOT IN (SELECT LocalIdentityID || '_' || IdentityID FROM tblIdentityTrust);");

		trans.Commit();

		if(trans.IsSuccessful()==false)
		{
			m_log->error("DBMaintenanceThread::Do1HourMaintenance transaction failed with SQLite error:"+trans.GetLastErrorStr()+" SQL="+trans.GetErrorSQL());
		}

	}

	m_log->debug("PeriodicDBMaintenance::Do1HourMaintenance end");
}

void DBMaintenanceThread::Do6HourMaintenance()
{
	SQLite3DB::Transaction trans(m_db);

	m_log->debug("PeriodicDBMaintenance::Do6HourMaintenance start");

	trans.Begin(SQLite3DB::Transaction::TRANS_IMMEDIATE);

	// if we remove a board and the reply boardid is still set to it, we need to replace it with a boardid that does exist
	SQLite3DB::Statement st=m_db->Prepare("SELECT MessageID FROM tblMessage WHERE ReplyBoardID NOT IN (SELECT BoardID FROM tblBoard);");
	SQLite3DB::Statement st2=m_db->Prepare("SELECT BoardID FROM tblMessageBoard WHERE MessageID=?;");
	SQLite3DB::Statement upd=m_db->Prepare("UPDATE tblMessage SET ReplyBoardID=? WHERE MessageID=?;");
	trans.Step(st);
	while(st.RowReturned())
	{
		// find a valid boardid for the message
		int messageid=0;
		int boardid=0;

		st.ResultInt(0,messageid);

		st2.Bind(0,messageid);
		trans.Step(st2);
		if(st2.RowReturned())
		{
			st2.ResultInt(0,boardid);
			upd.Bind(0,boardid);
			upd.Bind(1,messageid);
			trans.Step(upd);
			trans.Reset(upd);
		}
		trans.Reset(st2);
		
		trans.Step(st);
	}

	trans.Finalize(st);
	trans.Finalize(st2);
	trans.Finalize(upd);

	trans.Commit();
	if(trans.IsSuccessful()==false)
	{
		m_log->error("DBMaintenanceThread::Do6HourMaintenance transaction 1 failed with SQLite error:"+trans.GetLastErrorStr()+" SQL="+trans.GetErrorSQL());
	}

	if(m_db->IsProfiling())
	{
		std::string output("\nSQL\tCount\tTotalTime\n");
		std::map<std::string,SQLite3DB::DB::ProfileData> profiledata;
		m_db->GetProfileData(profiledata,true);
		for(std::map<std::string,SQLite3DB::DB::ProfileData>::const_iterator i=profiledata.begin(); i!=profiledata.end(); i++)
		{
			std::ostringstream ostr;
			ostr << (*i).first << "\t" << (*i).second.m_count << "\t" << (*i).second.m_time << "\n";
			output+=ostr.str();
		}
		m_log->information(output);
	}

	m_log->debug("PeriodicDBMaintenance::Do6HourMaintenance end");
}

void DBMaintenanceThread::Do1DayMaintenance()
{
	Poco::DateTime date;
	SQLite3DB::Transaction trans(m_db);

	m_log->debug("PeriodicDBMaintenance::Do1DayMaintenance start");

	trans.Begin(SQLite3DB::Transaction::TRANS_IMMEDIATE);

	// delete all puzzles 2 or more days old
	date=Poco::Timestamp();
	date-=Poco::Timespan(2,0,0,0,0);
	m_db->Execute("DELETE FROM tblIntroductionPuzzleInserts WHERE Day<='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"';");
	m_db->Execute("DELETE FROM tblIntroductionPuzzleRequests WHERE Day<='"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"';");

	// delete all identities we've never seen and were added more than 20 days ago
	// number of days needs be to greater than the number of days backwards in the trust list inserter, otherwise we'd delete them and add them again
	// from another trust list
	date=Poco::Timestamp();
	date-=Poco::Timespan(20,0,0,0,0);
	m_db->Execute("DELETE FROM tblIdentity WHERE LastSeen IS NULL AND WOTLastSeen IS NULL AND DateAdded<'"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"';");

	// delete old identity requests - we don't need them anymore
	date=Poco::Timestamp();
	date-=Poco::Timespan(2,0,0,0,0);
	m_db->Execute("DELETE FROM tblIdentityRequests WHERE Day<'"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"';");

	// delete old board list inserts/requests - we don't need them anymore
	date=Poco::Timestamp();
	date-=Poco::Timespan(2,0,0,0,0);
	m_db->Execute("DELETE FROM tblBoardListInserts WHERE Day<'"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"';");
	m_db->Execute("DELETE FROM tblBoardListRequests WHERE Day<'"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"';");

	// delete old local identity inserts - we don't need them anymore
	date=Poco::Timestamp();
	date-=Poco::Timespan(2,0,0,0,0);
	m_db->Execute("DELETE FROM tblLocalIdentityInserts WHERE Day<'"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"';");

	// delete old message list inserts/requests - we don't need them anymore
	// only delete those older than the max # of days backward we are downloading messages
	date=Poco::Timestamp();
	date-=Poco::Timespan(m_messagedownloadmaxdaysbackward,0,0,0,0);
	m_db->Execute("DELETE FROM tblMessageListInserts WHERE Day<'"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"';");
	m_db->Execute("DELETE FROM tblMessageListRequests WHERE Day<'"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"';");

	// delete old trust list inserts/requests - we don't need them anymore
	date=Poco::Timestamp();
	date-=Poco::Timespan(2,0,0,0,0);
	m_db->Execute("DELETE FROM tblTrustListInserts WHERE Day<'"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"';");
	m_db->Execute("DELETE FROM tblTrustListRequests WHERE Day<'"+Poco::DateTimeFormatter::format(date,"%Y-%m-%d")+"';");

	// delete trust lists from identities we aren't trusting anymore
	// m_db->Execute("DELETE FROM tblPeerTrust WHERE IdentityID NOT IN (SELECT IdentityID FROM tblIdentity WHERE (LocalTrustListTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinLocalTrustListTrust')) AND (PeerTrustListTrust IS NULL OR PeerTrustListTrust>=(SELECT OptionValue FROM tblOption WHERE Option='MinPeerTrustListTrust')));");

	trans.Commit();
	if(trans.IsSuccessful()==false)
	{
		m_log->error("DBMaintenanceThread::Do1DayMaintenance transaction failed with SQLite error:"+trans.GetLastErrorStr()+" SQL="+trans.GetErrorSQL());
	}

	trans.Begin(SQLite3DB::Transaction::TRANS_IMMEDIATE);

	// delete single use identities that are older than 7 days
	date=Poco::Timestamp();
	date-=Poco::Timespan(7,0,0,0,0);
	SQLite3DB::Statement st=m_db->Prepare("DELETE FROM tblIdentity WHERE SingleUse='true' AND DateAdded<?;");
	st.Bind(0,Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S"));
	trans.Step(st);

	// delete local single use identities that are older than 7 days
	date=Poco::Timestamp();
	date-=Poco::Timespan(7,0,0,0,0);
	st=m_db->Prepare("DELETE FROM tblLocalIdentity WHERE SingleUse='true' AND DateCreated<?;");
	st.Bind(0,Poco::DateTimeFormatter::format(date,"%Y-%m-%d %H:%M:%S"));
	trans.Step(st);

	// delete old messages
	if(m_deletemessagesolderthan>=0)
	{
		date=Poco::Timestamp();
		date-=Poco::Timespan(m_deletemessagesolderthan,0,0,0,0);
		m_log->trace("PeriodicDBMaintenance::Do1DayMaintenance deleting messages prior to "+Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
		st=m_db->Prepare("DELETE FROM tblMessage WHERE MessageDate<?;");
		st.Bind(0,Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
		trans.Step(st);
	}

	// delete old message requests
	date=Poco::Timestamp();
	date-=Poco::Timespan(m_messagedownloadmaxdaysbackward,0,0,0,0);
	st=m_db->Prepare("DELETE FROM tblMessageRequests WHERE Day<?;");
	st.Bind(0,Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
	trans.Step(st);

	// delete old frost message requests
	date=Poco::Timestamp();
	date-=Poco::Timespan(m_frostmaxdaysbackward,0,0,0,0);
	st=m_db->Prepare("DELETE FROM tblFrostMessageRequests WHERE Day<?;");
	st.Bind(0,Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
	trans.Step(st);

	// delete tblIdentityTrust for local identities and identities that have been deleted
	m_db->Execute("DELETE FROM tblIdentityTrust WHERE LocalIdentityID NOT IN (SELECT LocalIdentityID FROM tblLocalIdentity);");
	m_db->Execute("DELETE FROM tblIdentityTrust WHERE IdentityID NOT IN (SELECT IdentityID FROM tblIdentity);");

	// cap failure count
	m_db->Execute("UPDATE tblIdentity SET FailureCount=(SELECT OptionValue FROM tblOption WHERE Option='MaxFailureCount') WHERE FailureCount>(SELECT OptionValue FROM tblOption WHERE Option='MaxFailureCount');");
	// reduce failure count for each identity
	m_db->Execute("UPDATE tblIdentity SET FailureCount=0 WHERE FailureCount<(SELECT OptionValue FROM tblOption WHERE Option='FailureCountReduction');");
	m_db->Execute("UPDATE tblIdentity SET FailureCount=FailureCount-(SELECT OptionValue FROM tblOption WHERE Option='FailureCountReduction') WHERE FailureCount>=(SELECT OptionValue FROM tblOption WHERE Option='FailureCountReduction');");

	// delete null entries from tblMessageInserts
	date=Poco::Timestamp();
	st=m_db->Prepare("DELETE FROM tblMessageInserts WHERE MessageUUID IS NULL AND MessageXML IS NULL AND SendDate IS NULL AND Inserted='true' AND Day<?;");
	st.Bind(0,Poco::DateTimeFormatter::format(date,"%Y-%m-%d"));
	trans.Step(st);

	trans.Finalize(st);

	// If at least 2 days have passed without retrieving one of our own inserted messages, reset the date of the message insert so it will be inserted again.
	m_db->Execute("UPDATE tblMessageInserts SET Day=NULL, InsertIndex=NULL, Inserted='false' WHERE LENGTH(MessageXML)<=1000000 AND MessageUUID IN (SELECT tblMessageInserts.MessageUUID FROM tblMessageInserts LEFT JOIN tblMessage ON tblMessageInserts.MessageUUID=tblMessage.MessageUUID WHERE Inserted='true' AND SendDate>=(SELECT date('now','-' || (SELECT CASE WHEN OptionValue<=0 THEN 30 WHEN OptionValue>30 THEN 30 ELSE OptionValue END FROM tblOption WHERE Option='DeleteMessagesOlderThan') || ' days')) AND tblMessage.MessageUUID IS NULL AND tblMessageInserts.SendDate<date('now','-2 days'));");

	trans.Commit();
	if(trans.IsSuccessful()==false)
	{
		m_log->error("DBMaintenanceThread::Do1DayMaintenance transaction 3 failed with SQLite error:"+trans.GetLastErrorStr()+" SQL="+trans.GetErrorSQL());
	}

	// recount messages in each board
	m_db->Execute("UPDATE tblBoard SET MessageCount=(SELECT IFNULL(COUNT(*),0) FROM tblMessageBoard WHERE tblMessageBoard.BoardID=tblBoard.BoardID);");

	// remove FMS or WOT flags from identities that were never seen in 30 days
	m_db->Execute("UPDATE tblIdentity SET IsFMS=0 WHERE LastSeen IS NULL AND IsFMS=1 AND SolvedPuzzleCount=0 AND DateAdded<datetime('now','-30 days');");
	m_db->Execute("UPDATE tblIdentity SET IsWOT=0 WHERE WOTLastSeen IS NULL AND IsWOT=1 AND DateAdded<datetime('now','-30 days');");

	m_log->debug("PeriodicDBMaintenance::Do1DayMaintenance end");

}

void DBMaintenanceThread::run()
{
	m_log->debug("DBMaintenanceThread::run thread started.");

	try
	{

		LoadDatabase(m_log);
		Option option(m_db);
		std::string tempval("");

		m_deletemessagesolderthan=180;
		tempval="180";
		option.Get("DeleteMessagesOlderThan",tempval);
		StringFunctions::Convert(tempval,m_deletemessagesolderthan);

		m_messagedownloadmaxdaysbackward=5;
		tempval="5";
		option.Get("MessageDownloadMaxDaysBackward",tempval);
		StringFunctions::Convert(tempval,m_messagedownloadmaxdaysbackward);

		m_frostmaxdaysbackward=5;
		tempval="5";
		option.Get("FrostMessageMaxDaysBackward",tempval);
		StringFunctions::Convert(tempval,m_frostmaxdaysbackward);

		Poco::DateTime now;
		int i=0;

		do
		{
			now=Poco::Timestamp();

			/*
			if((m_last10minute+Poco::Timespan(0,0,10,0,0))<=now)
			{
				Do10MinuteMaintenance();
				m_last10minute=Poco::Timestamp();
			}
			*/
			if((m_last30minute+Poco::Timespan(0,0,30,0,0))<=now)
			{
				Do30MinuteMaintenance();
				m_last30minute=Poco::Timestamp();
			}
			if((m_last1hour+Poco::Timespan(0,1,0,0,0))<=now)
			{
				Do1HourMaintenance();
				m_last1hour=Poco::Timestamp();
			}
			if((m_last6hour+Poco::Timespan(0,6,0,0,0))<=now)
			{
				Do6HourMaintenance();
				m_last6hour=Poco::Timestamp();
			}
			if((m_last1day+Poco::Timespan(1,0,0,0,0))<=now)
			{
				Do1DayMaintenance();
				m_last1day=Poco::Timestamp();
			}

			i=0;
			while(i++<5 && !IsCancelled())
			{
				Poco::Thread::sleep(1000);
			}

		}while(!IsCancelled());
	}
	catch(SQLite3DB::Exception &e)
	{
		m_log->fatal("DBMaintenanceThread caught SQLite3DB::Exception "+e.what());
		((FMSApp *)&FMSApp::instance())->Terminate();
	}

	m_log->debug("DBMaintenanceThread::run thread exiting.");
}
