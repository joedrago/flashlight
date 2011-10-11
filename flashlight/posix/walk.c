#include "flwalk.h"
#include "flarray.h"
#include "flashlight.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

typedef struct PosixWalkDir
{
    char *path;
    DIR *dir;
} PosixWalkDir;

typedef struct PosixWalk
{
    flArray dirs;
} PosixWalk;

//typedef struct Walk
//{
//    char *path;
//    List *list;
//
//    char currentPath[WALK_MAXLEN+1];
//    char currentFilename[WALK_MAXLEN+1];
//    char currentExtension[WALK_MAXLEN+1];
//
//    void *platformData;
//} Walk;

Walk *walkCreate(struct List *list, const char *path)
{
    Walk *walk = calloc(sizeof(*walk), 1);
    PosixWalk *pwalk;
    PosixWalkDir *pwalkdir;

    walk->list = list;
    walk->path = path;
    pwalk = calloc(sizeof(*pwalk), 1);
    pwalkdir = calloc(sizeof(*pwalkdir), 1);
    pwalkdir->path = strdup(path);
    flArrayPush(&pwalk->dirs, pwalkdir);
    walk->platformData = pwalk;
    return walk;
}

void walkDestroy(Walk *walk)
{
    free(walk);
}

static walkPop(Walk *walk)
{
    PosixWalk *pwalk = walk->platformData;
    PosixWalkDir *pwalkdir = flArrayPop(&pwalk->dirs);
    if(pwalkdir->dir)
    {
        closedir(pwalkdir->dir);
    }
    free(pwalkdir->path);
    free(pwalkdir);
}

int walkNext(Walk *walk)
{
    PosixWalk *pwalk = walk->platformData;
    PosixWalkDir *pwalkdir;

    while(1)
    {
        pwalkdir = flArrayTop(&pwalk->dirs);
        if(!pwalkdir)
            return 0;

        if(pwalkdir->dir)
        {
            struct dirent *dent = readdir(pwalkdir->dir);
            struct stat st;
            if(dent)
            {
                char fullpath[WALK_MAXLEN + 1];
                strcpy(fullpath, pwalkdir->path);
                strcat(fullpath, "/");
                strcat(fullpath, dent->d_name);

                if((dent->d_name[0] != '.')
                   && (lstat(fullpath, &st) != -1))
                {
                    if(S_ISDIR(st.st_mode))
                    {
                        pwalkdir = calloc(sizeof(*pwalkdir), 1);
                        pwalkdir->path = strdup(fullpath);
                        flArrayPush(&pwalk->dirs, pwalkdir);
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
        else
        {
            pwalkdir->dir = opendir(pwalkdir->path);
            if(!pwalkdir->dir)
            {
                printf("ERROR: can't open dir '%s\n'", pwalkdir->path);
                walkPop(walk);
            }
        }
    }
    return 0;
}

