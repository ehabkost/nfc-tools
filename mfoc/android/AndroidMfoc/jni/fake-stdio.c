#include <stdarg.h>
#include <jni.h>
#include <stdio.h>

#include "mfoc-jni.h"

struct FILE {
	jobject obj;
};

static FILE _stdout;
FILE *stdout = &_stdout;

static FILE _stderr;
FILE *stderr = &_stderr;

jobject fake_stdio_obj;
JNIEnv *global_env;

#define IMPLEMENT_ME (abort())


JNIEXPORT void JNICALL Java_net_raisama_nfc_mfoc_NativeImplementation_setFakeStdioObject
  (JNIEnv *env, jobject obj, jobject fake_stdio, jobject out, jobject err)
{
	fake_stdio_obj = (*env)->NewGlobalRef(env, fake_stdio);
	_stdout.obj = (*env)->NewGlobalRef(env, out);
	_stderr.obj = (*env)->NewGlobalRef(env, err);
}

FILE *fopen(const char *filename, const char *mode)
{
	JNIEnv *env = global_env;
	jclass cls = (*env)->GetObjectClass(env, fake_stdio_obj);
    jmethodID mid = (*env)->GetMethodID(env, cls, "fopen", "(Ljava/lang/String;Ljava/lang/String;)Lnet/raisama/nfc/mfoc/FakeFile;");
	if (mid == NULL)
		abort();
    jobject obj = (*env)->CallObjectMethod(env, fake_stdio_obj, mid, (*env)->NewStringUTF(env, filename), (*env)->NewStringUTF(env, mode));
    FILE *f = malloc(sizeof(FILE));
    if (!f)
    	abort();
    f->obj = (*env)->NewGlobalRef(env, obj);
    return f;
}

static int vfprintf(FILE *f, const char *fmt, va_list ap)
{
	char *formatted;
	int len;
	JNIEnv *env = global_env;
	int i;

	/* We can't give the varargs to Java, so format the string in C
	 */
	len = vasprintf(&formatted, fmt, ap);

	if (len < 0)
		abort();

	jbyteArray jbytes = (*env)->NewByteArray(env, len);
	if (!jbytes)
		abort();
	(*env)->SetByteArrayRegion(env, jbytes, 0, len, formatted);

	jclass cls = (*env)->GetObjectClass(env, f->obj);
	jmethodID mid = (*env)->GetMethodID(env, cls, "print", "([B)V");
	if (mid == NULL)
		abort();
    (*env)->CallVoidMethod(env, f->obj, mid, jbytes);
    free(formatted);
    return len;
}

int fprintf(FILE *f, const char *fmt, ...)
{
	int r;
	va_list ap;
	va_start(ap, fmt);
	r = vfprintf(f, fmt, ap);
	va_end(ap);
	return r;
}

int printf(const char *fmt, ...)
{
	int r;
	va_list ap;
	va_start(ap, fmt);
	r = vfprintf(stdout, fmt, ap);
	va_end(ap);
	return r;
}


size_t fwrite(const void *ptr, size_t size, size_t nitems,
              FILE *stream)
{
	IMPLEMENT_ME;
}

