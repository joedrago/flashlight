#ifndef FLASHLIGHT_H
#define FLASHLIGHT_H

#include "flarray.h"

// --------------------------------------------------------------------------------------

typedef enum ListType
{
    LT_FILES = 0,

    LT_COUNT
} ListType;

typedef struct ListEntry
{
    struct List *list;
    char *path;
} ListEntry;

typedef struct List
{
    int type;
    char *path;
    flArray extensions;
    flArray entries;
} List;

// --------------------------------------------------------------------------------------

#define SEARCH_MAXLEN 1023

typedef struct Flashlight
{
    char *configFilename;
    flArray lists;
    flArray view; // ListEntry*

    char search[SEARCH_MAXLEN+1];
    int searchLen;

    void *jsonData;
} Flashlight;

Flashlight *flCreate(const char *configFilename);
void flDestroy(Flashlight *fl);
void flClear(Flashlight *fl);
void flReload(Flashlight *fl);  // rereads configuration, does refresh
void flRefresh(Flashlight *fl); // refreshes list caches

void flKey(Flashlight *fl, int key);

#endif

