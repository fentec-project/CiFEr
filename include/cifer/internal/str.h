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
* \brief Internal struct for dealing with strings.
*/

typedef struct cfe_string {
    char *str;
    size_t str_len;
} cfe_string;

/**
 * A helping function used in boolean_to_msp_iterative.
 */
int cfe_str_to_int(cfe_string *str_int);

/**
 * A helping function used in boolean_to_msp_iterative.
 */
void cfe_substring(cfe_string *out, cfe_string *in, size_t start, size_t stop);

/**
 * A helping function used in boolean_to_msp_iterative.
 */
void cfe_remove_spaces(cfe_string *out, cfe_string *source);


void cfe_string_free(cfe_string *str);

#endif
