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

#include "innerprod/simple/ddh.h"
#include "internal/keygen.h"
#include "internal/dlog.h"
#include "sample/uniform.h"

cfe_error cfe_ddh_init(cfe_ddh *s, size_t l, size_t modulus_len, mpz_t bound) {
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

    cleanup:
    cfe_elgamal_free(&key);
    mpz_clear(check);
    return err;
}

// res should be uninitialized!
void cfe_ddh_copy(cfe_ddh *res, cfe_ddh *s) {
    res->l = s->l;
    mpz_init_set(res->bound, s->bound);
    mpz_init_set(res->g, s->g);
    mpz_init_set(res->p, s->p);
}

void cfe_ddh_free(cfe_ddh *s) {
    mpz_clears(s->bound, s->g, s->p, NULL);
}

void cfe_ddh_master_keys_init(cfe_vec *msk, cfe_vec *mpk, cfe_ddh *s) {
    cfe_vec_inits(s->l, msk, mpk, NULL);
}

void cfe_ddh_ciphertext_init(cfe_vec *ciphertext, cfe_ddh *s) {
    cfe_vec_init(ciphertext, s->l + 1);
}

void cfe_ddh_generate_master_keys(cfe_vec *msk, cfe_vec *mpk, cfe_ddh *s) {
    mpz_t x, p_min_1;
    mpz_inits(x, p_min_1, NULL);
    mpz_sub_ui(p_min_1, s->p, 1);
    for (size_t i = 0; i < s->l; i++) {
        cfe_uniform_sample_range_i_mpz(x, 2, p_min_1);
        cfe_vec_set(msk, x, i);

        mpz_powm(x, s->g, x, s->p);
        cfe_vec_set(mpk, x, i);
    }

    mpz_clears(x, p_min_1, NULL);
}

cfe_error cfe_ddh_derive_key(mpz_t res, cfe_ddh *s, cfe_vec *msk, cfe_vec *y) {
    if (!cfe_vec_check_bound(y, s->bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    mpz_t p_min_one;
    mpz_init(p_min_one);
    cfe_vec_dot(res, msk, y);
    mpz_sub_ui(p_min_one, s->p, 1);
    mpz_mod(res, res, p_min_one);
    mpz_clear(p_min_one);
    return CFE_ERR_NONE;
}

cfe_error cfe_ddh_encrypt(cfe_vec *ciphertext, cfe_ddh *s, cfe_vec *x, cfe_vec *mpk) {
    if (!cfe_vec_check_bound(x, s->bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    mpz_t r, ct, t1, t2;
    mpz_inits(r, ct, t1, t2, NULL);
    cfe_uniform_sample_range_i_mpz(r, 1, s->p);

    mpz_powm(ct, s->g, r, s->p);
    cfe_vec_set(ciphertext, ct, 0);

    for (size_t i = 0; i < s->l; i++) {
        cfe_vec_get(t1, mpk, i);
        mpz_powm(t1, t1, r, s->p);

        cfe_vec_get(t2, x, i);
        mpz_powm(t2, s->g, t2, s->p);

        mpz_mul(ct, t1, t2);
        mpz_mod(ct, ct, s->p);
        cfe_vec_set(ciphertext, ct, i + 1);
    }

    mpz_clears(r, ct, t1, t2, NULL);
    return CFE_ERR_NONE;
}

cfe_error cfe_ddh_decrypt(mpz_t res, cfe_ddh *s, cfe_vec *ciphertext, mpz_t key, cfe_vec *y) {
    if (!cfe_vec_check_bound(y, s->bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    mpz_t num, ct, t1, denom, denom_inv, r, order, bound;
    mpz_inits(num, ct, t1, denom, denom_inv, r, order, bound, NULL);

    mpz_set_ui(num, 1);

    for (size_t i = 1; i < ciphertext->size; i++) {
        cfe_vec_get(ct, ciphertext, i);
        cfe_vec_get(r, y, i - 1);

        mpz_powm(t1, ct, r, s->p);

        mpz_mul(num, num, t1);
        mpz_mod(num, num, s->p);
    }

    cfe_vec_get(ct, ciphertext, 0);
    mpz_powm(denom, ct, key, s->p);
    mpz_invert(denom_inv, denom, s->p);
    mpz_mul(r, denom_inv, num);
    mpz_mod(r, r, s->p);

    mpz_sub_ui(order, s->p, 1);
    mpz_pow_ui(bound, s->bound, 2);
    mpz_mul_ui(bound, bound, s->l);

    cfe_error err = cfe_baby_giant_with_neg(res, r, s->g, s->p, order, bound);

    mpz_clears(num, ct, t1, denom, denom_inv, r, order, bound, NULL);
    return err;
}
