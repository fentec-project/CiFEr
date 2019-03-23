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
#include "test.h"

#include "innerprod/fullysec/damgard.h"
#include "sample/uniform.h"

MunitResult test_damgard_end_to_end(const MunitParameter *params, void *data) {
    size_t l = 3;
    size_t modulus_len = 64;
    mpz_t bound, bound_neg, key1, key2, xy_check, xy;
    mpz_inits(bound, bound_neg, key1, key2, xy_check, xy, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 10);
    mpz_neg(bound_neg, bound);


    cfe_damgard s, encryptor, decryptor;
    cfe_error err = cfe_damgard_init(&s, l, modulus_len, bound);
    munit_assert(err == 0);

    cfe_vec mpk, ciphertext, x, y;
    cfe_vec_inits(l, &x, &y, NULL);
    cfe_uniform_sample_range_vec(&x, bound_neg, bound);
    cfe_uniform_sample_range_vec(&y, bound_neg, bound);
    cfe_vec_dot(xy_check, &x, &y);

    cfe_damgard_sec_key msk;

    cfe_damgard_sec_key_init(&msk, &s);
    cfe_damgard_pub_key_init(&mpk, &s);
    cfe_damgard_generate_master_keys(&msk, &mpk, &s);

    cfe_damgard_fe_key key;
    cfe_damgard_fe_key_init(&key);
    err = cfe_damgard_derive_key(&key, &s, &msk, &y);
    munit_assert(err == 0);

    cfe_damgard_copy(&encryptor, &s);
    cfe_damgard_ciphertext_init(&ciphertext, &encryptor);
    err = cfe_damgard_encrypt(&ciphertext, &encryptor, &x, &mpk);
    munit_assert(err == 0);

    cfe_damgard_copy(&decryptor, &s);
    err = cfe_damgard_decrypt(xy, &decryptor, &ciphertext, &key, &y);
    munit_assert(err == 0);

    munit_assert(mpz_cmp(xy, xy_check) == 0);

    mpz_clears(bound, bound_neg, key1, key2, xy_check, xy, NULL);
    cfe_vec_frees(&x, &y, &mpk, &ciphertext, NULL);

    cfe_damgard_sec_key_free(&msk);
    cfe_damgard_derived_key_free(&key);
    cfe_damgard_free(&s);
    cfe_damgard_free(&encryptor);
    cfe_damgard_free(&decryptor);

    return MUNIT_OK;
}

MunitTest simple_ip_damgard_tests[] = {
        {(char *) "/end-to-end", test_damgard_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite damgard_suite = {
        (char *) "/innerprod/fullysec/damgard", simple_ip_damgard_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

