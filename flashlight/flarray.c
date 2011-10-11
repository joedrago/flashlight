#include "flarray.h"

#include <stdlib.h>

void flArraySquash(flArray *p)
{
    int head = 0;
    int tail = 0;
    while(tail < p->count)
    {
        if(p->data[tail] != NULL)
        {
            p->data[head] = p->data[tail];
            head++;
        }
        tail++;
    }
    p->count = head;
}

void flArrayInject(flArray *p, void *v, int n)
{
    int index = flArrayPush(p, v); // start with a push to cause the array to grow, if need be
    int injectIndex = index - n;    // calculate the proper home for the entry that is now at the endIndex
    if(injectIndex < 0)
    {
        //osDebugPrint("flArrayInject(): injectIndex is %d! Terrible things are happening!\n", injectIndex);
        injectIndex = 0;
    }
    while(index > injectIndex)
    {
        p->data[index] = p->data[index - 1];
        index--;
    }
    p->data[index] = v;
}

void flArrayReserve(flArray *p, int size)
{
    int newSize = p->count + size;
    if(newSize > p->capacity)
    {
        p->capacity = newSize;
        p->data = realloc(p->data, p->capacity * sizeof(char **));
    }
}

int flArrayPush(flArray *p, void *v)
{
    // Not using flArrayReserve here as I want this to double at capacity
    if(p->count == p->capacity)
    {
        int newSize = (p->capacity) ? p->capacity * 2 : 2;
        p->capacity = newSize;
        p->data = realloc(p->data, p->capacity * sizeof(char **));
    }
    p->data[p->count++] = v;
    return (p->count - 1);
}

void *flArrayPop(flArray *p)
{
    if(!p->count)
        return NULL;
    return p->data[--p->count];
}

void *flArrayTop(flArray *p)
{
    if(!p->count)
        return NULL;
    return p->data[p->count - 1];
}

int flArrayCount(flArray *p)
{
    return p->count;
}

void flArrayClear(flArray *p, flDestroyCB cb)
{
    if(cb)
    {
        int i;
        for(i = 0; i < p->count; i++)
            cb(p->data[i]);
    }
    p->count = 0;
    if(p->data)
        free(p->data);
    p->data = NULL;
    p->capacity = 0;
}
