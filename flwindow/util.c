#include "util.h"

#include <windows.h>
#include <string.h>

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
