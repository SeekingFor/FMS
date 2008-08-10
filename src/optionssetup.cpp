#include "../include/optionssetup.h"
#include "../include/db/sqlite3db.h"

#include <Poco/Message.h>

#include <string>
#include <sstream>

void SetupDefaultOptions()
{
	// OptionValue should always be inserted as a string, even if the option really isn't a string - just to keep the field data type consistent

	std::ostringstream tempstr;	// must set tempstr to "" between db inserts
	SQLite3DB::DB *db=SQLite3DB::DB::Instance();
	SQLite3DB::Statement st=db->Prepare("INSERT INTO tblOption(Option,OptionValue) VALUES(?,?);");
	SQLite3DB::Statement upd=db->Prepare("UPDATE tblOption SET Section=?, SortOrder=?, ValidValues=?, OptionDescription=? WHERE Option=?;");
	int order=0;

	// LogLevel
	tempstr.str("");
	tempstr << Poco::Message::PRIO_DEBUG;
	st.Bind(0,"LogLevel");
	st.Bind(1,tempstr.str());
	st.Step();
	st.Reset();
	upd.Bind(0,"Program");
	upd.Bind(1,order++);
	upd.Bind(2,"1|1 - Fatal Errors|2|2 - Critical Errors|3|3 - Errors|4|4 - Warnings|5|5 - Notices|6|6 - Informational Messages|7|7 - Debug Messages|8|8 - Trace Messages");
	upd.Bind(3,"The maximum logging level that will be written to file.  Higher levels will include all messages from the previous levels.");
	upd.Bind(4,"LogLevel");
	upd.Step();
	upd.Reset();

	st.Bind(0,"VacuumOnStartup");
	st.Bind(1,"false");
	st.Step();
	st.Reset();
	upd.Bind(0,"Program");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"VACUUM the database every time FMS starts.  This will defragment the free space in the database and create a smaller database file.  Vacuuming the database can be CPU and disk intensive.");
	upd.Bind(4,"VacuumOnStartup");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MessageBase");
	st.Bind(1,"fms");
	st.Step();
	st.Reset();
	upd.Bind(0,"Program");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"A unique string shared by all clients who want to communicate with each other.  This should not be changed unless you want to create your own separate communications network.");
	upd.Bind(4,"MessageBase");
	upd.Step();
	upd.Reset();

	st.Bind(0,"FMSVersionKey");
	st.Bind(1,"USK@0npnMrqZNKRCRoGojZV93UNHCMN-6UU3rRSAmP6jNLE,~BG-edFtdCC1cSH4O3BWdeIYa8Sw5DfyrSV-TKdO5ec,AQACAAE/fmsversion/");
	st.Step();
	st.Reset();
	upd.Bind(0,"Program");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The USK key which contains information about the latest version of FMS.");
	upd.Bind(4,"FMSVersionKey");
	upd.Step();
	upd.Reset();

	st.Bind(0,"FMSVersionEdition");
	st.Bind(1,"16");
	st.Step();
	st.Reset();
	upd.Bind(0,"Program");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The latest found edition of the FMS version USK.");
	upd.Bind(4,"FMSVersionEdition");
	upd.Step();
	upd.Reset();

	// StartNNTP
	st.Bind(0,"StartNNTP");
	st.Bind(1,"true");
	st.Step();
	st.Reset();
	upd.Bind(0,"NNTP Server");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Start NNTP server.");
	upd.Bind(4,"StartNNTP");
	upd.Step();
	upd.Reset();

	// NNTPListenPort
	st.Bind(0,"NNTPListenPort");
	st.Bind(1,"1119");
	st.Step();
	st.Reset();
	upd.Bind(0,"NNTP Server");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The port that the NNTP service will listen for incoming connections.");
	upd.Bind(4,"NNTPListenPort");
	upd.Step();
	upd.Reset();

	// NNTPBindAddresses
	st.Bind(0,"NNTPBindAddresses");
	st.Bind(1,"localhost,127.0.0.1");
	st.Step();
	st.Reset();
	upd.Bind(0,"NNTP Server");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"A comma separated list of valid IPv4 or IPv6 addresses/hostnames that the NNTP service will try to bind to.");
	upd.Bind(4,"NNTPBindAddresses");
	upd.Step();
	upd.Reset();

	st.Bind(0,"NNTPAllowPost");
	st.Bind(1,"true");
	st.Step();
	st.Reset();
	upd.Bind(0,"NNTP Server");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Allow posting messages from NNTP.  Setting to false will make the newsgroups read only.");
	upd.Bind(4,"NNTPAllowPost");
	upd.Step();
	upd.Reset();

	st.Bind(0,"StartHTTP");
	st.Bind(1,"true");
	st.Step();
	st.Reset();
	upd.Bind(0,"HTTP Server");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Start HTTP server.  WARNING: If you turn this off, you won't be able to access the administration pages.");
	upd.Bind(4,"StartHTTP");
	upd.Step();
	upd.Reset();

	st.Bind(0,"HTTPListenPort");
	st.Bind(1,"8080");
	st.Step();
	st.Reset();
	upd.Bind(0,"HTTP Server");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Port HTTP server will listen on.");
	upd.Bind(4,"HTTPListenPort");
	upd.Step();
	upd.Reset();

	st.Bind(0,"HTTPAccessControl");
	st.Bind(1,"-0.0.0.0/0,+127.0.0.1");
	st.Step();
	st.Reset();
	upd.Bind(0,"HTTP Server");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Comma separated list of addresses and/or subnet masks that are allowed access to the administration pages.  Default is localhost only. + allows a host, - denies a host.");
	upd.Bind(4,"HTTPAccessControl");
	upd.Step();
	upd.Reset();

	// StartFreenetUpdater
	st.Bind(0,"StartFreenetUpdater");
	st.Bind(1,"true");
	st.Step();
	st.Reset();
	upd.Bind(0,"Freenet Connection");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Set to true to start the Freenet Updater thread and connect to Freenet.  Set to false to prevent communication with Freenet.");
	upd.Bind(4,"StartFreenetUpdater");
	upd.Step();
	upd.Reset();

	// FCPHost
	st.Bind(0,"FCPHost");
	st.Bind(1,"127.0.0.1");
	st.Step();
	st.Reset();
	upd.Bind(0,"Freenet Connection");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Host name or address of Freenet node.");
	upd.Bind(4,"FCPHost");
	upd.Step();
	upd.Reset();

	// FCPPort
	st.Bind(0,"FCPPort");
	st.Bind(1,"9481");
	st.Step();
	st.Reset();
	upd.Bind(0,"Freenet Connection");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The port that Freenet is listening for FCP connections on.");
	upd.Bind(4,"FCPPort");
	upd.Step();
	upd.Reset();

	st.Bind(0,"FProxyPort");
	st.Bind(1,"8888");
	st.Step();
	st.Reset();
	upd.Bind(0,"Freenet Connection");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The port that Freenet is listening for http connections on.");
	upd.Bind(4,"FProxyPort");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MaxIdentityRequests");
	st.Bind(1,"5");
	st.Step();
	st.Reset();
	upd.Bind(0,"Requests");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Maximum number of concurrent requests for new Identity xml files");
	upd.Bind(4,"MaxIdentityRequests");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MaxIdentityIntroductionRequests");
	st.Bind(1,"5");
	st.Step();
	st.Reset();
	upd.Bind(0,"Requests");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Maximum number of concurrent identities requesting IdentityIntroduction xml files.  Each identity may have multiple requests pending.");
	upd.Bind(4,"MaxIdentityIntroductionRequests");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MaxIntroductionPuzzleRequests");
	st.Bind(1,"5");
	st.Step();
	st.Reset();
	upd.Bind(0,"Requests");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Maximum number of concurrent requests for new IntroductionPuzzle xml files");
	upd.Bind(4,"MaxIntroductionPuzzleRequests");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MaxTrustListRequests");
	st.Bind(1,"5");
	st.Step();
	st.Reset();
	upd.Bind(0,"Requests");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Maximum number of concurrent requests for new Trust Lists");
	upd.Bind(4,"MaxTrustListRequests");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MaxMessageListRequests");
	st.Bind(1,"5");
	st.Step();
	st.Reset();
	upd.Bind(0,"Requests");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Maximum number of concurrent requests for new Message Lists");
	upd.Bind(4,"MaxMessageListRequests");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MaxMessageRequests");
	st.Bind(1,"20");
	st.Step();
	st.Reset();
	upd.Bind(0,"Requests");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Maximum number of concurrent requests for new Messages");
	upd.Bind(4,"MaxMessageRequests");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MaxBoardListRequests");
	st.Bind(1,"5");
	st.Step();
	st.Reset();
	upd.Bind(0,"Requests");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The maximum number of concurrent requests for new Board Lists.  Set to 0 to disable.");
	upd.Bind(4,"MaxBoardListRequests");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MinLocalMessageTrust");
	st.Bind(1,"50");
	st.Step();
	st.Reset();
	upd.Bind(0,"Trust");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Specifies a local message trust level that a peer must have before its messages will be downloaded.");
	upd.Bind(4,"MinLocalMessageTrust");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MinPeerMessageTrust");
	st.Bind(1,"30");
	st.Step();
	st.Reset();
	upd.Bind(0,"Trust");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Specifies a peer message trust level that a peer must have before its messages will be downloaded.");
	upd.Bind(4,"MinPeerMessageTrust");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MinLocalTrustListTrust");
	st.Bind(1,"50");
	st.Step();
	st.Reset();
	upd.Bind(0,"Trust");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Specifies a local trust list trust level that a peer must have before its trust list will be included in the weighted average.  Any peers below this number will be excluded from the results.");
	upd.Bind(4,"MinLocalTrustListTrust");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MinPeerTrustListTrust");
	st.Bind(1,"30");
	st.Step();
	st.Reset();
	upd.Bind(0,"Trust");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Specifies a peer trust list trust level that a peer must have before its trust list will be included in the weighted average.  Any peers below this number will be excluded from the results.");
	upd.Bind(4,"MinPeerTrustListTrust");
	upd.Step();
	upd.Reset();

	st.Bind(0,"LocalTrustOverridesPeerTrust");
	st.Bind(1,"false");
	st.Step();
	st.Reset();
	upd.Bind(0,"Trust");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Set to true if you want your local trust levels to override the peer levels when determining which identities you will poll.");
	upd.Bind(4,"LocalTrustOverridesPeerTrust");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MessageDownloadMaxDaysBackward");
	st.Bind(1,"5");
	st.Step();
	st.Reset();
	upd.Bind(0,"Messages");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The maximum number of days backward that messages will be downloaded from each identity");
	upd.Bind(4,"MessageDownloadMaxDaysBackward");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MessageListDaysBackward");
	st.Bind(1,"5");
	st.Step();
	st.Reset();
	upd.Bind(0,"Messages");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The number of days backward that messages you have inserted will appear in your MessageLists");
	upd.Bind(4,"MessageListDaysBackward");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MaxPeerMessagesPerDay");
	st.Bind(1,"200");
	st.Step();
	st.Reset();
	upd.Bind(0,"Messages");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The maximum number of messages you will download from each peer on a given day.");
	upd.Bind(4,"MaxPeerMessagesPerDay");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MaxBoardsPerMessage");
	st.Bind(1,"8");
	st.Step();
	st.Reset();
	upd.Bind(0,"Messages");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The maximum number of boards a received message may be sent to.  Boards over this limit will be ignored.");
	upd.Bind(4,"MaxBoardsPerMessage");
	upd.Step();
	upd.Reset();

	st.Bind(0,"SaveMessagesFromNewBoards");
	st.Bind(1,"true");
	st.Step();
	st.Reset();
	upd.Bind(0,"Messages");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Set to true to automatically save messages posted to new boards.  Set to false to ignore messages to new boards.");
	upd.Bind(4,"SaveMessagesFromNewBoards");
	upd.Step();
	upd.Reset();

	st.Bind(0,"ChangeMessageTrustOnReply");
	st.Bind(1,"0");
	st.Step();
	st.Reset();
	upd.Bind(0,"Messages");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"How much the local message trust level of an identity should change when you reply to one of their messages.");
	upd.Bind(4,"ChangeMessageTrustOnReply");
	upd.Step();
	upd.Reset();

	st.Bind(0,"AddNewPostFromIdentities");
	st.Bind(1,"false");
	st.Step();
	st.Reset();
	upd.Bind(0,"Messages");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Set to true to automatically create new identities when you send a message using a new name.  If you set this to false, posting messages will fail until you manually create the identity.");
	upd.Bind(4,"AddNewPostFromIdentities");
	upd.Step();
	upd.Reset();

	st.Bind(0,"DeleteMessagesOlderThan");
	st.Bind(1,"180");
	st.Step();
	st.Reset();
	upd.Bind(0,"Messages");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Automatically delete messages older than this many days.");
	upd.Bind(4,"DeleteMessagesOlderThan");
	upd.Step();
	upd.Reset();

}
