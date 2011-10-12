#include "png.h"
#include "image.h"

#include <stdlib.h>

void pngerror(png_structp p, png_const_charp c) {}

Image *imageCreate(const char *filename)
{
    Image *image;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_infop end_info = NULL;
    png_bytep *row_pointers = NULL;
    FILE *f = NULL;
    unsigned char *rawdata = NULL;
    unsigned int i, j;
    int success = 0;

    f = fopen(filename, "rb");
    if(!f)
        return 0;

    image = calloc(1, sizeof(*image));

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, pngerror, pngerror);
    if(!png_ptr)
        goto fucked;

    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr)
        goto fucked;

    end_info = png_create_info_struct(png_ptr);
    if(!end_info)
        goto fucked;

    png_init_io(png_ptr, f);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    row_pointers = png_get_rows(png_ptr, info_ptr);

    rawdata = (unsigned char *)calloc(1, 4 * info_ptr->width * info_ptr->height);
    image->bits = rawdata;

    if(info_ptr->channels == 3)
    {
        for(j = 0; j < info_ptr->height; j++)
        {
            for(i = 0; i < info_ptr->width; i++)
            {
                int idx = 4 * (i + ((info_ptr->height - 1 - j) * info_ptr->width));
                rawdata[idx + 0] = row_pointers[j][(3 * i) + 2];
                rawdata[idx + 1] = row_pointers[j][(3 * i) + 1];
                rawdata[idx + 2] = row_pointers[j][(3 * i) + 0];
                rawdata[idx + 3] = 255;
            }
        }
    }
    else
    {
        for(j = 0; j < info_ptr->height; j++)
        {
            for(i = 0; i < info_ptr->width; i++)
            {
                int idx = 4 * (i + ((info_ptr->height - 1 - j) * info_ptr->width));
                rawdata[idx + 0] = row_pointers[j][(4 * i) + 2];
                rawdata[idx + 1] = row_pointers[j][(4 * i) + 1];
                rawdata[idx + 2] = row_pointers[j][(4 * i) + 0];
                rawdata[idx + 3] = row_pointers[j][(4 * i) + 3];
            }
        }
    }

    image->width = info_ptr->width;
    image->height = info_ptr->height;

    image->bitmapInfo.bmiHeader.biSize = sizeof(image->bitmapInfo.bmiHeader);
    image->bitmapInfo.bmiHeader.biWidth = image->width;
    image->bitmapInfo.bmiHeader.biHeight = image->height;
    image->bitmapInfo.bmiHeader.biPlanes = 1;
    image->bitmapInfo.bmiHeader.biBitCount = 32;
    image->bitmapInfo.bmiHeader.biCompression = BI_RGB;

    success = 1;

fucked:
    if(f)
        fclose(f);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    if(!success)
    {
        imageDestroy(image);
        image = NULL;
    }
    return image;
}


void imageDestroy(Image *image)
{
    if(!image)
        return;

    if(image->bits)
        free(image->bits);
    free(image);
}

void imageDraw(Image *image, HDC dc, int x, int y)
{
    StretchDIBits(dc,
                  x, y,
                  image->width, image->height,
                  0, 0,
                  image->width, image->height,
                  image->bits,
                  &image->bitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}

void imageDrawBackground(Image *image, HDC dc, int w, int h)
{
    int sw = image->width;
    int sh = image->height;
    int dw = w;
    int dh = h;
    int hsw = sw / 2;
    int hsh = sh / 2;
    int hdw = dw / 2;
    int hdh = dh / 2;

    // center/fill
    StretchDIBits(dc,
                  0, 0,
                  dw, dh,
                  hsw - 2, hsh - 2,
                  4, 4,
                  image->bits,
                  &image->bitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);

    // left edge
    StretchDIBits(dc,
                  0, 0,
                  hsw, dh,
                  0, hsh - 2,
                  hsw, 4,
                  image->bits,
                  &image->bitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);

    // right edge
    StretchDIBits(dc,
                  dw - hsw, 0,
                  hsw, dh,
                  hsw, hsh - 2,
                  hsw, 4,
                  image->bits,
                  &image->bitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);

    // top edge
    StretchDIBits(dc,
                  0, 0,
                  dw, hsh,
                  hsw - 2, hsh,
                  4, hsh,
                  image->bits,
                  &image->bitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);

    // bottom edge
    StretchDIBits(dc,
                  0, dh - hsh,
                  dw, hsh,
                  hsw - 2, 0,
                  4, hsh,
                  image->bits,
                  &image->bitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);

    // top left
    StretchDIBits(dc,
                  0, 0,
                  hsw, hsh,
                  0, hsh,
                  hsw, hsh,
                  image->bits,
                  &image->bitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);

    // top right
    StretchDIBits(dc,
                  dw - hsw, 0,
                  hsw, hsh,
                  hsw, hsh,
                  hsw, hsh,
                  image->bits,
                  &image->bitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);

    // bottom left
    StretchDIBits(dc,
                  0, dh - hsh,
                  hsw, hsh,
                  0, 0,
                  hsw, hsh,
                  image->bits,
                  &image->bitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);

    // bottom right
    StretchDIBits(dc,
                  dw - hsw, dh - hsh,
                  hsw, hsh,
                  hsw, 0,
                  hsw, hsh,
                  image->bits,
                  &image->bitmapInfo,
                  DIB_RGB_COLORS,
                  SRCCOPY);
}
