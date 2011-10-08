#ifndef STD_TYPES_H
#define STD_TYPES_H

typedef int bool;
typedef int S32;
typedef unsigned int U32;
typedef float F32;
typedef char S8;
typedef unsigned short U16;
typedef short S16;
typedef unsigned char U8;

#ifndef NULL
#define NULL 0
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

// TODO: Actually implement this
#define stdAssert(EXPR)

#endif
