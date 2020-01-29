/*
 * Copyright (c) 2018 XLAB d.o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CIFER_STR_H
#define CIFER_STR_H

#include <stddef.h>

/**
* \file
* \ingroup internal
* \brief Internal functions for dealing with strings.
*/

/**
 * cfe_string is an internal struct that holds a
 * string and its length.
 */
typedef struct cfe_string {
    char *str;
    size_t str_len;
} cfe_string;

/**
 * A helping function that transforms a string with
 * an integer to an int.
 */
int cfe_str_to_int(cfe_string *str_int);

/**
 * A helping function that takes a substring of a string.
 */
void cfe_substring(cfe_string *out, cfe_string *in, size_t start, size_t stop);

/**
 * A helping function that removes spaces in a string
 */
void cfe_remove_spaces(cfe_string *out, cfe_string *source);

/**
 * A function that frees the memory of a string in
 * a cfe_string struct.
 */
void cfe_string_free(cfe_string *str);

/**
 * The function concatenates given strings.
 *
 * @param out Result will be saved here
 * @param start Strings to bo joined
 */
void cfe_strings_concat(cfe_string *out, cfe_string *start, ...);

/**
 * The function converts a non-negative int into a string of its
 * decimal representation that can be used for hashing.
 *
 * @param out Result will be saved here
 * @param i Non-negative integer
 */
void cfe_int_to_str(cfe_string *out, int i);
#endif
