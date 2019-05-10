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

#include "cifer/data/vec.h"
#include "cifer/sample/uniform.h"

MunitResult test_vector_init_free(const MunitParameter params[], void *data) {
    cfe_vec v;
    cfe_vec_init(&v, 5);

    munit_assert(v.size == 5);

    cfe_vec_free(&v);

    return MUNIT_OK;
}

MunitResult test_vector_inits_frees(const MunitParameter params[], void *data) {
    cfe_vec v1, v2, v3;
    cfe_vec_inits(5, &v1, &v2, &v3, NULL);

    munit_assert(v1.size == 5);
    munit_assert(v2.size == 5);
    munit_assert(v3.size == 5);

    mpz_t x, y;
    mpz_init_set_ui(x, 42);
    mpz_init(y);

    cfe_vec_set(&v1, x, 3);

    cfe_vec_get(y, &v2, 3);

    munit_assert(mpz_cmp(x, y) != 0);

    mpz_clears(x, y, NULL);
    cfe_vec_frees(&v1, &v2, &v3, NULL);

    return MUNIT_OK;
}

MunitResult test_vector_get_set(const MunitParameter params[], void *data) {
    cfe_vec v;
    cfe_vec_init(&v, 5);

    mpz_t x;
    mpz_init(x);
    for (size_t i = 0; i < 5; i++) {
        mpz_set_si(x, i);
        cfe_vec_set(&v, x, i);
    }

    for (size_t i = 0; i < 5; i++) {
        cfe_vec_get(x, &v, i);
        munit_assert(mpz_cmp_si(x, i) == 0);
    }

    mpz_clear(x);
    cfe_vec_free(&v);

    return MUNIT_OK;
}

MunitResult test_vector_add(const MunitParameter params[], void *data) {
    cfe_vec v1, v2, res;
    cfe_vec_inits(5, &v1, &v2, &res, NULL);

    mpz_t x;
    mpz_init(x);
    for (size_t i = 0; i < 5; i++) {
        mpz_set_si(x, i);
        cfe_vec_set(&v1, x, i);
        cfe_vec_set(&v2, x, 4 - i);
    }

    cfe_vec_add(&res, &v1, &v2);

    for (size_t i = 0; i < 5; i++) {
        cfe_vec_get(x, &res, i);
        munit_assert(mpz_cmp_si(x, 4) == 0);
    }

    mpz_clear(x);
    cfe_vec_frees(&v1, &v2, &res, NULL);

    return MUNIT_OK;
}

MunitResult test_vector_dot(const MunitParameter *params, void *data) {
    cfe_vec v1, v2;
    cfe_vec_inits(5, &v1, &v2, NULL);

    mpz_t x;
    mpz_init(x);
    for (size_t i = 0; i < 5; i++) {
        mpz_set_si(x, i);
        cfe_vec_set(&v1, x, i);
        cfe_vec_set(&v2, x, 4 - i);
    }

    cfe_vec_dot(x, &v1, &v2);

    munit_assert(mpz_cmp_si(x, 10) == 0);

    mpz_clear(x);
    cfe_vec_frees(&v1, &v2, NULL);

    return MUNIT_OK;
}

MunitResult test_vector_mod(const MunitParameter params[], void *data) {
    cfe_vec v, res;
    cfe_vec_inits(5, &v, &res, NULL);
    size_t mod = 3;

    mpz_t x;
    mpz_init(x);
    for (size_t i = 0; i < 5; i++) {
        mpz_set_si(x, i);
        cfe_vec_set(&v, x, i);
    }

    mpz_set_si(x, mod);
    cfe_vec_mod(&res, &v, x);

    for (size_t i = 0; i < 5; i++) {
        cfe_vec_get(x, &res, i);
        munit_assert(mpz_cmp_si(x, i % mod) == 0);
    }

    mpz_clear(x);
    cfe_vec_frees(&v, &res, NULL);

    return MUNIT_OK;
}

MunitResult test_vector_check_bound(const MunitParameter params[], void *data) {
    cfe_vec v;
    cfe_vec_init(&v, 5);

    mpz_t x;
    mpz_init(x);
    for (size_t i = 0; i < 5; i++) {
        mpz_set_si(x, i);
        cfe_vec_set(&v, x, i);
    }

    mpz_set_si(x, 4);
    munit_assert_false(cfe_vec_check_bound(&v, x));

    mpz_set_si(x, 5);
    munit_assert_true(cfe_vec_check_bound(&v, x));

    mpz_clear(x);
    cfe_vec_free(&v);

    return MUNIT_OK;
}

MunitResult test_vector_append(const MunitParameter params[], void *data) {
    cfe_vec v1, v2;
    cfe_vec_init(&v1, 5);
    cfe_vec_init(&v2, v1.size + 1);

    mpz_t last;
    mpz_init_set_ui(last, 10);

    cfe_vec_append(&v2, &v1, last);

    mpz_t x, y;
    mpz_inits(x, y, NULL);
    for (size_t i = 0; i < v1.size; i++) {
        cfe_vec_get(x, &v1, i);
        cfe_vec_get(y, &v2, i);
        munit_assert(mpz_cmp(x, y) == 0);
    }
    cfe_vec_get(y, &v2, v1.size);
    munit_assert(mpz_cmp(y, last) == 0);

    mpz_clears(x, y, last, NULL);
    cfe_vec_frees(&v1, &v2, NULL);

    return MUNIT_OK;
}

MunitResult test_vector_join(const MunitParameter params[], void *data) {
    cfe_vec v1, v2, v3;
    cfe_vec_init(&v1, 5);
    cfe_vec_init(&v2, 3);
    cfe_vec_init(&v3, v1.size + v2.size);

    mpz_t upper;
    mpz_init_set_ui(upper, 10);
    cfe_uniform_sample_vec(&v1, upper);
    cfe_uniform_sample_vec(&v2, upper);

    cfe_vec_join(&v3, &v1, &v2);

    mpz_t x, y;
    mpz_inits(x, y, NULL);
    for (size_t i = 0; i < v1.size; i++) {
        cfe_vec_get(x, &v3, i);
        cfe_vec_get(y, &v1, i);
        munit_assert(mpz_cmp(x, y) == 0);
    }
    for (size_t i = 0; i < v2.size; i++) {
        cfe_vec_get(x, &v3, v1.size + i);
        cfe_vec_get(y, &v2, i);
        munit_assert(mpz_cmp(x, y) == 0);
    }

    mpz_clears(upper, x, y, NULL);
    cfe_vec_frees(&v1, &v2, &v3, NULL);

    return MUNIT_OK;
}

MunitResult test_vector_extract(const MunitParameter params[], void *data) {
    cfe_vec v, extracted;
    cfe_vec_init(&v, 10);
    cfe_vec_init(&extracted, 5);

    mpz_t upper;
    mpz_init_set_ui(upper, 10);
    cfe_uniform_sample_vec(&v, upper);

    size_t start = 2;
    size_t n = 5;
    cfe_vec_extract(&extracted, &v, start, n);

    mpz_t x, y;
    mpz_inits(x, y, NULL);
    for (size_t i = 0; i < n; i++) {
        cfe_vec_get(x, &extracted, i);
        cfe_vec_get(y, &v, start + i);
        munit_assert(mpz_cmp(x, y) == 0);
    }

    mpz_clears(upper, x, y, NULL);
    cfe_vec_frees(&v, &extracted, NULL);

    return MUNIT_OK;
}

MunitResult test_vector_mul_matrix(const MunitParameter params[], void *data) {
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
    cfe_vec_init(&v, 2);

    for (size_t i = 0; i < 2; i++) {
        mpz_set_si(x, i);
        cfe_vec_set(&v, x, i);
    }

    cfe_vec res;
    cfe_vec_init(&res, 3);

    cfe_vec_mul_matrix(&res, &v, &m);

    cfe_vec_get(x, &res, 0);
    munit_assert(mpz_cmp_ui(x, 1) == 0);
    cfe_vec_get(x, &res, 1);
    munit_assert(mpz_cmp_ui(x, 2) == 0);
    cfe_vec_get(x, &res, 2);
    munit_assert(mpz_cmp_ui(x, 3) == 0);

    mpz_clear(x);
    cfe_mat_free(&m);
    cfe_vec_frees(&v, &res, NULL);

    return MUNIT_OK;
}

MunitResult test_vector_poly_mul(const MunitParameter params[], void *data) {
    cfe_vec v1, v2, res;
    cfe_vec_inits(3, &v1, &v2, &res, NULL);

    mpz_t x;
    mpz_init(x);
    for (size_t i = 0; i < 3; i++) {
        mpz_set_si(x, i);
        cfe_vec_set(&v1, x, i);
        cfe_vec_set(&v2, x, 2 - i);
    }

    cfe_vec_poly_mul(&res, &v1, &v2);


    cfe_vec_get(x, &res, 0);
    munit_assert(mpz_cmp_si(x, -2) == 0);
    cfe_vec_get(x, &res, 1);
    munit_assert(mpz_cmp_si(x, 2) == 0);
    cfe_vec_get(x, &res, 2);
    munit_assert(mpz_cmp_si(x, 5) == 0);

    mpz_clear(x);
    cfe_vec_frees(&v1, &v2, &res, NULL);

    return MUNIT_OK;
}

MunitResult test_vector_FFT(const MunitParameter params[], void *data) {
    cfe_vec y, a;
    cfe_vec_inits(4, &y, &a, NULL);

    mpz_t x, q, root;
    mpz_init(x);

    // first test
    mpz_init_set_si(q, 13);
    for (size_t i = 0; i < 4; i++) {
        mpz_set_si(x, i);
        cfe_vec_set(&a, x, i);
    }
    mpz_init_set_si(root, 5);

    cfe_vec_FFT(&y, &a, root, q);

    cfe_vec_get(x, &y, 0);
    munit_assert(mpz_cmp_si(x, 6) == 0);
    cfe_vec_get(x, &y, 1);
    munit_assert(mpz_cmp_si(x, 1) == 0);
    cfe_vec_get(x, &y, 2);
    munit_assert(mpz_cmp_si(x, 11) == 0);
    cfe_vec_get(x, &y, 3);
    munit_assert(mpz_cmp_si(x, 8) == 0);

    // second test
    cfe_vec z;
    cfe_vec_init(&z, 8);
    mpz_set_si(q, 17);
    for (size_t i = 0; i < 4; i++) {
        mpz_set_si(x, i);
        cfe_vec_set(&a, x, i);
    }
    mpz_set_si(root, 2);

    cfe_vec_FFT(&z, &a, root, q);

    cfe_vec_get(x, &z, 0);
    munit_assert(mpz_cmp_si(x, 6) == 0);
    cfe_vec_get(x, &z, 1);
    munit_assert(mpz_cmp_si(x, 0) == 0);
    cfe_vec_get(x, &z, 4);
    munit_assert(mpz_cmp_si(x, 15) == 0);
    cfe_vec_get(x, &z, 5);
    munit_assert(mpz_cmp_si(x, 16) == 0);

    mpz_clears(x, q, root, NULL);
    cfe_vec_frees(&y, &a, &z, NULL);

    return MUNIT_OK;
}

MunitResult test_vector_poly_mul_FFT(const MunitParameter params[], void *data) {
    cfe_vec v1, v2, res;
    cfe_vec_inits(4, &v1, &v2, &res, NULL);

    mpz_t q, root, inv_root, inv_2n, x;
    mpz_init_set_si(q, 17);
    mpz_init_set_si(root, 2);
    mpz_init_set_si(inv_root, 9);
    mpz_init_set_si(inv_2n, 15);

    mpz_init(x);
    for (size_t i = 0; i < 4; i++) {
        mpz_set_si(x, i);
        cfe_vec_set(&v1, x, i);
        cfe_vec_set(&v2, x, 3 - i);
    }

    cfe_vec_poly_mul_FFT(&res, &v1, &v2, root, inv_root, inv_2n, q);

    cfe_vec_get(x, &res, 0);
    munit_assert(mpz_cmp_si(x, 9) == 0);
    cfe_vec_get(x, &res, 1);
    munit_assert(mpz_cmp_si(x, 0) == 0);
    cfe_vec_get(x, &res, 2);
    munit_assert(mpz_cmp_si(x, 8) == 0);
    cfe_vec_get(x, &res, 3);
    munit_assert(mpz_cmp_si(x, 14) == 0);

    // speed test
    mpz_set_si(q, 19686913);
    mpz_set_si(root, 4723);
    mpz_set_si(inv_root, 17540235);
    mpz_set_si(inv_2n, 19648462);

    cfe_vec u1, u2, re1, re2;
    cfe_vec_inits(256, &u1, &u2, &re1, &re2, NULL);
    for (size_t i = 0; i < 100; i++) {
        cfe_uniform_sample_vec(&u1, q);
        cfe_uniform_sample_vec(&u2, q);
        cfe_vec_poly_mul_FFT(&re1, &u1, &u2, root, inv_root, inv_2n, q);
    }

    mpz_clears(x, q, root, inv_root, inv_2n, NULL);
    cfe_vec_frees(&v1, &v2, &res, &u1, &u2, &re1, &re2, NULL);

    return MUNIT_OK;
}


MunitTest vector_tests[] = {
        {(char *) "/test-init-free",           test_vector_init_free,    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-inits-frees",         test_vector_inits_frees,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-get-set",             test_vector_get_set,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-append",              test_vector_append,       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-join",                test_vector_join,         NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-extract",             test_vector_extract,      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-add",                 test_vector_add,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-dot",                 test_vector_dot,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-mod",                 test_vector_mod,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-check-bound",         test_vector_check_bound,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-vector-mul-matrix",   test_vector_mul_matrix,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-vector-poly-mul",     test_vector_poly_mul,     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-vector-FFT",          test_vector_FFT,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/test-vector-poly-mul-FFT", test_vector_poly_mul_FFT, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                                     NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite vector_suite = {
        (char *) "/vector", vector_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
