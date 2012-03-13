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
    SK_PAGEUP,
    SK_PAGEDOWN,
    SK_HOME,
    SK_END,
    SK_F1,
    SK_F2,
    SK_F3,
    SK_F4,
    SK_F5,
    SK_F6,
    SK_F7,
    SK_F8,
    SK_F9,
    SK_F10,
    SK_F11,
    SK_F12,

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

typedef struct CommandInfo
{
    int command;
    void *extraData;
} CommandInfo;

// --------------------------------------------------------------------------------------

typedef enum Command
{
    COMMAND_NONE = 0,
    COMMAND_CLEAR,
    COMMAND_UNDO_CLEAR,
    COMMAND_ACTION,
    COMMAND_NAMED_ACTION,
    COMMAND_RELOAD,
    COMMAND_VIEW_PREV,
    COMMAND_VIEW_NEXT,
    COMMAND_VIEW_PAGEUP,
    COMMAND_VIEW_PAGEDOWN,
    COMMAND_VIEW_TOP,
    COMMAND_VIEW_BOTTOM,
    COMMAND_ACTION_PREV,
    COMMAND_ACTION_NEXT,
    COMMAND_CLIPBOARD_COPY,

    COMMAND_COUNT
} Command;

// --------------------------------------------------------------------------------------

typedef struct Action
{
    const char *name;
    const char *exec;
    Image *image;
    int autoClose;
    int console;
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
    CommandInfo commandInfo;
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

// These match window's mods (no FL_ prefix)
#define FL_MOD_ALT         0x0001
#define FL_MOD_CONTROL     0x0002
#define FL_MOD_SHIFT       0x0004
#define FL_MOD_WIN         0x0008

typedef struct Flashlight
{
    char *configFilename;
    flArray lists;
    flArray binds;
    flArray rules;
    flArray actions;

    unsigned int hotkeyModifiers;
    unsigned int hotkey;

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
    char searchPrev[SEARCH_MAXLEN + 1];
    int searchPrevLen;

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
void flAction(Flashlight *fl, const char *name);  // Performs currently selected action on currently selected text
void flKey(Flashlight *fl, KeyType type, int key);
void flCommand(Flashlight *fl, Command command, void *data);
void flOutput(Flashlight *fl, const char *text);
void flOutputBytes(Flashlight *fl, const char *text, int len);
void flClearSearch(Flashlight *fl);
void flUndoClearSearch(Flashlight *fl);
const char *flPath(const char *n1, const char *n2, const char *n3, const char *n4);

#endif
