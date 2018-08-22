// BitmapWidget.cc
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

#include <XPWidgets.h>
#include <GL/GL.h>
#include <string>
#include <fstream>
#include <XPLMGraphics.h>

#include "loaders/loaders.h"

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

using namespace std;

static const char kJPGHeader[3] = {0xFF, 0xD8, 0xFF};
static const char kPNGHeader[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
static const char kTIFF1Header[4] = {0x49, 0x49, 0x2A, 0x00};
static const char kTIFF2Header[4] = {0x4D, 0x4D, 0x00, 0x2A};
static const char kBMPHeader[3] = "BM";

enum
{
    widgetProperty_TextureID         = 1900
};

enum
{
    imageType_Unknown,
    imageType_BMP,
    imageType_PNG,
    imageType_JPG,
    imageType_TIFF
};

static int detect_image_type(const char* filePath)
{
    ifstream fin(filePath, ios::binary);
    unsigned char header[8];
    fin.read((char *)header, 8);

    switch(header[0]) {
    case 0xFF:
        return !memcmp(header, kJPGHeader, 3) ? imageType_JPG : imageType_Unknown;
    case 0x89:
        return !memcmp(header, kPNGHeader, 8) ? imageType_PNG : imageType_Unknown;
    case 0x49:
        return !memcmp(header, kTIFF1Header, 4) ? imageType_TIFF : imageType_Unknown;
    case 0x4D:
        return !memcmp(header, kTIFF2Header, 4) ? imageType_TIFF : imageType_Unknown;
    case 'B':
        return !memcmp(header, kBMPHeader, 2) ? imageType_BMP : imageType_Unknown;
    default:
        return imageType_Unknown;
    }
}

extern "C" int BitmapWidgetProc(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t inParam2)
{
    char buffer[4096];
    int texture;
    int left, top, right, bottom, screenw, screenh, type;

    switch(inMessage) {
    case xpMsg_Destroy:
        texture = (int)XPGetWidgetProperty(inWidget, widgetProperty_TextureID, NULL);
        XPLMBindTexture2d(0, 0);
        glDeleteTextures(1, (GLuint *)&texture);
        return 1;
    case xpMsg_Draw:
        XPGetWidgetGeometry(inWidget, &left, &top, &right, &bottom);
        XPLMGetScreenSize(&screenw, &screenh);
        texture = (int)XPGetWidgetProperty(inWidget, widgetProperty_TextureID, NULL);
        XPLMBindTexture2d(texture, 0);
        XPLMSetGraphicsState(0, 1, 0, 0, 1, 0, 0);

        glBegin(GL_QUADS);
        glTexCoord2i(0, 0); glVertex2i(left, bottom);
        glTexCoord2i(0, 1); glVertex2i(left, top);
        glTexCoord2i(1, 1); glVertex2i(right, top);
        glTexCoord2i(1, 0); glVertex2i(right, bottom);
        glEnd();

        return 1;
    case xpMsg_Paint:
        return 0;
    case xpMsg_Create:
    case xpMsg_DescriptorChanged:
        XPGetWidgetDescriptor(inWidget, buffer, 4096);
        type = detect_image_type(buffer);
        switch(type) {
        case imageType_PNG:
            texture = load_png(buffer);
            break;
        case imageType_BMP:
            texture = load_bmp(buffer);
            break;
        case imageType_JPG:
            texture = load_jpg(buffer);
            break;
        case imageType_TIFF:
            texture = load_tiff(buffer);
            break;
         default:
            return 0;
        }

        if(!texture) {
            return 0;
        }

        XPSetWidgetProperty(inWidget, widgetProperty_TextureID, (intptr_t)texture);
        return 1;
    default:
        return 1;
    }

    
}