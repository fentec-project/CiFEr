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

#include <stdio.h>
#include "cifer/test.h"
#include "cifer/internal/common.h"

int main(int argc, char *argv[]) {
    MunitSuite all_suites[] = {
//            keygen_suite,
//            matrix_suite,
//            prime_suite,
//            vector_suite,
//            dlog_suite,
//            big_suite,
//            string_suite,
//            ddh_suite,
//            ddh_multi_suite,
//            lwe_suite,
//            ring_lwe_suite,
//            damgard_suite,
//            damgard_multi_suite,
//            lwe_fully_secure_suite,
//            paillier_suite,
//            dmcfe_suite,
//            damgard_dec_multi_suite,
//            fhipe_suite,
//            fh_multi_ipe_suite,
//            policy_suite,
            gpsw_suite,
//            fame_suite,
//            dippe_suite,
//            uniform_suite,
//            normal_suite,
//            normal_cumulative_suite,
//            normal_negative_suite,
//            normal_double_suite,
//            normal_double_constant_suite,
//            normal_cdt_suite,
//            sgp_suite,
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
