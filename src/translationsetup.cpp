#include "../include/translationsetup.h"
#include "../include/global.h"

#include <Poco/Path.h>
#include <Poco/File.h>

void SetupTranslation(const std::string &languagefile)
{
	Poco::Path tdir;
	StringTranslation *st=Translation.get();

	st->SetDefault("web.option.true","true");
	st->SetDefault("web.option.false","false");

	st->SetDefault("web.navlink.links","Links");
	st->SetDefault("web.navlink.home","Home");
	st->SetDefault("web.navlink.options","Options");
	st->SetDefault("web.navlink.createidentity","Create Identity");
	st->SetDefault("web.navlink.localidentities","Local Identities");
	st->SetDefault("web.navlink.announceidentity","Announce Identity");
	st->SetDefault("web.navlink.addpeer","Add Peer");
	st->SetDefault("web.navlink.peermaintenance","Peer Maintenance");
	st->SetDefault("web.navlink.peertrust","Peer Trust");
	st->SetDefault("web.navlink.boardmaintenance","Board Maintenance");
	st->SetDefault("web.navlink.controlboards","Control Boards");
	st->SetDefault("web.navlink.insertedfiles","Inserted Files");
	st->SetDefault("web.navlink.browseforums","Browse Forums");
	st->SetDefault("web.navlink.browsefreenet","Browse Freenet");

	st->SetDefault("web.page.home.title","Home");
	st->SetDefault("web.page.home.fmsversion","FMS version");
	st->SetDefault("web.page.home.oldversion","You are running an old version of FMS.  Please update here:");
	st->SetDefault("web.page.home.oldreleaseinfo","You can see the release info");
	st->SetDefault("web.page.home.releaseinfohere","here");
	st->SetDefault("web.page.home.releaseinfo","Release info");
	st->SetDefault("web.page.home.checknewreleases","Check for new versions at the");
	st->SetDefault("web.page.home.fmsfreesite","FMS Freesite");
	st->SetDefault("web.page.home.admininstructions","Use these pages to administer your FMS installation.");
	st->SetDefault("web.page.home.messageswaiting","Messages waiting to be inserted:");
	st->SetDefault("web.page.home.showmessageswaiting","show messages");
	st->SetDefault("web.page.home.fileswaiting","Files waiting to be inserted:");
	st->SetDefault("web.page.home.shutdownfms","Shutdown FMS");

	st->SetDefault("web.page.options.title","Options");
	st->SetDefault("web.page.options.simple","Simple");
	st->SetDefault("web.page.options.advanced","Advanced");
	st->SetDefault("web.page.options.save","Save");
	st->SetDefault("web.page.options.requirerestart","Most options require a restart of FMS to take effect");

	st->SetDefault("web.option.section.Program","Program");
	st->SetDefault("web.option.section.NNTP Server","NNTP Server");
	st->SetDefault("web.option.section.HTTP Server","HTTP Server");
	st->SetDefault("web.option.section.Freenet Connection","Freenet Connection");
	st->SetDefault("web.option.section.Forum","Forum");
	st->SetDefault("web.option.section.Requests","Requests");
	st->SetDefault("web.option.section.Trust","Trust");
	st->SetDefault("web.option.section.Messages","Messages");
	st->SetDefault("web.option.section.Frost Support","Frost Support");

	st->SetDefault("web.option.Language.description","Select program language.");
	st->SetDefault("web.option.Language.english.prop","English");
	st->SetDefault("web.option.Language.spanish.prop","Spanish");
	st->SetDefault("web.option.Language.russian.prop","Russian");
	st->SetDefault("web.option.Language.french.prop","French (Français)");
	st->SetDefault("web.option.Language.unlisted.prop","Unlisted/Other");
	st->SetDefault("web.option.LogLevel.description","The maximum logging level that will be written to file.  Higher levels will include all messages from the previous levels.");
	st->SetDefault("web.option.LogLevel.1","1 - Fatal Errors");
	st->SetDefault("web.option.LogLevel.2","2 - Critical Errors");
	st->SetDefault("web.option.LogLevel.3","3 - Errors");
	st->SetDefault("web.option.LogLevel.4","4 - Warnings");
	st->SetDefault("web.option.LogLevel.5","5 - Notices");
	st->SetDefault("web.option.LogLevel.6","6 - Informational Messages");
	st->SetDefault("web.option.LogLevel.7","7 - Debug Messages");
	st->SetDefault("web.option.LogLevel.8","8 - Trace Messages");
	st->SetDefault("web.option.VacuumOnStartup.description","VACUUM the database every time FMS starts.  This will defragment the free space in the database and create a smaller database file.  Vacuuming the database can be CPU and disk intensive.");
	st->SetDefault("web.option.MessageBase.description","A unique string shared by all clients who want to communicate with each other.  This should not be changed unless you want to create your own separate communications network.");
	st->SetDefault("web.option.FMSVersionKey.description","The USK which contains information about the latest version of FMS.");
	st->SetDefault("web.option.FMSVersionEdition.description","The latest found edition of the FMS version USK.");
	st->SetDefault("web.option.StartNNTP.description","Start NNTP server.");
	st->SetDefault("web.option.NNTPListenPort.description","The port that the NNTP service will listen for incoming connections.");
	st->SetDefault("web.option.NNTPBindAddresses.description","A comma separated list of valid IPv4 or IPv6 addresses/hostnames that the NNTP service will try to bind to.");
	st->SetDefault("web.option.NNTPAllowPost.description","Allow posting messages from NNTP.  Setting to false will make the newsgroups read only.");
	st->SetDefault("web.option.NNTPAllGroups.description","Show all groups, even those you are not saving messages in, when accessing the group list via NNTP.  When a group is accessed in a manner other than through the group list, the switch to save messages to that group will be turned on automatically.");
	st->SetDefault("web.option.UniqueBoardMessageIDs.description","Use per board unique message ids for each message.  Turning this off will use global message ids unique across all boards.  Changing this value either way will require clearing any cache your newsreader keeps.");
	st->SetDefault("web.option.StartHTTP.description","Start HTTP server.  WARNING: If you turn this off, you won't be able to access the administration pages.");
	st->SetDefault("web.option.HTTPBindAddress.description","The IP address or hostname that the HTTP server will bind to.");
	st->SetDefault("web.option.HTTPListenPort.description","Port HTTP server will listen on.");
	st->SetDefault("web.option.HTTPAccessControl.description","Comma separated list of addresses and/or subnet masks that are allowed access to the administration pages.  Default is localhost only. + allows a host, - denies a host.");
	st->SetDefault("web.option.StartFreenetUpdater.description","Set to true to start the Freenet Updater thread and connect to Freenet.  Set to false to prevent communication with Freenet.");
	st->SetDefault("web.option.FCPHost.description","Host name or address of Freenet node.");
	st->SetDefault("web.option.FCPPort.description","The port that Freenet is listening for FCP connections on.");
	st->SetDefault("web.option.FProxyPort.description","The port that Freenet is listening for http connections on.");
	st->SetDefault("web.option.FCPTimeout.description","FCP Timeout in seconds.  If the connected Freenet node doesn't send any data to FMS in this this time period, FMS will try to reconnect to the node.");
	st->SetDefault("web.option.DefaultRequestPriorityClass.description","The default PriorityClass for requests.");
	st->SetDefault("web.option.DefaultRequestPriorityClass.0","0 - Emergency");
	st->SetDefault("web.option.DefaultRequestPriorityClass.1","1 - Very High");
	st->SetDefault("web.option.DefaultRequestPriorityClass.2","2 - High");
	st->SetDefault("web.option.DefaultRequestPriorityClass.3","3 - Medium");
	st->SetDefault("web.option.DefaultRequestPriorityClass.4","4 - Low");
	st->SetDefault("web.option.DefaultRequestPriorityClass.5","5 - Very Low");
	st->SetDefault("web.option.DefaultRequestPriorityClass.6","6 - Will Never Finish");
	st->SetDefault("web.option.DefaultInsertPriorityClass.description","The default PriorityClass for inserts.");
	st->SetDefault("web.option.DefaultInsertPriorityClass.0","0 - Emergency");
	st->SetDefault("web.option.DefaultInsertPriorityClass.1","1 - Very High");
	st->SetDefault("web.option.DefaultInsertPriorityClass.2","2 - High");
	st->SetDefault("web.option.DefaultInsertPriorityClass.3","3 - Medium");
	st->SetDefault("web.option.DefaultInsertPriorityClass.4","4 - Low");
	st->SetDefault("web.option.DefaultInsertPriorityClass.5","5 - Very Low");
	st->SetDefault("web.option.DefaultInsertPriorityClass.6","6 - Will Never Finish");
	st->SetDefault("web.option.ForumDetectLinks.description","Attempt to detect links to CHKs in plain text messages.");
	st->SetDefault("web.option.ForumShowSmilies.description","Change plain text emoticons into smiley images.");
	st->SetDefault("web.option.MaxIdentityRequests.description","Maximum number of concurrent requests for new Identity xml files");
	st->SetDefault("web.option.MaxIdentityIntroductionRequests.description","Maximum number of concurrent identities requesting IdentityIntroduction xml files.  Each identity may have multiple requests pending.");
	st->SetDefault("web.option.MaxIntroductionPuzzleRequests.description","Maximum number of concurrent requests for new IntroductionPuzzle xml files");
	st->SetDefault("web.option.MaxTrustListRequests.description","Maximum number of concurrent requests for new Trust Lists");
	st->SetDefault("web.option.MaxMessageListRequests.description","Maximum number of concurrent requests for new Message Lists");
	st->SetDefault("web.option.MaxOldMessageListRequests.description","Maximum number of concurrent requests for old Message Lists.  You can temporarily set this to a higher value if you are catching up on old messages from many days ago.  After you have downloaded the old messages, you may set this back to a low value.");
	st->SetDefault("web.option.MaxMessageRequests.description","Maximum number of concurrent requests for new Messages");
	st->SetDefault("web.option.MaxBoardListRequests.description","The maximum number of concurrent requests for new Board Lists.  Set to 0 to disable.");
	st->SetDefault("web.option.MaxFailureCount.description","The maximum number of failed message requests an identity must accumulate before you will completely ignore an identity.  Request failures can happen even under the best circumstances, and may accumulate rapidly, so it is best to keep this at a high level to avoid false positives.");
	st->SetDefault("web.option.FailureCountReduction.description","Each identity's failure count will be reduced by this amount every day.");
	st->SetDefault("web.option.MinLocalMessageTrust.description","Specifies a local message trust level that a peer must have before its messages will be downloaded.");
	st->SetDefault("web.option.MinPeerMessageTrust.description","Specifies a peer message trust level that a peer must have before its messages will be downloaded.");
	st->SetDefault("web.option.MinLocalTrustListTrust.description","Specifies a local trust list trust level that a peer must have before its trust list will be included in the weighted average.  Any peers below this number will be excluded from the results.");
	st->SetDefault("web.option.MinPeerTrustListTrust.description","Specifies a peer trust list trust level that a peer must have before its trust list will be included in the weighted average.  Any peers below this number will be excluded from the results.");
	st->SetDefault("web.option.LocalTrustOverridesPeerTrust.description","Set to true if you want your local trust levels to override the peer levels when determining which identities you will poll.");
	st->SetDefault("web.option.MessageDownloadMaxDaysBackward.description","The maximum number of days backward that messages will be downloaded from each identity");
	st->SetDefault("web.option.MessageListDaysBackward.description","The number of days backward that messages you have inserted will appear in your Message Lists");
	st->SetDefault("web.option.MaxPeerMessagesPerDay.description","The maximum number of messages you will download from each peer on a given day.");
	st->SetDefault("web.option.MaxBoardsPerMessage.description","The maximum number of boards a received message may be sent to.  Boards over this limit will be ignored.");
	st->SetDefault("web.option.SaveMessagesFromNewBoards.description","Set to true to automatically save messages posted to new boards.  Set to false to ignore messages to new boards.");
	st->SetDefault("web.option.ChangeMessageTrustOnReply.description","How much the local message trust level of an identity should change when you reply to one of their messages.");
	st->SetDefault("web.option.AddNewPostFromIdentities.description","Set to true to automatically create new identities when you send a message using a new name.  If you set this to false, posting messages will fail until you manually create the identity.");
	st->SetDefault("web.option.DeleteMessagesOlderThan.description","Automatically delete messages older than this many days.  Use -1 to keep messages forever.");
	st->SetDefault("web.option.DownloadFrostMessages.description","Enable downloading of Frost messages.");
	st->SetDefault("web.option.FrostMessageBase.description","A unique string used by Frost clients who want to communicate with each other.");
	st->SetDefault("web.option.FrostBoardPrefix.description","Messages to boards defined in FMS with this prefix will be downloaded from Frost.  You must manually add boards with this prefix to FMS to download Frost messages from those boards.  The prefix is removed to determine the name of the Frost board to download messages from.");
	st->SetDefault("web.option.FrostSaveAnonymousMessages.description","Save Frost messages posted by Anonymous authors.");
	st->SetDefault("web.option.FrostMessageMaxDaysBackward.description","The maximum number of days backward that Frost messages will be downloaded.");
	st->SetDefault("web.option.FrostMaxMessageRequests.description","The maximum number of concurrent requests for new Frost messages.");

	st->SetDefault("web.page.execquery.title","Execute Query");
	st->SetDefault("web.page.execquery.executequery","Execute Query");

	st->SetDefault("web.page.createidentity.title","Create Identity");
	st->SetDefault("web.page.createidentity.createdidentity","Created Identity");
	st->SetDefault("web.page.createidentity.create","Create");
	st->SetDefault("web.page.createidentity.aftercreateinstructions","You must have at least 1 local identity that has set explicit trust list trust for one or more peers who are publishing trust lists or you will not be able to learn about other identities.");

	st->SetDefault("web.page.localidentities.title","Local Identities");
	st->SetDefault("web.page.localidentities.exportidentities","Export Identities");
	st->SetDefault("web.page.localidentities.importidentities","Import Identities");
	st->SetDefault("web.page.localidentities.name","Name");
	st->SetDefault("web.page.localidentities.singleuse","Single Use");
	st->SetDefault("web.page.localidentities.publishtrustlist","Publish Trust List");
	st->SetDefault("web.page.localidentities.publishboardlist","Publish Board List");
	st->SetDefault("web.page.localidentities.publishfreesite","Publish Freesite");
	st->SetDefault("web.page.localidentities.minmessagedelay","Min Message Delay");
	st->SetDefault("web.page.localidentities.maxmessagedelay","Max Message Delay");
	st->SetDefault("web.page.localidentities.announced","Announced? *");
	st->SetDefault("web.page.localidentities.yes","Yes");
	st->SetDefault("web.page.localidentities.no","No");
	st->SetDefault("web.page.localidentities.update","Update");
	st->SetDefault("web.page.localidentities.delete","Delete");
	st->SetDefault("web.page.localidentities.confirmdelete","Are you sure you want to delete");
	st->SetDefault("web.page.localidentities.announceddescription","* An identity is considered successfully announced when you have downloaded a trust list from someone that contains the identity.  You must trust other identities' trust lists for this to happen.  The number in parenthesis is how many trust lists the identity appears in.  You may post messages before you are announced.");
	st->SetDefault("web.page.localidentities.singleusedescription","Single Use Identities will automatically be deleted 7 days after creation.");
	st->SetDefault("web.page.localidentities.delaydescription","Messages that each identity sends may be delayed by a random number of minutes between min and max.  Set both to 0 to send messages as soon as possible.");

	st->SetDefault("web.page.announceidentity.title","Announce Identity");
	st->SetDefault("web.page.announceidentity.selectidentity","Select Identity :");
	st->SetDefault("web.page.announceidentity.from","From :");
	st->SetDefault("web.page.announceidentity.instructions","Type the answers of a few of the following puzzles.  You don't need to get them all correct, but remember that they are case sensitive.  Getting announced will take some time and you must assign trust to other identities to see yourself announced.  DO NOT continuously solve captchas.  Solve 30 at most, wait a day, and if your identity has not been announced, repeat until it is.");
	st->SetDefault("web.page.announceidentity.waitforpuzzles","You must wait for some puzzles to be downloaded.  Make sure you have assigned trust to some other identities' trust lists and check back later.");
	st->SetDefault("web.page.announceidentity.announce","Announce");
	st->SetDefault("web.page.announceidentity.charactercaptcha.instructions","Type the characters that appear in the image");
	st->SetDefault("web.page.announceidentity.unlikecaptcha1.whichpanelunlike","Which panel is unlike the rest?");
	st->SetDefault("web.page.announceidentity.unlikecaptcha1.unlikeobject","What object appears in this panel?");
	st->SetDefault("web.page.announceidentity.unlikecaptcha1.similarobject","What object appears in the other panels?");
	st->SetDefault("web.page.announceidentity.chkaddtrust","Add trust list trust to the identity that published each solved captcha");
	st->SetDefault("web.page.announceidentity.txtaddtrust","The amount of trust list trust to set for each identity.  This will not overwrite any existing trust.");

	st->SetDefault("web.page.addpeer.title","Add Peer");
	st->SetDefault("web.page.addpeer.publickey","Public Key :");
	st->SetDefault("web.page.addpeer.validpubkey","The public key must be a valid SSK public key and include the / at the end");
	st->SetDefault("web.page.addpeer.add","Add");

	st->SetDefault("web.page.recentlyadded.title","Recently Added Peers");
	st->SetDefault("web.page.recentlyadded.name","Name");
	st->SetDefault("web.page.recentlyadded.dateadded","Date Added");
	st->SetDefault("web.page.recentlyadded.addedmethod","Added Method");
	st->SetDefault("web.page.recentlyadded.deleteselected","Deleted Selected");

	st->SetDefault("web.page.peermaintenance.title","Peer Maintenance");
	st->SetDefault("web.page.peermaintenance.instructions","Removing a peer will not remove the messages they sent, but will remove everything else about that peer, including their trust levels.");
	st->SetDefault("web.page.peermaintenance.recentlyadded","Recently Added Peers");
	st->SetDefault("web.page.peermaintenance.stats","Stats");
	st->SetDefault("web.page.peermaintenance.knownpeers","known peers");
	st->SetDefault("web.page.peermaintenance.neverseen","never seen");
	st->SetDefault("web.page.peermaintenance.remove","Remove");
	st->SetDefault("web.page.peermaintenance.lastseen20days","last seen more than 20 days ago");
	st->SetDefault("web.page.peermaintenance.lastsent30days","last sent a message more than 30 days ago");
	st->SetDefault("web.page.peermaintenance.neversent","never sent a message");
	st->SetDefault("web.page.peermaintenance.added20daysneversent","added more than 20 days ago and never sent a message");
	st->SetDefault("web.page.peermaintenance.lastseen20daysneversent","last seen more than 20 days ago and never sent a message");
	st->SetDefault("web.page.peermaintenance.lastseen","last seen");
	st->SetDefault("web.page.peermaintenance.daysago","days ago");
	st->SetDefault("web.page.peermaintenance.daysagonulltrust","days ago, and have null local trust");

	st->SetDefault("web.page.confirm.title","Confirm");
	st->SetDefault("web.page.confirm.continue","Continue");
	st->SetDefault("web.page.confirm.cancel","Cancel");

	st->SetDefault("web.page.peertrust.title","Peer Trust");
	st->SetDefault("web.page.peertrust.instructions","Message Trust is how much you trust the identity to post good messages. Trust List Trust is how much weight you want the trust list of that identity to have when calculating the total. The local trust levels are set by you, and the peer trust levels are calculated by a weighted average using other identities' trust lists.  Trust is recalculated once an hour from received trust lists.  You must have at least 1 identity created and have received the SSK keypair for it from Freenet before setting trust.");
	st->SetDefault("web.page.peertrust.notrustlist","* - This identity is not publishing a trust list");
	st->SetDefault("web.page.peertrust.search","Search");
	st->SetDefault("web.page.peertrust.loadtrustlist","Load Trust List of");
	st->SetDefault("web.page.peertrust.loadlist","Load List");
	st->SetDefault("web.page.peertrust.name","Name");
	st->SetDefault("web.page.peertrust.localmessagetrust","Local Message Trust");
	st->SetDefault("web.page.peertrust.messagecomment","Message Comment");
	st->SetDefault("web.page.peertrust.peermessagetrust","Peer Message Trust");
	st->SetDefault("web.page.peertrust.localtrustlisttrust","Local Trust List Trust");
	st->SetDefault("web.page.peertrust.trustcomment","Trust Comment");
	st->SetDefault("web.page.peertrust.peertrustlisttrust","Peer Trust List Trust");
	st->SetDefault("web.page.peertrust.messagecount","Message Count");
	st->SetDefault("web.page.peertrust.updatetrust","Update Trust");
	st->SetDefault("web.page.peertrust.previouspage","<-- Previous Page");
	st->SetDefault("web.page.peertrust.nextpage","Next Page -->");

	st->SetDefault("web.page.boards.title","Boards");
	st->SetDefault("web.page.boards.search","Search");
	st->SetDefault("web.page.boards.remove0messages","Remove boards with 0 messages");
	st->SetDefault("web.page.boards.remove","Remove");
	st->SetDefault("web.page.boards.addboard","Add Board");
	st->SetDefault("web.page.boards.name","Name");
	st->SetDefault("web.page.boards.description","Description");
	st->SetDefault("web.page.boards.savereceivedmessages","Save Received Messages *");
	st->SetDefault("web.page.boards.forum","Forum");
	st->SetDefault("web.page.boards.addedmethod","Added Method");
	st->SetDefault("web.page.boards.update","Update");
	st->SetDefault("web.page.boards.previouspage","<-- Previous Page");
	st->SetDefault("web.page.boards.nextpage","Next Page -->");
	st->SetDefault("web.page.boards.saveinstructions","* If you uncheck this box, any new messages you download that are posted to this board will be discarded.  When multiple local identities are used, it is best not to discard messages from any boards, as identifying which identities are the same person is much easier when their message lists are missing messages from the same boards.");

	st->SetDefault("web.page.controlboard.title","Control Boards");
	st->SetDefault("web.page.controlboard.instructions","These boards are special administration boards where sent messages will change the trust levels of the parent poster by ADDING these numbers to their current trust level.  These boards can not be used as regular boards, so make the name unique.  The change in trust levels can be negative or positive, but keep in mind that the minimum trust level is 0 and the maximum trust level is 100.  After the boards are created here, you may use your newreader to reply to a message to one or more of these boards, and the previous poster will have his trust levels changed as per the settings for that board.");
	st->SetDefault("web.page.controlboard.boardname","Board Name");
	st->SetDefault("web.page.controlboard.changemessagetrust","Change Message Trust");
	st->SetDefault("web.page.controlboard.changetrustlisttrust","Change Trust List Trust");
	st->SetDefault("web.page.controlboard.remove","Remove");
	st->SetDefault("web.page.controlboard.add","Add");

	st->SetDefault("web.page.insertedfiles.title","Inserted Files");
	st->SetDefault("web.page.insertedfiles.remove","Remove");

	st->SetDefault("web.page.versioninfo.release","Release");
	st->SetDefault("web.page.versioninfo.notes","Notes");
	st->SetDefault("web.page.versioninfo.changes","Changes");

	st->SetDefault("web.page.pendingmessages.messageswaiting","messages waiting to be inserted");
	st->SetDefault("web.page.pendingmessages.identity","Identity");
	st->SetDefault("web.page.pendingmessages.boards","Boards");
	st->SetDefault("web.page.pendingmessages.subject","Subject");
	st->SetDefault("web.page.pendingmessages.time","Time");
	st->SetDefault("web.page.pendingmessages.deletemessage","Delete Message");

	st->SetDefault("web.page.translate.title","Translate");
	st->SetDefault("web.page.translate.translatebutton","Translate");
	st->SetDefault("web.page.translate.gotonextuntranslated","Go to next untranslated string");

	st->SetDefault("web.page.forum.newposts","New Posts");
	st->SetDefault("web.page.forum.nonewposts","No New Posts");
	st->SetDefault("web.page.forum.signin","Sign In");
	st->SetDefault("web.page.forum.signout","Sign Out");

	st->SetDefault("web.page.forummain.header.new","New");
	st->SetDefault("web.page.forummain.header.forum","Forum");
	st->SetDefault("web.page.forummain.header.posts","Posts");
	st->SetDefault("web.page.forummain.header.lastpost","Last Post");
	st->SetDefault("web.page.forummain.posts","posts");
	st->SetDefault("web.page.forummain.lastposton","Last post on");
	st->SetDefault("web.page.forummain.in","in");
	st->SetDefault("web.page.forummain.by","by");

	st->SetDefault("web.page.forumthreads.forum","Forum :");
	st->SetDefault("web.page.forumthreads.markallread","Mark All Read");
	st->SetDefault("web.page.forumthreads.newpost","New Post");
	st->SetDefault("web.page.forumthreads.header.new","New");
	st->SetDefault("web.page.forumthreads.header.subject","Subject");
	st->SetDefault("web.page.forumthreads.header.startedby","Started By");
	st->SetDefault("web.page.forumthreads.header.replies","Replies");
	st->SetDefault("web.page.forumthreads.header.lastpost","Last Post");
	st->SetDefault("web.page.forumthreads.by","by");
	st->SetDefault("web.page.forumthreads.pages","Pages :");
	st->SetDefault("web.page.forumthreads.go","Go");

	st->SetDefault("web.page.forumviewthread.forum","Forum :");
	st->SetDefault("web.page.forumviewthread.firstunread","First Unread Message");
	st->SetDefault("web.page.forumviewthread.markunread","Mark Unread");
	st->SetDefault("web.page.forumviewthread.trust","Trust");
	st->SetDefault("web.page.forumviewthread.local","Local");
	st->SetDefault("web.page.forumviewthread.peer","Peer");
	st->SetDefault("web.page.forumviewthread.message","Message");
	st->SetDefault("web.page.forumviewthread.trustlist","Trust List");
	st->SetDefault("web.page.forumviewthread.on","on");
	st->SetDefault("web.page.forumviewthread.reply","Reply");

	st->SetDefault("web.page.forumcreatepost.forum","Forum :");
	st->SetDefault("web.page.forumcreatepost.from","From");
	st->SetDefault("web.page.forumcreatepost.subject","Subject");
	st->SetDefault("web.page.forumcreatepost.message","Message");
	st->SetDefault("web.page.forumcreatepost.send","Send");
	st->SetDefault("web.page.forumcreatepost.wrote","wrote :");
	st->SetDefault("web.page.forumcreatepost.successfulsend","You have sent your message.  It will show up in the thread after it has been successfully inserted and retrieved by FMS.");
	st->SetDefault("web.page.forumcreatepost.error.localidentity","You must select a local identity as the sender");
	st->SetDefault("web.page.forumcreatepost.error.subject","You must enter a subject");
	st->SetDefault("web.page.forumcreatepost.error.body","You must enter a message body");
	st->SetDefault("web.page.forumcreatepost.error.message","Could not create message");
		
	tdir.pushDirectory("translations");
	tdir=tdir.makeAbsolute();
	Poco::File transfile(tdir);
	transfile.createDirectories();
	tdir.setFileName("english.prop");
	st->SaveDefaultTranslation(tdir.toString());

	if(languagefile!="english.prop" && languagefile!="")
	{
		tdir.setFileName(languagefile);
		st->LoadLocalizedTranslation(tdir.toString());
	}

}
