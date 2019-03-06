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
#include <amcl/ecp_BN254.h>
#include <amcl/pair_BN254.h>

#include "internal/big.h"
#include "internal/common.h"
#include "test.h"
#include "abe/fame.h"
#include "abe/policy.h"
#include "sample/uniform.h"

MunitResult test_fame_end_to_end(const MunitParameter *params, void *data) {
    cfe_fame fame;
    cfe_fame_sec_key sk;
    cfe_fame_pub_key pk;
    cfe_fame_init(&fame);
    cfe_fame_sec_key_init(&sk);
    cfe_fame_generate_master_keys(&pk, &sk, &fame);

    char src[] = "2";
    octet dst;
//    dst.val = cfe_malloc(5 * sizeof(char));
    dst.val = "1999999999199999999919999999991";
    dst.max = 10;
    dst.len = 7;
//    OCT_fromHex(&dst, src);
    printf("%s\n", dst.val);
    OCT_output(&dst);
    ECP2_BN254 test;
    ECP2_BN254_mapit(&test, &dst);
    ECP2_BN254_output(&test);

    char *test1 = strings_concat("aa", "bbb", NULL);
    printf("%s\n", test1);

    char *test2 = int_to_str(984524);
    printf("%s\n", test2);

    ECP_BN254 test3;
    hash_G1(&test3, "123");
    ECP_BN254_output(&test3);
    return MUNIT_OK;
}

MunitTest fame_tests[] = {
        {(char *) "/end-to-end", test_fame_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite fame_suite = {
        (char *) "/abe/fame", fame_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
