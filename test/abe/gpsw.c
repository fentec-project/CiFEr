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
#include <internal/big.h>
#include <amcl/ecp_BN254.h>
#include <amcl/pair_BN254.h>
#include <internal/common.h>
#include "test.h"

#include "abe/gpsw.h"
#include "abe/policy.h"
#include "sample/uniform.h"

MunitResult test_gpsw_end_to_end(const MunitParameter *params, void *data) {

    cfe_gpsw gpsw;
    cfe_gpsw_init(&gpsw, 10);

    cfe_gpsw_pub_key pk;
    cfe_vec sk;
    generate_master_keys(&gpsw, &pk, &sk);

    FP12_BN254 msg;
    FP12_BN254_one(&msg);

    int *gamma= (int *) cfe_malloc(7 * sizeof(int));
    gamma[0] = 1;
    gamma[1] = 2;
    gamma[2] = 3;
    gamma[3] = 4;
    gamma[4] = 5;
    gamma[5] = 6;
    gamma[6] = 7;
//    int *gamma= (int *) cfe_malloc(1 * sizeof(int));
//    gamma[0] = 1;

    cfe_gpsw_cipher cipher;
    gpsw_encrypt(&cipher, &gpsw, &msg, gamma, 7, &pk);

    // define a boolean expression and make a corresponding msp structure
    char bool_exp[] = "(5 OR 3) AND ((2 OR 4) OR (1 AND 6))";
//    char bool_exp[] = "1";
    cfe_msp msp;
    boolean_to_msp(&msp, bool_exp, true);


    cfe_vec_G1 policy_keys;
    generate_policy_keys(&policy_keys, &gpsw, &msp, &sk);

    int *owned_atrib= (int *) cfe_malloc(3 * sizeof(int));
    owned_atrib[0] = 1;
    owned_atrib[1] = 3;
    owned_atrib[2] = 6;
//    int *owned_atrib= (int *) cfe_malloc(1 * sizeof(int));
//    owned_atrib[0] = 1;

    cfe_gpsw_keys keys;
    delegate_keys(&keys, &policy_keys, &msp, owned_atrib, 3);



    FP12_BN254 decryption;

    int check = gpsw_decrypt(&decryption, &cipher, &keys, &gpsw);
    munit_assert(check == 0);

    munit_assert(FP12_BN254_equals(&msg, &decryption) == 1);
// TODO check negative in FP12
    return MUNIT_OK;

}



MunitTest gpsw_tests[] = {
        {(char *) "/end-to-end", test_gpsw_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite gpsw_suite = {
        (char *) "/abe/gpsw", gpsw_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
