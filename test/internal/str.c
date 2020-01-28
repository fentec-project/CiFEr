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

#include "munit.h"

#include "cifer/internal/str.h"

MunitResult test_str_to_int(const MunitParameter params[], void *data) {
    // test conversion from string to int
    cfe_string test_str;
    test_str.str = (char *) "15";
    test_str.str_len = 2;

    int i = cfe_str_to_int(&test_str);
    munit_assert(i == 15);

    // return an error (-1) if string is corrupted
    cfe_string test_faulty_str;
    test_faulty_str.str = (char *) "124h234";
    i = cfe_str_to_int(&test_faulty_str);
    munit_assert(i == -1);

    return MUNIT_OK;
}

MunitTest string_tests[] = {
        {(char *) "/test-str-to-int",           test_str_to_int,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                                       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite string_suite = {
        (char *) "/internal/string", string_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
