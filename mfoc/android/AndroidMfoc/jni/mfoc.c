#include <jni.h>
#include "mfoc-jni.h"

JNIEXPORT jint JNICALL
Java_net_raisama_nfc_mfoc_NativeImplementation_sum(JNIEnv *env, jobject this, jint a, jint b)
{
	return a+b;
}