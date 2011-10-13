#include "jpath.h"

#include <stdio.h>
#include <string.h>

cJSON *jpathGet(cJSON *json, const char *path)
{
    cJSON *node = json;
    char tempPath[1024];
    char *tok;
    strncpy(tempPath, path, 1023);
    tempPath[1023] = 0;
    tok = strtok(tempPath, ".");
    for(; tok; tok = strtok(NULL, "."))
    {
        if(node->type != cJSON_Object)
            return NULL;
        node = cJSON_GetObjectItem(node, tok);
        if(!node)
            return NULL;
    }
    return node;
}

const char *jpathGetString(cJSON *json, const char *path, const char *def)
{
    cJSON *node = jpathGet(json, path);
    if(node && (node->type == cJSON_String))
        return node->valuestring;
    return def;
}

int jpathGetInt(cJSON *json, const char *path, int def)
{
    cJSON *node = jpathGet(json, path);
    if(node && (node->type == cJSON_Number))
        return node->valueint;
    return def;
}

int jpathGetBool(cJSON *json, const char *path, int def)
{
    cJSON *node = jpathGet(json, path);
    if(node)
    {
        if(node->type == cJSON_Number)
            return (node->valueint) ? 1 : 0;
        if(node->type == cJSON_True)
            return 1;
        if(node->type == cJSON_False)
            return 0;
    }
    return def;
}
