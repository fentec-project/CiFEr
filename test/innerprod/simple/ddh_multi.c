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
#include "innerprod/simple/ddh_multi.h"
#include "sample/uniform.h"

MunitResult test_ddh_multi_end_to_end(const MunitParameter *params, void *data) {
    size_t l = 3;
    size_t slots = 2;
    size_t modulus_len = 64;

    mpz_t bound, xy_check, xy;
    mpz_inits(bound, xy_check, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 10);

    cfe_ddh_multi m, decryptor;
    cfe_ddh_multi_enc encryptors[slots];
    cfe_error err = cfe_ddh_multi_init(&m, slots, l, modulus_len, bound);
    munit_assert(err == 0);

    cfe_mat x, y, ciphertext;
    cfe_mat_inits(slots, l, &x, &y, NULL);
    cfe_mat_init(&ciphertext, slots, l + 1);
    cfe_uniform_sample_mat(&y, bound);

    cfe_mat mpk;
    cfe_ddh_multi_sec_key msk;
    cfe_ddh_multi_fe_key key;

    cfe_ddh_multi_generate_master_keys(&mpk, &msk, &m);

    for (size_t i = 0; i < slots; i++) {
        cfe_ddh_multi_enc_init(&encryptors[i], &m);
    }

    err = cfe_ddh_multi_derive_key(&key, &m, &msk, &y);
    munit_assert(err == 0);

    for (size_t i = 0; i < slots; i++) {
        cfe_vec *x_vec = cfe_mat_get_row_ptr(&x, i);
        cfe_uniform_sample_vec(x_vec, bound);

        cfe_vec ct;
        cfe_vec *pub_key = cfe_mat_get_row_ptr(&mpk, i);
        cfe_vec *otp = cfe_mat_get_row_ptr(&msk.otp_key, i);
        err = cfe_ddh_multi_encrypt(&ct, &encryptors[i], x_vec, pub_key, otp);
        munit_assert(err == 0);

        cfe_mat_set_vec(&ciphertext, &ct, i);
        cfe_vec_free(&ct);
    }

    cfe_mat_dot(xy_check, &x, &y);
    mpz_mod(xy_check, xy_check, bound);

    cfe_ddh_multi_copy(&decryptor, &m);
    err = cfe_ddh_multi_decrypt(xy, &decryptor, &ciphertext, &key, &y);
    munit_assert(err == 0);

    munit_assert(mpz_cmp(xy, xy_check) == 0);

    mpz_clears(bound, xy_check, xy, NULL);
    cfe_mat_frees(&x, &y, &ciphertext, &mpk, NULL);
    cfe_ddh_multi_sec_key_free(&msk);
    cfe_ddh_multi_fe_key_free(&key);
    cfe_ddh_multi_free(&m);
    cfe_ddh_multi_free(&decryptor);
    for (size_t i = 0; i < slots; i++) {
        cfe_ddh_multi_enc_free(&encryptors[i]);
    }

    return MUNIT_OK;
}

MunitTest ddh_multi_simple_tests[] = {
        {(char *) "/end-to-end", test_ddh_multi_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite ddh_multi_suite = {
        (char *) "/innerprod/simple/ddh-multi", ddh_multi_simple_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
