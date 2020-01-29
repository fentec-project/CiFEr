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

#include "cifer/data/vec.h"
#include "cifer/test.h"
#include "cifer/sample/normal_cdt.h"
#include "cifer/sample/normal.h"


MunitResult test_normal_cdt(const MunitParameter *params, void *data) {
    size_t size = 100000;
    cfe_vec v;
    cfe_vec_init(&v, size);
    mpz_t sample;
    mpz_init(sample);

    for (size_t i = 0; i < size; i++) {
        cfe_normal_cdt_sample(sample);
        cfe_vec_set(&v, sample, i);
    }

    mpf_t me, var;
    mpf_inits(me, var, NULL);
    cfe_mean(me, &v);
    cfe_variance(var, &v);

    double mean_d = mpf_get_d(me);
    double var_d = mpf_get_d(var);

    double mean_low = 0.39;
    double mean_high = 0.41;
    double var_low = 0.48;
    double var_high = 0.5;

    munit_assert(mean_d > mean_low);
    munit_assert(mean_d < mean_high);
    munit_assert(var_d > var_low);
    munit_assert(var_d < var_high);

    mpz_clear(sample);
    mpf_clears(me, var, NULL);
    cfe_vec_free(&v);
    return MUNIT_OK;
}


MunitTest normal_cdt_tests[] = {
        {(char *) "/sigma=10", test_normal_cdt, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite normal_cdt_suite = {
        (char *) "/sample/normal_cdt", normal_cdt_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
