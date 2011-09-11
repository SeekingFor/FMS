#include "../include/optionssetup.h"
#include "../include/db/sqlite3db.h"
#include "../include/global.h"

#include <Poco/Message.h>

#include <string>
#include <sstream>

void SetupDefaultOptions(SQLite3DB::DB *db)
{
	// OptionValue should always be inserted as a string, even if the option really isn't a string - just to keep the field data type consistent

	db->Execute("BEGIN;");

	std::ostringstream tempstr;	// must set tempstr to "" between db inserts
	SQLite3DB::Statement st=db->Prepare("INSERT INTO tblOption(Option,OptionValue) VALUES(?,?);");
	SQLite3DB::Statement upd=db->Prepare("UPDATE tblOption SET Section=?, SortOrder=?, ValidValues=?, OptionDescription=?, DisplayType=?, DisplayParam1=?, DisplayParam2=?, Mode=? WHERE Option=?;");
	int order=0;

	// Language
	st.Bind(0,"Language");
	st.Bind(1,"english.prop");
	st.Step();
	st.Reset();
	upd.Bind(0,"Program");
	upd.Bind(1,order++);
	upd.Bind(2,"english.prop|English|french.prop|French|swedish.prop|Swedish|unlisted.prop|Unlisted/Other");
	upd.Bind(3,"Select program language.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"Language");
	upd.Step();
	upd.Reset();

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
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"LogLevel");
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
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"VacuumOnStartup");
	upd.Step();
	upd.Reset();

	st.Bind(0,"ProfileDBQueries");
	st.Bind(1,"false");
	st.Step();
	st.Reset();
	upd.Bind(0,"Program");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Profiles all database queries and periodically writes the stats to the fms log file.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"ProfileDBQueries");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"MessageBase");
	upd.Step();
	upd.Reset();

	st.Bind(0,"FMSVersionKey");
	st.Bind(1,"USK@0npnMrqZNKRCRoGojZV93UNHCMN-6UU3rRSAmP6jNLE,~BG-edFtdCC1cSH4O3BWdeIYa8Sw5DfyrSV-TKdO5ec,AQACAAE/fmsversion/");
	st.Step();
	st.Reset();
	upd.Bind(0,"Program");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The USK which contains information about the latest version of FMS.");
	upd.Bind(4,"textbox");
	upd.Bind(5,"80");
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"FMSVersionKey");
	upd.Step();
	upd.Reset();

	st.Bind(0,"FMSVersionEdition");
	st.Bind(1,FMS_VERSION_EDITION);
	st.Step();
	st.Reset();
	upd.Bind(0,"Program");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The latest found edition of the FMS version USK.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"FMSVersionEdition");
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
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"StartNNTP");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"NNTPListenPort");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"NNTPBindAddresses");
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
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"NNTPAllowPost");
	upd.Step();
	upd.Reset();

	st.Bind(0,"NNTPAllGroups");
	st.Bind(1,"false");
	st.Step();
	st.Reset();
	upd.Bind(0,"NNTP Server");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Show all groups, even those you are not saving messages in, when accessing the group list via NNTP.  When a group is accessed in a manner other than through the group list, the switch to save messages to that group will be turned on automatically.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"NNTPAllGroups");
	upd.Step();
	upd.Reset();

	st.Bind(0,"UniqueBoardMessageIDs");
	st.Bind(1,"true");
	st.Step();
	st.Reset();
	upd.Bind(0,"NNTP Server");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Use per board unique message ids for each message.  Turning this off will use global message ids unique across all boards.  Changing this value either way will require clearing any cache your newsreader keeps.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"UniqueBoardMessageIDs");
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
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"StartHTTP");
	upd.Step();
	upd.Reset();

	st.Bind(0,"HTTPBindAddress");
	st.Bind(1,"0.0.0.0");
	st.Step();
	st.Reset();
	upd.Bind(0,"HTTP Server");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The IP address or hostname that the HTTP server will bind to.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"HTTPBindAddress");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"HTTPListenPort");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"HTTPAccessControl");
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
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"StartFreenetUpdater");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"FCPHost");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"FCPPort");
	upd.Step();
	upd.Reset();

	st.Bind(0,"FProxyHost");
	st.Bind(1,"127.0.0.1");
	st.Step();
	st.Reset();
	upd.Bind(0,"Freenet Connection");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Host name or address of FProxy.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"FProxyHost");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"FProxyPort");
	upd.Step();
	upd.Reset();

	st.Bind(0,"FProxyProtocol");
	st.Bind(1,"http");
	st.Step();
	st.Reset();
	upd.Bind(0,"Freenet Connection");
	upd.Bind(1,order++);
	upd.Bind(2,"http|http|https|https");
	upd.Bind(3,"FProxy protocol.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"FProxyProtocol");
	upd.Step();
	upd.Reset();

	st.Bind(0,"FCPTimeout");
	st.Bind(1,"600");
	st.Step();
	st.Reset();
	upd.Bind(0,"Freenet Connection");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"FCP Timeout in seconds.  If the connected Freenet node doesn't send any data to FMS in this this time period, FMS will try to reconnect to the node.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"FCPTimeout");
	upd.Step();
	upd.Reset();

	st.Bind(0,"DefaultRequestPriorityClass");
	st.Bind(1,"2");
	st.Step();
	st.Reset();
	upd.Bind(0,"Freenet Connection");
	upd.Bind(1,order++);
	upd.Bind(2,"0|0 - Emergency|1|1 - Very High|2|2 - High|3|3 - Medium|4|4 - Low|5|5 - Very Low|6|6 - Will Never Finish");
	upd.Bind(3,"The default PriorityClass for requests.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"DefaultRequestPriorityClass");
	upd.Step();
	upd.Reset();

	st.Bind(0,"DefaultInsertPriorityClass");
	st.Bind(1,"2");
	st.Step();
	st.Reset();
	upd.Bind(0,"Freenet Connection");
	upd.Bind(1,order++);
	upd.Bind(2,"0|0 - Emergency|1|1 - Very High|2|2 - High|3|3 - Medium|4|4 - Low|5|5 - Very Low|6|6 - Will Never Finish");
	upd.Bind(3,"The default PriorityClass for inserts.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"DefaultInsertPriorityClass");
	upd.Step();
	upd.Reset();

	st.Bind(0,"ForumDetectLinks");
	st.Bind(1,"true");
	st.Step();
	st.Reset();
	upd.Bind(0,"Forum");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Attempt to detect links to CHKs in plain text messages.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"ForumDetectLinks");
	upd.Step();
	upd.Reset();

	st.Bind(0,"ForumShowSmilies");
	st.Bind(1,"false");
	st.Step();
	st.Reset();
	upd.Bind(0,"Forum");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Change plain text emoticons into smiley images.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"ForumShowSmilies");
	upd.Step();
	upd.Reset();

	st.Bind(0,"ForumShowSignatures");
	st.Bind(1,"true");
	st.Step();
	st.Reset();
	upd.Bind(0,"Forum");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Show signatures of post authors.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"ForumShowSignatures");
	upd.Step();
	upd.Reset();

	st.Bind(0,"ForumShowAvatars");
	st.Bind(1,"false");
	st.Step();
	st.Reset();
	upd.Bind(0,"Forum");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Show system generated avatars of post authors.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"ForumShowAvatars");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"MaxIdentityRequests");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"MaxIdentityIntroductionRequests");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"MaxIntroductionPuzzleRequests");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"MaxTrustListRequests");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"MaxMessageListRequests");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MaxOldMessageListRequests");
	st.Bind(1,"1");
	st.Step();
	st.Reset();
	upd.Bind(0,"Requests");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Maximum number of concurrent requests for old Message Lists.  You can temporarily set this to a higher value if you are catching up on old messages from many days ago.  After you have downloaded the old messages, you may set this back to a low value.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"MaxOldMessageListRequests");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"MaxMessageRequests");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"MaxBoardListRequests");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MaxFailureCount");
	st.Bind(1,"1000");
	st.Step();
	st.Reset();
	upd.Bind(0,"Requests");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The maximum number of failed message requests an identity must accumulate before you will completely ignore an identity.  Request failures can happen even under the best circumstances, and may accumulate rapidly, so it is best to keep this at a high level to avoid false positives.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"MaxFailureCount");
	upd.Step();
	upd.Reset();

	st.Bind(0,"FailureCountReduction");
	st.Bind(1,"500");
	st.Step();
	st.Reset();
	upd.Bind(0,"Requests");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Each identity's failure count will be reduced by this amount every day.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"FailureCountReduction");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"MinLocalMessageTrust");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"MinPeerMessageTrust");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"MinLocalTrustListTrust");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"MinPeerTrustListTrust");
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
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"LocalTrustOverridesPeerTrust");
	upd.Step();
	upd.Reset();

	st.Bind(0,"DownloadTrustListWhenNull");
	st.Bind(1,"true");
	st.Step();
	st.Reset();
	upd.Bind(0,"Trust");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Download trust lists from identities that you have not assigned trust list trust to.  Any new identities found in these trust lists will not be added to your known identities list.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"DownloadTrustListWhenNull");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"MessageDownloadMaxDaysBackward");
	upd.Step();
	upd.Reset();

	st.Bind(0,"MessageListDaysBackward");
	st.Bind(1,"10");
	st.Step();
	st.Reset();
	upd.Bind(0,"Messages");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The number of days backward that messages you have inserted will appear in your Message Lists");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"MessageListDaysBackward");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"MaxPeerMessagesPerDay");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"MaxBoardsPerMessage");
	upd.Step();
	upd.Reset();

	st.Bind(0,"SaveMessagesFromNewBoards");
	st.Bind(1,"true");
	st.Step();
	st.Reset();
	upd.Bind(0,"Messages");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Set to true to automatically create boards and save messages posted to them if you currently don't know about the board.  Set to false if you don't want messages posted only to new boards automatically added.  Boards from cross-posted messages are still added.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"SaveMessagesFromNewBoards");
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
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"ChangeMessageTrustOnReply");
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
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"AddNewPostFromIdentities");
	upd.Step();
	upd.Reset();

	st.Bind(0,"DeleteMessagesOlderThan");
	st.Bind(1,"180");
	st.Step();
	st.Reset();
	upd.Bind(0,"Messages");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Automatically delete messages older than this many days.  Use -1 to keep messages forever.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"DeleteMessagesOlderThan");
	upd.Step();
	upd.Reset();

	st.Bind(0,"AttachmentKeyType");
	st.Bind(1,"CHK@");
	st.Step();
	st.Reset();
	upd.Bind(0,"Messages");
	upd.Bind(1,order++);
	upd.Bind(2,"CHK@|Canonical key|SSK@|Random key");
	upd.Bind(3,"Key type used for inserting attachment. Canonical key (CHK@): This will always produce the same key for the same file, so is convenient for filesharing. However, if the bad guys can predict what files you are going to insert, they may be able to use this to trace you a lot more easily. Random key (SSK@). This is much safer than the first option, but the key will be different every time you or somebody else inserts the key. Use this if you are the original source of some sensitive data.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"AttachmentKeyType");
	upd.Step();
	upd.Reset();

#ifdef FROST_SUPPORT

	st.Bind(0,"DownloadFrostMessages");
	st.Bind(1,"false");
	st.Step();
	st.Reset();
	upd.Bind(0,"Frost Support");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Enable downloading of Frost messages.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"DownloadFrostMessages");
	upd.Step();
	upd.Reset();

	st.Bind(0,"FrostMessageBase");
	st.Bind(1,"news");
	st.Step();
	st.Reset();
	upd.Bind(0,"Frost Support");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"A unique string used by Frost clients who want to communicate with each other.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"FrostMessageBase");
	upd.Step();
	upd.Reset();

	st.Bind(0,"FrostBoardPrefix");
	st.Bind(1,"frost.");
	st.Step();
	st.Reset();
	upd.Bind(0,"Frost Support");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"Messages to boards defined in FMS with this prefix will be downloaded from Frost.  You must manually add boards with this prefix to FMS to download Frost messages from those boards.  The prefix is removed to determine the name of the Frost board to download messages from.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"advanced");
	upd.Bind(8,"FrostBoardPrefix");
	upd.Step();
	upd.Reset();

	st.Bind(0,"FrostSaveAnonymousMessages");
	st.Bind(1,"true");
	st.Step();
	st.Reset();
	upd.Bind(0,"Frost Support");
	upd.Bind(1,order++);
	upd.Bind(2,"true|true|false|false");
	upd.Bind(3,"Save Frost messages posted by Anonymous authors.");
	upd.Bind(4,"select");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"FrostSaveAnonymousMessages");
	upd.Step();
	upd.Reset();

	st.Bind(0,"FrostMessageMaxDaysBackward");
	st.Bind(1,"5");
	st.Step();
	st.Reset();
	upd.Bind(0,"Frost Support");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The maximum number of days backward that Frost messages will be downloaded.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"FrostMessageMaxDaysBackward");
	upd.Step();
	upd.Reset();

	st.Bind(0,"FrostMaxMessageRequests");
	st.Bind(1,"20");
	st.Step();
	st.Reset();
	upd.Bind(0,"Frost Support");
	upd.Bind(1,order++);
	upd.Bind(2);
	upd.Bind(3,"The maximum number of concurrent requests for new Frost messages.");
	upd.Bind(4,"textbox");
	upd.Bind(5);
	upd.Bind(6);
	upd.Bind(7,"simple");
	upd.Bind(8,"FrostMaxMessageRequests");
	upd.Step();
	upd.Reset();

#endif	// FROST_SUPPORT

	db->Execute("COMMIT;");

}
