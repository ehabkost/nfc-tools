#ifndef __FAKE_STDIO_H__
#define __FAKE_STDIO_H__

/* Fake stdio.h
 */

#include <stdlib.h>
#include <jni.h>

#define FILE fakeFILE
#define stdout fakestdout
#define stderr fakestderr
#define fopen fakefopen
#define printf fakeprintf
#define fprintf fakefprintf
#define fwrite fakefwrite

typedef struct FILE FILE;

extern FILE *stdout, *stderr;
extern JNIEnv *global_env;

FILE *fopen(const char *filename, const char *mode);
int fprintf(FILE *f, const char *fmt, ...);
int printf(const char *fmt, ...);

size_t fwrite(const void *ptr, size_t size, size_t nitems,
              FILE *stream);


/* These ones are not from stdio.h, but it's easier to redefine it here:
 */
#define exit fake_exit
#define abort fake_abort

#endif /* __FAKE_STDIO_H__ */