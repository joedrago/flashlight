#include "flwalk.h"

#include <string.h>

void walkParsePath(Walk *walk)
{
    char *ext = strrchr(walk->currentPath, '.');
    walk->currentExtension[0] = 0;
    if(ext)
    {
        strcpy(walk->currentExtension, ext+1);
    }
}

