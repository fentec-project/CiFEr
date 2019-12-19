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
#include <cifer/sample/normal_cdt.h>
#include "cifer/internal/errors.h"
#include "cifer/sample/normal_double_constant.h"
#include "cifer/sample/uniform.h"

void cfe_normal_double_constant_init(cfe_normal_double_constant *s, mpz_t k) {
    mpz_inits(s->k, s->twice_k, NULL);
    mpz_set(s->k, k);
    mpz_mul_ui(s->twice_k, s->k, 2);

    mpf_t k_square;
    mpf_inits(k_square, s->k_square_inv, NULL);
    mpf_set_z(k_square, k);
    mpf_mul(k_square, k_square, k_square);
    mpf_ui_div(s->k_square_inv, 1, k_square);

    mpf_clear(k_square);
}

void cfe_normal_double_constant_free(cfe_normal_double_constant *s) {
    mpz_clears(s->k, s->twice_k, NULL);
    mpf_clear(s->k_square_inv);
}

void cfe_normal_double_constant_sample(mpz_t res, cfe_normal_double_constant *s) {
    // prepare values used in the loop
    mpz_t x, y, check_val;
    mpz_inits(x, y, check_val, NULL);
    int sign;
    int zero_check;

    while (true) {
        sign = 1;

        // sample a small gaussian value
        cfe_normal_cdt_sample(x);

        // sample a value
        cfe_uniform_sample(y, s->twice_k);
        if (mpz_cmp(y, s->k) >= 0) {
            sign = -1;
            mpz_sub(y, y, s->k);
        }

        // partially calculate the result and the
        // probability of accepting the result
        mpz_mul(res, s->k, x);
        mpz_mul_ui(check_val, res, 2);
        mpz_add(check_val, check_val, y);
        mpz_mul(check_val, check_val, y);
        mpz_add(res, res, y);

        // zero_check == 1 if and only if sign == 1 and res == 0
        zero_check = mpz_cmp_ui(res, 0) + sign;

        if (cfe_bernoulli(check_val, s->k_square_inv) && zero_check != 1) {
            // calculate the output
            mpz_mul_si(res, res, sign);
            break;
        }
    }

    mpz_clears(x, y, check_val, NULL);
}
