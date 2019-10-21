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

#include <sodium.h>

#include "cifer/internal/common.h"
#include "cifer/sample/uniform.h"

bool cfe_bit_sample(void) {
    return (bool) randombytes_uniform(2);
}

void cfe_uniform_sample(mpz_t res, mpz_t upper) {
    // determine the size of buffer to read random bytes in and allocate it
    size_t n_bits = mpz_sizeinbase(upper, 2);
    size_t n_bytes = ((n_bits - 1) / 8) + 1;
    uint8_t *rand_bytes = (uint8_t *) cfe_malloc(n_bytes * sizeof(uint8_t));

    while (1) {
        randombytes_buf(rand_bytes, n_bytes); // get random bytes

        // make a big integer number from random bytes
        // result is always positive
        mpz_import(res, n_bytes, 1, 1, 0, 0, rand_bytes);

        // random number too big, divide it
        if (mpz_sizeinbase(res, 2)  > n_bits) {
            mpz_fdiv_r_2exp(res, res, n_bits);
        }
        // if we're below the upper bound, we have a valid random number
        if (mpz_cmp(res, upper) < 0) {
            break;
        }
    }

    free(rand_bytes);
}

void cfe_uniform_sample_i(mpz_t res, size_t upper) {
    mpz_t upper_z;
    mpz_init_set_ui(upper_z, upper);

    cfe_uniform_sample(res, upper_z);

    mpz_clear(upper_z);
}

void cfe_uniform_sample_range(mpz_t res, mpz_t min, mpz_t max) {
    mpz_t max_sub_min;
    mpz_init(max_sub_min);
    mpz_sub(max_sub_min, max, min);

    cfe_uniform_sample(res, max_sub_min); // sets res to be from [0, max-min)
    mpz_add(res, res, min);                     // sets res to be from [min, max)

    mpz_clear(max_sub_min);
}

void cfe_uniform_sample_range_i_mpz(mpz_t res, int min, mpz_t max) {
    mpz_t min_z;
    mpz_init_set_si(min_z, min);

    cfe_uniform_sample_range(res, min_z, max);

    mpz_clear(min_z);
}

void cfe_uniform_sample_range_i_i(mpz_t res, int min, int max) {
    mpz_t min_z, max_z;
    mpz_init_set_si(min_z, min);
    mpz_init_set_si(max_z, max);

    cfe_uniform_sample_range(res, min_z, max_z);

    mpz_clears(min_z, max_z, NULL);
}

void cfe_uniform_sample_vec(cfe_vec *res, mpz_t max) {
    for (size_t i = 0; i < res->size; i++) {
        cfe_uniform_sample(res->vec[i], max);
    }
}

void cfe_uniform_sample_range_vec(cfe_vec *res, mpz_t lower, mpz_t upper) {
    for (size_t i = 0; i < res->size; i++) {
        cfe_uniform_sample_range(res->vec[i], lower, upper);
    }
}

void cfe_uniform_sample_mat(cfe_mat *res, mpz_t max) {
    for (size_t i = 0; i < res->rows; i++) {
        cfe_uniform_sample_vec(&res->mat[i], max);
    }
}

void cfe_uniform_sample_range_mat(cfe_mat *res, mpz_t lower, mpz_t upper) {
    for (size_t i = 0; i < res->rows; i++) {
        cfe_uniform_sample_range_vec(&res->mat[i], lower, upper);
    }
}

void cfe_uniform_sample_vec_det(cfe_vec *res, mpz_t max, unsigned char *key) {
    size_t n_bits_max = mpz_sizeinbase(max, 2);
    size_t n_bytes_for_vec = ((n_bits_max * res->size - 1) / 8) + 1;

    mpz_t num, el;
    mpz_inits(num, el, NULL);

    for (size_t i = 3;; i++) {
        uint8_t *rand_bytes = (uint8_t *) cfe_malloc(i * n_bytes_for_vec * sizeof(uint8_t));
        randombytes_buf_deterministic(rand_bytes, i * n_bytes_for_vec, key);

        mpz_import(num, i * n_bytes_for_vec, 1, 1, 0, 0, rand_bytes);

        free(rand_bytes);

        size_t j = 0, k = 0;
        while (j <= (i * n_bytes_for_vec * 8 - n_bits_max)) {
            mpz_fdiv_r_2exp(el, num, n_bits_max);
            mpz_fdiv_q_2exp(num, num, n_bits_max);

            if (mpz_cmp(el, max) < 0) {
                cfe_vec_set(res, el, k);
                k++;
            }
            if (k == res->size) {
                break;
            }
            j += n_bits_max;
        }
        if (k == res->size) {
            break;
        }
    }
    mpz_clears(num, el, NULL);
}

void cfe_uniform_sample_mat_det(cfe_mat *res, mpz_t max, unsigned char *key) {
    cfe_vec v;
    cfe_vec_init(&v, res->rows * res->cols);
    cfe_uniform_sample_vec_det(&v, max, key);
    cfe_mat_from_vec(res, &v);
    cfe_vec_free(&v);
}
