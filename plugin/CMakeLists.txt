CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 OLD)
ENDIF(COMMAND CMAKE_POLICY)

ADD_DEFINITIONS(-DJNI_SUPPORT)

FOREACH(SRC ${FMS_SRC} ${FMS_INC})
	SET(PLUGIN_SRC ${PLUGIN_SRC} "../${SRC}")
ENDFOREACH(SRC)

FIND_PACKAGE(JNI)
IF(JNI_FOUND)
	INCLUDE_DIRECTORIES(${JAVA_INCLUDE_PATH})
	INCLUDE_DIRECTORIES(${JAVA_INCLUDE_PATH2})
ELSE(JNI_FOUND)
	MESSAGE(SEND_ERROR "Unable to find Java include path!")
ENDIF(JNI_FOUND)

ADD_LIBRARY(fms-plugin SHARED ${PLUGIN_SRC})

# BEGIN Library linking
# link dl - For SQLite3 and shttpd - not for FreeBSD
IF(CMAKE_COMPILER_IS_GNUCC)
	IF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	ELSE(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
		TARGET_LINK_LIBRARIES(fms-plugin dl)
	ENDIF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# add -lcompat only for FreeBSD
IF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	IF(CMAKE_COMPILER_IS_GNUCXX)
		TARGET_LINK_LIBRARIES(fms-plugin compat)
	ENDIF(CMAKE_COMPILER_IS_GNUCXX)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

# link ws2_32 and iphlpapi for Windows
IF(WIN32)
	TARGET_LINK_LIBRARIES(fms-plugin ws2_32)
	TARGET_LINK_LIBRARIES(fms-plugin iphlpapi)
	ADD_DEFINITIONS(-D_WIN32_WINNT=0x0500)
ENDIF(WIN32)

# add -lxnet and -lsocket on solaris
IF(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	TARGET_LINK_LIBRARIES(fms-plugin xnet)
	TARGET_LINK_LIBRARIES(fms-plugin socket)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "SunOS")

IF(POCO_FOUNDATION_LIBRARY)
	ADD_DEFINITIONS(-DPOCO_HAVE_IPv6)
	IF(WIN32 OR POCO_STATIC)
		ADD_DEFINITIONS(-DPOCO_STATIC)
	ENDIF(WIN32 OR POCO_STATIC)
	
	# We don't need to explicitly link under MSVC, POCO uses #pragmas to specify linked libraries.
	IF(POCO_UTIL_LIBRARY)
		IF(NOT MSVC)
			TARGET_LINK_LIBRARIES(fms-plugin ${POCO_UTIL_LIBRARY})
		ENDIF(NOT MSVC)
	ELSE(POCO_UTIL_LIBRARY)
		MESSAGE(FATAL_ERROR "Could not find system Poco Util library.  You must set the location manually.")
	ENDIF(POCO_UTIL_LIBRARY)
	IF(POCO_XML_LIBRARY)
		IF(NOT MSVC)
			TARGET_LINK_LIBRARIES(fms-plugin ${POCO_XML_LIBRARY})
		ENDIF(NOT MSVC)
	ELSE(POCO_XML_LIBRARY)
		MESSAGE(FATAL_ERROR "Could not find system Poco XML library.  You must set the location manually.")
	ENDIF(POCO_XML_LIBRARY)
	IF(POCO_NET_LIBRARY)
		IF(NOT MSVC)
			TARGET_LINK_LIBRARIES(fms-plugin ${POCO_NET_LIBRARY})
		ENDIF(NOT MSVC)
	ELSE(POCO_NET_LIBRARY)
		MESSAGE(FATAL_ERROR "Could not find system Poco Net library.  You must set the location manually.")
	ENDIF(POCO_NET_LIBRARY)
	IF(NOT MSVC)
		TARGET_LINK_LIBRARIES(fms-plugin ${POCO_FOUNDATION_LIBRARY})
	ENDIF(NOT MSVC)
ELSE(POCO_FOUNDATION_LIBRARY)
	MESSAGE(FATAL_ERROR "Could not find system Poco Foundation library.  You must set the location manually.")
ENDIF(POCO_FOUNDATION_LIBRARY)

IF(NOT WIN32)
	IF(PTHREADS_LIBRARY)
		TARGET_LINK_LIBRARIES(fms-plugin ${PTHREADS_LIBRARY})
	ELSE(PTHREADS_LIBRARY)
		MESSAGE(FATAL_ERROR "Could not find system pthread library.  You must set the location manually.")
	ENDIF(PTHREADS_LIBRARY)
ENDIF(NOT WIN32)

IF(SQLITE3_LIBRARY)
	MESSAGE(STATUS "Linking against system SQLite3 library.")
	TARGET_LINK_LIBRARIES(fms-plugin ${SQLITE3_LIBRARY})
ELSE(SQLITE3_LIBRARY)
	IF(NOT USE_BUNDLED_SQLITE)
		MESSAGE(STATUS "Could not find system SQLite3 library.  Will compile from included source.")
	ENDIF(NOT USE_BUNDLED_SQLITE)
	TARGET_LINK_LIBRARIES(fms-plugin sqlite3)
	INCLUDE_DIRECTORIES(../libs/sqlite3)
ENDIF(SQLITE3_LIBRARY)

IF(DO_CHARSET_CONVERSION)
	ADD_DEFINITIONS(-DDO_CHARSET_CONVERSION)
	IF(ICONV_LIBRARY)
		TARGET_LINK_LIBRARIES(fms-plugin ${ICONV_LIBRARY})
	ELSE(ICONV_LIBRARY)
		IF(WIN32)
			MESSAGE(FATAL_ERROR "Could not find iconv library.  You must set the location manually, or turn off charset conversion.")
		ENDIF(WIN32)
	ENDIF(ICONV_LIBRARY)
ENDIF(DO_CHARSET_CONVERSION)

IF(ALTERNATE_CAPTCHA)
	IF(FREEIMAGE_LIBRARY)
		TARGET_LINK_LIBRARIES(fms-plugin ${FREEIMAGE_LIBRARY})
		ADD_DEFINITIONS("-DALTERNATE_CAPTCHA")
	ELSE(FREEIMAGE_LIBRARY)
		MESSAGE(FATAL_ERROR "Could not find the FreeImage library.  Set the location manually, or turn off the alternate captchas.")
	ENDIF(FREEIMAGE_LIBRARY)
ENDIF(ALTERNATE_CAPTCHA)

IF(AUDIO_CAPTCHA)
	TARGET_LINK_LIBRARIES(fms-plugin espeak)
	ADD_DEFINITIONS("-DAUDIO_CAPTCHA")
	INCLUDE_DIRECTORIES(../libs/espeak/src)
ENDIF(AUDIO_CAPTCHA)

IF(FROST_SUPPORT)
	TARGET_LINK_LIBRARIES(fms-plugin polarssl)
	INCLUDE_DIRECTORIES(../libs/polarssl/include)

	ADD_DEFINITIONS(-DFROST_SUPPORT)
	
ENDIF(FROST_SUPPORT)

# FreeBSD won't find Poco include files without this
IF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	INCLUDE_DIRECTORIES(AFTER /usr/local/include)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

# Mac OS X won't find Poco include files without this
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	INCLUDE_DIRECTORIES(AFTER /opt/local/include)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
# END Library linking

SET(PLUGIN_NAME "FMS-")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	SET(PLUGIN_NAME "${PLUGIN_NAME}win-")
    if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    	SET(PLUGIN_NAME "${PLUGIN_NAME}amd64")
    else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    	SET(PLUGIN_NAME "${PLUGIN_NAME}i386")
    endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )
endif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	SET(PLUGIN_NAME "${PLUGIN_NAME}linux-")
    if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    	SET(PLUGIN_NAME "${PLUGIN_NAME}amd64")
    else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    	SET(PLUGIN_NAME "${PLUGIN_NAME}i386")
    endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
	SET(PLUGIN_NAME "${PLUGIN_NAME}freebsd-")
	IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
		SET(PLUGIN_NAME "${PLUGIN_NAME}amd64")
	ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
		SET(PLUGIN_NAME "${PLUGIN_NAME}i386")
	ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	SET(PLUGIN_NAME "${PLUGIN_NAME}osx-")
	#TODO - detect PPC?
	IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
		SET(PLUGIN_NAME "${PLUGIN_NAME}amd64")
	ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
		SET(PLUGIN_NAME "${PLUGIN_NAME}i386")
	ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

SET_PROPERTY(TARGET fms-plugin PROPERTY PREFIX "")
SET_PROPERTY(TARGET fms-plugin PROPERTY OUTPUT_NAME ${PLUGIN_NAME})
SET_PROPERTY(TARGET fms-plugin PROPERTY LIBRARY_OUTPUT_DIRECTORY "bin")
SET_PROPERTY(TARGET fms-plugin PROPERTY RUNTIME_OUTPUT_DIRECTORY "bin")
