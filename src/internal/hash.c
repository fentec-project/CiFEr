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
#include <assert.h>

#include "cifer/internal/common.h"
#include "cifer/internal/hash.h"

void cfe_hash_G1(ECP_BN254 *g, cfe_string *str) {
    octet tmp_str = {(int) str->str_len, 0, str->str};
    char h[MODBYTES_256_56];
    octet tmp_hash = {0, sizeof(h), h};
    mhashit(SHA256, -1, &tmp_str, &tmp_hash);
    ECP_BN254_mapit(g, &tmp_hash);
}

void cfe_hash_G2(ECP2_BN254 *g, cfe_string *str) {
    octet tmp_str = {(int) str->str_len, 0, str->str};
    char h[MODBYTES_256_56];
    octet tmp_hash = {0, sizeof(h), h};
    mhashit(SHA256, -1, &tmp_str, &tmp_hash);
    ECP2_BN254_mapit(g, &tmp_hash);
}

void cfe_vec_to_string(cfe_string *out, cfe_vec *v) {
    size_t alloc_len = 0;
    char *tmp_string;
    for (size_t i = 0; i < v->size; i++) {
        alloc_len += mpz_sizeinbase(v->vec[i], 32) + 1;
        // add one char more for "-" sign if negative
        if (mpz_sgn(v->vec[i]) < 0) {
            alloc_len++;
        }
    }

    // allocate memory for the result
    out->str = (char *) cfe_malloc((alloc_len) * sizeof(char));
    out->str_len = alloc_len - 1;

    // set the string
    size_t j = 0;
    for (size_t i = 0; i < v->size; i++) {
        alloc_len = mpz_sizeinbase(v->vec[i], 32) + 1;
        if (mpz_sgn(v->vec[i]) < 0) {
            alloc_len++;
        }
        tmp_string = mpz_get_str(NULL, 32, v->vec[i]);
        for (size_t k = 0; k < alloc_len - 1; k++) {
            out->str[j] = tmp_string[k];
            j++;
        }
        free(tmp_string);
        out->str[j] = ' ';
        j++;
    }

    out->str[j - 1] = '\0';
}
