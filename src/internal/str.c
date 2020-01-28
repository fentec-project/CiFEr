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

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

#include "cifer/internal/common.h"
#include "cifer/internal/str.h"

void cfe_string_free(cfe_string *str) {
    free(str->str);
}

int cfe_str_to_int(cfe_string *str_int) {
    int result = 0;
    for (size_t i = 0; i < str_int->str_len; i++) {
        if ((str_int->str[i] < '0') || (str_int->str[i] > '9')) {
            return -1;
        } else {
            result = (result * 10) + ((str_int->str[i]) - '0');
        }
    }
    return result;
}

void cfe_substring(cfe_string *out, cfe_string *in, size_t start, size_t stop) {
    out->str = (char *) cfe_malloc(sizeof(char) * (stop - start + 1));
    out->str_len = stop - start;
    for (size_t i = start; i < stop; i++) {
        out->str[i - start] = in->str[i];
    }
    out->str[stop - start] = '\0';
}

void cfe_remove_spaces(cfe_string *out, cfe_string *source) {
    size_t count = 0;
    for (size_t i = 0; i < source->str_len; i++) {
        if (source->str[i] != ' ') {
            count++;
        }
    }

    out->str = (char *) cfe_malloc(sizeof(char) * (count + 1));
    count = 0;
    for (size_t i = 0; i < source->str_len; i++) {
        if (source->str[i] != ' ') {
            out->str[count] = source->str[i];
            count++;
        }
    }
    out->str[count] = '\0';
    out->str_len = count;
}


void cfe_strings_concat(cfe_string *out, cfe_string *start, ...) {
    // find the length of all the strings
    va_list ap;
    cfe_string *str = start;
    va_start(ap, start);
    size_t j = 0;
    while (str != NULL) {
        j += str->str_len;
        str = va_arg(ap, cfe_string*);
    }
    va_end(ap);

    // allocate memory for the result
    out->str = (char *) cfe_malloc((j + 1) * sizeof(char));
    out->str_len = j;

    // set the string
    str = start;
    va_start(ap, start);
    j = 0;
    while (str != NULL) {
        for (size_t i = 0; i < str->str_len; i++) {
            out->str[j] = str->str[i];
            j++;
        }
        str = va_arg(ap, cfe_string*);
    }
    va_end(ap);

    // end the string
    out->str[j] = '\0';
}

// cfe_int_to_str changes a non-negative int into a string of its
// decimal representation
void cfe_int_to_str(cfe_string *out, int i) {
    int len;
    if (i == 0) {
        len = 1;
    } else {
        len = (int) log10(i) + 1;
    }
    out->str = (char *) cfe_malloc((len + 1) * sizeof(char));
    out->str_len = (size_t) len;

    sprintf(out->str, "%d", i);
}
