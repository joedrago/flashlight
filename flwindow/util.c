#include "util.h"

#include <windows.h>
#include <string.h>

const char *flashlightPath(const char *n1, const char *n2, const char *n3)
{
    static char sBasePath[MAX_PATH] = ".";//{0};
    static char ret[MAX_PATH];
    if(sBasePath[0] == 0)
        GetModuleFileName(GetModuleHandle(NULL), sBasePath, MAX_PATH);
    strcpy(ret, sBasePath);
    if(n1)
    {
        strcat(ret, "\\");
        strcat(ret, n1);
    }
    if(n2)
    {
        strcat(ret, "\\");
        strcat(ret, n2);
    }
    if(n3)
    {
        strcat(ret, "\\");
        strcat(ret, n3);
    }
    return ret;
}
