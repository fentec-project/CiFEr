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
#include "cifer/internal/keygen.h"
#include "cifer/innerprod/simple/lwe.h"
#include "cifer/sample/uniform.h"

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
    mpz_inits(expect, res, NULL);
    cfe_vec_dot(expect, &x, &y);

    cfe_lwe s;
    cfe_error err = cfe_lwe_init(&s, l, B, B, n);
    munit_assert(!err);

    cfe_mat SK, PK; // secret and public keys
    cfe_lwe_sec_key_init(&SK, &s);
    cfe_lwe_generate_sec_key(&SK, &s);
    cfe_lwe_pub_key_init(&PK, &s);
    err = cfe_lwe_generate_pub_key(&PK, &s, &SK);
    munit_assert(!err);

    cfe_vec sk_y, ct;
    cfe_lwe_fe_key_init(&sk_y, &s);
    err = cfe_lwe_derive_key(&sk_y, &s, &SK, &y);
    munit_assert(!err);

    cfe_lwe_ciphertext_init(&ct, &s);
    err = cfe_lwe_encrypt(&ct, &s, &x, &PK);
    munit_assert(!err);

    err = cfe_lwe_decrypt(res, &s, &ct, &sk_y, &y);
    munit_assert(!err);

    munit_assert(mpz_cmp(res, expect) == 0);

    mpz_clears(B, B_neg, expect, res, NULL);
    cfe_vec_frees(&x, &y, &sk_y, &ct, NULL);
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
