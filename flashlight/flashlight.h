#ifndef FLASHLIGHT_H
#define FLASHLIGHT_H

#include <stdlib.h>

#include "flarray.h"
#include "image.h"
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
    Image *image;
    int autoClose;
} Action;

// --------------------------------------------------------------------------------------

typedef struct Rule
{
    flArray extensions;
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

typedef enum FlashlightEvent
{
    FE_ACTION = 0,
    FE_CONSOLE,

    FE_COUNT
} FlashlightEvent;

typedef void (*flashlightEventFunc)(struct Flashlight *fl, FlashlightEvent e, void *data);

// --------------------------------------------------------------------------------------

#define SEARCH_MAXLEN 1023

typedef struct Flashlight
{
    char *configFilename;
    flArray lists;
    flArray binds;
    flArray rules;
    flArray actions;

    flArray scrollback;
    int scrollbackLimit;
    int scrollbackHeight;

    flArray view; // ListEntry*
    int viewHeight;
    int viewOffset;
    int viewIndex;

    flArray viewActions;
    int viewActionIndex;

    char search[SEARCH_MAXLEN + 1];
    int searchLen;

    cJSON *jsonData;

    flashlightEventFunc eventFunc;
} Flashlight;

cJSON *flLoadJSON(const char *filename);

Flashlight *flCreate(const char *configFilename);
void flSetEventFunc(Flashlight *fl, flashlightEventFunc eventFunc);
void flSetViewHeight(Flashlight *fl, int viewHeight);
void flSetScrollbackHeight(Flashlight *fl, int height);
void flDestroy(Flashlight *fl);
void flClear(Flashlight *fl);
void flReload(Flashlight *fl);  // rereads configuration, does refresh
void flRefresh(Flashlight *fl); // refreshes list caches
void flAction(Flashlight *fl);  // Performs currently selected action on currently selected text
void flKey(Flashlight *fl, KeyType type, int key);
void flCommand(Flashlight *fl, Command command);
void flOutput(Flashlight *fl, const char *text);
const char *flPath(const char *n1, const char *n2, const char *n3, const char *n4);

#endif
