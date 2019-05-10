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

#include "cifer/data/mat.h"
#include "cifer/sample/uniform.h"

MunitResult test_matrix_init_free(const MunitParameter params[], void *data) {
    cfe_mat m;
    cfe_mat_init(&m, 5, 5);

    munit_assert(m.rows == 5 && m.cols == 5);

    cfe_mat_free(&m);

    return MUNIT_OK;
}

MunitResult test_matrix_inits_frees(const MunitParameter params[], void *data) {
    cfe_mat m1, m2, m3;
    cfe_mat_inits(3, 4, &m1, &m2, &m3, NULL);

    munit_assert(m1.rows == 3 && m1.cols == 4);
    munit_assert(m2.rows == 3 && m2.cols == 4);
    munit_assert(m3.rows == 3 && m3.cols == 4);

    mpz_t x, y;
    mpz_init_set_ui(x, 42);
    mpz_init(y);

    cfe_mat_set(&m1, x, 0, 2);

    cfe_mat_get(y, &m3, 0, 2);

    munit_assert(mpz_cmp(x, y) != 0);

    mpz_clears(x, y, NULL);
    cfe_mat_frees(&m1, &m2, &m3, NULL);

    return MUNIT_OK;
}

MunitResult test_matrix_get_set(const MunitParameter params[], void *data) {
    cfe_mat m;
    cfe_mat_init(&m, 5, 5);

    mpz_t x;
    mpz_init(x);
    for (size_t i = 0; i < 5; i++) {
        for (size_t j = 0; j < 5; j++) {
            mpz_set_si(x, i * 5 + j);
            cfe_mat_set(&m, x, i, j);
        }
    }

    for (size_t i = 0; i < 5; i++) {
        for (size_t j = 0; j < 5; j++) {
            cfe_mat_get(x, &m, i, j);
            munit_assert(mpz_cmp_si(x, i * 5 + j) == 0);
        }
    }

    mpz_clear(x);
    cfe_mat_free(&m);
    return MUNIT_OK;
}

MunitResult test_matrix_get_set_vec(const MunitParameter params[], void *data) {
    cfe_mat m;
    cfe_mat_init(&m, 5, 5);

    cfe_vec v1, v2;
    cfe_vec_inits(5, &v1, &v2, NULL);

    mpz_t x;
    mpz_init(x);

    for (size_t i = 0; i < 5; i++) {
        for (size_t j = 0; j < 5; j++) {
            mpz_set_si(x, i * 5 + j);
            cfe_vec_set(&v1, x, j);
        }

        cfe_mat_set_vec(&m, &v1, i);
    }

    for (size_t i = 0; i < 5; i++) {
        cfe_mat_get_row(&v2, &m, i);

        for (size_t j = 0; j < 5; j++) {
            cfe_vec_get(x, &v2, j);
            munit_assert(mpz_cmp_si(x, i * 5 + j) == 0);
        }
    }

    mpz_clear(x);
    cfe_vec_frees(&v1, &v2, NULL);
    cfe_mat_free(&m);

    return MUNIT_OK;
}

MunitResult test_matrix_get_row(const MunitParameter params[], void *data) {
    cfe_mat m;
    cfe_mat_init(&m, 2, 3);

    mpz_t upper;
    mpz_init_set_ui(upper, 10);
    cfe_uniform_sample_mat(&m, upper);

    cfe_vec v;
    cfe_vec_init(&v, 3);
    cfe_mat_get_row(&v, &m, 1);

    mpz_t x, y;
    mpz_inits(x, y, NULL);
    for (size_t i = 0; i < m.cols; i++) {
        cfe_vec_get(x, &v, i);
        cfe_mat_get(y, &m, 1, i);
        munit_assert(mpz_cmp(x, y) == 0);
    }

    mpz_clears(upper, x, y, NULL);
    cfe_vec_free(&v);
    cfe_mat_free(&m);

    return MUNIT_OK;
}

MunitResult test_matrix_get_row_ptr(const MunitParameter params[], void *data) {
    cfe_mat m;
    cfe_mat_init(&m, 3, 3);

    mpz_t x, y;
    mpz_init_set_ui(x, 42);
    mpz_init(y);

    cfe_vec *v = cfe_mat_get_row_ptr(&m, 1);
    cfe_vec_set(v, x, 1);

    cfe_mat_get(y, &m, 1, 1);

    munit_assert(mpz_cmp(x, y) == 0);

    mpz_clears(x, y, NULL);
    cfe_mat_free(&m);

    return MUNIT_OK;
}

MunitResult test_matrix_get_col(const MunitParameter params[], void *data) {
    cfe_mat m;
    cfe_mat_init(&m, 2, 3);

    mpz_t upper;
    mpz_init_set_ui(upper, 10);
    cfe_uniform_sample_mat(&m, upper);

    cfe_vec v;
    cfe_vec_init(&v, 2);
    cfe_mat_get_col(&v, &m, 1);

    mpz_t x, y;
    mpz_inits(x, y, NULL);
    for (size_t i = 0; i < m.rows; i++) {
        cfe_vec_get(x, &v, i);
        cfe_mat_get(y, &m, i, 1);
        munit_assert(mpz_cmp(x, y) == 0);
    }

    mpz_clears(upper, x, y, NULL);
    cfe_vec_free(&v);
    cfe_mat_free(&m);

    return MUNIT_OK;
}

MunitResult test_matrix_add(const MunitParameter params[], void *data) {
    cfe_mat m1, m2, res;
    cfe_mat_inits(5, 5, &m1, &m2, &res, NULL);

    mpz_t x;
    mpz_init(x);

    for (size_t i = 0; i < 5; i++) {
        for (size_t j = 0; j < 5; j++) {
            mpz_set_si(x, i * 5 + j);
            cfe_mat_set(&m1, x, i, j);
            cfe_mat_set(&m2, x, 4 - i, 4 - j);
        }
    }

    cfe_mat_add(&res, &m1, &m2);

    for (size_t i = 0; i < 5; i++) {
        for (size_t j = 0; j < 5; j++) {
            cfe_mat_get(x, &res, i, j);
            munit_assert(mpz_cmp_si(x, 24) == 0);
        }
    }

    mpz_clear(x);
    cfe_mat_frees(&m1, &m2, &res, NULL);

    return MUNIT_OK;
}

MunitResult test_matrix_mod(const MunitParameter params[], void *data) {
    cfe_mat m, res;
    cfe_mat_inits(5, 5, &m, &res, NULL);

    size_t mod = 12;

    mpz_t x;
    mpz_init(x);

    for (size_t i = 0; i < 5; i++) {
        for (size_t j = 0; j < 5; j++) {
            mpz_set_si(x, i * 5 + j);
            cfe_mat_set(&m, x, i, j);
        }
    }

    mpz_set_si(x, mod);
    cfe_mat_mod(&res, &m, x);

    for (size_t i = 0; i < 5; i++) {
        for (size_t j = 0; j < 5; j++) {
            cfe_mat_get(x, &res, i, j);
            munit_assert(mpz_cmp_si(x, (i * 5 + j) % mod) == 0);
        }
    }

    mpz_clear(x);
    cfe_mat_frees(&m, &res, NULL);

    return MUNIT_OK;
}

MunitResult test_matrix_transpose(const MunitParameter params[], void *data) {
    cfe_mat m, m_t;
    cfe_mat_init(&m, 2, 3);
    cfe_mat_init(&m_t, 3, 2);

    mpz_t upper;
    mpz_init_set_ui(upper, 10);
    cfe_uniform_sample_mat(&m, upper);
    cfe_mat_transpose(&m_t, &m);

    mpz_t x, y;
    mpz_inits(x, y, NULL);
    for (size_t i = 0; i < m.rows; i++) {
        for (size_t j = 0; j < m.cols; j++) {
            cfe_mat_get(x, &m, i, j);
            cfe_mat_get(y, &m_t, j, i);
            munit_assert(mpz_cmp(x, y) == 0);
        }
    }

    mpz_clears(x, y, upper, NULL);
    cfe_mat_frees(&m, &m_t, NULL);

    return MUNIT_OK;
}

MunitResult
test_matrix_check_bound(const MunitParameter params[], void *data) {
    cfe_mat m;
    cfe_mat_init(&m, 5, 5);

    mpz_t x;
    mpz_init(x);

    for (size_t i = 0; i < 5; i++) {
        for (size_t j = 0; j < 5; j++) {
            mpz_set_si(x, i + j);
            cfe_mat_set(&m, x, i, j);
        }
    }

    mpz_set_si(x, 7);
    munit_assert_false(cfe_mat_check_bound(&m, x));

    mpz_set_si(x, 9);
    munit_assert_true(cfe_mat_check_bound(&m, x));

    mpz_clear(x);
    cfe_mat_free(&m);

    return MUNIT_OK;
}

MunitResult test_matrix_mul_vec(const MunitParameter params[], void *data) {
    mpz_t x;
    mpz_init(x);

    cfe_mat m;
    cfe_mat_init(&m, 2, 3);

    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < 3; j++) {
            mpz_set_si(x, i + j);
            cfe_mat_set(&m, x, i, j);
        }
    }

    cfe_vec v;
    cfe_vec_init(&v, 3);

    for (size_t i = 0; i < 3; i++) {
        mpz_set_si(x, i);
        cfe_vec_set(&v, x, i);
    }

    cfe_vec res;
    cfe_vec_init(&res, 2);

    cfe_mat_mul_vec(&res, &m, &v);

    cfe_vec_get(x, &res, 0);
    munit_assert(mpz_cmp_ui(x, 5) == 0);
    cfe_vec_get(x, &res, 1);
    munit_assert(mpz_cmp_ui(x, 8) == 0);

    mpz_clear(x);
    cfe_mat_free(&m);
    cfe_vec_frees(&v, &res, NULL);

    return MUNIT_OK;
}

MunitResult test_matrix_mul(const MunitParameter *params, void *data) {
    mpz_t x;
    mpz_init(x);

    cfe_mat m1;
    cfe_mat_init(&m1, 2, 3);

    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < 3; j++) {
            mpz_set_si(x, i + j);
            cfe_mat_set(&m1, x, i, j);
        }
    }

    cfe_mat m2;
    cfe_mat_init(&m2, 3, 2);

    for (size_t i = 0; i < 3; i++) {
        for (size_t j = 0; j < 2; j++) {
            mpz_set_si(x, i + j);
            cfe_mat_set(&m2, x, i, j);
        }
    }

    cfe_mat res;
    cfe_mat_init(&res, 2, 2);
    cfe_mat_mul(&res, &m1, &m2);

    cfe_mat_get(x, &res, 0, 0);
    munit_assert(mpz_cmp_ui(x, 5) == 0);
    cfe_mat_get(x, &res, 1, 0);
    munit_assert(mpz_cmp_ui(x, 8) == 0);
    cfe_mat_get(x, &res, 0, 1);
    munit_assert(mpz_cmp_ui(x, 8) == 0);
    cfe_mat_get(x, &res, 1, 1);
    munit_assert(mpz_cmp_ui(x, 14) == 0);

    mpz_clear(x);
    cfe_mat_frees(&m1, &m2, &res, NULL);

    return MUNIT_OK;
}


MunitResult test_matrix_dot(const MunitParameter params[], void *data) {
    mpz_t x, res;
    mpz_inits(x, res, NULL);

    cfe_mat m1, m2;
    cfe_mat_inits(5, 5, &m1, &m2, NULL);

    for (size_t i = 0; i < 5; i++) {
        for (size_t j = 0; j < 5; j++) {
            mpz_set_ui(x, i + j);
            cfe_mat_set(&m1, x, i, j);
            cfe_mat_set(&m2, x, i, j);
        }
    }

    cfe_mat_dot(res, &m1, &m2);
    munit_assert(mpz_cmp_ui(res, 500) == 0);

    mpz_clears(x, res, NULL);
    cfe_mat_frees(&m1, &m2, NULL);

    return MUNIT_OK;
}

MunitResult test_matrix_to_vec(const MunitParameter params[], void *data) {
    mpz_t upper;
    mpz_init_set_ui(upper, 10);

    cfe_mat m;
    cfe_mat_init(&m, 2, 3);
    cfe_uniform_sample_mat(&m, upper);

    cfe_vec v;
    cfe_vec_init(&v, m.rows * m.cols);

    cfe_mat_to_vec(&v, &m);

    mpz_t x, y;
    mpz_inits(x, y, NULL);
    for (size_t i = 0; i < m.rows; i++) {
        for (size_t j = 0; j < m.cols; j++) {
            cfe_mat_get(x, &m, i, j);
            cfe_vec_get(y, &v, m.cols * i + j);
            munit_assert(mpz_cmp(x, y) == 0);
        }
    }

    mpz_clears(upper, x, y, NULL);
    cfe_vec_free(&v);
    cfe_mat_free(&m);

    return MUNIT_OK;
}

MunitResult test_matrix_from_vec(const MunitParameter params[], void *data) {
    mpz_t upper;
    mpz_init_set_ui(upper, 10);

    cfe_vec v;
    cfe_vec_init(&v, 10);
    cfe_uniform_sample_vec(&v, upper);

    cfe_mat m;
    cfe_mat_init(&m, 2, 5);
    cfe_uniform_sample_mat(&m, upper);

    cfe_mat_from_vec(&m, &v);

    mpz_t x, y;
    mpz_inits(x, y, NULL);
    for (size_t i = 0; i < m.rows; i++) {
        for (size_t j = 0; j < m.cols; j++) {
            cfe_mat_get(x, &m, i, j);
            cfe_vec_get(y, &v, m.cols * i + j);
            munit_assert(mpz_cmp(x, y) == 0);
        }
    }

    mpz_clears(upper, x, y, NULL);
    cfe_vec_free(&v);
    cfe_mat_free(&m);

    return MUNIT_OK;
}

MunitTest matrix_tests[] = {
        {(char *) "/test-init-free",   test_matrix_init_free,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-inits-frees", test_matrix_inits_frees, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-get-set",     test_matrix_get_set,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-get-set-vec", test_matrix_get_set_vec, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-get-row",     test_matrix_get_row,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-get-row-ptr", test_matrix_get_row_ptr, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-get-col",     test_matrix_get_col,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-add",         test_matrix_add,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-mod",         test_matrix_mod,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-transpose",   test_matrix_transpose,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-check-bound", test_matrix_check_bound, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-mul",         test_matrix_mul,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-mul-vec",     test_matrix_mul_vec,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-dot",         test_matrix_dot,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-to-vec",      test_matrix_to_vec,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-from-vec",    test_matrix_from_vec,    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite matrix_suite = {
        (char *) "/matrix", matrix_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
