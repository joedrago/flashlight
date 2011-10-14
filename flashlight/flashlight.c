#include "flashlight.h"
#include "flwalk.h"
#include "flexec.h"
#include "jpath.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "cJSON.h"

#ifdef FL_UNIX
#define BACKSPACE 127
#else
#define BACKSPACE 8
#endif

static struct SpecialKeyTable
{
    const char *name;
    int key;
} sKeyTable[] =
{
    {"up", SK_UP },
    {"down", SK_DOWN },
    {"left", SK_LEFT },
    {"right", SK_RIGHT },
    {0, 0}
};

static int nameToKey(const char *name)
{
    struct SpecialKeyTable *p = sKeyTable;
    while(p->name)
    {
        if(!strcmp(p->name, name))
            return p->key;
        p++;
    }
    return 0;
}

static struct CommandNameTable
{
    const char *name;
    Command command;
} sCommandNameTable[] =
{
    {"clear", COMMAND_CLEAR },
    {"viewNext", COMMAND_VIEW_NEXT },
    {"viewPrev", COMMAND_VIEW_PREV },
    {"actionNext", COMMAND_ACTION_NEXT },
    {"actionPrev", COMMAND_ACTION_PREV },
    {0, 0}
};

static int nameToCommand(const char *name)
{
    struct CommandNameTable *p = sCommandNameTable;
    while(p->name)
    {
        if(!strcmp(p->name, name))
            return p->command;
        p++;
    }
    return COMMAND_NONE;
}

static const char *flstristr(const char *haystack, const char *needle)
{
    if(!*needle)
    {
        return haystack;
    }
    for(; *haystack; ++haystack)
    {
        if(toupper(*haystack) == toupper(*needle))
        {
            /*
             * Matched starting char -- loop through remaining chars.
             */
            const char *h, *n;
            for(h = haystack, n = needle; *h && *n; ++h, ++n)
            {
                if(toupper(*h) != toupper(*n))
                {
                    break;
                }
            }
            if(!*n)    /* matched all of 'needle' to null termination */
            {
                return haystack; /* return the start of the match */
            }
        }
    }
    return 0;
}

cJSON *flLoadJSON(const char *filename)
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
            char *json = (char *)malloc(size + 1);
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
    Flashlight *fl = calloc(1, sizeof(*fl));
    fl->configFilename = strdup(configFilename);
    fl->viewHeight = 1;
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
    flArrayClear(&l->extensions, NULL);
    free(l);
}

static void ruleDestroy(Rule *r)
{
    flArrayClear(&r->extensions, NULL);
    free(r);
}

static void actionDestroy(Action *a)
{
    if(a->image)
        imageDestroy(a->image);
    free(a);
}

void flClear(Flashlight *fl)
{
    flArrayClear(&fl->view, NULL);
    flArrayClear(&fl->lists, (flDestroyCB)listDestroy);
    flArrayClear(&fl->rules, (flDestroyCB)ruleDestroy);
    flArrayClear(&fl->actions, (flDestroyCB)actionDestroy);
    if(fl->jsonData)
    {
        cJSON_Delete((cJSON *)fl->jsonData);
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
                char *text = calloc(1, len + 1);
                memcpy(text, front, len);
                flArrayPush(&list->extensions, text);
                front = p + 1;
            }
        }
        if(!*p)
            break;
        p++;
    }
}

static Action *findAction(flArray *actions, const char *name)
{
    int i;
    for(i = 0; i < actions->count; i++)
    {
        Action *action = actions->data[i];
        if(!strcmp(action->name, name))
        {
            return action;
        }
    }
    return NULL;
}

static void jsonAppendStringArray(cJSON *json, flArray *arr)
{
    if(json && (json->type == cJSON_Array))
    {
        int i;
        int count = cJSON_GetArraySize(json);
        for(i = 0; i < count; i++)
        {
            cJSON *jsonStr = cJSON_GetArrayItem(json, i);
            if(jsonStr && (jsonStr->type == cJSON_String))
            {
                flArrayPush(arr, jsonStr->valuestring);
            }
        }
    }
}

const char *flPath(const char *n1, const char *n2, const char *n3, const char *n4)
{
    static char sBasePath[MAX_PATH] = {0};
    static char ret[MAX_PATH];
#ifdef _DEBUG
    sBasePath[0] = '.';
#endif
    if(sBasePath[0] == 0)
    {
        char *p;
        GetModuleFileName(GetModuleHandle(NULL), sBasePath, MAX_PATH);
        p = strrchr(sBasePath, '\\');
        if(p) *p = 0;
    }
    strcpy(ret, sBasePath);
    if(n1)
    {
        strcat(ret, "\\");
        strcat(ret, n1);
    }
    if(n2)
    {
        strcat(ret, "\\");
        strcat(ret, n2);
    }
    if(n3)
    {
        strcat(ret, "\\");
        strcat(ret, n3);
    }
    if(n4)
    {
        strcat(ret, "\\");
        strcat(ret, n4);
    }
    return ret;
}

static Image *loadActionImage(const char *theme, const char *baseName)
{
    Image *image = NULL;
    char fullpath[1024];
    if(theme)
    {
        strcpy(fullpath, flPath("themes", theme, "images", baseName));
        strcat(fullpath, ".png");
        image = imageCreate(fullpath);
    }
    if(!image)
    {
        strcpy(fullpath, flPath("images", baseName, NULL, NULL));
        strcat(fullpath, ".png");
        image = imageCreate(fullpath);
    }
    return image;
}

static void flBuild(Flashlight *fl)
{
    cJSON *json = (cJSON *)fl->jsonData;
    cJSON *lists = cJSON_GetObjectItem(json, "lists");
    cJSON *actions = cJSON_GetObjectItem(json, "actions");
    cJSON *rules = cJSON_GetObjectItem(json, "rules");
    cJSON *binds = cJSON_GetObjectItem(json, "binds");
    if(lists && (lists->type == cJSON_Array))
    {
        int count = cJSON_GetArraySize(lists);
        int i;
        for(i = 0; i < count; i++)
        {
            cJSON *listData = cJSON_GetArrayItem(lists, i);
            List *list = calloc(1, sizeof(*list));
            list->type = LT_FILES;
            list->path = getString(listData, "path");
            jsonAppendStringArray(jpathGet(listData, "extensions"), &list->extensions);
            flArrayPush(&fl->lists, list);
        }
    }
    if(binds && (binds->type == cJSON_Array))
    {
        int count = cJSON_GetArraySize(binds);
        int i;
        for(i = 0; i < count; i++)
        {
            cJSON *bindData = cJSON_GetArrayItem(binds, i);
            Bind *bind = calloc(1, sizeof(*bind));
            Command command = nameToCommand(getString(bindData, "command"));
            const char *keyStr = getString(bindData, "key");
            int keyLen = strlen(keyStr);
            if(keyLen > 1)
            {
                bind->key = nameToKey(keyStr);
                if(bind->key)
                    bind->keyType = KT_SPECIAL;
            }
            else
            {
                bind->keyType = KT_CONTROL;
                bind->key = keyStr[0] - 96; // converts 'a' -> 1
            }
            if((bind->key == 0) || (bind->keyType == KT_UNKNOWN) || (command == COMMAND_NONE))
            {
                free(bind);
            }
            else
            {
                bind->action = getString(bindData, "action");
                bind->command = command;
                flArrayPush(&fl->binds, bind);
            }
        }
    }
    if(actions && (actions->type == cJSON_Array))
    {
        int count = cJSON_GetArraySize(actions);
        int i;
        for(i = 0; i < count; i++)
        {
            cJSON *actionData = cJSON_GetArrayItem(actions, i);
            Action *action = calloc(1, sizeof(*action));
            action->name = getString(actionData, "name");
            action->image = loadActionImage(getString(json, "theme"), getString(actionData, "image"));
            action->exec = getString(actionData, "exec");
            flArrayPush(&fl->actions, action);
        }
    }
    if(rules && (rules->type == cJSON_Array))
    {
        int count = cJSON_GetArraySize(rules);
        int i;
        for(i = 0; i < count; i++)
        {
            cJSON *ruleData = cJSON_GetArrayItem(rules, i);
            Rule *rule = calloc(1, sizeof(*rule));
            jsonAppendStringArray(jpathGet(ruleData, "extensions"), &rule->extensions);
            rule->action = findAction(&fl->actions, getString(ruleData, "action"));
            if(rule->action)
            {
                flArrayPush(&fl->rules, rule);
            }
            else
            {
                free(rule);
            }
        }
    }
}

void flReload(Flashlight *fl)
{
    flClear(fl);

    fl->jsonData = flLoadJSON(fl->configFilename);
    if(!fl->jsonData)
        return;

    flBuild(fl);
    flRefresh(fl);
}

static int stringExistsInArrayInsensitive(flArray *arr, const char *str)
{
    int i;
    for(i = 0; i < arr->count; i++)
    {
        if(!stricmp(arr->data[i], str))
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
        if((!l->extensions.count) || stringExistsInArrayInsensitive(&l->extensions, walk->currentExtension))
        {
            ListEntry *entry = calloc(1, sizeof(*entry));
            entry->path = strdup(walk->currentPath);
            entry->list = l;
            flArrayPush(&l->entries, entry);
        }
    }
    walkDestroy(walk);
}

static void flViewReset(Flashlight *fl)
{
    fl->view.count = 0;
    fl->viewOffset = 0;
    fl->viewIndex = 0;

    fl->viewActions.count = 0;
    fl->viewActionIndex = 0;
}

static int extensionMatches(const char *path, const char *ext)
{
    const char *pathExt = strrchr(path, '.');
    if(pathExt)
        pathExt++;
    if(!strcmp(pathExt, ext))
        return 1;
    return 0;
}

static void flMatchActions(Flashlight *fl)
{
    fl->viewActions.count = 0;
    fl->viewActionIndex = 0;
    if(fl->view.count < 1)
    {
        return;
    }
    else
    {
        int i;
        ListEntry *listEntry = fl->view.data[fl->viewIndex];
        const char *selectedPath = listEntry->path;
        for(i = 0; i < fl->rules.count; i++)
        {
            Rule *rule = fl->rules.data[i];
            const char *ext = strrchr(selectedPath, '.');
            if(ext)
                ext++;
            if((!rule->extensions.count) || (ext && stringExistsInArrayInsensitive(&rule->extensions, ext)))
            {
                flArrayPush(&fl->viewActions, rule->action);
            }
        }
    }
}

static void flThink(Flashlight *fl)
{
    int i;
    int totalEntries = 0;
    flViewReset(fl);
    for(i = 0; i < fl->lists.count; i++)
    {
        List *l = fl->lists.data[i];
        totalEntries += l->entries.count;
    }
    flArrayReserve(&fl->view, totalEntries);
    for(i = 0; i < fl->lists.count; i++)
    {
        int j;
        List *l = fl->lists.data[i];
        for(j = 0; j < l->entries.count; j++)
        {
            ListEntry *e = l->entries.data[j];
            if((fl->searchLen == 0)
               || (flstristr(e->path, fl->search)))
            {
                flArrayPush(&fl->view, e);
            }
        }
    }
    flMatchActions(fl);
}

void flRefresh(Flashlight *fl)
{
    int i;
    for(i = 0; i < fl->lists.count; i++)
    {
        List *l = fl->lists.data[i];
        flRefreshFiles(l);
    }
    flThink(fl);
}

void flKey(Flashlight *fl, KeyType type, int key)
{
    if((type == KT_CONTROL) || (type == KT_SPECIAL))
    {
        int i;
        for(i = 0; i < fl->binds.count; i++)
        {
            Bind *bind = fl->binds.data[i];
            if((bind->key == key) && (bind->keyType == type))
            {
                flCommand(fl, bind->command);
            }
        }
    }
    else if(type == KT_NORMAL)
    {
        //printf("KEY: %d\n", key);
        if(key == BACKSPACE)
        {
            if(fl->searchLen > 0)
            {
                fl->searchLen--;
                fl->search[fl->searchLen] = 0;
                flThink(fl);
            }
        }
        else if((key == 10) || (key == 13))
        {
            flCommand(fl, COMMAND_ACTION);
        }
        else if(key == 18)
        {
            flRefresh(fl);
            fl->searchLen = 0;
            fl->search[fl->searchLen] = 0;
            flThink(fl);
        }
        else if(key > 31 && key < 127)
        {
            if(fl->searchLen < SEARCH_MAXLEN)
            {
                fl->searchLen++;
                fl->search[fl->searchLen - 1] = key;
                fl->search[fl->searchLen] = 0;
                flThink(fl);
            }
        }
        else if(key == 11)
        {
            flCommand(fl, COMMAND_VIEW_PREV);
        }
        else if(key == 12)
        {
            flCommand(fl, COMMAND_VIEW_NEXT);
        }
    }
}

static void flSetViewIndex(Flashlight *fl, int newIndex)
{
    if(!fl->view.count)
    {
        fl->viewOffset = 0;
        fl->viewIndex = 0;
    }
    else
    {
        fl->viewIndex = newIndex;
        if(fl->viewIndex < 0)
            fl->viewIndex = 0;
        if(fl->viewIndex >= fl->view.count)
            fl->viewIndex = fl->view.count - 1;
        if(fl->viewIndex < fl->viewOffset)
            fl->viewOffset = fl->viewIndex;
        else if(fl->viewIndex >= fl->viewOffset + fl->viewHeight)
            fl->viewOffset = fl->viewIndex - fl->viewHeight + 1;
    }
    flMatchActions(fl);
}

static void flSetViewActionIndex(Flashlight *fl, int newIndex)
{
    if(!fl->viewActions.count)
        return;

    if(newIndex < 0)
        newIndex = 0;
    if(newIndex >= fl->viewActions.count)
        newIndex = fl->viewActions.count - 1;
    fl->viewActionIndex = newIndex;
}

void flAction(Flashlight *fl)
{
    if((fl->view.count == 0) || (fl->viewActions.count == 0))
    {
        return;
    }
    else
    {
        ListEntry *listEntry = fl->view.data[fl->viewIndex];
        const char *selectedPath = listEntry->path;
        Action *action = fl->viewActions.data[fl->viewActionIndex];
        flExec(action, selectedPath);
        if(fl->eventFunc)
            fl->eventFunc(fl, FE_ACTION);
    }
}

void flCommand(Flashlight *fl, Command command)
{
    switch(command)
    {
    case COMMAND_RELOAD:
    {
        flReload(fl);
        break;
    }
    case COMMAND_ACTION:
    {
        flAction(fl);
        break;
    }
    case COMMAND_CLEAR:
    {
        fl->searchLen = 0;
        fl->search[fl->searchLen] = 0;
        flThink(fl);
        break;
    }
    case COMMAND_VIEW_PREV:
    {
        flSetViewIndex(fl, fl->viewIndex - 1);
        break;
    }
    case COMMAND_VIEW_NEXT:
    {
        flSetViewIndex(fl, fl->viewIndex + 1);
        break;
    }
    case COMMAND_ACTION_PREV:
    {
        flSetViewActionIndex(fl, fl->viewActionIndex - 1);
        break;
    }
    case COMMAND_ACTION_NEXT:
    {
        flSetViewActionIndex(fl, fl->viewActionIndex + 1);
        break;
    }
    }
}

void flSetEventFunc(Flashlight *fl, flashlightEventFunc eventFunc)
{
    fl->eventFunc = eventFunc;
}

void flSetViewHeight(Flashlight *fl, int viewHeight)
{
    fl->viewHeight = viewHeight;
    flThink(fl);
}
