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

#include <amcl/pair_BN254.h>

#include "cifer/quadratic/sgp.h"
#include "cifer/internal/keygen.h"
#include "cifer/internal/dlog.h"
#include "cifer/internal/big.h"
#include "cifer/sample/uniform.h"
#include "cifer/data/vec_curve.h"
#include "cifer/internal/common.h"

cfe_error cfe_sgp_init(cfe_sgp *s, size_t n, mpz_t bound) {
    cfe_error err = CFE_ERR_NONE;
    mpz_t bound_check;
    s->n = n;
    mpz_inits(s->bound, s->mod, bound_check, NULL);
    mpz_set(s->bound, bound);
    BIG_256_56_rcopy(s->modBig, CURVE_Order_BN254);
    mpz_from_BIG_256_56(s->mod, s->modBig);
    mpz_pow_ui(bound_check, bound, 3);
    mpz_mul_ui(bound_check, bound_check, n * n);
    if (mpz_cmp(bound_check, s->mod) > 0) {
        err = CFE_ERR_BOUND_CHECK_FAILED;
    }
    mpz_clear(bound_check);
    return err;
}

void cfe_sgp_free(cfe_sgp *s) {
    mpz_clears(s->bound, s->mod, NULL);
}

void cfe_sgp_sec_key_init(cfe_sgp_sec_key *msk, cfe_sgp *sgp) {
    cfe_vec_inits(sgp->n, &(msk->s), &(msk->t), NULL);
}

void cfe_sgp_sec_key_free(cfe_sgp_sec_key *msk) {
    cfe_vec_frees(&(msk->s), &(msk->t), NULL);
}

void cfe_sgp_sec_key_generate(cfe_sgp_sec_key *msk, cfe_sgp *sgp) {
    cfe_uniform_sample_vec(&(msk->s), sgp->bound);
    cfe_uniform_sample_vec(&(msk->t), sgp->bound);
}

cfe_error cfe_sgp_derive_key(ECP2_BN254 *key, cfe_sgp_sec_key *msk, cfe_mat *f, cfe_sgp *sgp) {
    if (cfe_mat_check_bound(f, sgp->bound) == false) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    };
    mpz_t res;
    mpz_init(res);

    cfe_mat_mul_x_mat_y(res, f, &(msk->s), &(msk->t));
    ECP2_BN254_generator(key);

    if (mpz_cmp_ui(res, 0) == -1) {
        mpz_neg(res, res);
        ECP2_BN254_neg(key);
    }

    BIG_256_56 res_b;
    BIG_256_56_from_mpz(res_b, res);
    ECP2_BN254_mul(key, res_b);
    mpz_clear(res);

    return CFE_ERR_NONE;
}

void cfe_sgp_cipher_init(cfe_sgp_cipher *cipher, cfe_sgp *s) {
    ECP_BN254_generator(&(cipher->g1MulGamma));
    cipher->a = (cfe_vec_G1 *) cfe_malloc(s->n * sizeof(cfe_vec_G1));
    cipher->b = (cfe_vec_G2 *) cfe_malloc(s->n * sizeof(cfe_vec_G2));
    for (size_t i = 0; i < s->n; i++) {
        cfe_vec_G1_init(&(cipher->a[i]), 2);
        cfe_vec_G2_init(&(cipher->b[i]), 2);
    }
    cipher->n = s->n;
}

void cfe_sgp_cipher_free(cfe_sgp_cipher *cipher) {
    for (size_t i = 0; i < cipher->n; i++) {
        cfe_vec_G1_free(&(cipher->a[i]));
        cfe_vec_G2_free(&(cipher->b[i]));
    }
    free(cipher->a);
    free(cipher->b);
}

cfe_error cfe_sgp_encrypt(cfe_sgp_cipher *cipher, cfe_sgp *s, cfe_vec *x, cfe_vec *y, cfe_sgp_sec_key *msk) {
    if ((cfe_vec_check_bound(x, s->bound) == false) || (cfe_vec_check_bound(y, s->bound) == false)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    cfe_mat W, W_inv, W_inv_tr;
    cfe_mat_inits(2, 2, &W, &W_inv, &W_inv_tr, NULL);
    cfe_uniform_sample_mat(&W, s->mod);
    cfe_error err;
    err = cfe_mat_inverse_mod(&W, &W_inv, s->mod);
    if (err != CFE_ERR_NONE) {
        return err;
    }
    cfe_mat_transpose(&W_inv_tr, &W_inv);

    mpz_t gamma, x_i, y_i, s_i, t_i, tmp, minus;
    mpz_inits(gamma, x_i, y_i, s_i, t_i, tmp, minus, NULL);
    cfe_uniform_sample(gamma, s->mod);
    mpz_set_si(minus, -1);
    cfe_vec v, v_i;
    cfe_vec_inits(2, &v, &v_i, NULL);

    for (size_t i = 0; i < s->n; i++) {
        cfe_vec_get(x_i, x, i);
        cfe_vec_get(s_i, &(msk->s), i);
        mpz_mul(tmp, gamma, s_i);
        mpz_mod(tmp, tmp, s->mod);

        cfe_vec_set(&v, x_i, 0);
        cfe_vec_set(&v, tmp, 1);

        cfe_mat_mul_vec(&v_i, &W_inv_tr, &v);
        cfe_vec_mod(&v_i, &v_i, s->mod);
        cfe_vec_mul_G1(&(cipher->a[i]), &v_i);

        cfe_vec_get(y_i, y, i);
        cfe_vec_get(t_i, &(msk->t), i);
        mpz_neg(t_i, t_i);
        cfe_vec_set(&v, y_i, 0);
        cfe_vec_set(&v, t_i, 1);

        cfe_mat_mul_vec(&v_i, &W, &v);
        cfe_vec_mod(&v_i, &v_i, s->mod);
        cfe_vec_mul_G2(&(cipher->b[i]), &v_i);
    }

    BIG_256_56 gamma_b;
    BIG_256_56_from_mpz(gamma_b, gamma);
    ECP_BN254_mul(&(cipher->g1MulGamma), gamma_b);

    cfe_mat_frees(&W, &W_inv, &W_inv_tr, NULL);
    cfe_vec_frees(&v, &v_i, NULL);
    mpz_clears(gamma, x_i, y_i, s_i, t_i, tmp, minus, NULL);

    return CFE_ERR_NONE;
}

cfe_error cfe_sgp_decrypt(mpz_t res, cfe_sgp_cipher *cipher, ECP2_BN254 *key, cfe_mat *f, cfe_sgp *s) {
    FP12_BN254 prod;
    PAIR_BN254_ate(&prod, key, &(cipher->g1MulGamma));
    PAIR_BN254_fexp(&prod);
    mpz_t el, res_bound;
    mpz_inits(el, res_bound, NULL);
    ECP2_BN254 t2, t4;
    FP12_BN254 p1, p2, r;
    BIG_256_56 el_b;

    for (size_t i = 0; i < f->rows; i++) {
        for (size_t j = 0; j < f->cols; j++) {
            cfe_mat_get(el, f, i, j);

            if(mpz_cmp_ui(el, 0) != 0) {
                ECP_BN254 t1, t3;
                t1 = cipher->a[i].vec[0];
                t3 = cipher->a[i].vec[1];

                t2 = cipher->b[j].vec[0];
                t4 = cipher->b[j].vec[1];

                PAIR_BN254_ate(&p1, &t2, &t1);
                PAIR_BN254_fexp(&p1);
                PAIR_BN254_ate(&p2, &t4, &t3);
                PAIR_BN254_fexp(&p2);

                FP12_BN254_mul(&p1, &p2); // p1 stores the multiplied value

                if (mpz_cmp_ui(el, 0) == -1) {
                    mpz_neg(el, el);
                    FP12_BN254_inv(&p1, &p1);
                }

                BIG_256_56_from_mpz(el_b, el);
                FP12_BN254_pow(&r, &p1, el_b);
                FP12_BN254_mul(&prod, &r); // prod stores the multiplied value
            }
        }
    }

    ECP_BN254 g1;
    ECP_BN254_generator(&g1);
    ECP2_BN254 g2;
    ECP2_BN254_generator(&g2);

    FP12_BN254 gt;
    PAIR_BN254_ate(&gt, &g2, &g1);
    PAIR_BN254_fexp(&gt);

    mpz_pow_ui(res_bound, s->bound, 3);
    mpz_mul_ui(res_bound, res_bound, s->n * s->n);

    cfe_error err;
    err = cfe_baby_giant_FP12_BN256_with_neg(res, &prod, &gt, res_bound);
    mpz_clears(el, res_bound, NULL);
    return err;
}
