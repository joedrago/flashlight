#ifndef JPATH_H
#define JPATH_H

#include <stdlib.h>
#include "cJSON.h"

const char *jpathGetString(cJSON *json, const char *path, const char *def);
int jpathGetInt(cJSON *json, const char *path, int def);

#endif
