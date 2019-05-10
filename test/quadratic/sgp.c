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

#include <gmp.h>
#include "cifer/test.h"
#include "cifer/quadratic/sgp.h"
#include "cifer/sample/uniform.h"

#include <amcl/big_256_56.h>
#include <cifer/internal/big.h>

MunitResult test_sgp_end_to_end(const MunitParameter *params, void *data) {
    /*
    size_t l = 3;
    size_t modulus_len = 128;

    mpz_t bound, bound_neg, func_key, xy_check, xy;
    mpz_inits(bound, bound_neg, xy_check, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 15);
    mpz_neg(bound_neg, bound);

    cfe_ddh s, encryptor, decryptor;
    cfe_error err = cfe_ddh_init(&s, l, modulus_len, bound);
    munit_assert(err == 0);

    cfe_vec msk, mpk, ciphertext, x, y;
    cfe_vec_inits(l, &x, &y, NULL);
    cfe_uniform_sample_range_vec(&x, bound_neg, bound);
    cfe_uniform_sample_range_vec(&y, bound_neg, bound);
    cfe_vec_dot(xy_check, &x, &y);

    cfe_ddh_generate_master_keys(&msk, &mpk, &s);

    err = cfe_ddh_derive_key(func_key, &s, &msk, &y);
    munit_assert(err == 0);

    cfe_ddh_copy(&encryptor, &s);
    err = cfe_ddh_encrypt(&ciphertext, &encryptor, &x, &mpk);
    munit_assert(err == 0);

    cfe_ddh_copy(&decryptor, &s);
    err = cfe_ddh_decrypt(xy, &decryptor, &ciphertext, func_key, &y);
    munit_assert(err == 0);

    munit_assert(mpz_cmp(xy, xy_check) == 0);

    mpz_clears(bound, bound_neg, func_key, xy_check, xy, NULL);
    cfe_vec_frees(&x, &y, &msk, &mpk, &ciphertext, NULL);

    cfe_ddh_free(&s);
    cfe_ddh_free(&encryor);
    cfe_ddh_free(&decryptor);
     */
    size_t n = 3;

    mpz_t b;
    mpz_init_set_ui(b, 8);
    cfe_mat f;
    cfe_mat_init(&f, n, n);
    cfe_uniform_sample_mat(&f, b);
    //cfe_mat_free(&f);

//    cfe_mat_print(&f);
//    printf("\n------------__----------___-----------__------\n");

    int err = 1;

    cfe_sgp s;
    err = cfe_sgp_init(&s, n, b);

    cfe_sgp_sec_key msk;
    cfe_sgp_sec_key_init(&msk, &s);

    cfe_sgp_generate_master_key(&msk, &s);

//    printf("\n msk.s msk.s\n");
//    cfe_vec_print(&(msk.s));
//
//    printf("\n msk.t msk.t\n");
//    cfe_vec_print(&(msk.t));

    cfe_vec x, y;
    cfe_vec_inits(s.n, &x, &y, NULL);

    cfe_uniform_sample_vec(&x, s.bound);
    cfe_uniform_sample_vec(&y, s.bound);

//    printf("\n x x x \n");
//    cfe_vec_print(&x);
//
//    printf("\n y y y \n");
//    cfe_vec_print(&y);

    cfe_sgp_cipher ciphertext;
    cfe_sgp_cipher_init(&ciphertext, &s);
    cfe_sgp_encrypt(&ciphertext, &s, &x, &y, &msk);

//
//    // debug start
//    mpz_t gamma;
//    mpz_init(gamma);
//    mpz_set_ui(gamma, 1);
//
//    cfe_mat W;
//    cfe_mat_init(&W, 2, 2);
//
//    // debug
//
//    //cfe_uniform_sample_mat(&W, s->mod);
//    mpz_t bla, tra;
//    mpz_inits(bla, tra, NULL);
//    mpz_set_ui(bla, 2);
//    mpz_set_ui(tra, 3);
//    cfe_mat_set(&W, bla, 0, 0);
//    cfe_mat_set(&W, tra, 0, 1);
//    cfe_mat_set(&W, tra, 1, 0);
//    cfe_mat_set(&W, bla, 1, 1);
//
//    cfe_mat W_inv;
//    cfe_mat_init(&W_inv, W.rows, W.cols);
//    cfe_inverse_mod(&W, &W_inv, s.mod);
//    cfe_mat_print(&W_inv);
//
//    cfe_mat check;
//    cfe_mat_init(&check, W.rows, W.cols);
//    cfe_mat_mul(&check, &W, &W_inv);
//
//    cfe_mat check_mod;
//    cfe_mat_init(&check_mod, W.rows, W.cols);
//
//    cfe_mat_mod(&check_mod, &check, s.mod);
//
//    cfe_mat_print(&check_mod);
//
//    cfe_mat W_inv_tr;
//    cfe_mat_init(&W_inv_tr, 2, 2);
//    cfe_mat_transpose(&W_inv_tr, &W_inv);
//
//    mpz_t x_i, y_j, s_i, t_j, tmp;
//    mpz_inits(x_i, y_j, s_i, t_j, tmp, NULL);
//    cfe_vec v;
//    cfe_vec_init(&v, 2);
//
//    cfe_vec a_i;
//    cfe_vec_init(&a_i, 2);
//
//    cfe_vec b_j;
//    cfe_vec_init(&b_j, 2);
//
//    mpz_t minus;
//    mpz_init_set_si(minus, -1);
//
//    mpz_t res;
//    mpz_init(res);
//    cfe_mat_mul_x_mat_y(res, &f, &(msk.s), &(msk.t));
//
//    mpz_t left;
//    mpz_init(left);
//    mpz_mul(left, gamma, res);
//
//    mpz_t a_i_0, a_i_1, b_j_0, b_j_1;
//    mpz_inits(a_i_0, a_i_1, b_j_0, b_j_1, NULL);
//
//    mpz_t sum;
//    mpz_init(sum);
//    mpz_set_ui(sum, 0);
//
//    for (int i = 0; i < f.rows; i++) {
//        for (int j = 0; j < f.cols; j++) {
//            cfe_vec_get(x_i, &x, i);
//            cfe_vec_get(s_i, &(msk.s), i);
//            mpz_mul(tmp, gamma, s_i);
//
//            cfe_vec_set(&v, x_i, 0);
//            cfe_vec_set(&v, tmp, 1);
//
//            cfe_mat_mul_vec(&a_i, &W_inv_tr, &v);
//            cfe_vec_mod(&a_i, &a_i, s.mod);
//
//            cfe_vec_get(y_j, &y, j);
//            cfe_vec_get(t_j, &(msk.t), j);
//            mpz_mul(t_j, t_j, minus);
//            cfe_vec_set(&v, y_j, 0);
//            cfe_vec_set(&v, t_j, 1);
//
//            cfe_mat_mul_vec(&b_j, &W, &v);
//            cfe_vec_mod(&b_j, &b_j, s.mod);
//
//
//
//            mpz_t el;
//            mpz_init(el);
//            cfe_mat_get(el, &f, i, j);
//
//            cfe_vec_get(a_i_0, &a_i, 0);
//            cfe_vec_get(a_i_1, &a_i, 1);
//
//            cfe_vec_get(b_j_0, &b_j, 0);
//            cfe_vec_get(b_j_1, &b_j, 1);
//
//            // check: a_i * b_j = x_i * y_j - gamma * s_i * t_j
//            mpz_t l, xy, st, foo;
//            mpz_inits(l, xy, st, foo, NULL);
//            cfe_vec_dot(l, &a_i, &b_j);
//            mpz_mul(xy, x_i, y_j);
//            mpz_mul(st, s_i, t_j);
//            mpz_mul(st, gamma, st);
//            mpz_sub(foo, xy, st);
//
//            printf("\n aaaaaaaaaaaaaa \n");
//            gmp_printf ("l %Zd\n", l);
//            gmp_printf ("foo %Zd\n", foo);
//
//
//
//            mpz_t tmp1, tmp2;
//            mpz_inits(tmp1, tmp2, NULL);
//            mpz_mul(tmp1, a_i_0, b_j_0);
//            mpz_mul(tmp2, a_i_1, b_j_1);
//            mpz_add(tmp1, tmp1, tmp2);
//
//            mpz_mul(tmp1, tmp1, el);
//            mpz_add(sum, sum, tmp1);
//        }
//    }
//
//    mpz_t check1;
//    mpz_init(check1);
//    mpz_add(check1, left, sum);
//    printf("\n---------------_____________------------------\n");
//    gmp_printf ("check1 %Zd\n", check1);
//
//    // debug end

    ECP2_BN254 key;
    cfe_sgp_derive_key(&key, &msk, &f);

//    cfe_sgp_decrypt(&ciphertext, &key, &f);

    mpz_t(xy);
    mpz_init(xy);
    cfe_mat_mul_x_mat_y(xy, &f, &x, &y);



//    gmp_printf ("\nxy %Zd\n", xy);

//    ECP_BN254 g1;
//    ECP_BN254_generator(&g1);
//    ECP2_BN254 g2;
//    ECP2_BN254_generator(&g2);
//
//    FP12_BN254 gt, r;
//    PAIR_BN254_ate(&gt, &g1, &g2);
//    PAIR_BN254_fexp(&gt);
//
//    BIG_256_56 xy_b;
//    BIG_256_56_from_mpz(xy_b, xy);
//
//    FP12_BN254_pow(&r, &gt, xy_b);
//
//    printf("\n!!!!!!!!!!!!----------------!!!!!!!!!!\n");
//    FP12_BN254_output(&r);

    munit_assert(err == 0);


    return MUNIT_OK;
}

MunitTest simple_sgp_tests[] = {
        {(char *) "/end-to-end", test_sgp_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite sgp_suite = {
        (char *) "/quadratic/sgp", simple_sgp_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

