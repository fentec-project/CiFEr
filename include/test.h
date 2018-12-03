/* *
 * Copyright (C) 2018 XLAB d.o.o.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of either:
 *
 *     * the GNU Lesser General Public License as published by the Free
 *     Software Foundation; either version 3 of the License, or (at your
 *     option) any later version.
 *
 * or
 *
 *     * the GNU General Public License as published by the Free Software
 *     Foundation; either version 2 of the License, or (at your option) any
 *     later version.
 *
 * or both in parallel, as here.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CIFER_TEST_H
#define CIFER_TEST_H

#include <munit.h>

MunitSuite prime_suite;
MunitSuite keygen_suite;
MunitSuite matrix_suite;
MunitSuite vector_suite;
MunitSuite dlog_suite;
MunitSuite uniform_suite;
MunitSuite normal_cumulative_suite;
MunitSuite normal_negative_suite;
MunitSuite normal_double_suite;
MunitSuite ddh_suite;
MunitSuite damgard_suite;
MunitSuite ddh_multi_suite;
MunitSuite damgard_multi_suite;
MunitSuite lwe_suite;
MunitSuite lwe_fully_secure_suite;
MunitSuite ring_lwe_suite;
MunitSuite paillier_suite;

#endif
