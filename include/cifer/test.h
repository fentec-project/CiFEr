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

#ifndef CIFER_TEST_H
#define CIFER_TEST_H

#include <munit.h>

extern MunitSuite prime_suite;
extern MunitSuite keygen_suite;
extern MunitSuite matrix_suite;
extern MunitSuite vector_suite;
extern MunitSuite dlog_suite;
extern MunitSuite big_suite;
extern MunitSuite string_suite;
extern MunitSuite uniform_suite;
extern MunitSuite normal_suite;
extern MunitSuite normal_cumulative_suite;
extern MunitSuite normal_negative_suite;
extern MunitSuite normal_double_suite;
extern MunitSuite normal_double_constant_suite;
extern MunitSuite normal_cdt_suite;
extern MunitSuite ddh_suite;
extern MunitSuite damgard_suite;
extern MunitSuite ddh_multi_suite;
extern MunitSuite damgard_multi_suite;
extern MunitSuite lwe_suite;
extern MunitSuite lwe_fully_secure_suite;
extern MunitSuite ring_lwe_suite;
extern MunitSuite paillier_suite;
extern MunitSuite dmcfe_suite;
extern MunitSuite damgard_dec_multi_suite;
extern MunitSuite fhipe_suite;
extern MunitSuite fh_multi_ipe_suite;
extern MunitSuite policy_suite;
extern MunitSuite gpsw_suite;
extern MunitSuite fame_suite;
extern MunitSuite dippe_suite;
extern MunitSuite sgp_suite;

#endif
