#include "theme.h"
#include "flashlight.h"
#include "jpath.h"
#include "util.h"

Theme *themeCreate(const char *name)
{
    cJSON *imageArray;
    Theme *theme = calloc(1, sizeof(*theme));
    const char *themeJsonFilename = flashlightPath("themes", name, "theme.json");

    theme->jsonData = flLoadJSON(themeJsonFilename);
    if(theme->jsonData)
    {
        const char *fontName = jpathGetString(theme->jsonData, "font.name", "Courier New");
        const char *fontWeightStr = jpathGetString(theme->jsonData, "font.weight", "normal");
        const char *fontFilename = jpathGetString(theme->jsonData, "font.filename", NULL);
        int fontSize = jpathGetInt(theme->jsonData, "font.size", 12);
        int fontWeight = (!strcmp(fontWeightStr, "bold")) ? FW_BOLD : FW_NORMAL;

        theme->initialX = jpathGetInt(theme->jsonData, "initialX", 10);
        theme->initialY = jpathGetInt(theme->jsonData, "initialY", 10);
        theme->initialWidth = jpathGetInt(theme->jsonData, "initialWidth", 300);
        theme->initialHeight = jpathGetInt(theme->jsonData, "initialHeight", 100);

        if(fontFilename)
        {
            fontFilename = flashlightPath("themes", name, fontFilename);
            AddFontResourceEx(fontFilename, FR_PRIVATE | FR_NOT_ENUM, 0);
        }
        theme->font = CreateFont(fontSize, 0, 0, 0, /*FW_DONTCARE*/ fontWeight, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                 CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, fontName);

        imageArray = jpathGet(theme->jsonData, "images");
        if(imageArray && imageArray->type == cJSON_Array)
        {
            int count = cJSON_GetArraySize(imageArray);
            int i;
            for(i = 0; i < count; i++)
            {
                cJSON *imageData = cJSON_GetArrayItem(imageArray, i);
                const char *filename = jpathGetString(imageData, "filename", NULL);
                const char *typeStr = jpathGetString(imageData, "type", "border");
                ThemeImageType type = TIT_BORDER;
                if(filename)
                {
                    ThemeImage *themeImage = calloc(1, sizeof(*themeImage));
                    themeImage->image = imageCreate(flashlightPath("themes", name, filename));
                    if(themeImage->image)
                    {
                        flArrayPush(&theme->images, themeImage);
                    }
                    else
                    {
                        free(themeImage);
                    }
                }
            }
        }
    }

    return theme;
}

static void themeImageDestroy(ThemeImage *themeImage)
{
    imageDestroy(themeImage->image);
    free(themeImage);
}

void themeDestroy(Theme *theme)
{
    flArrayClear(&theme->images, (flDestroyCB)themeImageDestroy);
    DeleteObject(theme->font);
    if(theme->jsonData)
        cJSON_Delete(theme->jsonData);
    free(theme);
}
