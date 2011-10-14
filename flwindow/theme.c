#include "theme.h"
#include "flashlight.h"
#include "jpath.h"
#include "util.h"

Theme *themeCreate(const char *name)
{
    cJSON *imageArray;
    Theme *theme = calloc(1, sizeof(*theme));
    const char *themeJsonFilename = flPath("themes", name, "theme.json", NULL);

    theme->jsonData = flLoadJSON(themeJsonFilename);
    if(theme->jsonData)
    {
        const char *fontName = jpathGetString(theme->jsonData, "font.name", "Courier New");
        const char *fontWeightStr = jpathGetString(theme->jsonData, "font.weight", "normal");
        const char *fontFilename = jpathGetString(theme->jsonData, "font.filename", NULL);
        int fontSize = jpathGetInt(theme->jsonData, "font.size", 12);
        int fontWeight = (!strcmp(fontWeightStr, "bold")) ? FW_BOLD : FW_NORMAL;

        theme->textHeight = jpathGetInt(theme->jsonData, "textHeight", 20);

        theme->initialX = jpathGetInt(theme->jsonData, "initialX", 10);
        theme->initialY = jpathGetInt(theme->jsonData, "initialY", 10);
        theme->initialWidth = jpathGetInt(theme->jsonData, "initialWidth", 300);
        theme->initialHeight = jpathGetInt(theme->jsonData, "initialHeight", 100);

        theme->searchPadding.left = jpathGetInt(theme->jsonData, "searchPadding.l", 0);
        theme->searchPadding.top = jpathGetInt(theme->jsonData, "searchPadding.t", 0);
        theme->searchPadding.right = jpathGetInt(theme->jsonData, "searchPadding.r", 0);
        theme->searchPadding.bottom = jpathGetInt(theme->jsonData, "searchPadding.b", 0);

        theme->searchMargins.left = jpathGetInt(theme->jsonData, "searchMargins.l", 0);
        theme->searchMargins.top = jpathGetInt(theme->jsonData, "searchMargins.t", 0);
        theme->searchMargins.right = jpathGetInt(theme->jsonData, "searchMargins.r", 0);
        theme->searchMargins.bottom = jpathGetInt(theme->jsonData, "searchMargins.b", 0);

        theme->listMargins.left = jpathGetInt(theme->jsonData, "listMargins.l", 0);
        theme->listMargins.top = jpathGetInt(theme->jsonData, "listMargins.t", 40);
        theme->listMargins.right = jpathGetInt(theme->jsonData, "listMargins.r", 0);
        theme->listMargins.bottom = jpathGetInt(theme->jsonData, "listMargins.b", 0);

        theme->actionMargins.left = jpathGetInt(theme->jsonData, "actionMargins.l", 0);
        theme->actionMargins.top = jpathGetInt(theme->jsonData, "actionMargins.t", 20);
        theme->actionMargins.right = jpathGetInt(theme->jsonData, "actionMargins.r", 0);
        theme->actionMargins.bottom = jpathGetInt(theme->jsonData, "actionMargins.b", 0);

        theme->actionSpacing = jpathGetInt(theme->jsonData, "actionSpacing", 5);

        theme->searchBackgroundColor = parseColor(jpathGet(theme->jsonData, "searchBackgroundColor"), 255, 255, 255);
        theme->searchTextColor = parseColor(jpathGet(theme->jsonData, "searchTextColor"), 0, 0, 0);
        theme->listTextInactiveColor = parseColor(jpathGet(theme->jsonData, "listTextInactiveColor"), 0, 0, 0);
        theme->listTextActiveColor = parseColor(jpathGet(theme->jsonData, "listTextActiveColor"), 0, 0, 0);

        if(fontFilename)
        {
            fontFilename = flPath("themes", name, fontFilename, NULL);
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
                    themeImage->image = imageCreate(flPath("themes", name, "images", filename));
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
