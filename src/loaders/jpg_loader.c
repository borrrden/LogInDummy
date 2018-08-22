// jpg_loader.c
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

#ifdef HAVE_JPG

#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>
#include "loaders.h"

struct my_error_mgr
{
    struct jpeg_error_mgr pub;

    jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr* my_error_ptr;

static void my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr)cinfo->err;
    (*cinfo->err->output_message)(cinfo);
    XPLMDebugString("LogInDummy: libjpg encountered an error\n");
    longjmp(myerr->setjmp_buffer, 1);
}

int load_jpg(const char* filePath)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    FILE* fp = fopen(filePath, "rb");
    if(!fp) {
        char buffer[4096];
        sprintf(buffer, "LogInDummy: Unable to open %s\n", filePath);
        XPLMDebugString(buffer);
        return 0;
    }

    JSAMPARRAY buffer;
    int row_stride;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if(setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        return 0;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
    unsigned char* data = malloc(row_stride * cinfo.output_height);
    if(!data) {
        XPLMDebugString("LogInDummy: Unable to allocate memory for bitmap\n");
        return 0;
    }

    unsigned char* start = data + row_stride * (cinfo.output_height - 1);
    while(cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(start - row_stride * (cinfo.output_scanline-1), buffer[0], row_stride);
    }
    
    int texture = bitmap_to_gl(data, cinfo.output_width, cinfo.output_height, GL_RGB);

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);

    return texture;
}

#else

int load_jpg(const char* filePath)
{
    XPLMDebugString("LogInDummy: JPEG file selected, but plugin not compiled with JPEG support!\n");
    return 0;
}
#endif
