#ifndef FLEXEC_H
#define FLEXEC_H

#include "flashlight.h"

typedef void (*flConsoleOutputFunc)(Flashlight *fl, void *userData, const char *text);

int flExec(Flashlight *fl, Action *action, const char *path, flConsoleOutputFunc consoleOutputFunc, void *userData);

#endif
