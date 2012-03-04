COMPILING
---------
Compiling FMS requires CMake, Poco ( version >=1.3.6 ) and iconv if you want to
do charset conversion.  Other required libraries are bundled with FMS.

To compile, run these commands from the source directory:
cmake -D I_HAVE_READ_THE_README=ON .
make

Compiling with the bundled SQLite library is on by default.  If you do not want
to use the bundled SQLite library, add a -D USE_BUNDLED_SQLITE=OFF to the cmake
command.  To turn off charset conversion to UTF-8 when sending messages, add a
-D DO_CHARSET_CONVERSION=OFF.  Compiling with charset conversion turned on is
recommended.  If you would like to compile using the alternate captchas, add a
-D ALTERNATE_CAPTCHA=ON to the cmake command line. This option requires the
FreeImage library to be installed.  Enable generation of audio captchas by
adding -D AUDIO_CAPTCHA=ON to the cmake command line.

Query logging may be turned on by adding a -D QUERY_LOG=ON.  This will create a
file called query.log in the working directory.  Straight SQL statements will
be captured, as well as the setup of prepared statements.  Each step through a
prepared statement is also logged, but the details are not, so there should be
no sensitive information in this log file.

If you would like to build the FMS Freenet plugin add a -D BUILD_PLUGIN=ON to
the CMake command line.  This will build the binary for the plugin in the 
plugin/bin directory.  Place binaries from other platforms you want included in
the jar inside the plugin/bin directory.  You can then run ant in the plugin 
directory to build the jar.

UPGRADING
---------
*ALWAYS* make a copy of your current FMS installation before continuing.  First
shut down FMS, make a copy of the directory, and then replace all files except
the database with those from the new version.  You may keep the same database
unless otherwise noted in the release information.

INSTALLATION
------------
Place the binary, any templates, and the fonts and images directories in a
directory of your choice.  Windows users may need to download the runtime DLLs
available from the fms Freesite and place in the fms directory if they are not
already installed on the system.  On the first run, a database file will also
be created in this directory.  Make sure the user that runs FMS has read/write
access to this directory.

RUNNING
-------
You may run FMS in console mode by running the binary directly.  You can view
available command line options by typing /help on Windows and --help on other
platforms.  If you are running *nix and would like to run as a daemon, use the
--daemon argument.  On Windows, /registerService will install FMS as a service,
and /unregisterService will uninstall the service.  Use the /displayName=name
argument when installing the service to set the service name to whatever you
want.  You will need to manually start the service unless you change the
startup type in the service properties.

FMS must run a good portion of the day every day to work properly.  The slower
your Freenet connection is, the longer FMS must be run to find the
communications of other identities.  You will not have a good experience only
running FMS a few hours a day.

You may place custom SQL commands in a file named fmsrc.sql in the FMS working
directory.  These commands will be executed once after the database is opened
in each thread.  PRAGMA commands that affect how SQLite functions are 
typically placed here.

PROBLEM TROUBLESHOOTING AND REPORTING
-------------------------------------
Failure to follow the following reporting guidelines for each and every
separate report will reduce the liklihood the issue will be looked into.

1. If you've made any alterations at all to the source, recompile from the
   latest released version without your alterations.

2. Shut down FMS from the web interface, reboot your computer, and start FMS
   back up.  If the problem persists, continue with the next step.
   
3. Remove fmsrc.sql from the FMS directory if it exists.

4. Shut down FMS from the web interface, delete the fms.db3 file (make a backup
   first if you so desire), and restart FMS.  If the problem persists, continue
   with the next step.  If the issue goes away, your old database was probably
   corrupt and cannot be used anymore.

5. With FMS running, go the options page in the web interface and change the
   logging level to trace.  Then restart FMS.

6. If the issue persists, create a post in the fms group with a descriptive
   subject and a body that contains:
   a. A description of the issue
   b. The steps needed to reproduce the issue
   c. What you've already done to troubleshoot the issue
   d. Your operating system
   e. The type of disk and filesystem FMS is running on
   f. If you're using a precompiled FMS binary or compile it yourself
   g. The startup lines from the log file, up to and including the startup 
      complete line, as well as any portions of the log file that pertain to 
	  the issue you are experiencing.  Make sure to anonymize any IP addresses, 
	  host names, subnet masks, keys, etc. from the log that you don't want 
	  people to know about.
   h. Any other information that would be helpful to know about the problem

   Any vulgar language or otherwise uncivil remarks in the post will result in 
   the post being ignored.

CAPTCHAS
--------
There are currently 3 main types of captchas that FMS uses to introduce new 
identities.

The first type is an image that contains scrambled and obscured characters.  In
order to solve this type of captcha, you must type in the exact characters that
appear in the image from left to right.  If your FMS binary has been compiled
with alternate captchas turned on, you have the ability to add bitmap fonts to
the fonts directory, which will be used by this captcha generator.  Please see
the existing fonts in this directory for the format, and approximate size that
the font must be.

The second type of captcha contains 4 small images of objects.  3 of the images
will contain the same object.  The 4th will contain a different object.  You
must identify which image is different, what it contains, and what is in the
other 3 images.  You should type the object names in lower case in singular
form.  In order for FMS to generate this type of captcha, please follow these
instructions.  First, create a directory called unlikeimages in the FMS working
directory.  In this directory you will place 100x100 pixel bitmap images that
contain objects that most people can identify.  The names of the images must be
in the format: object_##.bmp , where object is the singular lower case name of
what the image contains, and ## is an identifier of any length.  Using numbers
such as 01, 02, 03, etc. as the identifiers is advisable.  Please pick images
where there is no question in what it contains.  Something like a telephone may
seem like a great idea, but should the object name be phone or telephone?
Other ambiguity can occur with animals, where there may be many individual
species that most would classify under a more generic term.  Elephants, birds,
foxes, monkeys, etc. are some examples of this.  Please use the generic term
that most people would identify the object as.  It is important to have a large
collection of images when using this type of captcha, and you obviously need at
least 3 images of each type of object.  Also, don't share your image collection
with other users.  This would give any potential malicious user all the
information they need to easily solve your captchas.

The third type of captcha is an audio file in which the speaker will announce a
series of letters.  There is some background noise in the audio that should be
ignored.  The letters that are clearly spoken should be entered in lower case
without spaces between them.

EXITING
-------
To exit FMS running in console mode, press CTRL+C while at the console.  You
can also use the shutdown button on the web interface to close FMS.  As a last
resort, you may kill the process.

WEB INTERFACE
-------------
By default, a web interface for administration will be running at http://
localhost:8080.  You can use the interface to configure and administer FMS.
There is also a forum built into the web interface so you can read and send
messages without needing to use a newsreader.

TRANSLATIONS
------------
FMS provides facilities to translate most of the text that shows up on the user
interface.  First you must select the language you wish to translate into on
the options page of the web interface.  If your language does not appear,
select the unlisted/other option.  Now go to http://localhost:8080/translate.
htm to view all the strings that can be translated.  Untranslated strings will
show up highlighted, but please keep in mind that the original string may
change from time to time, so it is best to compare all strings occasionally to
make sure they have the same content.  Click the translate button to translate
a particular string.  When translating individual strings, there is a checkbox
that may be clicked to immediately go to the next untranslated string.  After
you are satisfied with your translation, locate the appropriate .prop file in
the translations directory of FMS.  Upload this file into Freenet and create a
post on the fms board with the name of the language and the URI of the uploaded
file.

NNTP CONFIGURATION
------------------
By default, the NNTP server will listen on port 1119.  Configure your
newsreader to connect to the machine running FMS on this port.  Use the web
interface to create an identity and use the name of the identity as the
username for the newsgroup account.  The email address may be anything, as it
is discarded when posting messages.

POSTING MESSAGES
----------------
You must set your newsreader to use UTF-8 when posting messages unless you have
compiled with charset conversion turned on.  All headers of the message that
aren't needed will be stripped and all headers necessary for the proper sending
of the message will be replaced with sanitized ones.  Any non-text attachment
to the message will be inserted as a regular file and the key added to the body
of the message when received.  Keep the attachments small, as the message can't
be inserted until all attachments are inserted.  Text attachments will be
inlined with the message body.  Cross posting is fine, but remember that each
identity can set a limit to the number of boards each message may be cross
posted to.

CONTROL BOARDS
--------------
Control boards are special boards that will add/remove trust from an identity.
Create control boards in the web interface, and then reply to an identity's
message to a control board to change the trust of the identity as per the
settings for the board.  You may cross post to a regular board and a control
board with the same message.  The control boards will be stripped from the
message before inserting into Freenet.

FREESITES
---------
Each identity has the option to publish a freesite.  A generic HTML template
called site-template.htm is used to insert the site.  You can customize the
template by placing an HTML file called identityname-template.htm in the same
directory as the fms binary.  In the template, the string [LINKS] will be
replaced by a <ul> list of links and [CONTENT] will be replaced by the page
content.  [IDENTITYNAME] will be replaced by the name of the identity inserting
the Freesite.  The Freesite will be inserted once a day and contain your last
10 posts and your trust list if you are publishing it.  The site will be
inserted to a USK accessible via: USK@yourpublickey.../fms/0/

You may add extra files to your Freesite by creating a file called identityname-
files.txt that contains a list of files to add to the Freesite.  There should
be one file per line, and the path to each file may be absolute or relative to
the working directory, but you MUST use / as the path separator.  Files cannot
be named index.htm, trustlist.htm, or files.htm.

TRUST
-----
Trust is the most important element of FMS.  It determines which identities you
will download messages from and thus your overall experience.  Trust values can
range from 0 to 100, with 0 being distrust, 100 being fully trust, and 50 being
neutral.  A blank trust value means undertermined trust.  Do not give trust to
arbitrary identities.  Pick whom you trust wisely.  The settings for minimum
trust before downloading messages and trust lists can be changed on the web
interface.

You must have a local identity created before you can set trust levels.  Even
if you don't want to post messages, you must still create an identity, but you
do not have to announce it.  This way, no-one will know about that identity and
you will be able to set trust.  If you have multiple identities, each with
different trust levels for peers, the highest trust level set for a peer will
determine if messages/trust lists are downloaded from them.

A note on NULL trust:  If you neither trust or distrust an identity, they will
have NULL trust (no trust at all).  You will download messages and trust lists
from identities with NULL peer trust as long as the local trust level is at or
above your configured minimum.  You will also download messages from identities
with NULL local message trust (the peer message trust must be NULL or >= your
configured minimum as well), but you will not download trust lists from
identities with NULL local trust list trust.

NNTP EXTENSIONS
---------------
The following commands are available through the NNTP connection.  The client
must have authenticated for the commands to work.  Comments MUST be surrounded
by ".

XSETTRUST MESSAGE userid@keypart val
XSETTRUST TRUSTLIST userid@keypart val
XSETTRUST MESSAGECOMMENT userid@keypart "comment"
XSETTRUST TRUSTLISTCOMMENT userid@keypart "comment"

Responses:
2xx Trust Set
4xx Unknown ID or other error
5xx Syntax error

XGETTRUST MESSAGE userid@keypart
XGETTRUST TRUSTLIST userid@keypart
XGETTRUST PEERMESSAGE userid@keypart
XGETTRUST PEERTRUSTLIST userid@keypart

Responses:
2xx val
4xx Unknown ID or other error
5xx Syntax error

XGETTRUSTLIST
trust values will be 0 to 100 or can be the string "null" without quotes.

Responses:
2xx Trust List Follows
userid@keypart TAB messagetrust TAB trustlisttrust TAB peermessagetrust TAB peertrustlisttrust TAB messagecomment TAB trustlistcomment
.
4xx other error
