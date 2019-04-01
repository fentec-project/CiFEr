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

#include "sample/normal_negative.h"
#include "sample/uniform.h"

void cfe_normal_negative_init(cfe_normal_negative *s, mpf_t sigma, size_t n) {
    cfe_normal_init(&s->nor, sigma, n);

    mpf_t cut_f, sqrt_n;
    mpf_inits(cut_f, sqrt_n, NULL);
    mpf_sqrt_ui(sqrt_n, s->nor.n);
    mpf_mul(cut_f, s->nor.sigma, sqrt_n);

    mpz_inits(s->cut, s->twice_cut_plus_one, NULL);
    mpz_set_f(s->cut, cut_f);
    mpz_mul_ui(s->twice_cut_plus_one, s->cut, 2);
    mpz_add_ui(s->twice_cut_plus_one, s->twice_cut_plus_one, 1);

    cfe_normal_precomp_exp(&s->nor);

    mpf_clears(cut_f, sqrt_n, NULL);
}

void cfe_normal_negative_free(cfe_normal_negative *s) {
    cfe_normal_free(&s->nor);
    mpz_clears(s->cut, s->twice_cut_plus_one, NULL);
}

void cfe_normal_negative_sample(mpz_t res, cfe_normal_negative *s) {
    mpf_t u_f;
    mpf_init2(u_f, s->nor.n);

    mpz_t n, n_square, u;
    mpz_inits(n, n_square, u, NULL);

    // TODO add an exit condition
    while (1) {
        // random sample from the interval
        cfe_uniform_sample(n, s->twice_cut_plus_one);
        mpz_sub(n, n, s->cut);
        mpz_mul(n_square, n, n);

        // sample again to decide if we except the sampled value
        cfe_uniform_sample(u, s->nor.pow_n);
        mpf_set_z(u_f, u);
        mpf_div(u_f, u_f, s->nor.pow_nf);

        if (cfe_normal_is_exp_greater(&s->nor, u_f, n_square) == false) {
            mpz_set(res, n);
            break;
        }
    }

    mpz_clears(n, n_square, u, NULL);
    mpf_clear(u_f);
}

void cfe_normal_negative_sample_vec(cfe_vec *res, cfe_normal_negative *s) {
    for (size_t i = 0; i < res->size; i++) {
        cfe_normal_negative_sample(res->vec[i], s);
    }
}

void cfe_normal_negative_sample_mat(cfe_mat *res, cfe_normal_negative *s) {
    for (size_t i = 0; i < res->rows; i++) {
        cfe_normal_negative_sample_vec(&res->mat[i], s);
    }
}
