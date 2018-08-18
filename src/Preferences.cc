#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "ini.h"
#include "Preferences.h"
#include <cstdarg>
using namespace std;

struct ini_file
{
    mINI::INIFile inner;
    mINI::INIStructure contents;

    explicit ini_file(const string &path)
        :inner(mINI::INIFile(path))
    {
        struct stat buffer;
        if(stat(path.c_str(), &buffer) == 0) {
            inner.read(contents);
        }
    }
};

extern "C" {
    typedef struct ini_file ini_file_t;

    ini_file_t* ini_create(const char* path)
    {
        return new ini_file(path);
    }

    void ini_free(ini_file_t* i)
    {
        delete i;
    }

    char* ini_get_value(ini_file_t* i, const char* section, const char* key)
    {
        auto str = i->contents[section][key];
        if(str.empty()) {
            return nullptr;
        }

        auto retVal = (char *)malloc(str.length() + 1);
        strcpy(retVal, str.c_str());
        return retVal;
    }

    void ini_write_value(ini_file_t* i, const char* section, const char* key, const char* value)
    {
        i->contents[section][key] = value;
    }

    void ini_flush(ini_file_t* i)
    {
        i->inner.write(i->contents);
    }
}