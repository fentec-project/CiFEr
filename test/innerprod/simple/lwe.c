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

#include "test.h"
#include "internal/keygen.h"
#include "innerprod/simple/lwe.h"
#include "sample/uniform.h"

MunitResult test_lwe(const MunitParameter *params, void *data) {
    // Length of data vectors x, y
    size_t l = 4;
    size_t n = 128;

    // message space size
    mpz_t B, B_neg;
    mpz_init_set_ui(B, 10000);
    mpz_init(B_neg);
    mpz_neg(B_neg, B);

    // Operands for the inner product
    cfe_vec x, y;
    cfe_vec_inits(l, &x, &y, NULL);
    cfe_uniform_sample_range_vec(&x, B_neg, B);
    cfe_uniform_sample_range_vec(&y, B_neg, B);

    mpz_t expect, res;
    mpz_init(expect);
    cfe_vec_dot(expect, &x, &y);

    cfe_lwe s;
    cfe_error err = cfe_lwe_init(&s, l, B, B, n);
    munit_assert(!err);

    cfe_mat SK, PK; // secret and public keys
    cfe_lwe_generate_sec_key(&SK, &s);
    err = cfe_lwe_generate_pub_key(&PK, &s, &SK);
    munit_assert(!err);

    cfe_vec sk_y, c;
    err = cfe_lwe_derive_key(&sk_y, &s, &SK, &y);
    munit_assert(!err);

    err = cfe_lwe_encrypt(&c, &s, &x, &PK);
    munit_assert(!err);

    err = cfe_lwe_decrypt(res, &s, &c, &sk_y, &y);
    munit_assert(!err);

    munit_assert(mpz_cmp(res, expect) == 0);

    mpz_clears(B, B_neg, expect, res, NULL);
    cfe_vec_frees(&x, &y, &sk_y, &c, NULL);
    cfe_mat_frees(&SK, &PK, NULL);
    cfe_lwe_free(&s);

    return MUNIT_OK;
}

MunitTest lwe_tests[] = {
        {(char *) "/end-to-end", test_lwe, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite lwe_suite = {
        (char *) "/innerprod/simple/lwe", lwe_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
