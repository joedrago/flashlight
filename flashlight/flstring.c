#include "flstring.h"

#include <string.h>

int flStrlen(const char *a)
{
    if(!a)
        return 0;
    return (int)strlen(a);
}

int flStrcmp(const char *a, const char *b)
{
    // Hopefully this layer of abstraction will pay off someday!
    return strcmp(a, b);
}

char *flStrdup(const char *s)
{
    return strdup(s);
}

void flStrcpy(char *dst, const char *src)
{
    strcpy(dst, src);
}

void flStrcpy2(char *dst, const char *src1, const char *src2)
{
    flStrcpy(dst, src1);
    flStrcat(dst, src2);
}

void flStrcpy3(char *dst, const char *src1, const char *src2, const char *src3)
{
    flStrcpy(dst, src1);
    flStrcat(dst, src2);
    flStrcat(dst, src3);
}

void flStrcat(char *dst, const char *src)
{
    strcat(dst, src);
}

void flStrncpy(char *dst, const char *src, int len)
{
    strncpy(dst, src, len);
}
