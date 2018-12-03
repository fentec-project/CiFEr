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
#include "innerprod/fullysec/lwe_fs.h"
#include "sample/uniform.h"

MunitResult test_lwe_fully_secure(const MunitParameter *params, void *data) {
    size_t l = 4;  /* dimensionality of vector space for the inner product */
    size_t n = 64;  /* security parameter */

    // maximal size of the entry of the message and other operand
    mpz_t bound_x, bound_x_neg, bound_y, bound_y_neg;
    mpz_inits(bound_x, bound_x_neg, bound_y, bound_y_neg, NULL);
    mpz_set_ui(bound_x, 1000);
    mpz_set_ui(bound_y, 1000);
    mpz_neg(bound_x_neg, bound_x);
    mpz_neg(bound_y_neg, bound_y);

    cfe_vec x, y;
    cfe_vec_inits(l, &x, &y, NULL);
    cfe_uniform_sample_range_vec(&x, bound_x_neg, bound_x);
    cfe_uniform_sample_range_vec(&y, bound_y_neg, bound_y);

    mpz_t expect, res;
    mpz_init(expect);
    cfe_vec_dot(expect, &x, &y);

    cfe_lwe_fs s;
    cfe_error err = cfe_lwe_fs_init(&s, l, n, bound_x, bound_y);
    munit_assert(!err);

    cfe_mat SK;
    err = cfe_lwe_fs_generate_sec_key(&SK, &s);
    munit_assert(!err);

    cfe_mat PK;
    cfe_lwe_fs_generate_pub_key(&PK, &s, &SK);

    cfe_vec z_y;
    err = cfe_lwe_fs_derive_key(&z_y, &s, &y, &SK);
    munit_assert(!err);

    cfe_vec cipher;
    err = cfe_lwe_fs_encrypt(&cipher, &s, &x, &PK);
    munit_assert(!err);

    err = cfe_lwe_fs_decrypt(res, &s, &cipher, &z_y, &y);
    munit_assert(!err);

    munit_assert(mpz_cmp(res, expect) == 0);

    cfe_lwe_fs_free(&s);
    mpz_clears(bound_x, bound_x_neg, bound_y, bound_y_neg, res, expect, NULL);
    cfe_vec_frees(&z_y, &cipher, &x, &y, NULL);
    cfe_mat_frees(&SK, &PK, NULL);

    return MUNIT_OK;
}

MunitTest lwe_fully_secure_tests[] = {
        {(char *) "/end-to-end", test_lwe_fully_secure, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite lwe_fully_secure_suite = {
        (char *) "/innerprod/fullysec/lwe-fs", lwe_fully_secure_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
