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

#include <amcl/pair_BN254.h>

#include "cifer/innerprod/fullysec/fhipe.h"
#include "cifer/internal/big.h"
#include "cifer/sample/uniform.h"
#include "cifer/internal/dlog.h"

cfe_error cfe_fhipe_init(cfe_fhipe *c, size_t l, mpz_t bound_x, mpz_t bound_y) {
    cfe_error err = CFE_ERR_NONE;
    mpz_t check, order;
    mpz_inits(check, order, NULL);
    mpz_from_BIG_256_56(order, (int64_t *) CURVE_Order_BN254);

    mpz_mul(check, bound_x, bound_y);
    mpz_mul_ui(check, check, l);

    if (mpz_cmp(check, order) >= 0) {
        err = CFE_ERR_PRECONDITION_FAILED;
        goto cleanup;
    }

    mpz_inits(c->bound_x, c->bound_y, c->order, NULL);

    c->l = l;
    mpz_set(c->bound_x, bound_x);
    mpz_set(c->bound_y, bound_y);
    mpz_set(c->order, order);

    cleanup:
    mpz_clears(check, order, NULL);
    return err;
}

void cfe_fhipe_copy(cfe_fhipe *res, cfe_fhipe *c) {
    mpz_inits(res->bound_x, res->bound_y, res->order, NULL);

    res->l = c->l;
    mpz_set(res->bound_x, c->bound_x);
    mpz_set(res->bound_y, c->bound_y);
    mpz_set(res->order, c->order);
}

void cfe_fhipe_free(cfe_fhipe *c) {
    mpz_clears(c->bound_x, c->bound_y, c->order, NULL);
}

void cfe_fhipe_master_key_init(cfe_fhipe_sec_key *sec_key, cfe_fhipe *c) {
    cfe_mat_inits(c->l, c->l, &(sec_key->B), &(sec_key->B_star), NULL);
}

void cfe_fhipe_master_key_free(cfe_fhipe_sec_key *sec_key) {
    cfe_mat_frees(&(sec_key->B), &(sec_key->B_star), NULL);
}

cfe_error cfe_fhipe_generate_master_key(cfe_fhipe_sec_key *sec_key, cfe_fhipe *c) {
    mpz_t exp, det;
    mpz_inits(exp, det, NULL);
    BIG_256_56 exp_big;

    cfe_uniform_sample(exp, c->order);
    BIG_256_56_from_mpz(exp_big, exp);
    ECP_BN254_generator(&(sec_key->g1));
    ECP_BN254_mul(&(sec_key->g1), exp_big);

    cfe_uniform_sample(exp, c->order);
    BIG_256_56_from_mpz(exp_big, exp);
    ECP2_BN254_generator(&(sec_key->g2));
    ECP2_BN254_mul(&(sec_key->g2), exp_big);

    cfe_uniform_sample_mat(&sec_key->B, c->order);

    cfe_mat B_inv;
    cfe_mat_init(&B_inv, c->l, c->l);
    cfe_error err = cfe_mat_inverse_mod_gauss(&B_inv, det, &sec_key->B, c->order);
    if (err != CFE_ERR_NONE) {
        goto cleanup;
    }

    cfe_mat_transpose(&sec_key->B_star, &B_inv);
    cfe_mat_mul_scalar(&sec_key->B_star, &sec_key->B_star, det);
    cfe_mat_mod(&sec_key->B_star, &sec_key->B_star, c->order);

    cleanup:
    mpz_clears(exp, det, NULL);
    cfe_mat_free(&B_inv);

    return err;
}

void cfe_fhipe_fe_key_init(cfe_fhipe_fe_key *fe_key, cfe_fhipe *c) {
    cfe_vec_G1_init(&fe_key->k2, c->l);
}

void cfe_fhipe_fe_key_free(cfe_fhipe_fe_key *fe_key) {
    cfe_vec_G1_free(&fe_key->k2);
}

cfe_error cfe_fhipe_derive_fe_key(cfe_fhipe_fe_key *fe_key, cfe_vec *y, cfe_fhipe_sec_key *sec_key, cfe_fhipe *c) {
    if (!cfe_vec_check_bound(y, c->bound_y)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    mpz_t alpha, det;
    mpz_inits(alpha, det, NULL);
    BIG_256_56 alpha_big, det_big;

    cfe_uniform_sample(alpha, c->order);
    BIG_256_56_from_mpz(alpha_big, alpha);

    cfe_mat_determinant_gauss(det, &sec_key->B, c->order);
    BIG_256_56_from_mpz(det_big, det);

    ECP_BN254_copy(&fe_key->k1, &sec_key->g1);
    ECP_BN254_mul(&fe_key->k1, alpha_big);
    ECP_BN254_mul(&fe_key->k1, det_big);

    cfe_vec alpha_B_y, B_y;
    cfe_vec_inits(c->l, &alpha_B_y, &B_y, NULL);
    cfe_mat_mul_vec(&B_y, &sec_key->B, y);
    cfe_vec_mod(&B_y, &B_y, c->order);
    cfe_vec_mul_scalar(&alpha_B_y, &B_y, alpha);
    cfe_vec_mod(&alpha_B_y, &alpha_B_y, c->order);

    cfe_vec_G1 vec_g1;
    cfe_vec_G1_init(&vec_g1, c->l);
    for (size_t i = 0; i < c->l; i++) {
        ECP_BN254_copy(&vec_g1.vec[i], &sec_key->g1);
    }

    cfe_vec_mul_vec_G1(&fe_key->k2, &alpha_B_y, &vec_g1);

    mpz_clears(alpha, det, NULL);
    cfe_vec_frees(&alpha_B_y, &B_y, NULL);
    cfe_vec_G1_free(&vec_g1);

    return CFE_ERR_NONE;
}

void cfe_fhipe_ciphertext_init(cfe_fhipe_ciphertext *cipher, cfe_fhipe *c) {
    cfe_vec_G2_init(&cipher->c2, c->l);
}

void cfe_fhipe_ciphertext_free(cfe_fhipe_ciphertext *cipher) {
    cfe_vec_G2_free(&cipher->c2);
}

cfe_error cfe_fhipe_encrypt(cfe_fhipe_ciphertext *cipher, cfe_vec *x, cfe_fhipe_sec_key *sec_key, cfe_fhipe *c) {
    if (!cfe_vec_check_bound(x, c->bound_x)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }
    mpz_t beta;
    mpz_init(beta);
    BIG_256_56 beta_big;

    cfe_uniform_sample(beta, c->order);
    BIG_256_56_from_mpz(beta_big, beta);

    ECP2_BN254_copy(&cipher->c1, &sec_key->g2);
    ECP2_BN254_mul(&cipher->c1, beta_big);

    cfe_vec beta_B_star_x;
    cfe_vec_init(&beta_B_star_x, c->l);
    cfe_mat_mul_vec(&beta_B_star_x, &sec_key->B_star, x);
    cfe_vec_mod(&beta_B_star_x, &beta_B_star_x, c->order);
    cfe_vec_mul_scalar(&beta_B_star_x, &beta_B_star_x, beta);
    cfe_vec_mod(&beta_B_star_x, &beta_B_star_x, c->order);

    cfe_vec_G2 vec_g2;
    cfe_vec_G2_init(&vec_g2, c->l);
    for (size_t i = 0; i < c->l; i++) {
        ECP2_BN254_copy(&vec_g2.vec[i], &sec_key->g2);
    }

    cfe_vec_mul_vec_G2(&cipher->c2, &beta_B_star_x, &vec_g2);

    mpz_clear(beta);
    cfe_vec_free(&beta_B_star_x);
    cfe_vec_G2_free(&vec_g2);

    return CFE_ERR_NONE;
}

cfe_error cfe_fhipe_decrypt(mpz_t res, cfe_fhipe_ciphertext *cipher, cfe_fhipe_fe_key *fe_key, cfe_fhipe *c) {
    FP12_BN254 d1, d2, paired_i;
    PAIR_BN254_ate(&d1, &cipher->c1, &fe_key->k1);
    PAIR_BN254_fexp(&d1);

    FP12_BN254_one(&d2);
    for (size_t i = 0; i < c->l; i++) {
        PAIR_BN254_ate(&paired_i, &cipher->c2.vec[i], &fe_key->k2.vec[i]);
        PAIR_BN254_fexp(&paired_i);
        FP12_BN254_mul(&d2, &paired_i);
    }

    mpz_t res_bound;
    mpz_init(res_bound);
    mpz_mul(res_bound, c->bound_x, c->bound_y);
    mpz_mul_ui(res_bound, res_bound, c->l);

    cfe_error err = cfe_baby_giant_FP12_BN256_with_neg(res, &d2, &d1, res_bound);

    mpz_clear(res_bound);

    return err;
}
