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

#include "cifer/sample/normal_double_constant.h"
#include "cifer/sample/normal_cdt.h"
#include "cifer/test.h"
#include "cifer/sample/normal_double.h"

MunitResult test_normal_double_constant(const MunitParameter *params, void *data) {
    mpz_t k, sample;
    mpz_inits(k, sample, NULL);
    double mean_low, mean_high, var_low, var_high;
    mean_low = -0.01;
    mean_high = 0.01;
    var_low = cfe_sigma_cdt * cfe_sigma_cdt - 0.02;
    var_high = cfe_sigma_cdt * cfe_sigma_cdt + 0.02;

    const char *sigma = munit_parameters_get(params, "sigma");
    if (strcmp(sigma, "1 * sqrt(1/(2*ln(2)))") == 0) {
        mpz_set_ui(k, 1);
    } else if (strcmp(sigma, "10 * sqrt(1/(2*ln(2)))") == 0) {
        mpz_set_ui(k, 10);
        mean_low *= 10;
        mean_high *= 10;
        var_low *= 100;
        var_high *= 100;
    } else if (strcmp(sigma, "100 * sqrt(1/(2*ln(2)))") == 0) {
        mpz_set_ui(k, 100);
        mean_low *= 100;
        mean_high *= 100;
        var_low *= 10000;
        var_high *= 10000;
    }

    cfe_normal_double_constant s;
    cfe_normal_double_constant_init(&s, k);

    size_t size = 100000;
    cfe_vec v;
    cfe_vec_init(&v, size);

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

    munit_assert(mean_d > mean_low);
    munit_assert(mean_d < mean_high);
    munit_assert(var_d > var_low);
    munit_assert(var_d < var_high);

    mpz_clears(k, sample, NULL);
    mpf_clears(me, var, NULL);
    cfe_vec_free(&v);
    cfe_normal_double_constant_free(&s);

    return MUNIT_OK;
}

char *normal_double_constant_param[] = {
        (char *) "1 * sqrt(1/(2*ln(2)))",
        (char *) "10 * sqrt(1/(2*ln(2)))",
        (char *) "100 * sqrt(1/(2*ln(2)))",
        NULL
};

MunitParameterEnum normal_double_constant_params[] = {
        {(char *) "sigma", normal_double_constant_param},
        {NULL, NULL},
};

MunitTest normal_double_constant_tests[] = {
        {(char *) "/mean_var_test", test_normal_double_constant, NULL, NULL, MUNIT_TEST_OPTION_NONE, normal_double_constant_params},
        {NULL, NULL,                                             NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite normal_double_constant_suite = {
        (char *) "/sample/normal_double_constant", normal_double_constant_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
