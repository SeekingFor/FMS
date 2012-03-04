#ifndef _flip_jni_
#define _flip_jni_

#ifdef JNI_SUPPORT

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_plugins_FMS_FMSPlugin_StartFMS(JNIEnv *env, jobject obj);
JNIEXPORT void JNICALL Java_plugins_FMS_FMSPlugin_StopFMS(JNIEnv *env, jobject obj);
JNIEXPORT jstring JNICALL Java_plugins_FMS_FMSPlugin_VersionString(JNIEnv *env, jobject obj);
JNIEXPORT jlong JNICALL Java_plugins_FMS_FMSPlugin_VersionLong(JNIEnv *env, jobject obj);
JNIEXPORT jlong JNICALL Java_plugins_FMS_FMSPlugin_GetHTTPListenPort(JNIEnv *env, jobject obj);

#ifdef __cplusplus
}
#endif

#endif	// JNI_SUPPORT

#endif	// _flip_jni_
