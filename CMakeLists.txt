PROJECT(fms)

SET(FMS_SRC 
src/base64.cpp
src/board.cpp
src/boardlist.cpp
src/commandthread.cpp
src/datetime.cpp
src/global.cpp
src/hex.cpp
src/logfile.cpp
src/main.cpp
src/message.cpp
src/messagelist.cpp
src/option.cpp
src/socketdefines.cpp
src/stringfunctions.cpp
src/uuidgenerator.cpp
src/db/sqlite3db.cpp
src/db/sqlite3recordset.cpp
src/db/sqlite3statement.cpp
src/freenet/fcpv2.cpp
src/freenet/freenetmasterthread.cpp
src/freenet/freenetssk.cpp
src/freenet/identityinserter.cpp
src/freenet/identityintroductioninserter.cpp
src/freenet/identityintroductionrequester.cpp
src/freenet/identityintroductionxml.cpp
src/freenet/identityrequester.cpp
src/freenet/identityxml.cpp
src/freenet/introductionpuzzleinserter.cpp
src/freenet/introductionpuzzleremover.cpp
src/freenet/introductionpuzzlerequester.cpp
src/freenet/introductionpuzzlexml.cpp
src/freenet/messageinserter.cpp
src/freenet/messagelistinserter.cpp
src/freenet/messagelistrequester.cpp
src/freenet/messagelistxml.cpp
src/freenet/messagerequester.cpp
src/freenet/messagexml.cpp
src/freenet/trustlistinserter.cpp
src/freenet/trustlistrequester.cpp
src/freenet/trustlistxml.cpp
src/freenet/unkeyedidcreator.cpp
src/freenet/captcha/simplecaptcha.cpp
src/freenet/captcha/easybmp/EasyBMP.cpp
src/freenet/captcha/easybmp/EasyBMP_Font.cpp
src/freenet/captcha/easybmp/EasyBMP_Geometry.cpp
src/http/httpthread.cpp
src/http/ipagehandler.cpp
src/http/pages/addpeerpage.cpp
src/http/pages/announceidentitypage.cpp
src/http/pages/createidentitypage.cpp
src/http/pages/homepage.cpp
src/http/pages/localidentitiespage.cpp
src/http/pages/optionspage.cpp
src/http/pages/peertrustpage.cpp
src/http/pages/showcaptchapage.cpp
src/nntp/nntpconnection.cpp
src/nntp/nntplistener.cpp
src/nntp/uwildmat.cpp
src/nntp/mime/Mime.cpp
src/nntp/mime/MimeChar.cpp
src/nntp/mime/MimeCode.cpp
src/nntp/mime/MimeType.cpp
src/pthreadwrapper/guard.cpp
src/pthreadwrapper/mutex.cpp
src/pthreadwrapper/runnable.cpp
src/pthreadwrapper/thread.cpp
src/xyssl/sha1.c
)

ADD_DEFINITIONS(-DTIXML_USE_STL)

# was for ZThreads
#IF(CMAKE_COMPILER_IS_GNUCXX)
#	#ADD_DEFINITIONS(-fpermissive)
#	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
#ENDIF(CMAKE_COMPILER_IS_GNUCXX)

ADD_EXECUTABLE(fms ${FMS_SRC})

# For SQLite3 and shttpd
IF(CMAKE_COMPILER_IS_GNUCC)
	TARGET_LINK_LIBRARIES(fms dl)
ENDIF(CMAKE_COMPILER_IS_GNUCC)

IF(WIN32)
	TARGET_LINK_LIBRARIES(fms ws2_32)
ENDIF(WIN32)

FIND_LIBRARY(SQLITE3_LIBRARY NAMES sqlite3 sqlite3_s)
FIND_LIBRARY(TINYXML_LIBRARY NAMES tinyxml tinyxml_s)
FIND_LIBRARY(PTHREADS_LIBRARY NAMES pthread pthreads pthreadvc2)
FIND_LIBRARY(SHTTPD_LIBRARY NAMES shttpd)

IF(SQLITE3_LIBRARY)
	TARGET_LINK_LIBRARIES(fms ${SQLITE3_LIBRARY})
ELSE(SQLITE3_LIBRARY)
	MESSAGE(STATUS "Could not find system SQLite library.  Will compile from included source.")
	ADD_LIBRARY(sqlite3 libs/sqlite3/sqlite3.c)
	TARGET_LINK_LIBRARIES(fms sqlite3)
	INCLUDE_DIRECTORIES(libs/sqlite3)
ENDIF(SQLITE3_LIBRARY)

IF(TINYXML_LIBRARY)
	TARGET_LINK_LIBRARIES(fms ${TINYXML_LIBRARY})
ELSE(TINYXML_LIBRARY)
	MESSAGE(STATUS "Could not find system TinyXML library.  Will compile from included source.")
	ADD_LIBRARY(tinyxml libs/tinyxml/tinystr.cpp libs/tinyxml/tinyxml.cpp libs/tinyxml/tinyxmlerror.cpp libs/tinyxml/tinyxmlparser.cpp)
	TARGET_LINK_LIBRARIES(fms tinyxml)
	INCLUDE_DIRECTORIES(libs/tinyxml)
ENDIF(TINYXML_LIBRARY)

IF(PTHREADS_LIBRARY)
	TARGET_LINK_LIBRARIES(fms ${PTHREADS_LIBRARY})
ELSE(PTHREADS_LIBRARY)
	MESSAGE(FATAL ERROR "Could not find pthreads library.  You must set the location manually.")
ENDIF(PTHREADS_LIBRARY)

IF(SHTTPD_LIBRARY)
	TARGET_LINK_LIBRARIES(fms ${SHTTPD_LIBRARY})
ELSE(SHTTPD_LIBRARY)
	MESSAGE(STATUS "Could not find shttpd library.  Will compile from included source.")
	ADD_DEFINITIONS(-DEMBEDDED)
	INCLUDE_DIRECTORIES(libs/shttpd/include)
	IF(WIN32)
		ADD_DEFINITIONS(-DNO_GUI)
		SET(SHTTPD_PLATFORM_SRC libs/shttpd/compat_win32.c)
	ELSE(WIN32)
		SET(SHTTPD_PLATFORM_SRC libs/shttpd/compat_unix.c)
	ENDIF(WIN32)
	ADD_LIBRARY(shttpd ${SHTTPD_PLATFORM_SRC} libs/shttpd/auth.c libs/shttpd/cgi.c libs/shttpd/config.c libs/shttpd/io_cgi.c libs/shttpd/io_dir.c libs/shttpd/io_emb.c libs/shttpd/io_file.c libs/shttpd/io_socket.c libs/shttpd/io_ssi.c libs/shttpd/io_ssl.c libs/shttpd/log.c libs/shttpd/md5.c libs/shttpd/mime_type.c libs/shttpd/shttpd.c libs/shttpd/string.c)
	TARGET_LINK_LIBRARIES(fms shttpd)
ENDIF(SHTTPD_LIBRARY)
