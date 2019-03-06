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

#include <stdio.h>
#include "test.h"
#include "internal/common.h"

int main(int argc, char *argv[]) {
    MunitSuite all_suites[] = {
//            keygen_suite,
//            matrix_suite,
//            prime_suite,
//            vector_suite,
//            dlog_suite,
//            big_suite,
//            ddh_suite,
//            ddh_multi_suite,
//            lwe_suite,
//            ring_lwe_suite,
//            damgard_suite,
//            damgard_multi_suite,
//            lwe_fully_secure_suite,
//            paillier_suite,
//            policy_suite,
//            gpsw_suite,
            fame_suite,
//            uniform_suite,
//            normal_cumulative_suite,
//            normal_negative_suite,
//            normal_double_suite,
            {NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE}
    };

    MunitSuite main_suite = {
            (char *) "",
            NULL,
            all_suites,
            1,
            MUNIT_SUITE_OPTION_NONE
    };

    if (cfe_init()) {
        perror("Insufficient entropy available for random generation\n");
        return CFE_ERR_INIT;
    }

    return munit_suite_main(&main_suite, NULL, argc, argv);
}
