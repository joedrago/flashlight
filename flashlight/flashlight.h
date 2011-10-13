#ifndef FLASHLIGHT_H
#define FLASHLIGHT_H

#include <stdlib.h>

#include "flarray.h"
#include "cJSON.h"

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

typedef enum SpecialKey
{
    SK_UP = 1,
    SK_DOWN,
    SK_LEFT,
    SK_RIGHT,

    SK_COUNT
} SpecialKey;

typedef enum KeyType
{
    KT_UNKNOWN = 0,
    KT_NORMAL,
    KT_CONTROL,
    KT_SPECIAL,

    KT_COUNT
} KeyType;

// --------------------------------------------------------------------------------------

typedef enum Command
{
    COMMAND_NONE = 0,
    COMMAND_CLEAR,
    COMMAND_ACTION,
    COMMAND_RELOAD,
    COMMAND_VIEW_PREV,
    COMMAND_VIEW_NEXT,
    COMMAND_ACTION_PREV,
    COMMAND_ACTION_NEXT,

    COMMAND_COUNT
} Command;

// --------------------------------------------------------------------------------------

typedef struct Action
{
    const char *name;
    const char *exec;
} Action;

// --------------------------------------------------------------------------------------

typedef struct Rule
{
    const char *extension;
    Action *action;
} Rule;

// --------------------------------------------------------------------------------------

typedef struct Bind
{
    KeyType keyType;
    int key;
    const char *action;
    Command command;
} Bind;

// --------------------------------------------------------------------------------------

#define SEARCH_MAXLEN 1023

typedef struct Flashlight
{
    char *configFilename;
    flArray lists;
    flArray binds;
    flArray rules;
    flArray actions;

    flArray view; // ListEntry*
    int viewHeight;
    int viewOffset;
    int viewIndex;

    flArray viewActions;
    int viewActionIndex;

    char search[SEARCH_MAXLEN + 1];
    int searchLen;

    cJSON *jsonData;
} Flashlight;

cJSON *flLoadJSON(const char *filename);

Flashlight *flCreate(const char *configFilename, int viewHeight);
void flDestroy(Flashlight *fl);
void flClear(Flashlight *fl);
void flReload(Flashlight *fl);  // rereads configuration, does refresh
void flRefresh(Flashlight *fl); // refreshes list caches
void flAction(Flashlight *fl);  // Performs currently selected action on currently selected text
void flKey(Flashlight *fl, KeyType type, int key);
void flCommand(Flashlight *fl, Command command);

#endif
