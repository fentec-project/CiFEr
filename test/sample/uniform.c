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

#include <sodium/randombytes.h>
#include "cifer/test.h"
#include "cifer/sample/uniform.h"

MunitResult test_uniform(const MunitParameter *params, void *data) {
    mpz_t r, bound;
    mpz_inits(r, bound, NULL);
    size_t b = 8;
    mpz_set_ui(bound, 10);

    cfe_uniform_sample_i(r, b); // we expect r to be assigned a value from [0, b)

    munit_assert(mpz_cmp_ui(r, 0) >= 0);    // r >= 0
    munit_assert(mpz_cmp_si(r, b) < 0);     // r < b

    cfe_vec v;
    cfe_vec_init(&v, 10);
    unsigned char key[randombytes_SEEDBYTES];
    for (size_t i=0; i<32; i++) {
        key[i] = 'a';
    }
    cfe_uniform_sample_vec_det(&v, bound, key);

    mpz_clears(r, bound, NULL);
    cfe_vec_free(&v);

    return MUNIT_OK;
}

MunitResult test_uniform_range(const MunitParameter *params, void *data) {
    int lower[] = {-10, -10, 5};
    int upper[] = {-5, 11, 10};

    mpz_t a, a_sub_1, b, r;
    mpz_inits(a, a_sub_1, b, r, NULL);

    for (int i = 0; i < 3; i++) {
        mpz_set_si(a, lower[i]);
        mpz_set_si(b, upper[i]);
        mpz_sub_ui(a_sub_1, a, 1);

        cfe_uniform_sample_range(r, a, b);

        munit_assert(mpz_cmp(a_sub_1, r));  // a <= r
        munit_assert(mpz_cmp(r, b));        // r < b

        cfe_uniform_sample_range_i_i(r, lower[i], upper[i]);

        munit_assert(mpz_cmp(a_sub_1, r));  // a <= r
        munit_assert(mpz_cmp(r, b));        // r < b

        cfe_uniform_sample_range_i_mpz(r, lower[i], b);

        munit_assert(mpz_cmp(a_sub_1, r));  // a <= r
        munit_assert(mpz_cmp(r, b));        // r < b
    };

    mpz_clears(a, a_sub_1, b, r, NULL);

    return MUNIT_OK;
}

MunitTest uniform_tests[] = {
        {(char *) "/below", test_uniform,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/range", test_uniform_range, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite uniform_suite = {
        (char *) "/sample/uniform", uniform_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
