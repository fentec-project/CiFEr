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
