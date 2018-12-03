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
