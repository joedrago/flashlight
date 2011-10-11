#ifndef FLWALK_H
#define FLWALK_H

#define WALK_MAXLEN 2047

typedef struct Walk
{
    const char *path;
    struct List *list;

    char currentPath[WALK_MAXLEN + 1];
    char currentFilename[WALK_MAXLEN + 1];
    char currentExtension[WALK_MAXLEN + 1];

    void *platformData;
} Walk;

// Helper funcs in flwalk.c
void walkParsePath(Walk *walk);

// Implemented by posix,etc
Walk *walkCreate(struct List *list, const char *path);
void walkDestroy(Walk *walk);
int walkNext(Walk *walk);

#endif

