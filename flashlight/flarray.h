#ifndef FL_ARRAY_H
#define FL_ARRAY_H

#include "fltypes.h"

typedef struct flArray
{
    int count;
    int capacity;
    void **data;
} flArray;

int flArrayPush(flArray *p, void *v);
void *flArrayPop(flArray *p);
void *flArrayTop(flArray *p);
int flArrayCount(flArray *p);
void flArraySquash(flArray *p);  // Removes all NULL entries
void flArrayInject(flArray *p, void *v, int n);  // Injects v n entries from the end (0 being equivalent to Push)

typedef void (*flDestroyCB)(void *p);
void flArrayClear(flArray *p, flDestroyCB cb); // token is for elements, not for the array itself

#endif
