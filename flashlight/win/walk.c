#include "flwalk.h"
#include "flarray.h"
#include "flashlight.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

typedef struct WinWalkDir
{
    char *path;
    HANDLE h;
} WinWalkDir;

typedef struct WinWalk
{
    flArray dirs;
    WIN32_FIND_DATA wfd;
} WinWalk;

Walk *walkCreate(struct List *list, const char *path)
{
    Walk *walk = calloc(sizeof(*walk), 1);
    WinWalk *wwalk;
    WinWalkDir *wwalkdir;

    walk->list = list;
    walk->path = path;
    wwalk = calloc(sizeof(*wwalk), 1);
    wwalkdir = calloc(sizeof(*wwalkdir), 1);
    wwalkdir->path = strdup(path);
    wwalkdir->h = INVALID_HANDLE_VALUE;
    flArrayPush(&wwalk->dirs, wwalkdir);
    walk->platformData = wwalk;
    return walk;
}

void walkDestroy(Walk *walk)
{
    WinWalk *wwalk = walk->platformData;
    flArrayClear(&wwalk->dirs, NULL);
    free(wwalk);
    free(walk);
}

static walkPop(Walk *walk)
{
    WinWalk *wwalk = walk->platformData;
    WinWalkDir *wwalkdir = flArrayPop(&wwalk->dirs);
    if(wwalkdir->h != INVALID_HANDLE_VALUE)
        FindClose(wwalkdir->h);
    free(wwalkdir->path);
    free(wwalkdir);
}

int walkNext(Walk *walk)
{
    WinWalk *wwalk = walk->platformData;
    WinWalkDir *wwalkdir;

    while(1)
    {
        int haveFilename = 0;
        wwalkdir = flArrayTop(&wwalk->dirs);
        if(!wwalkdir)
            return 0;

        if(wwalkdir->h == INVALID_HANDLE_VALUE)
        {
            char fullpath[WALK_MAXLEN + 1];
            strcpy(fullpath, wwalkdir->path);
            strcat(fullpath, "\\*");
            wwalkdir->h = FindFirstFile(fullpath, &wwalk->wfd);
            haveFilename = (wwalkdir->h == INVALID_HANDLE_VALUE) ? 0 : 1;
        }
        else
        {
            haveFilename = FindNextFile(wwalkdir->h, &wwalk->wfd) ? 1 : 0;
        }

        if(haveFilename)
        {
            char fullpath[WALK_MAXLEN + 1];
            strcpy(fullpath, wwalkdir->path);
            strcat(fullpath, "\\");
            strcat(fullpath, wwalk->wfd.cFileName);

            if(wwalk->wfd.cFileName[0] != '.')
            {
                if(wwalk->wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    wwalkdir = calloc(sizeof(*wwalkdir), 1);
                    wwalkdir->h = INVALID_HANDLE_VALUE;
                    wwalkdir->path = strdup(fullpath);
                    flArrayPush(&wwalk->dirs, wwalkdir);
                }
                else
                {
                    strcpy(walk->currentPath, fullpath);
                    walkParsePath(walk);
                    return 1;
                }
            }
        }
        else
        {
            walkPop(walk);
        }
    }
    return 0;
}

