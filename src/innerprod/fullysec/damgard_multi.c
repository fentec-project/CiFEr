/* *
 * Copyright (C) 2018 XLAB d.o.o.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of either:
 *
 *     * the GNU Lesser General Public License as published by the Free
 *     Software Foundation; either version 3 of the License, or (at your
 *     option) any later version.
 *
 * or
 *
 *     * the GNU General Public License as published by the Free Software
 *     Foundation; either version 2 of the License, or (at your option) any
 *     later version.
 *
 * or both in parallel, as here.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#include "innerprod/fullysec/damgard_multi.h"
#include "internal/common.h"
#include "sample/uniform.h"

cfe_error cfe_damgard_multi_init(cfe_damgard_multi *m, size_t slots, size_t l, size_t modulus_len, mpz_t bound) {
    cfe_damgard s;
    cfe_error err = cfe_damgard_init(&s, l, modulus_len, bound);
    if (err) {
        return err;
    }

    m->slots = slots;
    m->scheme = s;
    return CFE_ERR_NONE;
}

void cfe_damgard_multi_free(cfe_damgard_multi *m) {
    cfe_damgard_free(&m->scheme);
}

void cfe_damgard_multi_copy(cfe_damgard_multi *res, cfe_damgard_multi *m) {
    res->slots = m->slots;
    cfe_damgard s;
    cfe_damgard_copy(&s, &m->scheme);
    res->scheme = s;
}

void cfe_damgard_multi_enc_init(cfe_damgard_multi_enc *e, cfe_damgard_multi *m) {
    cfe_damgard e_scheme;
    cfe_damgard_copy(&e_scheme, &m->scheme);
    e->scheme = e_scheme;
}

void cfe_damgard_multi_enc_free(cfe_damgard_multi_enc *e) {
    cfe_damgard_free(&e->scheme);
}

void cfe_damgard_multi_sec_key_free(cfe_damgard_multi_sec_key *key) {
    for (size_t i = 0; i < key->num_keys; i++) {
        cfe_damgard_sec_key_free(&key->msk[i]);
    }
    free(key->msk);
    cfe_mat_free(&key->otp);
}

void cfe_damgard_multi_fe_key_free(cfe_damgard_multi_fe_key *key) {
    cfe_vec_frees(&key->keys1, &key->keys2, NULL);
    mpz_clear(key->z);
}

void cfe_damgard_multi_generate_master_keys(cfe_mat *mpk, cfe_damgard_multi_sec_key *msk, cfe_damgard_multi *m) {
    msk->msk = cfe_malloc(sizeof(cfe_damgard_sec_key) * m->slots);
    msk->num_keys = m->slots;
    cfe_mat_inits(m->slots, m->scheme.l, mpk, &msk->otp, NULL);

    cfe_vec rand_v;
    cfe_vec_init(&rand_v, m->scheme.l);

    for (size_t i = 0; i < m->slots; i++) {
        cfe_vec mpk_v;
        cfe_damgard_generate_master_keys(&msk->msk[i], &mpk_v, &m->scheme);
        cfe_mat_set_vec(mpk, &mpk_v, i);

        cfe_vec_free(&mpk_v);

        cfe_uniform_sample_vec(&rand_v, m->scheme.bound);
        cfe_mat_set_vec(&msk->otp, &rand_v, i);
    }

    cfe_vec_free(&rand_v);
}

cfe_error
cfe_damgard_multi_derive_key(cfe_damgard_multi_fe_key *res, cfe_damgard_multi *m, cfe_damgard_multi_sec_key *msk,
                             cfe_mat *y) {
    if (!cfe_mat_check_bound(y, m->scheme.bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    cfe_error err = CFE_ERR_NONE;

    mpz_init(res->z);

    cfe_mat_dot(res->z, &msk->otp, y);
    mpz_mod(res->z, res->z, m->scheme.bound);

    cfe_vec_init(&res->keys1, m->scheme.l);
    cfe_vec_init(&res->keys2, m->scheme.l);

    for (size_t i = 0; i < m->slots; i++) {
        cfe_damgard_fe_key derived_key;

        err = cfe_damgard_derive_key(&derived_key, &m->scheme, &msk->msk[i], cfe_mat_get_row_ptr(y, i));
        if (err) {
            cfe_damgard_derived_key_free(&derived_key);
            cfe_damgard_multi_fe_key_free(res);
            break;
        }

        cfe_vec_set(&res->keys1, derived_key.key1, i);
        cfe_vec_set(&res->keys2, derived_key.key2, i);

        cfe_damgard_derived_key_free(&derived_key);
    }

    return err;
}

cfe_error
cfe_damgard_multi_encrypt(cfe_vec *ciphertext, cfe_damgard_multi_enc *e, cfe_vec *x, cfe_vec *pub_key, cfe_vec *otp) {
    if (!cfe_vec_check_bound(x, e->scheme.bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    cfe_vec otp_mod;
    cfe_vec_init(&otp_mod, x->size);
    cfe_vec_add(&otp_mod, x, otp);
    cfe_vec_mod(&otp_mod, &otp_mod, e->scheme.bound);

    cfe_error err = cfe_damgard_encrypt(ciphertext, &e->scheme, &otp_mod, pub_key);

    cfe_vec_free(&otp_mod);
    return err;
}

cfe_error cfe_damgard_multi_decrypt(mpz_t res, cfe_damgard_multi *m, cfe_mat *ciphertext, cfe_damgard_multi_fe_key *key,
                                    cfe_mat *y) {
    if (!cfe_mat_check_bound(y, m->scheme.bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    cfe_error err = 0;

    mpz_t sum, c;
    mpz_inits(sum, res, NULL);

    cfe_damgard_fe_key derived_key;
    mpz_init(derived_key.key1);
    mpz_init(derived_key.key2);

    for (size_t i = 0; i < m->slots; i++) {
        cfe_vec_get(derived_key.key1, &key->keys1, i);
        cfe_vec_get(derived_key.key2, &key->keys2, i);

        err = cfe_damgard_decrypt(c, &m->scheme, cfe_mat_get_row_ptr(ciphertext, i), &derived_key,
                                  cfe_mat_get_row_ptr(y, i));
        if (err) {
            mpz_clear(c);
            break;
        }

        mpz_add(sum, sum, c);
        mpz_clear(c);
    }

    mpz_sub(res, sum, key->z);
    mpz_mod(res, res, m->scheme.bound);

    cfe_damgard_derived_key_free(&derived_key);
    mpz_clear(sum);
    return err;
}
