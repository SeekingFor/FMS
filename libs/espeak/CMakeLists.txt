CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 OLD)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(libespeak)

SET(LIBESPEAK_SRC
src/compiledict.cpp
src/dictionary.cpp
src/intonation.cpp
src/klatt.cpp
src/numbers.cpp
src/phonemelist.cpp
src/readclause.cpp
src/setlengths.cpp
src/sonic.cpp
src/speak_lib.cpp
src/synthdata.cpp
src/synthesize.cpp
src/synth_mbrola.cpp
src/translate.cpp
src/tr_languages.cpp
src/voices.cpp
src/wavegen.cpp
)

IF(WIN32)
ADD_DEFINITIONS(-DPLATFORM_WINDOWS)
ENDIF(WIN32)

INCLUDE_DIRECTORIES(src)
ADD_LIBRARY(espeak ${LIBESPEAK_SRC})

IF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" AND CMAKE_COMPILER_IS_GNUCC)
	SET_TARGET_PROPERTIES(espeak PROPERTIES COMPILE_FLAGS "-fPIC ${COMPILE_FLAGS} -fPIC")
ENDIF("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" AND CMAKE_COMPILER_IS_GNUCC)
