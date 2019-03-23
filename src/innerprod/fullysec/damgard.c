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
#include <gmp.h>

#include "innerprod/fullysec/damgard.h"
#include "internal/keygen.h"
#include "internal/dlog.h"
#include "sample/uniform.h"


cfe_error cfe_damgard_init(cfe_damgard *s, size_t l, size_t modulus_len, mpz_t bound) {
    cfe_elgamal key;
    if (cfe_elgamal_init(&key, modulus_len)) {
        return CFE_ERR_PARAM_GEN_FAILED;
    }

    cfe_error err = CFE_ERR_NONE;

    mpz_t check;
    mpz_init(check);

    mpz_pow_ui(check, bound, 2);
    mpz_mul_ui(check, check, l);

    if (mpz_cmp(check, key.p) >= 0) {
        err = CFE_ERR_PRECONDITION_FAILED;
        goto cleanup;
    }

    s->l = l;
    mpz_init_set(s->bound, bound);
    mpz_init_set(s->g, key.g);
    mpz_init_set(s->p, key.p);
    mpz_init(s->h);
    cfe_uniform_sample_range_i_mpz(s->h, 1, s->p);
    mpz_powm(s->h, key.g, s->h, s->p);

    cleanup:
    cfe_elgamal_free(&key);
    mpz_clear(check);
    return err;
}

void cfe_damgard_free(cfe_damgard *s) {
    mpz_clears(s->bound, s->g, s->p, s->h, NULL);
}

// res should be uninitialized!
void cfe_damgard_copy(cfe_damgard *res, cfe_damgard *s) {
    res->l = s->l;
    mpz_init_set(res->bound, s->bound);
    mpz_init_set(res->g, s->g);
    mpz_init_set(res->p, s->p);
    mpz_init_set(res->h, s->h);
}

void cfe_damgard_sec_key_init(cfe_damgard_sec_key *msk, cfe_damgard *s) {
    cfe_vec_inits(s->l, &msk->s, &msk->t, NULL);
}

void cfe_damgard_pub_key_init(cfe_vec *mpk, cfe_damgard *s) {
    cfe_vec_init(mpk, s->l);
}

void cfe_damgard_sec_key_free(cfe_damgard_sec_key *key) {
    cfe_vec_frees(&key->s, &key->t, NULL);
}

void cfe_damgard_derived_key_free(cfe_damgard_fe_key *key) {
    mpz_clears(key->key1, key->key2, NULL);
}

// mpk must be uninitialized!
void cfe_damgard_generate_master_keys(cfe_damgard_sec_key *msk, cfe_vec *mpk, cfe_damgard *s) {
    mpz_t s_i, t_i, y1, y2, r, p_min_1;
    mpz_inits(s_i, t_i, y1, y2, r, p_min_1, NULL);

    mpz_sub_ui(p_min_1, s->p, 1);
    for (size_t i = 0; i < s->l; i++) {
        cfe_uniform_sample_range_i_mpz(s_i, 2, p_min_1);
        cfe_vec_set(&msk->s, s_i, i);

        cfe_uniform_sample_range_i_mpz(t_i, 2, p_min_1);
        cfe_vec_set(&msk->t, t_i, i);

        mpz_powm(y1, s->g, s_i, s->p);
        mpz_powm(y2, s->h, t_i, s->p);

        mpz_mul(r, y1, y2);
        mpz_mod(r, r, s->p);
        cfe_vec_set(mpk, r, i);
    }

    mpz_clears(s_i, t_i, y1, y2, r, p_min_1, NULL);
}

void cfe_damgard_fe_key_init(cfe_damgard_fe_key *fe_key) {
    mpz_inits(fe_key->key1, fe_key->key2, NULL);
}

cfe_error
cfe_damgard_derive_key(cfe_damgard_fe_key *fe_key, cfe_damgard *s, cfe_damgard_sec_key *msk, cfe_vec *y) {
    if (!cfe_vec_check_bound(y, s->bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    mpz_t p_min_1;
    mpz_init(p_min_1);
    mpz_sub_ui(p_min_1, s->p, 1);

    cfe_vec_dot(fe_key->key1, &msk->s, y);
    mpz_mod(fe_key->key1, fe_key->key1, p_min_1);

    cfe_vec_dot(fe_key->key2, &msk->t, y);
    mpz_mod(fe_key->key2, fe_key->key2, p_min_1);

    mpz_clear(p_min_1);
    return CFE_ERR_NONE;
}

void cfe_damgard_ciphertext_init(cfe_vec *ciphertext, cfe_damgard *s) {
    cfe_vec_init(ciphertext, s->l + 2);
}

cfe_error cfe_damgard_encrypt(cfe_vec *ciphertext, cfe_damgard *s, cfe_vec *x, cfe_vec *mpk) {
    if (!cfe_vec_check_bound(x, s->bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    mpz_t r, ct, t1, t2;
    mpz_inits(r, ct, t1, t2, NULL);
    cfe_uniform_sample_range_i_mpz(r, 1, s->p);

    mpz_powm(ct, s->g, r, s->p);
    cfe_vec_set(ciphertext, ct, 0);
    mpz_powm(ct, s->h, r, s->p);
    cfe_vec_set(ciphertext, ct, 1);

    for (size_t i = 0; i < s->l; i++) {
        cfe_vec_get(t1, mpk, i);
        mpz_powm(t1, t1, r, s->p);

        cfe_vec_get(t2, x, i);
        mpz_powm(t2, s->g, t2, s->p);

        mpz_mul(ct, t1, t2);
        mpz_mod(ct, ct, s->p);
        cfe_vec_set(ciphertext, ct, i + 2);
    }

    mpz_clears(r, ct, t1, t2, NULL);
    return CFE_ERR_NONE;
}


cfe_error cfe_damgard_decrypt(mpz_t res, cfe_damgard *s, cfe_vec *ciphertext, cfe_damgard_fe_key *key, cfe_vec *y) {
    if (!cfe_vec_check_bound(y, s->bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    mpz_t num, ct, t1, t2, denom, denom_inv, r, order, bound;
    mpz_inits(num, ct, t1, t2, denom, denom_inv, r, order, bound, NULL);

    mpz_set_ui(num, 1);

    for (size_t i = 2; i < ciphertext->size; i++) {
        cfe_vec_get(ct, ciphertext, i);
        cfe_vec_get(r, y, i - 2);

        mpz_powm(t1, ct, r, s->p);

        mpz_mul(num, num, t1);
        mpz_mod(num, num, s->p);
    }

    cfe_vec_get(t1, ciphertext, 0);
    mpz_powm(t1, t1, key->key1, s->p);

    cfe_vec_get(t2, ciphertext, 1);
    mpz_powm(t2, t2, key->key2, s->p);

    mpz_mul(denom, t1, t2);
    mpz_mod(denom, denom, s->p);
    mpz_invert(denom_inv, denom, s->p);
    mpz_mul(r, denom_inv, num);
    mpz_mod(r, r, s->p);

    mpz_sub_ui(order, s->p, 1);
    mpz_pow_ui(bound, s->bound, 2);
    mpz_mul_ui(bound, bound, s->l);

    cfe_error err = cfe_baby_giant_with_neg(res, r, s->g, s->p, order, bound);

    mpz_clears(num, ct, t1, t2, denom, denom_inv, r, order, bound, NULL);
    return err;
}
