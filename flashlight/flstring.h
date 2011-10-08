#ifndef FL_STRING_H
#define FL_STRING_H

#include "fltypes.h"

int flStrlen(const char *a);
int flStrcmp(const char *a, const char *b);
char *flStrdup(const char *s);
void flStrcpy(char *dst, const char *src);
void flStrcpy2(char *dst, const char *src1, const char *src2); // copies two strings
void flStrcpy3(char *dst, const char *src1, const char *src2, const char *src3); // copies three strings. These functions are lame.
void flStrcat(char *dst, const char *src);
void flStrncpy(char *dst, const char *src, int len);

#endif
