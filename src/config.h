/*
* config.h
*
* Copyright (c) 2017 Teklad
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef _TCONFIG_H_
#define _TCONFIG_H_
#include <stdbool.h>

typedef struct ini_table_s ini_table_s;

/**
 * @brief Creates an empty ini_table_s struct for writing new entries to.
 * @return ini_table_s*
 */
ini_table_s* ini_table_create();

/**
 * @brief Free up all the allocated resources in the ini_table_s struct.
 * @param table
 */
void ini_table_destroy(ini_table_s* table);

/**
 * @brief Creates an ini_table_s struct filled with data from the specified
 *        `file'.  Returns NULL if the file can not be read.
 * @param table
 * @param file
 * @return ini_table_s*
 */
bool ini_table_read_from_file(ini_table_s* table, const char* file);

/**
 * @brief Writes the specified ini_table_s struct to the specified `file'.
 *        Returns false if the file could not be opened for writing, otherwise
 *        true.
 * @param table
 * @param file
 * @return bool
 */
bool ini_table_write_to_file(ini_table_s* table, const char* file);

/**
 * @brief Creates a new entry in the `table' containing the `key' and `value'
 *        provided if it does not exist.  Otherwise, modifies an exsiting `key'
 *        with the new `value'
 * @param table
 * @param section_name
 * @param key
 * @param value
 */
void ini_table_create_entry(ini_table_s* table, const char* section_name,
        const char* key, const char* value);

/**
 * @brief Checks for the existance of an entry in the specified `table'.  Returns
 *        false if the entry does not exist, otherwise true.
 * @param table
 * @param section_name
 * @param key
 * @return bool
 */
bool ini_table_check_entry(ini_table_s* table, const char* section_name,
    const char* key);

/**
 * @brief Retrieves the unmodified value of the specified `key' in `section_name'.
 *        Returns NULL if the entry does not exist, otherwise a pointer to the 
 *        entry value data.
 * @param table
 * @param section_name
 * @param key
 * @return const char*
 */
const char* ini_table_get_entry(ini_table_s* table, const char* section_name,
        const char* key);

/**
 * @brief Retrieves the value of the specified `key' in `section_name', converted
 *        to int.  Returns false on failure, otherwise true.
 * @param table
 * @param section_name
 * @param key
 * @param [out]value
 * @return int
 */
bool ini_table_get_entry_as_int(ini_table_s* table, const char* section_name,
    const char* key, int* value);

/**
 * @brief Retrieves the value of the specified `key' in `section_name', converted
 *        to bool.  Returns false on failure, true otherwise.
 * @param table
 * @param section_name
 * @param key
 * @param [out]value
 * @return bool
 */
bool ini_table_get_entry_as_bool(ini_table_s* table, const char* section_name,
    const char* key, bool* value);

#endif//_TCONFIG_H_