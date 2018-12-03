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
#include "internal/prime.h"

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
