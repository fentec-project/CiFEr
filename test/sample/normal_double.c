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
#include "sample/normal_double.h"

void test_normal_double_helper(double sigma_d, double first_sigma_d, double mean_low, double mean_high, double var_low,
                               double var_high) {
    mpf_t sigma, first_sigma;
    mpf_init_set_d(sigma, sigma_d);
    mpf_init_set_d(first_sigma, first_sigma_d);
    size_t n = 256;

    cfe_normal_double s;
    cfe_error err = cfe_normal_double_init(&s, sigma, n, first_sigma);
    munit_assert(err == 0);

    size_t size = 10000;
    cfe_vec v;
    cfe_vec_init(&v, size);

    mpz_t sample;
    mpz_init(sample);

    for (size_t i = 0; i < size; i++) {
        cfe_normal_double_sample(sample, &s);
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
    mpf_clears(sigma, first_sigma, me, var, NULL);
    cfe_vec_free(&v);
    cfe_normal_double_free(&s);
}

MunitResult test_normal_double1(const MunitParameter *params, void *data) {
    test_normal_double_helper(10.0, 1.0, -2, 2, 90, 110);
    return MUNIT_OK;
}

MunitResult test_normal_double2(const MunitParameter *params, void *data) {
    test_normal_double_helper(9.0, 1.5, -2, 2, 70, 110);
    return MUNIT_OK;
}

MunitResult test_normal_double_fail(const MunitParameter *params, void *data) {
    mpf_t sigma, first_sigma;
    mpf_init_set_d(sigma, 1.5);
    mpf_init_set_ui(first_sigma, 1);
    size_t n = 256;

    cfe_normal_double s;
    cfe_error err = cfe_normal_double_init(&s, sigma, n, first_sigma);
    munit_assert(err);

    mpf_clears(sigma, first_sigma, NULL);
    cfe_normal_double_free(&s);
    return MUNIT_OK;
}


MunitTest normal_double_tests[] = {
        {(char *) "/sigmas=[1,10]",  test_normal_double1,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/sigmas=[1.5,9]", test_normal_double2,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/check_sigma",    test_normal_double_fail, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite normal_double_suite = {
        (char *) "/sample/normal_double", normal_double_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
