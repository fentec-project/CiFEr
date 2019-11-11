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

#include "cifer/test.h"
#include "cifer/innerprod/fullysec/fhipe.h"
#include "cifer/sample/uniform.h"

MunitResult test_fhipe_end_to_end(const MunitParameter *params, void *data) {
    size_t l = 10;
    mpz_t bound, bound_neg, xy_check, xy;
    mpz_inits(bound, bound_neg, xy_check, xy, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 10);
    mpz_neg(bound_neg, bound);

    cfe_fhipe fhipe;
    cfe_fhipe_init(&fhipe, l, bound, bound);

    return MUNIT_OK;
}

MunitTest fully_secure_fhipe_tests[] = {
        {(char *) "/end-to-end", test_fhipe_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite fhipe_suite = {
        (char *) "/innerprod/fullysec/fhipe", fully_secure_fhipe_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

