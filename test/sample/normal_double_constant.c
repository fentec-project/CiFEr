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

#include <cifer/sample/normal_double_constant.h>
#include "cifer/test.h"
#include "cifer/sample/normal_double.h"

void test_normal_double_constant_helper(mpz_t k, double mean_low, double mean_high,
                                        double var_low,
                                        double var_high) {

    cfe_normal_double_constant s;
    cfe_normal_double_constant_init(&s, k);

    size_t size = 100000;
    cfe_vec v;
    cfe_vec_init(&v, size);

    mpz_t sample;
    mpz_init(sample);

    for (size_t i = 0; i < size; i++) {
        cfe_normal_double_constant_sample(sample, &s);
        cfe_vec_set(&v, sample, i);
    }

    mpf_t me, var;
    mpf_inits(me, var, NULL);
    cfe_mean(me, &v);
    cfe_variance(var, &v);

    double mean_d = mpf_get_d(me);
    double var_d = mpf_get_d(var);

    gmp_printf("%f %f \n", mean_d, var_d);

    munit_assert(mean_d > mean_low);
    munit_assert(mean_d < mean_high);
    munit_assert(var_d > var_low);
    munit_assert(var_d < var_high);

    mpz_clear(sample);
    mpf_clears(me, var, NULL);
    cfe_vec_free(&v);
    cfe_normal_double_constant_free(&s);
}

MunitResult test_normal_double_constant1(const MunitParameter *params, void *data) {
    mpz_t k;
    mpz_init_set_ui(k, 1);
    double sigmaCDTSquare = 0.84932180028801904272150283410;
    sigmaCDTSquare *= sigmaCDTSquare;

    test_normal_double_constant_helper(k, -0.1, 0.1, sigmaCDTSquare -0.02, sigmaCDTSquare + 0.02);
    return MUNIT_OK;
}

MunitResult test_normal_double_constant2(const MunitParameter *params, void *data) {
    mpz_t k;
    mpz_init_set_ui(k, 10);
    double sigmaCDTSquare = 0.84932180028801904272150283410;
    sigmaCDTSquare *= sigmaCDTSquare;
    test_normal_double_constant_helper(k, -0.1, 0.1, 100 * (sigmaCDTSquare -0.02), 100 * (sigmaCDTSquare + 0.02));
    return MUNIT_OK;
}

MunitTest normal_double_constant_tests[] = {
        {(char *) "/sigmas=[1,10]",  test_normal_double_constant1, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/sigmas=[1.5,9]", test_normal_double_constant2, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                               NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite normal_double_constant_suite = {
        (char *) "/sample/normal_double_constant", normal_double_constant_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
