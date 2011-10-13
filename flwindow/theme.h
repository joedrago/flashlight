#ifndef THEME_H
#define THEME_H

#include <windows.h>

#include "flarray.h"
#include "image.h"
#include "cJSON.h"

typedef enum ThemeImageType
{
    TIT_BORDER = 0,

    TIT_COUNT // Two.
} ThemeImageType;

typedef struct ThemeImage
{
    int type;
    Image *image;
} ThemeImage;

typedef struct Theme
{
    flArray images;
    HFONT font;

    int initialX;
    int initialY;
    int initialWidth;
    int initialHeight;

    cJSON *jsonData;
} Theme;

Theme *themeCreate(const char *name);
void themeDestroy(Theme *theme);

#endif
