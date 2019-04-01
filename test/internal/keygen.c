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

#include "test.h"
#include "internal/keygen.h"

MunitResult test_elgamal_init_free(const MunitParameter *params, void *data) {
    cfe_elgamal key;
    cfe_error err = cfe_elgamal_init(&key, 32);
    munit_assert(err == 0);

    // only check that struct fields were set to something
    munit_assert(mpz_cmp_ui(key.p, 0));
    munit_assert(mpz_cmp_ui(key.g, 0));
    munit_assert(mpz_cmp_ui(key.q, 0));

    cfe_elgamal_free(&key);
    return MUNIT_OK;
}


MunitTest keygen_tests[] = {
        {(char *) "/elgamal-init-free", test_elgamal_init_free, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite keygen_suite = {
        (char *) "/keygen", keygen_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
