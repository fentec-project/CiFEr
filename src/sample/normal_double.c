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
#include "cifer/internal/errors.h"
#include "cifer/sample/normal_double.h"
#include "cifer/sample/uniform.h"

cfe_error cfe_normal_double_init(cfe_normal_double *s, mpf_t sigma, size_t n, mpf_t first_sigma) {
    cfe_error err = CFE_ERR_NONE;

    // use this to check if the struct was initialized
    s->nor.pre_exp.vec = NULL;
    mpf_t k_f;
    mpf_init(k_f);
    mpf_div(k_f, sigma, first_sigma);

    // sigma should be a multiple of first_sigma
    if (!mpf_integer_p(k_f)) {
        err = CFE_ERR_PRECONDITION_FAILED;
        goto cleanup;
    }

    mpz_inits(s->k, s->twice_k, NULL);
    mpz_set_f(s->k, k_f);
    mpz_mul_ui(s->twice_k, s->k, 2);

    cfe_normal_init(&s->nor, sigma, n);
    cfe_normal_cumulative_init(&s->sampler_cumu, first_sigma, n, false);

    cfe_normal_precomp_exp(&s->nor);

    cleanup:
    mpf_clear(k_f);
    return err;
}

void cfe_normal_double_free(cfe_normal_double *s) {
    // check if the struct was initialized
    if (s->nor.pre_exp.vec != NULL) {
        cfe_normal_free(&s->nor);
        cfe_normal_cumulative_free(&s->sampler_cumu);
        mpz_clears(s->k, s->twice_k, NULL);
    }
}

void cfe_normal_double_sample(mpz_t res, cfe_normal_double *s) {
    // prepare values used in the loop
    mpz_t x, y, u, check_val;
    mpz_inits(x, y, u, check_val, NULL);
    mpf_t u_f;
    mpf_init2(u_f, s->nor.n);

    while (1) {
        int sign = 1;

        // sample a small gaussian value
        cfe_normal_cumulative_sample(x, &s->sampler_cumu);

        // sample a value
        cfe_uniform_sample(y, s->twice_k);
        if (mpz_cmp(y, s->k) >= 0) {
            sign = -1;
            mpz_sub(y, y, s->k);
        }

        // calculate the value according to whose probability
        // the sample will be accepted
        mpz_mul(check_val, s->k, x);
        mpz_mul_ui(check_val, check_val, 2);
        mpz_add(check_val, check_val, y);
        mpz_mul(check_val, check_val, y);

        // sample if we accept the value
        cfe_uniform_sample(u, s->nor.pow_n);

        // sample u_f a such that u_f/precision_f is approximately
        // a random value from [0, 1]
        mpf_set_z(u_f, u);
        mpf_div(u_f, u_f, s->nor.pow_nf);

        if (cfe_normal_is_exp_greater(&s->nor, u_f, check_val) == false) {
            // calculate the output
            mpz_mul(res, x, s->k);
            mpz_add(res, res, y);
            mpz_mul_si(res, res, sign);

            // in case the value is 0 we need to sample again to decide if we
            // accept the value, otherwise we return the value
            if (mpz_cmp_ui(res, 0) == 0) {
                if (cfe_bit_sample()) {
                    break;
                }
            } else {
                break;
            }
        }
    }

    mpf_clear(u_f);
    mpz_clears(x, y, u, check_val, NULL);
}

void cfe_normal_double_sample_vec(cfe_vec *res, cfe_normal_double *s) {
    for (size_t i = 0; i < res->size; i++) {
        cfe_normal_double_sample(res->vec[i], s);
    }
}

void cfe_normal_double_sample_mat(cfe_mat *res, cfe_normal_double *s) {
    for (size_t i = 0; i < res->rows; i++) {
        cfe_normal_double_sample_vec(&res->mat[i], s);
    }
}
