#include <jni.h>
#include "mfoc-jni.h"

#include <stdlib.h>
#include <stdio.h>

int missing_func(void);

JNIEXPORT jint JNICALL
Java_net_raisama_nfc_mfoc_NativeImplementation_sum(JNIEnv *env, jobject this, jint a, jint b)
{
	return a+b;
}


int main(int argc, char * const argv[]);

JNIEXPORT void JNICALL Java_net_raisama_nfc_mfoc_NativeImplementation_callMain
  (JNIEnv *env, jobject this)
{
	char * const argv[] = {"fmoc", "-O", "/sdcard/fmoc-output.test", NULL};
	if (!freopen("/sdcard/fmoc.out", "a", stdout))
		exit(53);
	if (!freopen("/sdcard/fmoc.out", "a", stderr))
		exit(54);
	fprintf(stderr, "main!\n");
	fflush(stderr);
	fprintf(stdout, "outmain!\n");
	fflush(stdout);

	main(3, argv);
}