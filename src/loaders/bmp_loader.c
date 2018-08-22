// bmp_loader.c
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

#include <stdio.h>
#include <stdlib.h>
#include <XPLMUtilities.h>
#include "loaders.h"

static inline int read_le(unsigned char* data)
{
    int sum = 0;
    sum += data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    return sum;
}

int load_bmp(const char* filePath)
{
    FILE* fp = fopen(filePath, "rb");
    if(!fp) {
        char buffer[4096];
        sprintf(buffer, "LogInDummy: Unable to open %s\n", filePath);
        XPLMDebugString(buffer);
        return 0;
    }

    unsigned char info[54];
    if(fread(info, sizeof(unsigned char), 54, fp) != 54) {
        XPLMDebugString("LogInDummy: Unexpected end of file while reading BMP header\n");
        fclose(fp);
        return 0;
    }

    int data_offset = read_le(&info[10]);
    if(fseek(fp, data_offset, SEEK_SET) != 0) {
        XPLMDebugString("LogInDummy: failed to seek to bitmap data in BMP\n");
        fclose(fp);
        return 0;
    }

    int width = read_le(&info[18]);
    int height = read_le(&info[22]);
    int pad = 0;
    if((width * 3) % 4 != 0) pad = 4 - ((width * 3) %4);
    int row_size = (width + pad) * 3;
    int size = row_size * height;
    unsigned char* data = malloc(sizeof(unsigned char) * size);
    if(!data) {
        XPLMDebugString("LogInDummy: Unable to allocate memory to load BMP\n");
        fclose(fp);
        return 0;
    }

    unsigned char* pos = data;
    for(int i = 0; i < height; i++) {
        size_t read = fread(pos, sizeof(unsigned char), row_size-pad, fp);
        if(read != row_size-pad) {
            XPLMDebugString("LogInDummy: Unexpected end of file while reading BMP data\n");
            free(data);
            return 0;
        }

        pos += row_size - pad;

        for(int p = 0; p < pad; i++) {
            *pos = 0;
            pos++;
        }
    }

    for(int i = 0; i < size - 2; i += 3) {
        unsigned char tmp = data[i];
        data[i] = data[i+2];
        data[i+2] = tmp;
    }

    int texture = bitmap_to_gl(data, width, height, GL_RGB);
    free(data);
    return texture;
}
