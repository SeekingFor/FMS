#include "../include/jniwrapper.h"
#include "../include/global.h"
#include "../include/fmsapp.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include <Poco/Thread.h>
#include <Poco/Util/ServerApplication.h>

#ifdef JNI_SUPPORT

#if defined(_WIN32) && defined(POCO_WIN32_UTF8)
	int wmain(int argc, wchar_t** argv);
#elif defined(POCO_VXWORKS)
	int pocoSrvMain(const char* appName, ...);
#else
	int main(int argc, char **argv);
#endif

void mainwrapper(void *param)
{
#ifdef _WIN32
	_mkdir("FMS");
#else
	mkdir("FMS",0777);
#endif
	global::basepath="FMS/";
#if defined(_WIN32) && defined(POCO_WIN32_UTF8)
	wchar_t *args[]={L"FMS"};
	wmain(1,args);
#elif defined(POCO_VXWORKS)
	pocoSrvMain("FMS");
#else
	char *args[]={"FMS"};
	main(1,args);
#endif
}

Poco::Thread FMSThread;

extern "C" JNIEXPORT void JNICALL Java_plugins_FMS_FMSPlugin_StartFMS(JNIEnv *env, jobject obj)
{
	global::shutdown=false;
	FMSThread.start(mainwrapper,0);
	return;
}

extern "C" JNIEXPORT void JNICALL Java_plugins_FMS_FMSPlugin_StopFMS(JNIEnv *env, jobject obj)
{
	global::shutdown=true;
	FMSThread.join();
	return;
}

extern "C" JNIEXPORT jstring JNICALL Java_plugins_FMS_FMSPlugin_VersionString(JNIEnv *env, jobject obj)
{
	return env->NewStringUTF(FMS_VERSION);
}

extern "C" JNIEXPORT jlong JNICALL Java_plugins_FMS_FMSPlugin_VersionLong(JNIEnv *env, jobject obj)
{
	return FMS_VERSION_LONG;
}

extern "C" JNIEXPORT jlong JNICALL Java_plugins_FMS_FMSPlugin_GetHTTPListenPort(JNIEnv *env, jobject obj)
{
	return global::httplistenport;
}

#endif	// JNI_SUPPORT
