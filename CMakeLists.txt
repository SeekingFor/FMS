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
src/nntp/nntpconnection.cpp
src/nntp/nntplistener.cpp
src/nntp/uwildmat.cpp
src/nntp/mime/Mime.cpp
src/nntp/mime/MimeChar.cpp
src/nntp/mime/MimeCode.cpp
src/nntp/mime/MimeType.cpp
src/xyssl/sha1.c
)

ADD_DEFINITIONS(-DTIXML_USE_STL)

ADD_LIBRARY(sqlite3 libs/sqlite3/sqlite3.c)
ADD_LIBRARY(tinyxml libs/tinyxml/tinystr.cpp libs/tinyxml/tinyxml.cpp libs/tinyxml/tinyxmlerror.cpp libs/tinyxml/tinyxmlparser.cpp)

ADD_EXECUTABLE(fms ${FMS_SRC})

INCLUDE_DIRECTORIES(libs/sqlite3 libs/tinyxml)

TARGET_LINK_LIBRARIES(fms sqlite3 tinyxml)

IF(WIN32)
	TARGET_LINK_LIBRARIES(fms ws2_32)
ENDIF(WIN32)

#FIND_LIBRARY(SQLITE3_LIBRARY NAMES sqlite3 sqlite3_s)
#FIND_LIBRARY(TINYXML_LIBRARY NAMES tinyxml tinyxml_s)
FIND_LIBRARY(ZTHREADS_LIBRARY NAMES zthread ZThread zthread_s)

#IF(SQLITE3_LIBRARY)
#	TARGET_LINK_LIBRARIES(fms ${SQLITE3_LIBRARY})
#ELSE(SQLITE3_LIBRARY)
#	MESSAGE(FATAL_ERROR "Could not find SQLite library.  You must set the location manually.")
#ENDIF(SQLITE3_LIBRARY)

#IF(TINYXML_LIBRARY)
#	TARGET_LINK_LIBRARIES(fms ${TINYXML_LIBRARY})
#ELSE(TINYXML_LIBRARY)
#	MESSAGE(FATAL_ERROR "Could not find TinyXML library.  You must set the location manually.")
#ENDIF(TINYXML_LIBRARY)

IF(ZTHREADS_LIBRARY)
	TARGET_LINK_LIBRARIES(fms ${ZTHREADS_LIBRARY})
ELSE(ZTHREADS_LIBRARY)
	MESSAGE(FATAL_ERROR "Could not find ZThreads library.  You must set the location manually.")
ENDIF(ZTHREADS_LIBRARY)
