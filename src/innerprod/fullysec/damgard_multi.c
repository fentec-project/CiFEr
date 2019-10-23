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
#include <cifer/internal/dlog.h>

#include "cifer/innerprod/fullysec/damgard_multi.h"
#include "cifer/internal/common.h"
#include "cifer/sample/uniform.h"

cfe_error cfe_damgard_multi_init(cfe_damgard_multi *m, size_t slots, size_t l, size_t modulus_len, mpz_t bound) {
    cfe_damgard s;
    cfe_error err = cfe_damgard_init(&s, l, modulus_len, bound);
    if (err) {
        return err;
    }

    mpz_set(s.bound, s.p);
    m->slots = slots;
    m->scheme = s;
    mpz_init_set(m->bound, bound);

    return CFE_ERR_NONE;
}

void cfe_damgard_multi_free(cfe_damgard_multi *m) {
    cfe_damgard_free(&m->scheme);
    mpz_clear(m->bound);
}

void cfe_damgard_multi_copy(cfe_damgard_multi *res, cfe_damgard_multi *m) {
    res->slots = m->slots;
    cfe_damgard_copy(&(res->scheme), &(m->scheme));
    mpz_init_set(res->bound, m->bound);
}

void cfe_damgard_multi_client_init(cfe_damgard_multi_client *e, cfe_damgard_multi *m) {
    cfe_damgard_copy(&(e->scheme), &(m->scheme));
    mpz_init_set(e->bound, m->bound);
}

void cfe_damgard_multi_client_free(cfe_damgard_multi_client *e) {
    cfe_damgard_free(&e->scheme);
    mpz_clear(e->bound);
}

void cfe_damgard_multi_sec_key_free(cfe_damgard_multi_sec_key *key) {
    for (size_t i = 0; i < key->num_keys; i++) {
        cfe_damgard_sec_key_free(&key->msk[i]);
    }
    free(key->msk);
    cfe_mat_free(&key->otp);
}

void cfe_damgard_multi_master_keys_init(cfe_mat *mpk, cfe_damgard_multi_sec_key *msk, cfe_damgard_multi *m) {
    msk->msk = cfe_malloc(sizeof(cfe_damgard_sec_key) * m->slots);
    msk->num_keys = m->slots;
    cfe_mat_inits(m->slots, m->scheme.l, mpk, &msk->otp, NULL);
}

void cfe_damgard_multi_generate_master_keys(cfe_mat *mpk, cfe_damgard_multi_sec_key *msk, cfe_damgard_multi *m) {
    cfe_vec mpk_v;
    cfe_damgard_pub_key_init(&mpk_v, &m->scheme);

    for (size_t i = 0; i < m->slots; i++) {
        cfe_damgard_sec_key_init(&msk->msk[i], &m->scheme);
        cfe_damgard_generate_master_keys(&msk->msk[i], &mpk_v, &m->scheme);
        cfe_mat_set_vec(mpk, &mpk_v, i);
    }

    cfe_uniform_sample_mat(&(msk->otp), m->scheme.q);

    cfe_vec_free(&mpk_v);
}

void cfe_damgard_multi_fe_key_init(cfe_damgard_multi_fe_key *fe_key, cfe_damgard_multi *m) {
    fe_key->slots = m->slots;
    mpz_init(fe_key->z);
    fe_key->keys = (cfe_damgard_fe_key *) cfe_malloc(sizeof(cfe_damgard_fe_key) * m->slots);
    for (size_t i = 0; i < m->slots; i++) {
        cfe_damgard_fe_key_init(&(fe_key->keys[i]));
    }
}


void cfe_damgard_multi_fe_key_free(cfe_damgard_multi_fe_key *key) {
    for (size_t i = 0; i < key->slots; i++) {
        cfe_damgard_derived_key_free(&(key->keys[i]));
    }
    free(key->keys);
    mpz_clear(key->z);
}

cfe_error cfe_damgard_multi_derive_key(cfe_damgard_multi_fe_key *fe_key, cfe_damgard_multi *m,
        cfe_damgard_multi_sec_key *msk, cfe_mat *y) {
    if (!cfe_mat_check_bound(y, m->bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    cfe_error err = CFE_ERR_NONE;

    cfe_mat_dot(fe_key->z, &msk->otp, y);
    mpz_mod(fe_key->z, fe_key->z, m->scheme.q);

    for (size_t i = 0; i < m->slots; i++) {
        err = cfe_damgard_derive_key(&(fe_key->keys[i]), &m->scheme, &msk->msk[i], cfe_mat_get_row_ptr(y, i));
        if (err) {
            cfe_damgard_multi_fe_key_free(fe_key);
            break;
        }
    }

    return err;
}

void cfe_damgard_multi_ciphertext_init(cfe_vec *ciphertext, cfe_damgard_multi_client *e) {
    cfe_damgard_ciphertext_init(ciphertext, &e->scheme);
}

cfe_error cfe_damgard_multi_encrypt(cfe_vec *ciphertext, cfe_damgard_multi_client *e, cfe_vec *x, cfe_vec *pub_key, cfe_vec *otp) {
    if (!cfe_vec_check_bound(x, e->bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    cfe_vec otp_add_x;
    cfe_vec_init(&otp_add_x, x->size);

    cfe_vec_add(&otp_add_x, x, otp);
    cfe_vec_mod(&otp_add_x, &otp_add_x, e->scheme.q);

    cfe_error err = cfe_damgard_encrypt(ciphertext, &e->scheme, &otp_add_x, pub_key);

    cfe_vec_free(&otp_add_x);
    return err;
}

cfe_error cfe_damgard_multi_decrypt(mpz_t res, cfe_damgard_multi *m, cfe_vec *ciphertext, cfe_damgard_multi_fe_key *key,
                                    cfe_mat *y) {
    if (!cfe_mat_check_bound(y, m->bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    mpz_t order, bound, z_exp, z_exp_inv, r, num, denom, denom_inv, t1, t2;
    mpz_inits(order, bound, z_exp, z_exp_inv, r, num, denom, denom_inv, t1, t2, NULL);
    mpz_set_ui(r, 1);

    for (size_t i = 0; i < m->slots; i++) {
        mpz_set_ui(num, 1);

        for (size_t j = 0; j < m->scheme.l; j++) {
            mpz_powm(t1, ciphertext[i].vec[j+2], y->mat[i].vec[j], m->scheme.p);
            mpz_mul(num, num, t1);
            mpz_mod(num, num, m->scheme.p);
        }

        mpz_powm(t1, ciphertext[i].vec[0], key->keys[i].key1, m->scheme.p);
        mpz_powm(t2, ciphertext[i].vec[1], key->keys[i].key2, m->scheme.p);
        mpz_mul(denom, t1, t2);
        mpz_mod(denom, denom, m->scheme.p);

        mpz_invert(denom_inv, denom, m->scheme.p);

        mpz_mul(r, r, num);
        mpz_mul(r, r, denom_inv);
        mpz_mod(r, r, m->scheme.p);
    }

    mpz_powm(z_exp, m->scheme.g, key->z, m->scheme.p);
    mpz_invert(z_exp_inv, z_exp, m->scheme.p);

    mpz_mul(r, r, z_exp_inv);
    mpz_mod(r, r, m->scheme.p);


    mpz_sub_ui(order, m->scheme.p, 1);
    mpz_pow_ui(bound, m->bound, 2);
    mpz_mul_ui(bound, bound, m->slots*m->scheme.l);

    cfe_error err = cfe_baby_giant_with_neg(res, r, m->scheme.g, m->scheme.p, order, bound);
    mpz_clears(order, bound, z_exp, z_exp_inv, r, num, denom, denom_inv, t1, t2, NULL);

    return err;
}
