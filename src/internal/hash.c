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

#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <amcl/pbc_support.h>

#include "cifer/internal/common.h"
#include "cifer/internal/hash.h"


// cfe_hash_G1 hashes a string of length MODBYTES_256_56
// into the elliptic group element represented by ECP_BN254.
void cfe_hash_G1(ECP_BN254 *g, char *str) {
    octet tmp_str;
    char h[MODBYTES_256_56];
    octet tmp_hash = {0,sizeof(h),h};
    tmp_str.val = str;
    tmp_str.len = (int) strlen(str);
    mhashit(SHA256, -1, &tmp_str, &tmp_hash);
    ECP_BN254_mapit(g, &tmp_hash);
}

// cfe_hash_G2 hashes a string of length MODBYTES_256_56
// into the elliptic group element represented by ECP2_BN254.
void cfe_hash_G2(ECP2_BN254 *g, char *str) {
    octet tmp_str;
    char h[MODBYTES_256_56];
    octet tmp_hash = {0,sizeof(h),h};
    tmp_str.val = str;
    tmp_str.len = (int) strlen(str);
    mhashit(SHA256, -1, &tmp_str, &tmp_hash);
    ECP2_BN254_mapit(g, &tmp_hash);
}

// cfe_vec_to_string outputs a string needed for hashing
// representing the vector
char *cfe_vec_to_string(cfe_vec *v) {
    size_t alloc_len = 0;
    char *tmp_string;
    for (size_t i = 0; i < v->size; i++) {
        tmp_string = mpz_get_str(NULL, 62, v->vec[i]);
        alloc_len += strlen(tmp_string) + 1;
        free(tmp_string);
    }

    // allocate memory for the result
    char *res = (char *) cfe_malloc((alloc_len) * sizeof(char));

    // set the string
    size_t j = 0;
    for (size_t i = 0; i < v->size; i++) {
        tmp_string = mpz_get_str(NULL, 62, v->vec[i]);
        for (size_t k = 0; k < strlen(tmp_string); k++) {
            res[j] = tmp_string[k];
            j++;
        }
        free(tmp_string);
        res[j] = ' ';
        j++;
    }

    res[alloc_len - 1] = '\0';

    return res;
}

// cfe_strings_concat concatenates given strings
char *cfe_strings_concat(char *start, ...) {
    // find the length of all the strings
    va_list ap;
    char *str = start;
    va_start(ap, start);
    size_t j = 0;
    while (str != NULL) {
        for (size_t i = 0; i < strlen(str); i++) {
            j++;
        }
        str = va_arg(ap, char*);
    }
    va_end(ap);

    // allocate memory for the result
    char *res = (char *) cfe_malloc((j + 1) * sizeof(char));

    // set the string
    str = start;
    va_start(ap, start);
    j = 0;
    while (str != NULL) {
        for (size_t i = 0; i < strlen(str); i++) {
            res[j] = str[i];
            j++;
        }
        str = va_arg(ap, char*);
    }
    va_end(ap);

    // end the string
    res[j] = '\0';

    return res;
}

// cfe_int_to_str changes a non-negative int into a string of its
// decimal representation
char *cfe_int_to_str(int i) {
    int len;
    if (i == 0) {
        len = 1;
    } else {
        len = (int) log10(i) + 1;
    }
    char *result = (char *) cfe_malloc((len + 1) * sizeof(char));

    sprintf(result, "%d", i);

    return result;
}
