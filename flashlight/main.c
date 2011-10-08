#include "flarray.h"
#include "flstring.h"

#include <stdlib.h>
#include <stdio.h>
#include "cjson.h"

cJSON *loadConfig(const char *filename)
{
    cJSON *config = NULL;
    FILE *f = fopen(filename, "rb");
    if(f)
    {
        int size;
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        if(size > 0)
        {
            char *json = (char*)malloc(size+1);
            fread(json, size, 1, f);
            json[size] = 0;
            config = cJSON_Parse(json);
            if(!config)
            {
                cJSONErrorInfo *err = cJSON_GetError();
                printf("ERROR: failed to parse json [%s:%d]: %s\n", filename, err->lineno, err->desc);
            }
            free(json);
        }
        else
        {
            printf("ERROR: config is zero bytes [%s]\n", filename);
        }
        fclose(f);
    }
    else
    {
        printf("ERROR: Can't read config [%s]\n", filename);
    }
    return config;
}

int main(int argc, char **argv)
{
    cJSON *config = loadConfig("config.json");
    if(config)
    {
        printf("got config\n");
    }
    return 0;
}

