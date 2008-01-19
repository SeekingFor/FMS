PROJECT(fms)

SET(FMS_SRC 
src/base64.cpp
src/commandthread.cpp
src/datetime.cpp
src/hex.cpp
src/identitytestglobal.cpp
src/logfile.cpp
src/main.cpp
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
src/freenet/trustlistinserter.cpp
src/freenet/trustlistrequester.cpp
src/freenet/trustlistxml.cpp
src/freenet/unkeyedidcreator.cpp
src/freenet/captcha/simplecaptcha.cpp
src/freenet/captcha/easybmp/EasyBMP.cpp
src/freenet/captcha/easybmp/EasyBMP_Font.cpp
src/freenet/captcha/easybmp/EasyBMP_Geometry.cpp
src/xyssl/sha1.c
)

ADD_DEFINITIONS(-DTIXML_USE_STL)

ADD_EXECUTABLE(fms ${FMS_SRC})

FIND_LIBRARY(SQLITE3_LIBRARY NAMES sqlite3 sqlite3_s)
FIND_LIBRARY(TINYXML_LIBRARY NAMES tinyxml tinyxml_s)
FIND_LIBRARY(ZTHREADS_LIBRARY NAMES zthread zthread_s)

IF(WIN32)
	TARGET_LINK_LIBRARIES(fms ws2_32)
ENDIF(WIN32)

IF(SQLITE3_LIBRARY)
	TARGET_LINK_LIBRARIES(fms ${SQLITE3_LIBRARY})
ENDIF(SQLITE3_LIBRARY)
IF(TINYXML_LIBRARY)
	TARGET_LINK_LIBRARIES(fms ${TINYXML_LIBRARY})
ENDIF(TINYXML_LIBRARY)
IF(ZTHREADS_LIBRARY)
	TARGET_LINK_LIBRARIES(fms ${ZTHREADS_LIBRARY})
ENDIF(ZTHREADS_LIBRARY)
