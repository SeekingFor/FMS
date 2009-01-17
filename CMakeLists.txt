CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 OLD)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(fms)

OPTION(USE_BUNDLED_SQLITE "Use the bundled SQLite3 library." ON)
IF(APPLE)
	SET(USE_BUNDLED_SQLITE ON)
ENDIF(APPLE)
OPTION(DO_CHARSET_CONVERSION "Do charset conversion on sent messages to UTF-8.  This requires libiconv." ON)
OPTION(I_HAVE_READ_THE_README "I have fully read and understood the readme.txt." OFF)
OPTION(ALTERNATE_CAPTCHA "Use alternate captcha.  This requires the FreeImage library." OFF)
OPTION(QUERY_LOG "Log all database queries made by FMS." OFF)

SET(FMS_SRC
src/base64.cpp
src/bitmapvalidator.cpp
src/board.cpp
src/boardlist.cpp
src/dbconversions.cpp
src/dbmaintenancethread.cpp
src/dbsetup.cpp
src/fmsapp.cpp
src/global.cpp
src/hex.cpp
src/ipaddressacl.cpp
src/localidentity.cpp
src/main.cpp
src/message.cpp
src/messagelist.cpp
src/messagethread.cpp
src/option.cpp
src/optionssetup.cpp
src/socketdefines.cpp
src/stringfunctions.cpp
src/threadbuilder.cpp
src/db/sqlite3db.cpp
src/db/sqlite3recordset.cpp
src/db/sqlite3statement.cpp
src/freenet/boardlistinserter.cpp
src/freenet/boardlistrequester.cpp
src/freenet/boardlistxml.cpp
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
src/freenet/inactivemessagelistrequester.cpp
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
src/freenet/unknownidentityrequester.cpp
src/freenet/captcha/simplecaptcha.cpp
src/freenet/captcha/easybmp/EasyBMP.cpp
src/freenet/captcha/easybmp/EasyBMP_Font.cpp
src/freenet/captcha/easybmp/EasyBMP_Geometry.cpp
src/http/fmshttprequesthandlerfactory.cpp
src/http/httpthread.cpp
src/http/identityexportxml.cpp
src/http/ipagehandler.cpp
src/http/multipartparser.cpp
src/http/pages/addpeerpage.cpp
src/http/pages/announceidentitypage.cpp
src/http/pages/boardspage.cpp
src/http/pages/confirmpage.cpp
src/http/pages/controlboardpage.cpp
src/http/pages/createidentitypage.cpp
src/http/pages/execquerypage.cpp
src/http/pages/forumcreatepostpage.cpp
src/http/pages/forummainpage.cpp
src/http/pages/forumthreadspage.cpp
src/http/pages/forumviewthreadpage.cpp
src/http/pages/homepage.cpp
src/http/pages/insertedfilespage.cpp
src/http/pages/localidentitiespage.cpp
src/http/pages/optionspage.cpp
src/http/pages/peerdetailspage.cpp
src/http/pages/peermaintenancepage.cpp
src/http/pages/peertrustpage.cpp
src/http/pages/recentlyaddedpage.cpp
src/http/pages/showcaptchapage.cpp
src/http/pages/showimagepage.cpp
src/http/pages/showpendingmessagepage.cpp
src/http/pages/versioninfopage.cpp
src/nntp/extensiontrust.cpp
src/nntp/nntpconnection.cpp
src/nntp/nntplistener.cpp
src/nntp/uwildmat.cpp
src/nntp/mime/Mime.cpp
src/nntp/mime/MimeChar.cpp
src/nntp/mime/MimeCode.cpp
src/nntp/mime/MimeType.cpp
src/threadwrapper/threadedexector.cpp
src/unicode/ConvertUTF.c
src/unicode/unicodeformatter.cpp
src/unicode/utfconversion.cpp
)

SET(FMS_PLATFORM_SRC )

IF(DO_CHARSET_CONVERSION)
	SET(FMS_SRC ${FMS_SRC} src/charsetconverter.cpp)
ENDIF(DO_CHARSET_CONVERSION)

IF(ALTERNATE_CAPTCHA)
	SET(FMS_SRC ${FMS_SRC} 
	src/freenet/captcha/alternatecaptcha1.cpp
	src/freenet/captcha/alternatecaptcha2.cpp
	src/freenet/captcha/alternatecaptchafonts.cpp
	src/freenet/captcha/freeimage/bitmap.cpp
	src/freenet/captcha/freeimage/font.cpp)
ELSE(ALTERNATE_CAPTCHA)
	MESSAGE(STATUS "You are using the old captcha generator.  Add a -D ALTERNATE_CAPTCHA=ON to the command line to use the alternate captcha generator.")
ENDIF(ALTERNATE_CAPTCHA)

# Get a relative list of include files.
FILE(GLOB_RECURSE FMS_INC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} include/*.h)

# Create source groups for all sources and includes, in their original directory structure.
FOREACH(SRC ${FMS_SRC} ${FMS_INC})
	STRING(REGEX REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "Files" REL_DIR "${SRC}")
	STRING(REGEX REPLACE "[\\\\/][^\\\\/]*$" "" REL_DIR "${REL_DIR}")
	STRING(REGEX REPLACE "^[\\\\/]" "" REL_DIR "${REL_DIR}")
	IF(REL_DIR)
		STRING(REPLACE "/" "\\" REL_DIR "${REL_DIR}")
		SOURCE_GROUP(${REL_DIR} FILES ${SRC})
	ELSE(REL_DIR)
		SOURCE_GROUP(Files FILES ${SRC})
	ENDIF(REL_DIR)
ENDFOREACH(SRC)

IF(NOT I_HAVE_READ_THE_README)
	MESSAGE(FATAL_ERROR "You must fully read the readme.txt before continuing.")
ENDIF(NOT I_HAVE_READ_THE_README)

ADD_EXECUTABLE(fms ${FMS_SRC} ${FMS_PLATFORM_SRC} ${FMS_INC})

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
IF(ALTERNATE_CAPTCHA)
	FIND_FILE(FREEIMAGE_LIBRARY NAMES libfreeimage.a PATHS /usr/lib/)
	IF(NOT FREEIMAGE_LIBRARY)
		FIND_LIBRARY(FREEIMAGE_LIBRARY NAMES FreeImage libFreeImage libfreeimage freeimage)
	ENDIF(NOT FREEIMAGE_LIBRARY)
ENDIF(ALTERNATE_CAPTCHA)

IF(POCO_FOUNDATION_LIBRARY)
	# We don't need to explicitly link under MSVC, POCO uses #pragmas to specify linked libraries.
	IF(NOT MSVC)
		TARGET_LINK_LIBRARIES(fms ${POCO_FOUNDATION_LIBRARY})
	ENDIF(NOT MSVC)
	ADD_DEFINITIONS(-DPOCO_HAVE_IPv6)
	IF(WIN32)
		ADD_DEFINITIONS(-DPOCO_STATIC)
	ENDIF(WIN32)
	IF(POCO_UTIL_LIBRARY)
		IF(NOT MSVC)
			TARGET_LINK_LIBRARIES(fms ${POCO_UTIL_LIBRARY})
		ENDIF(NOT MSVC)
	ELSE(POCO_UTIL_LIBRARY)
		MESSAGE(FATAL_ERROR "Could not find system Poco Util library.  You must set the location manually.")
	ENDIF(POCO_UTIL_LIBRARY)
	IF(POCO_NET_LIBRARY)
		IF(NOT MSVC)
			TARGET_LINK_LIBRARIES(fms ${POCO_NET_LIBRARY})
		ENDIF(NOT MSVC)
	ELSE(POCO_NET_LIBRARY)
		MESSAGE(FATAL_ERROR "Could not find system Poco Net library.  You must set the location manually.")
	ENDIF(POCO_NET_LIBRARY)
	IF(POCO_XML_LIBRARY)
		IF(NOT MSVC)
			TARGET_LINK_LIBRARIES(fms ${POCO_XML_LIBRARY})
		ENDIF(NOT MSVC)
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
	MESSAGE(STATUS "Linking against system SQLite3 library.")
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

IF(ALTERNATE_CAPTCHA)
	IF(FREEIMAGE_LIBRARY)
		TARGET_LINK_LIBRARIES(fms ${FREEIMAGE_LIBRARY})
		ADD_DEFINITIONS("-DALTERNATE_CAPTCHA")
	ELSE(FREEIMAGE_LIBRARY)
		MESSAGE(FATAL_ERROR "Could not find the FreeImage library.  Set the location manually, or turn off the alternate captchas.")
	ENDIF(FREEIMAGE_LIBRARY)
ENDIF(ALTERNATE_CAPTCHA)

# FreeBSD won't find Poco include files without this
IF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	INCLUDE_DIRECTORIES(AFTER /usr/local/include)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

IF(QUERY_LOG)
	ADD_DEFINITIONS(-DQUERY_LOG)
ENDIF(QUERY_LOG)
