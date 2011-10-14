#ifndef UTIL_H
#define UTIL_H

#include <windows.h>

#include "cJSON.h"

COLORREF parseColor(cJSON *json, int dr, int dg, int db);

#endif
