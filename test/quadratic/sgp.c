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
#include <amcl/big_256_56.h>
#include <cifer/internal/big.h>

#include "cifer/test.h"
#include "cifer/quadratic/sgp.h"
#include "cifer/sample/uniform.h"

MunitResult test_sgp_end_to_end(const MunitParameter *params, void *data) {
    // set the parameters
    size_t l = 5;
    mpz_t b, b_neg;
    mpz_inits(b, b_neg, NULL);
    mpz_set_si(b, 8);
    mpz_neg(b_neg, b);
    cfe_error err;

    // create a scheme
    cfe_sgp s;
    err = cfe_sgp_init(&s, l, b);
    munit_assert(err == 0);

    // create a master secret key
    cfe_sgp_sec_key msk;
    cfe_sgp_sec_key_init(&msk, &s);
    cfe_sgp_generate_sec_key(&msk, &s);

    // take random vectors x, y
    cfe_vec x, y;
    cfe_vec_inits(s.l, &x, &y, NULL);
    cfe_uniform_sample_range_vec(&x, b_neg, b);
    cfe_uniform_sample_range_vec(&y, b_neg, b);

    // encrypt the vectors
    cfe_sgp_cipher cipher;
    cfe_sgp_cipher_init(&cipher, &s);
    err = cfe_sgp_encrypt(&cipher, &s, &x, &y, &msk);
    munit_assert(err == 0);

    // derive keys and decrypt the value x*m*y for a
    // random matrix f
    cfe_mat m;
    cfe_mat_init(&m, l, l);
    cfe_uniform_sample_range_mat(&m, b_neg, b);
    ECP2_BN254 key;
    err = cfe_sgp_derive_key(&key, &s, &msk, &m);
    munit_assert(err == 0);
    mpz_t dec, xy;
    mpz_inits(dec, xy , NULL);
    cfe_sgp_decrypt(dec, &s, &cipher, &key, &m);
    cfe_mat_mul_x_mat_y(xy, &m, &x, &y);
    munit_assert(mpz_cmp(dec, xy) == 0);

    cfe_vec_frees(&x, &y, NULL);
    cfe_mat_free(&m);
    cfe_sgp_free(&s);
    cfe_sgp_sec_key_free(&msk);
    cfe_sgp_cipher_free(&cipher);
    mpz_clears(b, b_neg, xy, dec, NULL);

    return MUNIT_OK;
}

MunitTest simple_sgp_tests[] = {
        {(char *) "/end-to-end", test_sgp_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite sgp_suite = {
        (char *) "/quadratic/sgp", simple_sgp_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

