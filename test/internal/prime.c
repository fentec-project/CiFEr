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
#include "cifer/internal/prime.h"

MunitResult test_is_safe_prime(const MunitParameter params[], void *data) {
    mpz_t n;
    mpz_init(n);

    mpz_set_ui(n, 4);   // not a prime
    munit_assert(!cfe_is_safe_prime(n));

    mpz_set_ui(n, 701); // prime, but not a safe prime
    munit_assert(!cfe_is_safe_prime(n));

    mpz_set_ui(n, 359); // safe prime
    munit_assert(cfe_is_safe_prime(n));

    mpz_clear(n);

    return MUNIT_OK;
}

MunitResult test_get_prime(const MunitParameter params[], void *data) {
    mpz_t p;
    mpz_init(p);

    cfe_error err = cfe_get_prime(p, 512, false);

    munit_assert(!err);
    munit_assert(mpz_probab_prime_p(p, 20));

    err = cfe_get_prime(p, 512, true);

    munit_assert(!err);

    munit_assert(cfe_is_safe_prime(p));
    munit_assert(mpz_probab_prime_p(p, 20));

    mpz_clear(p);

    return MUNIT_OK;
}

MunitResult test_get_safe_prime(const MunitParameter params[], void *data) {
    mpz_t p;
    mpz_init(p);

    cfe_error err = cfe_get_prime(p, 512, true);

    munit_assert(!err);

    munit_assert(cfe_is_safe_prime(p));
    munit_assert(mpz_probab_prime_p(p, 20));

    mpz_clear(p);

    return MUNIT_OK;
}

MunitTest prime_tests[] = {
        {(char *) "/prime-generation/common", test_get_prime,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/prime-generation/safe",   test_get_safe_prime, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/safe-prime-check",        test_is_safe_prime,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                               NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite prime_suite = {
        (char *) "/primes", prime_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
