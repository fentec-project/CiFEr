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
#include <math.h>

#include <cifer/data/vec.h>
#include "cifer/test.h"
#include "cifer/sample/normal_cdt.h"
#include "cifer/sample/normal.h"
#include "cifer/sample/uniform.h"


MunitResult test_bernoulli(const MunitParameter *params, void *data) {
    size_t size = 100000;
    cfe_vec v;
    cfe_vec_init(&v, size);
    mpz_t t, bound, one, zero;
    mpz_inits(t, bound, one, zero, NULL);
    mpz_set_ui(bound, 100);
    mpz_set_ui(one, 1);
    mpz_set_ui(zero, 0);

    mpf_t k_square_inv;
    mpf_init(k_square_inv);
    mpf_set_d(k_square_inv, 0.01);
    cfe_uniform_sample(t, bound);
    mpz_set_ui(t, 10);

    double t_d = mpz_get_d(t);
    double k_square_inv_d = mpf_get_d(k_square_inv);
    double me_correct = pow(2, -t_d * k_square_inv_d);

    for (size_t i = 0; i < size; i++) {
        bool s = cfe_bernoulli(t, k_square_inv);
        if (s) {
            cfe_vec_set(&v, one, i);
        } else {
            cfe_vec_set(&v, zero, i);
        }
    }

    mpf_t me;
    mpf_init(me);
    cfe_mean(me, &v);

    double mean_d = mpf_get_d(me);

    double mean_low = me_correct - 0.01;
    double mean_high = me_correct + 0.01;
    munit_assert(mean_d > mean_low);
    munit_assert(mean_d < mean_high);

    mpf_clears(me, k_square_inv, NULL);
    mpz_clears(t, bound, one, zero, NULL);
    cfe_vec_free(&v);
    return MUNIT_OK;
}


MunitTest normal_tests[] = {
        {(char *) "/bernoulli", test_bernoulli, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                 NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite normal_suite = {
        (char *) "/sample/normal", normal_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
