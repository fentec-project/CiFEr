/*
 * Copyright (c) 2018 XLAB d.o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cifer/test.h"
#include "cifer/innerprod/fullysec/lwe_fs.h"
#include "cifer/sample/uniform.h"

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
    mpz_inits(expect, res, NULL);
    cfe_vec_dot(expect, &x, &y);

    cfe_lwe_fs s;
    cfe_error err = cfe_lwe_fs_init(&s, l, n, bound_x, bound_y);
    munit_assert(!err);

    cfe_mat SK;
    cfe_lwe_fs_sec_key_init(&SK, &s);
    err = cfe_lwe_fs_generate_sec_key(&SK, &s);
    munit_assert(!err);

    cfe_mat PK;
    cfe_lwe_fs_pub_key_init(&PK, &s);
    cfe_lwe_fs_generate_pub_key(&PK, &s, &SK);

    cfe_vec z_y;
    cfe_lwe_fs_fe_key_init(&z_y, &s);
    err = cfe_lwe_fs_derive_key(&z_y, &s, &y, &SK);
    munit_assert(!err);

    cfe_vec ciphertext;
    cfe_lwe_fs_ciphertext_init(&ciphertext, &s);
    err = cfe_lwe_fs_encrypt(&ciphertext, &s, &x, &PK);
    munit_assert(!err);

    err = cfe_lwe_fs_decrypt(res, &s, &ciphertext, &z_y, &y);
    munit_assert(!err);

    munit_assert(mpz_cmp(res, expect) == 0);

    cfe_lwe_fs_free(&s);
    mpz_clears(bound_x, bound_x_neg, bound_y, bound_y_neg, res, expect, NULL);
    cfe_vec_frees(&z_y, &ciphertext, &x, &y, NULL);
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
