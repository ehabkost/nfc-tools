#include <jni.h>
#include "mfoc-jni.h"

#include <stdlib.h>
#include <stdio.h>

#include <setjmp.h>
#include <getopt.h>

int main(int argc, char * const argv[]);

extern jobject fake_stdio_obj;
static jmp_buf exit_jmp;
static int exit_status;

JNIEXPORT jint JNICALL Java_net_raisama_nfc_mfoc_NativeImplementation_nativeMain
  (JNIEnv *env, jobject this)
{
	global_env = env;
	if (setjmp(exit_jmp))
		goto out;

	/* Reset some libraries: */

	/* getopt: */
	optreset = 1;
	optind = 1;


	char * const argv[] = {"fmoc", "-O", "/sdcard/fmoc-output.test", NULL};
	exit_status = main(3, argv);
out:
	global_env = NULL;
	return exit_status;
}


void exit(int status)
{
	exit_status = status;
    longjmp(exit_jmp, 1);
}

void abort()
{
	fprintf(stderr, "Aborted\n");
	exit(128+6);
}