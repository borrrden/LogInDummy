// png_loader.c
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

#ifdef HAVE_PNG
#include <png.h>
#include <XPLMGraphics.h>
#include "loaders.h"

int load_png(const char* filePath)
{
    FILE* fp = fopen(filePath, "rb");
    if(!fp) {
        char buffer[4096];
        sprintf(buffer, "LogInDummy: Unable to open %s\n", filePath);
        XPLMDebugString(buffer);
        return 0;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr) {
        XPLMDebugString("LogInDummy: Unable to create read struct\n");
        fclose(fp);
        return 0;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
        XPLMDebugString("LogInDummy: Unable to create info struct\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        return 0;
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if(!end_info) {
        XPLMDebugString("LogInDummy: Unable to create end info struct\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return 0;
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        XPLMDebugString("LogInDummy: libpng encountered an error\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }

    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;

    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type, NULL, NULL, NULL);
    png_read_update_info(png_ptr, info_ptr);
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes - 1) % 4);
    png_byte* image_data = malloc(rowbytes * temp_height * sizeof(png_byte)+15);
    if(!image_data) {
        XPLMDebugString("LogInDummy: Error allocating memory for PNG image data!\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }

    png_bytep* row_pointers = malloc(temp_height * sizeof(png_bytep));
    if(!row_pointers) {
        XPLMDebugString("LogInDummy: Error allocating memory for PNG row pointers!\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(image_data);
        fclose(fp);
        return 0;
    }

    for(int i = 0; i < temp_height; i++) {
        row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
    }

    png_read_image(png_ptr, row_pointers);

    int texture = bitmap_to_gl(image_data, temp_width, temp_height, GL_RGBA);

    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(image_data);
    free(row_pointers);
    fclose(fp);
    return texture;
}

#else
int load_png(const char* filePath)
{
    XPLMDebugString("LogInDummy: PNG file selected, but plugin not compiled with PNG support!\n");
    return 0;
}
#endif