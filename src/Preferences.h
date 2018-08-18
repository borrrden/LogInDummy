#pragma once


#ifdef __cplusplus
extern "C" {
#endif
    typedef struct ini_file ini_file_t;

    ini_file_t* ini_create(const char* path);
    void ini_free(ini_file_t*);

    char* ini_get_value(ini_file_t*, const char* section, const char* key);
    void ini_write_value(ini_file_t*, const char* section, const char* key, const char* value);
    void ini_flush(ini_file_t*);

#ifdef __cplusplus
}
#endif