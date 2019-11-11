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

void cfe_fhipe_generate_master_key_init(cfe_fhipe_sec_key *sec_key, cfe_fhipe *c) {
    cfe_mat_inits(c->l, c->l, &(sec_key->B), &(sec_key->B_star), NULL);
}

void cfe_fhipe_generate_master_key_free(cfe_fhipe_sec_key *sec_key) {
    cfe_mat_frees(&(sec_key->B), &(sec_key->B_star), NULL);
}

void cfe_fhipe_generate_master_key(cfe_fhipe_sec_key *sec_key, cfe_fhipe *c) {
    mpz_t exp;
    mpz_init(exp);
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

}













