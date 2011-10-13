#ifndef UTIL_H
#define UTIL_H

#include <windows.h>

#include "cJSON.h"

const char *flashlightPath(const char *n1, const char *n2, const char *n3);

COLORREF parseColor(cJSON *json, int dr, int dg, int db);

#endif
