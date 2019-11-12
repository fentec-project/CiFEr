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



#include <gmp.h>
#include <cifer/internal/big.h>
#include <cifer/sample/uniform.h>
#include "cifer/innerprod/fullysec/fhipe.h"

void cfe_fhipe_init(cfe_fhipe *c, size_t l, mpz_t bound_x, mpz_t bound_y) {
    mpz_inits(c->bound_x, c->bound_y, c->order, NULL);

    c->l = l;
    mpz_set(c->bound_x, bound_x);
    mpz_set(c->bound_y, bound_y);
    BIG_256_56_rcopy(c->order_big, CURVE_Order_BN254);
    mpz_from_BIG_256_56(c->order, c->order_big);
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
        return err;
    }

    cfe_mat_transpose(&sec_key->B_star, &B_inv);
    cfe_mat_mul_scalar(&sec_key->B_star, &sec_key->B_star, det);
    cfe_mat_mod(&sec_key->B_star, &sec_key->B_star, c->order);

    return CFE_ERR_NONE;
}

void cfe_fhipe_FE_key_init(cfe_fhipe_FE_key *fe_key, cfe_fhipe *c) {
    cfe_vec_G1_init(&fe_key->k2, c->l);
}

void cfe_fhipe_derive_FE_key(cfe_fhipe_FE_key *fe_key, cfe_vec *y, cfe_fhipe_sec_key *sec_key, cfe_fhipe *c) {
    mpz_t alpha, det;
    mpz_inits(alpha, det, NULL);
    BIG_256_56 alpha_big;

    cfe_uniform_sample(alpha, c->order);
    BIG_256_56_from_mpz(alpha_big, alpha);

    cfe_mat_determinant_gauss(det, &sec_key->B, c->order);

    ECP_BN254_copy(&fe_key->k1, &sec_key->g1);
    ECP_BN254_mul(&fe_key->k1, alpha_big);

    cfe_vec alpha_B_y;
    cfe_vec_init(&alpha_B_y, c->l);

    cfe_vec_G1 vec_g1;
    cfe_vec_G1_init(&vec_g1, c->l);

    for (size_t i = 0; i < c->l; i++) {
        ECP_BN254_copy(&vec_g1.vec[i], &sec_key->g1);
    }

    cfe_vec_mul_vec_G1(&fe_key->k2, &alpha_B_y, &vec_g1);
}












