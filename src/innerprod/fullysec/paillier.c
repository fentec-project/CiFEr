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
#include <gmp.h>

#include "cifer/innerprod/fullysec/paillier.h"
#include "cifer/internal/prime.h"
#include "cifer/sample/uniform.h"
#include "cifer/sample/normal_double_constant.h"
#include "cifer/sample/normal_cdt.h"

cfe_error cfe_paillier_init(cfe_paillier *s, size_t l, size_t lambda, size_t bit_len, mpz_t bound_x, mpz_t bound_y) {
    mpz_t p, q, n, n_square, check, g_prime, g, n_to_5, k_sigma;
    mpz_inits(p, q, n, n_square, check, g_prime, g, n_to_5, k_sigma, NULL);

    mpf_t sigma, sigma_cdt, k_sigma_f;
    mpf_inits(sigma, sigma_cdt, k_sigma_f, NULL);

    // generate two safe primes
    cfe_error err = cfe_get_prime(p, bit_len, true);
    if (err) {
        goto cleanup;
    }
    err = cfe_get_prime(q, bit_len, true);
    if (err) {
        goto cleanup;
    }
    // calculate n = p * q and n^2
    mpz_mul(n, p, q);
    mpz_mul(n_square, n, n);

    // check if the parameters of the scheme are compatible,
    // i.e. security parameter should be big enough that
    // the generated n is much greater than l and the bounds
    mpz_mul(check, bound_x, bound_x);
    mpz_mul_ui(check, check, l);
    if (mpz_cmp(n, check) < 1) {
        err = CFE_ERR_PARAM_GEN_FAILED;
        goto cleanup;
    }
    mpz_mul(check, bound_y, bound_y);
    mpz_mul_ui(check, check, l);
    if (mpz_cmp(n, check) < 1) {
        err = CFE_ERR_PARAM_GEN_FAILED;
        goto cleanup;
    }

    // generate a generator for the 2n-th residues subgroup of Z_n^2*
    cfe_uniform_sample(g_prime, n_square);
    mpz_powm(g, g_prime, n, n_square);
    mpz_powm_ui(g, g, 2, n_square);

    // check if generated g is invertible, which should be the case except with
    // negligible probability
    if (!mpz_invert(check, g, n_square)) {
        err = CFE_ERR_PARAM_GEN_FAILED;
        goto cleanup;
    }

    // calculate sigma
    mpz_pow_ui(n_to_5, n, 5);
    mpf_set_z(sigma, n_to_5);
    mpf_mul_ui(sigma, sigma, lambda);
    mpf_sqrt(sigma, sigma);
    mpf_add_ui(sigma, sigma, 2);

    // to sample with cfe_normal_double_constant sigma must be
    // a multiple of cfe_sigma_cdt = sqrt(1/(2ln(2))), hence we make
    // it such
    mpf_set_d(sigma_cdt, cfe_sigma_cdt);
    mpf_div(k_sigma_f, sigma, sigma_cdt);
    mpz_set_f(k_sigma, k_sigma_f);
    mpz_add_ui(k_sigma, k_sigma, 1);
    mpf_set_z(k_sigma_f, k_sigma);
    mpf_mul(sigma, k_sigma_f, sigma_cdt);

    // set the parameters for the scheme
    s->l = l;
    mpz_init_set(s->n, n);
    mpz_init_set(s->n_square, n_square);
    mpz_init_set(s->bound_x, bound_x);
    mpz_init_set(s->bound_y, bound_y);
    mpf_init_set(s->sigma, sigma);
    mpz_init_set(s->k_sigma, k_sigma);
    s->lambda = lambda;
    mpz_init_set(s->g, g);

    cleanup:
    mpz_clears(p, q, n, n_square, check, g_prime, g, n_to_5, k_sigma, NULL);
    mpf_clears(sigma, sigma_cdt, k_sigma_f, NULL);
    return err;
}

void cfe_paillier_free(cfe_paillier *s) {
    mpz_clears(s->n, s->n_square, s->bound_x, s->bound_y, s->g, s->k_sigma, NULL);
    mpf_clear(s->sigma);
}

// res should be uninitialized!
void cfe_paillier_copy(cfe_paillier *res, cfe_paillier *s) {
    res->l = s->l;
    res->lambda = s->lambda;
    mpz_init_set(res->bound_y, s->bound_y);
    mpz_init_set(res->bound_x, s->bound_x);
    mpz_init_set(res->g, s->g);
    mpz_init_set(res->n, s->n);
    mpz_init_set(res->n_square, s->n_square);
    mpf_init_set(res->sigma, s->sigma);
    mpz_init_set(res->k_sigma, s->k_sigma);
}

void cfe_paillier_master_keys_init(cfe_vec *msk, cfe_vec *mpk, cfe_paillier *s) {
    cfe_vec_inits(s->l, msk, mpk, NULL);
}

cfe_error cfe_paillier_generate_master_keys(cfe_vec *msk, cfe_vec *mpk, cfe_paillier *s) {
    if (msk->size != s->l || mpk->size != s->l) {
        return CFE_ERR_MALFORMED_INPUT;
    }

    mpf_t one;
    mpf_init_set_ui(one, 1);
    cfe_normal_double_constant sampler;
    cfe_normal_double_constant_init(&sampler, s->k_sigma);

    mpz_t x, y;
    mpz_inits(x, y, NULL);

    cfe_normal_double_constant_sample_vec(msk, &sampler);

    for (size_t i = 0; i < s->l; i++) {
        cfe_vec_get(y, msk, i);
        mpz_powm(x, s->g, y, s->n_square);
        cfe_vec_set(mpk, x, i);
    }

    mpf_clear(one);
    mpz_clears(x, y, NULL);
    cfe_normal_double_constant_free(&sampler);
    return CFE_ERR_NONE;
}

cfe_error cfe_paillier_derive_fe_key(mpz_t fe_key, cfe_paillier *s, cfe_vec *msk, cfe_vec *y) {
    if (!cfe_vec_check_bound(y, s->bound_y)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    cfe_vec_dot(fe_key, msk, y);
    return CFE_ERR_NONE;
}

void cfe_paillier_ciphertext_init(cfe_vec *ciphertext, cfe_paillier *s) {
    cfe_vec_init(ciphertext, s->l + 1);
}

cfe_error cfe_paillier_encrypt(cfe_vec *ciphertext, cfe_paillier *s, cfe_vec *x, cfe_vec *mpk) {
    if (!cfe_vec_check_bound(x, s->bound_x)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    mpz_t n_div_4, r, c_0, c_i, x_i, mpk_i, tmp1, tmp2;
    mpz_inits(n_div_4, r, c_0, c_i, x_i, mpk_i, tmp1, tmp2, NULL);
    mpz_div_ui(n_div_4, s->n, 4);
    cfe_uniform_sample(r, n_div_4);

    mpz_powm(c_0, s->g, r, s->n_square);
    cfe_vec_set(ciphertext, c_0, 0);

    for (size_t i = 0; i < s->l; i++) {
        cfe_vec_get(x_i, x, i);
        mpz_mul(tmp1, x_i, s->n);
        mpz_add_ui(tmp1, tmp1, 1);
        cfe_vec_get(mpk_i, mpk, i);
        mpz_powm(tmp2, mpk_i, r, s->n_square);
        mpz_mul(c_i, tmp1, tmp2);
        mpz_mod(c_i, c_i, s->n_square);
        cfe_vec_set(ciphertext, c_i, i + 1);
    }

    mpz_clears(n_div_4, r, c_0, c_i, x_i, mpk_i, tmp1, tmp2, NULL);
    return CFE_ERR_NONE;
}

cfe_error cfe_paillier_decrypt(mpz_t res, cfe_paillier *s, cfe_vec *ciphertext, mpz_t key, cfe_vec *y) {
    if (!cfe_vec_check_bound(y, s->bound_y)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    mpz_t key_neg, c_0, c_i, y_i, tmp, half_n;
    mpz_inits(key_neg, c_0, c_i, y_i, tmp, half_n, NULL);
    mpz_neg(key_neg, key);
    cfe_vec_get(c_0, ciphertext, 0);
    mpz_powm(res, c_0, key_neg, s->n_square);

    for (size_t i = 1; i < ciphertext->size; i++) {
        cfe_vec_get(c_i, ciphertext, i);
        cfe_vec_get(y_i, y, i - 1);
        mpz_powm(tmp, c_i, y_i, s->n_square);
        mpz_mul(res, res, tmp);
        mpz_mod(res, res, s->n_square);
    }

    mpz_sub_ui(res, res, 1);
    mpz_mod(res, res, s->n_square);
    mpz_div(res, res, s->n);

    mpz_div_ui(half_n, s->n, 2);
    if (mpz_cmp(res, half_n) > 0) {
        mpz_sub(res, res, s->n);
    }

    mpz_clears(key_neg, c_0, c_i, y_i, tmp, half_n, NULL);
    return CFE_ERR_NONE;
}
