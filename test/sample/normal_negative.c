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
#include "sample/normal_negative.h"

MunitResult test_normal_negative(const MunitParameter *params, void *data) {
    mpf_t sigma;
    mpf_init_set_ui(sigma, 10);
    size_t n = 256;

    double mean_low = -2;
    double mean_high = 2;
    double var_low = 90;
    double var_high = 110;

    cfe_normal_negative s;
    cfe_normal_negative_init(&s, sigma, n);

    size_t size = 10000;
    cfe_vec v;
    cfe_vec_init(&v, size);

    mpz_t sample;
    mpz_init(sample);

    for (size_t i = 0; i < size; i++) {
        cfe_normal_negative_sample(sample, &s);
        cfe_vec_set(&v, sample, i);
    }

    mpf_t me, var;
    mpf_inits(me, var, NULL);
    cfe_mean(me, &v);
    cfe_variance(var, &v);

    double mean_d = mpf_get_d(me);
    double var_d = mpf_get_d(var);

    munit_assert(mean_d > mean_low);
    munit_assert(mean_d < mean_high);
    munit_assert(var_d > var_low);
    munit_assert(var_d < var_high);

    mpz_clear(sample);
    mpf_clears(sigma, me, var, NULL);
    cfe_vec_free(&v);
    cfe_normal_negative_free(&s);
    return MUNIT_OK;
}


MunitTest normal_negative_tests[] = {
        {(char *) "/sigma=10", test_normal_negative, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                 NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite normal_negative_suite = {
        (char *) "/sample/normal_negative", normal_negative_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
