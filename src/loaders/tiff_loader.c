// tiff_loader.c
//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <http://unlicense.org>

#include <XPLMUtilities.h>

#ifdef HAVE_TIFF
#include <stdio.h>
#include <stdint.h>
#include <tiffio.h>
#include "loaders.h"
int load_tiff(const char* filePath)
{
    uint32_t width, height;
    TIFF* tif = TIFFOpen(filePath, "r");
    if(!tif) {
        char buffer[4096];
        sprintf(buffer, "LogInDummy: Unable to open %s\n", filePath);
        XPLMDebugString(buffer);
        return 0;
    }

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
    uint32_t* raster = _TIFFmalloc(width * height * sizeof(uint32_t));
    if(!raster) {
        TIFFClose(tif);
        XPLMDebugString("LogInDummy: Unable to allocate memory for bitmap\n");
        return 0;
    }

    if(!TIFFReadRGBAImage(tif, width, height, raster, 0)) {
        XPLMDebugString("LogInDummy: Failed to read TIFF file\n");
        TIFFClose(tif);
        _TIFFfree(raster);
        return 0;
    }

    int texture = bitmap_to_gl((unsigned char*)raster, width, height, GL_RGBA);
    TIFFClose(tif);
    _TIFFfree(raster);
    return texture;
}

#else
int load_tiff(const char* filePath)
{
    XPLMDebugString("LogInDummy: TIFF file selected, but plugin not compiled with TIFF support!\n");
    return 0;
}
#endif