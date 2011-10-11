#include "flashlight.h"
#include "flwalk.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "cJSON.h"

#ifdef FL_UNIX
#define BACKSPACE 127
#else
#define BACKSPACE 8
#endif

static const char *flstristr(const char *haystack, const char *needle)
{
   if ( !*needle )
   {
      return haystack;
   }
   for ( ; *haystack; ++haystack )
   {
      if ( toupper(*haystack) == toupper(*needle) )
      {
         /*
          * Matched starting char -- loop through remaining chars.
          */
         const char *h, *n;
         for ( h = haystack, n = needle; *h && *n; ++h, ++n )
         {
            if ( toupper(*h) != toupper(*n) )
            {
               break;
            }
         }
         if ( !*n ) /* matched all of 'needle' to null termination */
         {
            return haystack; /* return the start of the match */
         }
      }
   }
   return 0;
}

static cJSON *loadConfig(const char *filename)
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


Flashlight *flCreate(const char *configFilename)
{
    Flashlight *fl = calloc(sizeof(*fl), 1);
    fl->configFilename = strdup(configFilename);
    flReload(fl);
    return fl;
}

void flDestroy(Flashlight *fl)
{
    flClear(fl);
    free(fl->configFilename);
    free(fl);
}

void listEntryDestroy(ListEntry *e)
{
    free(e->path);
    free(e);
}

static void listDestroy(List *l)
{
    flArrayClear(&l->entries, (flDestroyCB)listEntryDestroy);
    flArrayClear(&l->extensions, free);
    free(l);
}

void flClear(Flashlight *fl)
{
    flArrayClear(&fl->view, NULL);
    flArrayClear(&fl->lists, (flDestroyCB)listDestroy);
    if(fl->jsonData)
    {
        cJSON_Delete((cJSON*)fl->jsonData);
        fl->jsonData = NULL;
    }
}

static cJSON *getMember(cJSON *json, const char *member, int type)
{
    cJSON *m = cJSON_GetObjectItem(json, member);
    if(m)
    {
        if(m->type == type)
        {
            return m;
        }
    }
    return NULL;
}

static char *getString(cJSON *json, const char *member)
{
    cJSON *m = getMember(json , member, cJSON_String);
    if(m)
        return m->valuestring;
    return NULL;
}

static void flParseExtensions(List *list, const char *extensions)
{
    const char *p = extensions;
    const char *front = p;
    while(1)
    {
        if((*p == 0) || (*p == ';'))
        {
            int len = (int)(p - front);
            if(len > 0)
            {
                char *text = calloc(len+1, 1);
                memcpy(text, front, len);
                flArrayPush(&list->extensions, text);
                front = p+1;
            }
        }
        if(!*p)
            break;
        p++;
    }
}

static void flBuild(Flashlight *fl)
{
    cJSON *json = (cJSON*)fl->jsonData;
    cJSON *lists = cJSON_GetObjectItem(json, "lists");
    if(lists && (lists->type == cJSON_Array))
    {
        int count = cJSON_GetArraySize(lists);
        int i;
        for(i=0; i<count; i++)
        {
            cJSON *listData = cJSON_GetArrayItem(lists, i);
            List *list = calloc(sizeof(*list), 1);
            list->type = LT_FILES;
            list->path = getString(listData, "path");
            flParseExtensions(list, getString(listData, "extensions"));
            flArrayPush(&fl->lists, list);
        }
    }
}

void flReload(Flashlight *fl)
{
    flClear(fl);

    fl->jsonData = loadConfig(fl->configFilename);
    if(!fl->jsonData)
        return;

    flBuild(fl);
    flRefresh(fl);
}

static int isValidExtension(List *list, const char *ext)
{
    int i;
    for(i=0; i<list->extensions.count; i++)
    {
        if(!strcmp(list->extensions.data[i], ext))
            return 1;
    }
    return 0;
}

void flClearFiles(List *l)
{
    flArrayClear(&l->entries, (flDestroyCB)listEntryDestroy);
}

void flRefreshFiles(List *l)
{
    Walk *walk = walkCreate(l, l->path);
    flClearFiles(l);
    while(walkNext(walk))
    {
        if(isValidExtension(l, walk->currentExtension))
        {
            ListEntry *entry = calloc(sizeof(*entry), 1);
            entry->path = strdup(walk->currentPath);
            entry->list = l;
            flArrayPush(&l->entries, entry);
        }
    }
    walkDestroy(walk);
}

static void flThink(Flashlight *fl)
{
    int i;
    fl->view.count = 0;
    for(i=0; i<fl->lists.count; i++)
    {
        int j;
        List *l = fl->lists.data[i];
        for(j=0; j<l->entries.count; j++)
        {
            ListEntry *e = l->entries.data[j];
            if((fl->searchLen == 0)
            || (flstristr(e->path, fl->search)))
            {
                flArrayPush(&fl->view, e);
            }
        }
    }
}

void flRefresh(Flashlight *fl)
{
    int i;
    for(i=0; i<fl->lists.count; i++)
    {
        List *l = fl->lists.data[i];
        flRefreshFiles(l);
    }
    flThink(fl);
}

void flKey(Flashlight *fl, int key)
{
    //printf("KEY: %d\n", key);
    if(key == BACKSPACE)
    {
        if(fl->searchLen > 0)
        {
            fl->searchLen--;
            fl->search[fl->searchLen] = 0;
        }
    }
    else if(key == 10)
    {
        fl->searchLen = 0;
        fl->search[fl->searchLen] = 0;
    }
    else if(key == 18)
    {
        flRefresh(fl);
        fl->searchLen = 0;
        fl->search[fl->searchLen] = 0;
    }
    else if(key > 31 && key < 127)
    {
        if(fl->searchLen < SEARCH_MAXLEN)
        {
            fl->searchLen++;
            fl->search[fl->searchLen-1] = key;
            fl->search[fl->searchLen] = 0;
        }
    }
    flThink(fl);
}

