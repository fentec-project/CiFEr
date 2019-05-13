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
    size_t n = 3;
    mpz_t b, b_neg;
    mpz_inits(b, b_neg, NULL);
    mpz_set_si(b, 8);
    mpz_neg(b_neg, b);

    cfe_sgp s;
    cfe_sgp_init(&s, n, b);

    cfe_sgp_sec_key msk;
    cfe_sgp_sec_key_init(&msk, &s);
    cfe_sgp_generate_master_key(&msk, &s);

    cfe_vec x, y;
    cfe_vec_inits(s.n, &x, &y, NULL);
    cfe_uniform_sample_range_vec(&x, b_neg, b);
    cfe_uniform_sample_range_vec(&y, b_neg, b);

    cfe_sgp_cipher cipher;
    cfe_sgp_cipher_init(&cipher, &s);
    cfe_sgp_encrypt(&cipher, &s, &x, &y, &msk);

    cfe_mat f;
    cfe_mat_init(&f, n, n);
    cfe_uniform_sample_range_mat(&f, b_neg, b);
    ECP2_BN254 key;
    cfe_sgp_derive_key(&key, &msk, &f);
    mpz_t dec;
    mpz_init(dec);
    cfe_sgp_decrypt(dec, &cipher, &key, &f, &s);

    mpz_t(xy);
    mpz_init(xy);
    cfe_mat_mul_x_mat_y(xy, &f, &x, &y);

    gmp_printf("%Zd, %Zd, \n", dec, xy);

//
//    ECP_BN254 g1;
//    ECP_BN254_generator(&g1);
//    ECP2_BN254 g2;
//    ECP2_BN254_generator(&g2);
//
//    FP12_BN254 gt, r;
//    PAIR_BN254_ate(&gt, &g2, &g1);
//    PAIR_BN254_fexp(&gt);
//
//    if (mpz_cmp_ui(xy, 0) == -1) {
//        mpz_neg(xy, xy);
//        FP12_BN254_inv(&gt, &gt);
//    }
//
//    BIG_256_56 xy_b;
//    BIG_256_56_from_mpz(xy_b, xy);
//    FP12_BN254_pow(&r, &gt, xy_b);

    munit_assert(mpz_cmp(dec, xy) == 0);

    cfe_vec_frees(&x, &y, NULL);
    cfe_mat_free(&f);
    cfe_sgp_free(&s);
    cfe_sgp_sec_key_free(&msk);
    cfe_sgp_cipher_free(&cipher, &s);
    mpz_clears(b, b_neg, xy, NULL);

    return MUNIT_OK;
}

MunitTest simple_sgp_tests[] = {
        {(char *) "/end-to-end", test_sgp_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite sgp_suite = {
        (char *) "/quadratic/sgp", simple_sgp_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

