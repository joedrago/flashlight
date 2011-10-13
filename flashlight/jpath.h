#ifndef JPATH_H
#define JPATH_H

#include <stdlib.h>
#include "cJSON.h"

cJSON *jpathGet(cJSON *json, const char *path);
const char *jpathGetString(cJSON *json, const char *path, const char *def);
int jpathGetInt(cJSON *json, const char *path, int def);
int jpathGetBool(cJSON *json, const char *path, int def);

#endif
