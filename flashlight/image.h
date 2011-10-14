#ifndef IMAGE_H
#define IMAGE_H

#include <windows.h>

typedef struct Image
{
    int width;
    int height;
    unsigned char *bits;

    BITMAPINFO bitmapInfo; // lame
} Image;

Image *imageCreate(const char *filename);
void imageDestroy(Image *image);

void imageDraw(Image *image, HDC dc, int x, int y);
void imageDrawScaledRop(Image *image, HDC dc, int x, int y, int w, int h, int rop);
void imageDrawTrans(Image *image, HDC dc, int x, int y, int w, int h, COLORREF trans);
void imageDrawBackground(Image *image, HDC dc, int w, int h);

#endif
