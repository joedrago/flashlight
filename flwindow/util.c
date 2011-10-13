#include "util.h"

#include <windows.h>
#include <string.h>

const char *flashlightPath(const char *n1, const char *n2, const char *n3)
{
    static char sBasePath[MAX_PATH] = {0};
    static char ret[MAX_PATH];
    if(sBasePath[0] == 0)
    {
        char *p;
        GetModuleFileName(GetModuleHandle(NULL), sBasePath, MAX_PATH);
        p = strrchr(sBasePath, '\\');
        if(p) *p = 0;
    }
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

COLORREF parseColor(cJSON *json, int dr, int dg, int db)
{
    int r = dr;
    int g = dg;
    int b = db;
    if(json && (json->type == cJSON_Array) && (cJSON_GetArraySize(json) >= 3))
    {
        r = cJSON_GetArrayItem(json, 0)->valueint;
        g = cJSON_GetArrayItem(json, 1)->valueint;
        b = cJSON_GetArrayItem(json, 2)->valueint;
    }
    return RGB(r, g, b);
}
