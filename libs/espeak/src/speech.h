#ifndef _SPEECH_H_WRAPPER_
#define _SPEECH_H_WRAPPER_

#ifdef _WIN32
#include "speech-win.h"
#else
#include "speech-other.h"
#endif

#endif	// _SPEECH_H_WRAPPER_
