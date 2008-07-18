CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 OLD)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(fms)

OPTION(USE_BUNDLED_SQLITE "Use the bundled SQLite3 library." OFF)
IF(APPLE)
	SET(USE_BUNDLED_SQLITE ON)
ENDIF(APPLE)
OPTION(DO_CHARSET_CONVERSION "Do charset conversion on sent messages to UTF-8.  This requires libiconv." ON)
OPTION(I_HAVE_READ_THE_README "I have fully read and understood the readme.txt." OFF)

SET(FMS_SRC 
src/base64.cpp
src/bitmapvalidator.cpp
src/board.cpp
src/boardlist.cpp
src/db
src/dbconversions.cpp
src/dbmaintenancethread.cpp
src/dbsetup.cpp
src/fmsapp.cpp
src/freenet
src/global.cpp
src/hex.cpp
src/http
src/ipaddressacl.cpp
src/localidentity.cpp
src/main.cpp
src/message.cpp
src/messagelist.cpp
src/nntp
src/option.cpp
src/optionssetup.cpp
src/socketdefines.cpp
src/stringfunctions.cpp
src/threadwrapper
src/db/sqlite3db.cpp
src/db/sqlite3recordset.cpp
src/db/sqlite3statement.cpp
src/freenet/boardlistinserter.cpp
src/freenet/boardlistrequester.cpp
src/freenet/boardlistxml.cpp
src/freenet/captcha
src/freenet/fcpv2.cpp
src/freenet/fileinserter.cpp
src/freenet/fmsversionrequester.cpp
src/freenet/fmsversionxml.cpp
src/freenet/freenetmasterthread.cpp
src/freenet/freenetssk.cpp
src/freenet/identityinserter.cpp
src/freenet/identityintroductioninserter.cpp
src/freenet/identityintroductionrequester.cpp
src/freenet/identityintroductionxml.cpp
src/freenet/identityrequester.cpp
src/freenet/identityxml.cpp
src/freenet/introductionpuzzleinserter.cpp
src/freenet/introductionpuzzlerequester.cpp
src/freenet/introductionpuzzlexml.cpp
src/freenet/messageinserter.cpp
src/freenet/messagelistinserter.cpp
src/freenet/messagelistrequester.cpp
src/freenet/messagelistxml.cpp
src/freenet/messagerequester.cpp
src/freenet/messagexml.cpp
src/freenet/siteinserter.cpp
src/freenet/trustlistinserter.cpp
src/freenet/trustlistrequester.cpp
src/freenet/trustlistxml.cpp
src/freenet/unkeyedidcreator.cpp
src/freenet/captcha/easybmp
src/freenet/captcha/simplecaptcha.cpp
src/freenet/captcha/easybmp/EasyBMP.cpp
src/freenet/captcha/easybmp/EasyBMP_Font.cpp
src/freenet/captcha/easybmp/EasyBMP_Geometry.cpp
src/http/fmshttprequesthandlerfactory.cpp
src/http/httpthread.cpp
src/http/identityexportxml.cpp
src/http/ipagehandler.cpp
src/http/multipartparser.cpp
src/http/pages
src/http/pages/addpeerpage.cpp
src/http/pages/announceidentitypage.cpp
src/http/pages/boardspage.cpp
src/http/pages/confirmpage.cpp
src/http/pages/controlboardpage.cpp
src/http/pages/createidentitypage.cpp
src/http/pages/execquerypage.cpp
src/http/pages/homepage.cpp
src/http/pages/insertedfilespage.cpp
src/http/pages/localidentitiespage.cpp
src/http/pages/optionspage.cpp
src/http/pages/peerdetailspage.cpp
src/http/pages/peermaintenancepage.cpp
src/http/pages/peertrustpage.cpp
src/http/pages/showcaptchapage.cpp
src/http/pages/versioninfopage.cpp
src/nntp/extensiontrust.cpp
src/nntp/mime
src/nntp/nntpconnection.cpp
src/nntp/nntplistener.cpp
src/nntp/uwildmat.cpp
src/nntp/mime/Mime.cpp
src/nntp/mime/MimeChar.cpp
src/nntp/mime/MimeCode.cpp
src/nntp/mime/MimeType.cpp
src/threadwrapper/threadedexector.cpp
)

SET(FMS_PLATFORM_SRC )

IF(DO_CHARSET_CONVERSION)
	SET(FMS_SRC ${FMS_SRC} src/charsetconverter.cpp)
ENDIF(DO_CHARSET_CONVERSION)

IF(NOT I_HAVE_READ_THE_README)
	MESSAGE(FATAL_ERROR "You must fully read the readme.txt before continuing.")
ENDIF(NOT I_HAVE_READ_THE_README)

ADD_EXECUTABLE(fms ${FMS_SRC} ${FMS_PLATFORM_SRC})

# link dl - For SQLite3 and shttpd - not for FreeBSD
IF(CMAKE_COMPILER_IS_GNUCC)
	IF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	ELSE(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
		TARGET_LINK_LIBRARIES(fms dl)
	ENDIF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# add -lcompat only for FreeBSD
IF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	IF(CMAKE_COMPILER_IS_GNUCXX)
		TARGET_LINK_LIBRARIES(fms compat)
	ENDIF(CMAKE_COMPILER_IS_GNUCXX)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

# link ws2_32 and iphlpapi for Windows
IF(WIN32)
	TARGET_LINK_LIBRARIES(fms ws2_32)
	TARGET_LINK_LIBRARIES(fms iphlpapi)
ENDIF(WIN32)

# add -lxnet and -lsocket on solaris
IF(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	TARGET_LINK_LIBRARIES(fms xnet)
	TARGET_LINK_LIBRARIES(fms socket)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "SunOS")

IF(NOT USE_BUNDLED_SQLITE)
	FIND_LIBRARY(SQLITE3_LIBRARY NAMES sqlite3 sqlite3_s)
ENDIF(NOT USE_BUNDLED_SQLITE)
FIND_LIBRARY(POCO_FOUNDATION_LIBRARY NAMES PocoFoundation PocoFoundationmt)
FIND_LIBRARY(POCO_NET_LIBRARY NAMES PocoNet PocoNetmt)
FIND_LIBRARY(POCO_UTIL_LIBRARY NAMES PocoUtil PocoUtilmt)
FIND_LIBRARY(POCO_XML_LIBRARY NAMES PocoXML PocoXMLmt)
FIND_LIBRARY(PTHREADS_LIBRARY NAMES pthread pthreads pthreadvc2)
FIND_LIBRARY(ICONV_LIBRARY NAMES iconv iconv_s libiconv libiconv_s)

IF(POCO_FOUNDATION_LIBRARY)
	TARGET_LINK_LIBRARIES(fms ${POCO_FOUNDATION_LIBRARY})
	ADD_DEFINITIONS(-DPOCO_HAVE_IPv6)
	IF(WIN32)
		ADD_DEFINITIONS(-DPOCO_STATIC)
	ENDIF(WIN32)
	IF(POCO_UTIL_LIBRARY)
		TARGET_LINK_LIBRARIES(fms ${POCO_UTIL_LIBRARY})
	ELSE(POCO_UTIL_LIBRARY)
		MESSAGE(FATAL_ERROR "Could not find system Poco Util library.  You must set the location manually.")
	ENDIF(POCO_UTIL_LIBRARY)
	IF(POCO_NET_LIBRARY)
		TARGET_LINK_LIBRARIES(fms ${POCO_NET_LIBRARY})
	ELSE(POCO_NET_LIBRARY)
		MESSAGE(FATAL_ERROR "Could not find system Poco Net library.  You must set the location manually.")
	ENDIF(POCO_NET_LIBRARY)
	IF(POCO_XML_LIBRARY)
		TARGET_LINK_LIBRARIES(fms ${POCO_XML_LIBRARY})
	ELSE(POCO_XML_LIBRARY)
		MESSAGE(FATAL_ERROR "Could not find system Poco XML library.  You must set the location manually.")
	ENDIF(POCO_XML_LIBRARY)
ELSE(POCO_FOUNDATION_LIBRARY)
	MESSAGE(FATAL_ERROR "Could not find system Poco Foundation library.  You must set the location manually.")
ENDIF(POCO_FOUNDATION_LIBRARY)

IF(NOT WIN32)
	IF(PTHREADS_LIBRARY)
		TARGET_LINK_LIBRARIES(fms ${PTHREADS_LIBRARY})
	ELSE(PTHREADS_LIBRARY)
		MESSAGE(FATAL_ERROR "Could not find system pthread library.  You must set the location manually.")
	ENDIF(PTHREADS_LIBRARY)
ENDIF(NOT WIN32)

IF(SQLITE3_LIBRARY)
	TARGET_LINK_LIBRARIES(fms ${SQLITE3_LIBRARY})
ELSE(SQLITE3_LIBRARY)
	IF(NOT USE_BUNDLED_SQLITE)
		MESSAGE(STATUS "Could not find system SQLite3 library.  Will compile from included source.")
	ENDIF(NOT USE_BUNDLED_SQLITE)
	ADD_LIBRARY(sqlite3 libs/sqlite3/sqlite3.c)
	TARGET_LINK_LIBRARIES(fms sqlite3)
	INCLUDE_DIRECTORIES(libs/sqlite3)
ENDIF(SQLITE3_LIBRARY)

IF(DO_CHARSET_CONVERSION)
	ADD_DEFINITIONS(-DDO_CHARSET_CONVERSION)
	IF(ICONV_LIBRARY)
		TARGET_LINK_LIBRARIES(fms ${ICONV_LIBRARY})
	ELSE(ICONV_LIBRARY)
		IF(WIN32)
			MESSAGE(FATAL_ERROR "Could not find iconv library.  You must set the location manually, or turn off charset conversion.")
		ENDIF(WIN32)
	ENDIF(ICONV_LIBRARY)
ENDIF(DO_CHARSET_CONVERSION)

# FreeBSD won't find Poco include files without this
IF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	INCLUDE_DIRECTORIES(AFTER /usr/local/include)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
