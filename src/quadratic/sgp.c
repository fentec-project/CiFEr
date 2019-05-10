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

#include "cifer/quadratic/sgp.h"
#include "cifer/internal/keygen.h"
#include "cifer/internal/dlog.h"
#include "cifer/internal/big.h"
#include "cifer/sample/uniform.h"
#include "cifer/data/vec_curve.h"

#include <amcl/big_256_56.h>
#include <cifer/internal/common.h>


cfe_error cfe_sgp_init(cfe_sgp *s, size_t n, mpz_t bound) {
    /*
    cfe_elgamal key;
    if (cfe_elgamal_init(&key, modulus_len)) {
        return CFE_ERR_PARAM_GEN_FAILED;
    }
     */

    cfe_error err = CFE_ERR_NONE;

    /*
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
     */
    s->n = n;
    mpz_init_set(s->bound, bound);
    BIG_256_56_rcopy(s->modBig, CURVE_Order_BN254);
    mpz_init(s->mod);
    mpz_from_BIG_256_56(s->mod, s->modBig);

    return err;
}


void cfe_sgp_sec_key_init(cfe_sgp_sec_key *msk, cfe_sgp *sgp) {
    cfe_vec_inits(sgp->n, &(msk->s), &(msk->t), NULL);
}

// msk should be initialized!
void cfe_sgp_generate_master_key(cfe_sgp_sec_key *msk, cfe_sgp *sgp) {
    cfe_uniform_sample_vec(&(msk->s), sgp->bound);
    cfe_uniform_sample_vec(&(msk->t), sgp->bound);

}

// cfe_extract_submatrix returns a matrix obtained from m by removing row i and column j.
// It returns an error if i >= number of rows of m, or if j >= number of
// columns of m.
cfe_error cfe_extract_submatrix(cfe_mat *m, cfe_mat *min, size_t i, size_t j) {
    if (i >= m->rows || j >= m->cols) {
        return CFE_ERR_MALFORMED_INPUT;
    }

    size_t ind1 = 0;
    mpz_t val;
    mpz_init(val);
    for (size_t k = 0; k < m->rows - 1; k++) {
        if (k == i) {
            ind1++;
        }
        size_t ind2 = 0;
        for (size_t l = 0; l < m->cols - 1; l++) {
            if (l == j) {
               ind2++;
            }
            cfe_mat_get(val, m, ind1, ind2);
            cfe_mat_set(min, val, k, l);
            ind2++;
        }
        ind1++;
    }
    mpz_clear(val);

    return 0;
}

void cfe_determinant(cfe_mat *m, mpz_t det) {
    if (m->rows == 1) {
        cfe_mat_get(det, m, 0, 0);
        return;
    }
    mpz_set_si(det, 0);
    mpz_t sign;
    mpz_init_set_ui(sign, 1);
    mpz_t minus;
    mpz_init_set_si(minus, -1);

    mpz_t val;
    mpz_init(val);
    mpz_t minor;
    mpz_init(minor);
    for (size_t i = 0; i < m->rows; i++) {
        cfe_mat min;
        cfe_mat_init(&min, m->rows - 1, m->cols - 1);
        cfe_extract_submatrix(m, &min, 0, i);

        cfe_mat_get(val, m, 0, i);

        cfe_determinant(&min, minor);

        mpz_mul(minor, minor, val);
        mpz_mul(minor, minor, sign);
        mpz_mul(sign, sign, minus);
        mpz_add(det, det, minor);
        cfe_mat_free(&min);
    }
    mpz_clears(val, minor, NULL);
}

cfe_error cfe_inverse_mod(cfe_mat *m, cfe_mat *inverse_mat, mpz_t mod) {
    mpz_t det;
    mpz_init(det);
    cfe_determinant(m, det);

    mpz_mod(det, det, mod);

    if(mpz_cmp_si(det, 0) == 0) {
        return CFE_ERR_NO_INVERSE;
    }

    mpz_t det_inv;
    mpz_init(det_inv);
    mpz_invert(det_inv, det, mod);

    mpz_t sign;
    mpz_init(sign);
    mpz_t minus;
    mpz_init_set_si(minus, -1);

    cfe_mat min;
    mpz_t minor;
    mpz_t val;
    cfe_mat_init(&min, m->rows - 1, m->cols - 1);
    mpz_inits(minor, val, NULL);

    cfe_mat transposed;
    cfe_mat_init(&transposed, m->rows, m->cols);
    for (size_t i = 0; i < m->rows; i++) {
        for (size_t j = 0; j < m->cols; j++) {
            cfe_extract_submatrix(m, &min, i, j);
            cfe_determinant(&min, minor);

            mpz_mod(minor, minor, mod);
            mpz_pow_ui(sign, minus, i+j);

            mpz_mul(val, minor, det_inv);
            mpz_mul(val, val, sign);
            mpz_mod(val, val, mod);

            cfe_mat_set(&transposed, val, i, j);
        }
    }
    mpz_clears(sign, minus, minor, val, NULL);
    cfe_mat_transpose(inverse_mat, &transposed);
    cfe_mat_frees(&min, &transposed, NULL);

    return 0;
}

void cfe_mat_mul_x_mat_y(mpz_t res, cfe_mat *mat, cfe_vec *x, cfe_vec *y) {
    cfe_vec t;
    cfe_vec_init(&t, x->size);
    cfe_mat_mul_vec(&t, mat, y);
    cfe_vec_dot(res, &t, x);
}

void cfe_sgp_derive_key(ECP2_BN254 *key, cfe_sgp_sec_key *msk, cfe_mat *f) {
    mpz_t res;
    mpz_init(res);

    cfe_mat_mul_x_mat_y(res, f, &(msk->s), &(msk->t));

    BIG_256_56 res_b;
    BIG_256_56_from_mpz(res_b, res);

    ECP2_BN254_generator(key);
    ECP2_BN254_mul(key, res_b);

}

void cfe_sgp_cipher_init(cfe_sgp_cipher *cipher, cfe_sgp *s) {
    ECP_BN254_generator(&(cipher->g1MulGamma));
    cipher->a = (cfe_vec_G1 *) cfe_malloc(s->n * sizeof(cfe_vec_G1));
    cipher->b = (cfe_vec_G2 *) cfe_malloc(s->n * sizeof(cfe_vec_G2));
    for (size_t i = 0; i < s->n; i++) {
        cfe_vec_G1_init(&(cipher->a[i]), 2);
        cfe_vec_G2_init(&(cipher->b[i]), 2);
    }
}

cfe_error cfe_sgp_encrypt(cfe_sgp_cipher *ciphertext, cfe_sgp *s, cfe_vec *x, cfe_vec *y, cfe_sgp_sec_key *msk) {
//    printf("\nencrypt\n");
    mpz_t gamma;
    mpz_init(gamma);

    // debug:
    //cfe_uniform_sample(gamma, s->mod);
    mpz_set_ui(gamma, 1);

    cfe_mat W;
    cfe_mat_init(&W, 2, 2);

    // debug
    //cfe_uniform_sample_mat(&W, s->mod);
    mpz_t bla, tra;
    mpz_inits(bla, tra, NULL);
    mpz_set_ui(bla, 2);
    mpz_set_ui(tra, 3);
    cfe_mat_set(&W, bla, 0, 0);
    cfe_mat_set(&W, tra, 0, 1);
    cfe_mat_set(&W, tra, 1, 0);
    cfe_mat_set(&W, bla, 1, 1);

    //gmp_printf ("tra %Zd\n", det);

    cfe_mat W_inv;
    cfe_mat_init(&W_inv, W.rows, W.cols);
    cfe_inverse_mod(&W, &W_inv, s->mod);
//    printf("\nW_inv\n");
//    cfe_mat_print(&W_inv);

    // remove
    cfe_mat check;
    cfe_mat_init(&check, W.rows, W.cols);
    cfe_mat_mul(&check, &W, &W_inv);

    cfe_mat check_mod;
    cfe_mat_init(&check_mod, W.rows, W.cols);

//    cfe_mat_mod(&check_mod, &check, s->mod);
//    cfe_mat_print(&check_mod);
    // til here

    cfe_mat W_inv_tr;
    cfe_mat_init(&W_inv_tr, 2, 2);
    cfe_mat_transpose(&W_inv_tr, &W_inv);

    mpz_t x_i, y_i, s_i, t_i, tmp;
    mpz_inits(x_i, y_i, s_i, t_i, tmp, NULL);
    cfe_vec v;
    cfe_vec_init(&v, 2);

    cfe_vec v_i;
    cfe_vec_init(&v_i, 2);

    mpz_t minus;
    mpz_init_set_si(minus, -1);

    //printf("\n??????????????????????????\n");
    //cfe_mat_print(&W_inv_tr);

    for (size_t i = 0; i < s->n; i++) {
        cfe_vec_get(x_i, x, i);
        cfe_vec_get(s_i, &(msk->s), i);
        mpz_mul(tmp, gamma, s_i);
        mpz_mod(tmp, tmp, s->mod);

        cfe_vec_set(&v, x_i, 0);
        cfe_vec_set(&v, tmp, 1);

        cfe_mat_mul_vec(&v_i, &W_inv_tr, &v);
        cfe_vec_mod(&v_i, &v_i, s->mod);

//        printf("\n?????????????11111111?????????????\n");
//        cfe_vec_print(&v);
//        cfe_vec_print(&v_i);

        cfe_vec_mul_G1(&(ciphertext->a[i]), &v_i);


        cfe_vec_get(y_i, y, i);
        cfe_vec_get(t_i, &(msk->t), i);
        mpz_neg(t_i, t_i);
        cfe_vec_set(&v, y_i, 0);
        cfe_vec_set(&v, t_i, 1);

        cfe_mat_mul_vec(&v_i, &W, &v);
        cfe_vec_mod(&v_i, &v_i, s->mod);

//        printf("\n?????????????22222?????????????\n");
//        cfe_vec_print(&v);
//        cfe_vec_print(&v_i);

        cfe_vec_mul_G2(&(ciphertext->b[i]), &v_i);
    }

    BIG_256_56 gamma_b;
    BIG_256_56_from_mpz(gamma_b, gamma);
    ECP_BN254_mul(&(ciphertext->g1MulGamma), gamma_b);


//    FP12_BN254 debug;
//    FP12_BN254_one(&debug);
//    for (int i = 0; i < s->n; i++) {
//        for (int j = 0; j < s->n; j++) {
//            ECP_BN254 t1, t3;
//            t1 = ciphertext->a[i].vec[0];
//            t3 = ciphertext->a[i].vec[1];
//            ECP2_BN254 t2, t4;
//            t2 = ciphertext->b[j].vec[0];
//            t4 = ciphertext->b[j].vec[1];
//
//
//            //PAIR_BN254_ate(&p1, &(ciphertext->a[i].vec[0]), &(ciphertext->b[j].vec[0]));
//            //PAIR_BN254_fexp(&p1);
//
//            FP12_BN254 p1, p2, r;
//            PAIR_BN254_ate(&p1, &t1, &t2);
//            PAIR_BN254_fexp(&p1);
//
//            //PAIR_BN254_ate(&p2, &(ciphertext->a[i].vec[1]), &(ciphertext->b[j].vec[1]));
//            //PAIR_BN254_fexp(&p2);
//
//            //PAIR_BN254_ate(&p1, &(ciphertext->a[i].vec[0]), &(ciphertext->b[j].vec[0]));
//            //PAIR_BN254_fexp(&p1);
//
//            PAIR_BN254_ate(&p2, &t3, &t4);
//            PAIR_BN254_fexp(&p2);
//
//            //printf("\n--------------!!----------------\n");
//            //FP12_BN254_output(&p2);
//
//            //ECP_BN254_output(&(ciphertext->a[i].vec[0]));
//            //ECP_BN254_output(&t1);
//            //ECP2_BN254_output(&(ciphertext->b[j].vec[0]));
//            //FP12_BN254_output(&p1);
//            //FP12_BN254_output(&p2);
//
//            FP12_BN254_mul(&p1, &p2);
//
//            //printf("\n--------\n");
//            FP12_BN254_mul(&debug, &p1);
//            //FP12_BN254_output(&debug);
//        }
//
//    }

    //printf("\n+++++++++++++++\n");
    //FP12_BN254_output(&debug);

//    printf("\n end encrypt \n");

    return 0;
}






void cfe_sgp_decrypt(cfe_sgp_cipher *cipher, ECP2_BN254 *key, cfe_mat *f) {

//    FP12_BN254 prod;
//
//    FP12_BN254_one(&prod);
//    PAIR_BN254_ate(&prod, &(cipher->g1MulGamma), key);
//    PAIR_BN254_fexp(&prod);
//
//    for (int i = 0; i < f->rows; i++) {
//        for (int j = 0; j < f->cols; j++) {
//            mpz_t el;
//            mpz_init(el);
//            cfe_mat_get(el, f, i, j);
//
//            if(mpz_cmp_ui(el, 0) != 0) {
//                ECP_BN254 t1, t3;
//                t1 = cipher->a[i].vec[0];
//                t3 = cipher->a[i].vec[1];
//                ECP2_BN254 t2, t4;
//                t2 = cipher->b[j].vec[0];
//                t4 = cipher->b[j].vec[1];
//
////                printf("\n----!!!!!!!!!!-----\n");
////                ECP_BN254_output(&t1);
////                ECP2_BN254_output(&t2);
//
//                // t1 is Infinity, t2 is not
//                // t3 is Infinity, t4 is not (sometimes is t4)
//
//                FP12_BN254 p1, p2, r;
//
//                PAIR_BN254_ate(&p1, &t1, &t2);
//                PAIR_BN254_fexp(&p1);
//
//                PAIR_BN254_ate(&p2, &t3, &t4);
//                PAIR_BN254_fexp(&p2);
//
//
//                //FP12_BN254_output(&p2);
//
//                //ECP2_BN254_output(&t2);
//                //FP12_BN254_output(&p1);
//                //FP12_BN254_output(&p2);
//
//
//                //printf("\n22222222222222222222\n");
//                //FP12_BN254_output(&p1);
//                // p1 is zero here
//
//                FP12_BN254_mul(&p1, &p2); // p1 stores the multiplied value
//
//                BIG_256_56 el_b;
//                BIG_256_56_from_mpz(el_b, el);
//
//                // p1 is zero here
//                // p2 is zero sometimes
//
//                // TODO: do we need to handle specially el which are < 0?
//                FP12_BN254_pow(&r, &p1, &el_b);
//
//                FP12_BN254_mul(&prod, &r); // prod stores the multiplied value
//                //FP12_BN254_mul(&prod, &p1); // prod stores the multiplied value
//
//                //printf("\n----@----\n");
//                //FP12_BN254_output(&prod);
//            }
//        }
//    }
//
//    printf("\n!!!!1111!!!!!!!!!!!!!!!!!!\n");
//    FP12_BN254_output(&prod);

}



