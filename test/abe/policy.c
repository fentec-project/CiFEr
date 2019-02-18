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

#include "munit.h"
#include <stdbool.h>
#include <stdio.h>
#include <sample/uniform.h>

#include "abe/policy.h"


MunitResult test_boolean_to_msp(const MunitParameter params[], void *data) {
    // define a boolean expression and make a corresponding msp structure
    char bool_exp[] = "(5 OR 3) AND ((2 OR 4) OR (1 AND 6))";
    cfe_msp msp;
    boolean_to_msp(&msp, bool_exp, true);

    // create parameters to test msp
    cfe_mat sub_mat, sub_mat_transpose;
    cfe_mat_init(&sub_mat, 2, msp.mat->cols);
    cfe_mat_init(&sub_mat_transpose, msp.mat->cols, 2);

    // take a submatrix of the msp matrix on the second and forth row
    // simulating that one has attribute 3 and 4.
    sub_mat.mat[0] = msp.mat->mat[1];
    sub_mat.mat[1] = msp.mat->mat[3];

    // test if the rows of sub_mat span tha vector [1, 1,..., 1]
    // using gaussian elimination
    cfe_mat_transpose(&sub_mat_transpose, &sub_mat);
    cfe_vec one_vec, x;
    cfe_vec_init(&x, 2);
    mpz_t one, p;
    mpz_init_set_ui(one, 1);
    cfe_vec_const(&one_vec, sub_mat.cols, one);

    mpz_init_set_ui(p, 17);
    int check = gaussian_elimination(&x, &sub_mat_transpose, &one_vec, p);


    munit_assert(check == 0);

    // clearup
    cfe_msp_free(&msp);
    cfe_mat_frees(&sub_mat, &sub_mat_transpose, NULL);
    cfe_vec_frees(&one_vec, &x, NULL);
    mpz_clears(one, p, NULL);

    return MUNIT_OK;
}

MunitResult test_gaussian_elimination(const MunitParameter params[], void *data) {
    // define parameters
    mpz_t p;
    mpz_init_set_ui(p, 17);
    cfe_mat mat;
    cfe_mat_init(&mat, 100, 50);
    cfe_vec x_test, x, vec;
    cfe_vec_init(&x_test, 50);
    cfe_vec_init(&vec, 100);

    // sample random mat, x_test and calculate vec = mat * x_test
    cfe_uniform_sample_vec(&x_test, p);
    cfe_uniform_sample_mat(&mat, p);
    cfe_mat_mul_vec(&vec, &mat, &x_test);
    cfe_vec_mod(&vec, &vec, p);

    // use gaussian elimination to get x solving the equation vec = mat * x
    gaussian_elimination(&x, &mat, &vec, p);

    // check if the result is correct, i.e. x_test = x
    for (size_t i = 0; i < x.size; i++) {
        munit_assert(mpz_cmp(x.vec[i], x_test.vec[i]) == 0);
    }

    // clearup
    mpz_clear(p);
    cfe_vec_frees(&x_test, &x, &vec, NULL);
    cfe_mat_free(&mat);
    return MUNIT_OK;
}





MunitTest policy_tests[] = {
        {(char *) "/test-boolean_to_msp",       test_boolean_to_msp,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-gaussian-elimination", test_gaussian_elimination, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL,                                  NULL,                      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite policy_suite = {
        (char *) "/abe/policy", policy_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
