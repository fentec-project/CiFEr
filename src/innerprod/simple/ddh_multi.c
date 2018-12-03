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

#include "innerprod/simple/ddh_multi.h"
#include "sample/uniform.h"

cfe_error cfe_ddh_multi_init(cfe_ddh_multi *m, size_t slots, size_t l, size_t modulus_len, mpz_t bound) {
    cfe_ddh s;
    cfe_error err = cfe_ddh_init(&s, l, modulus_len, bound);
    if (err) {
        return err;
    }

    m->slots = slots;
    m->scheme = s;
    return CFE_ERR_NONE;
}

void cfe_ddh_multi_free(cfe_ddh_multi *m) {
    cfe_ddh_free(&m->scheme);
}

void cfe_ddh_multi_copy(cfe_ddh_multi *res, cfe_ddh_multi *m) {
    res->slots = m->slots;
    cfe_ddh s;
    cfe_ddh_copy(&s, &m->scheme);
    res->scheme = s;
}

void cfe_ddh_multi_enc_init(cfe_ddh_multi_enc *e, cfe_ddh_multi *m) {
    cfe_ddh e_scheme;
    cfe_ddh_copy(&e_scheme, &m->scheme);
    e->scheme = e_scheme;
}

void cfe_ddh_multi_enc_free(cfe_ddh_multi_enc *e) {
    cfe_ddh_free(&e->scheme);
}

void cfe_ddh_multi_sec_key_free(cfe_ddh_multi_sec_key *key) {
    cfe_mat_frees(&key->msk, &key->otp_key, NULL);
}

void cfe_ddh_multi_fe_key_free(cfe_ddh_multi_fe_key *key) {
    cfe_vec_free(&key->keys);
    mpz_clear(key->otp_key);
}

// mpk should be uninitialized!
void cfe_ddh_multi_generate_master_keys(cfe_mat *mpk, cfe_ddh_multi_sec_key *msk, cfe_ddh_multi *m) {
    cfe_mat_inits(m->slots, m->scheme.l, mpk, &msk->msk, &msk->otp_key, NULL);

    cfe_vec rand_v;
    cfe_vec_init(&rand_v, m->scheme.l);

    for (size_t i = 0; i < m->slots; i++) {
        cfe_vec msk_v, mpk_v;
        cfe_ddh_generate_master_keys(&msk_v, &mpk_v, &m->scheme);
        cfe_mat_set_vec(&msk->msk, &msk_v, i);
        cfe_mat_set_vec(mpk, &mpk_v, i);

        cfe_vec_frees(&msk_v, &mpk_v, NULL);

        cfe_uniform_sample_vec(&rand_v, m->scheme.bound);
        cfe_mat_set_vec(&msk->otp_key, &rand_v, i);
    }

    cfe_vec_free(&rand_v);
}


cfe_error
cfe_ddh_multi_derive_key(cfe_ddh_multi_fe_key *res, cfe_ddh_multi *m, cfe_ddh_multi_sec_key *msk, cfe_mat *y) {
    if (!cfe_mat_check_bound(y, m->scheme.bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    cfe_error err = CFE_ERR_NONE;

    mpz_t z, key;
    mpz_init(z);

    cfe_mat_dot(z, &msk->otp_key, y);
    mpz_mod(z, z, m->scheme.bound);

    mpz_init_set(res->otp_key, z);
    cfe_vec_init(&res->keys, m->scheme.l);

    for (size_t i = 0; i < m->slots; i++) {
        err = cfe_ddh_derive_key(key, &m->scheme, cfe_mat_get_row_ptr(&msk->msk, i), cfe_mat_get_row_ptr(y, i));
        if (err) {
            cfe_ddh_multi_fe_key_free(res);
            mpz_clear(key);
            break;
        }

        cfe_vec_set(&res->keys, key, i);
        mpz_clear(key);
    }

    mpz_clear(z);
    return err;
}

cfe_error cfe_ddh_multi_encrypt(cfe_vec *ciphertext, cfe_ddh_multi_enc *e, cfe_vec *x, cfe_vec *pub_key, cfe_vec *otp) {
    if (!cfe_vec_check_bound(x, e->scheme.bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    cfe_vec otp_mod;
    cfe_vec_init(&otp_mod, x->size);
    cfe_vec_add(&otp_mod, x, otp);
    cfe_vec_mod(&otp_mod, &otp_mod, e->scheme.bound);

    cfe_error err = cfe_ddh_encrypt(ciphertext, &e->scheme, &otp_mod, pub_key);

    cfe_vec_free(&otp_mod);
    return err;
}

cfe_error
cfe_ddh_multi_decrypt(mpz_t res, cfe_ddh_multi *m, cfe_mat *ciphertext, cfe_ddh_multi_fe_key *key, cfe_mat *y) {
    if (!cfe_mat_check_bound(y, m->scheme.bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    cfe_error err = CFE_ERR_NONE;

    mpz_t sum, c, k;
    mpz_inits(sum, k, res, NULL);

    for (size_t i = 0; i < m->slots; i++) {
        cfe_vec_get(k, &key->keys, i);
        err = cfe_ddh_decrypt(c, &m->scheme, cfe_mat_get_row_ptr(ciphertext, i), k, cfe_mat_get_row_ptr(y, i));
        if (err) {
            mpz_clear(c);
            break;
        }

        mpz_add(sum, sum, c);
        mpz_clear(c);
    }

    mpz_sub(res, sum, key->otp_key);
    mpz_mod(res, res, m->scheme.bound);

    mpz_clears(sum, k, NULL);
    return err;
}
