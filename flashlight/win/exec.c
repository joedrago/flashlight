#include "flexec.h"

#include <windows.h>
#include <stdio.h>

#define EXECBUFSIZE 8192
static char sExecBuffer[EXECBUFSIZE];
static char sVarBuffer[128];

int flExec(Action *action, const char *path)
{
    char *p = sExecBuffer;
    char *front = NULL;
    int pathLen = strlen(path);
    memset(sExecBuffer, 0, EXECBUFSIZE);
    strcpy(sExecBuffer, "cmd /c start \"flashlight\" ");
    strcat(sExecBuffer, action->exec);

    for(; *p; p++)
    {
        if(*p == '!')
        {
            if(front)
            {
                int len = p - (front + 1);
                if(len > 0)
                {
                    memcpy(sVarBuffer, front + 1, len);
                    sVarBuffer[len] = 0;

                    if(!strcmp(sVarBuffer, "*")
                       || !strcmp(sVarBuffer, "0")
                       || !strcmp(sVarBuffer, "1"))
                    {
                        memmove(front + pathLen, p + 1, EXECBUFSIZE - ((front + pathLen) - sExecBuffer));
                        memcpy(front, path, pathLen);
                    }
                }
                p = front + pathLen - 1;
                front = NULL;
            }
            else
            {
                front = p;
            }
        }
    }

    system(sExecBuffer);
    return 1;
}
