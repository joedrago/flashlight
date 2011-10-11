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
    int viewHeight;
    int viewOffset;
    int viewIndex;

    char search[SEARCH_MAXLEN + 1];
    int searchLen;

    void *jsonData;
} Flashlight;

Flashlight *flCreate(const char *configFilename, int viewHeight);
void flDestroy(Flashlight *fl);
void flClear(Flashlight *fl);
void flReload(Flashlight *fl);  // rereads configuration, does refresh
void flRefresh(Flashlight *fl); // refreshes list caches

typedef enum KeyType
{
    KT_NORMAL = 0,
    KT_CONTROL,

    KT_COUNT
} KeyType;

void flKey(Flashlight *fl, KeyType type, int key);

typedef enum Command
{
    COMMAND_NONE = 0,
    COMMAND_RELOAD,
    COMMAND_VIEW_PREV,
    COMMAND_VIEW_NEXT,
    COMMAND_ACTION_PREV,
    COMMAND_ACTION_NEXT,

    COMMAND_COUNT
} Command;
void flCommand(Flashlight *fl, Command command);

#endif
