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

#include <amcl/fp12_BN254.h>
#include <cifer/internal/big.h>
#include <amcl/ecp_BN254.h>
#include <amcl/ecp2_BN254.h>
#include <amcl/pair_BN254.h>
#include <cifer/internal/common.h>
#include "cifer/internal/keygen.h"
#include "cifer/sample/uniform.h"
#include "cifer/test.h"
#include "cifer/internal/dlog.h"

typedef struct dlog_params {
    mpz_t h; // public key
    mpz_t g; // generator
    mpz_t p; // modulus
    mpz_t x; // private key
    mpz_t q; // order of g
} dlog_params;

typedef struct dlog_BN254_params {
    FP12_BN254 h; // value
    FP12_BN254 g; // generator
    mpz_t bound; // bound for exponent
    mpz_t x; // exponent
    mpz_t q; // order of g
} dlog_BN254_params;

void fixed_dlog_params_small(dlog_params *dp) {
    mpz_inits(dp->h, dp->g, dp->p, dp->x, dp->q, NULL);
    mpz_set_ui(dp->h, 188);
    mpz_set_ui(dp->g, 212);
    mpz_set_ui(dp->p, 227);
    mpz_set_ui(dp->x, 101);
    mpz_set_ui(dp->q, 113);
}

void random_dlog_params(dlog_params *dp, size_t modulus_len) {
    mpz_inits(dp->h, dp->g, dp->p, dp->x, dp->q, NULL);
    cfe_elgamal key;
    cfe_elgamal_init(&key, modulus_len);

    mpz_set(dp->g, key.g);
    mpz_set(dp->p, key.p);
    mpz_set(dp->q, key.q);

    cfe_uniform_sample(dp->x, dp->q);
    mpz_powm(dp->h, dp->g, dp->x, dp->p);

    cfe_elgamal_free(&key);
}

MunitResult test_baby_step_giant_step_fixed(const MunitParameter params[], void *data) {
    dlog_params dp;
    fixed_dlog_params_small(&dp);

    mpz_t res;
    mpz_init(res);

    cfe_error err = cfe_baby_giant(res, dp.h, dp.g, dp.p, dp.q, NULL);

    munit_assert(err == 0);
    munit_assert(mpz_cmp(res, dp.x) == 0);

    mpz_clears(dp.h, dp.g, dp.p, dp.x, dp.q, res, NULL);
    return MUNIT_OK;
}

MunitResult test_baby_step_giant_step_bounded(const MunitParameter params[], void *data) {
    dlog_params dp;
    random_dlog_params(&dp, 128);
    mpz_t res, bound;
    mpz_inits(res, bound, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 20);

    cfe_uniform_sample(dp.x, bound);
    mpz_powm(dp.h, dp.g, dp.x, dp.p);

    cfe_error err = cfe_baby_giant(res, dp.h, dp.g, dp.p, dp.q, bound);

    munit_assert(err == 0);
    munit_assert(mpz_cmp(res, dp.x) == 0);

    mpz_clears(dp.h, dp.g, dp.p, dp.x, dp.q, res, bound, NULL);
    return MUNIT_OK;
}

MunitResult test_baby_step_giant_step(const MunitParameter params[], void *data) {
    dlog_params dp;
    random_dlog_params(&dp, 32);

    mpz_t res;
    mpz_init(res);

    cfe_error err = cfe_baby_giant(res, dp.h, dp.g, dp.p, dp.q, NULL);

    munit_assert(err == 0);
    munit_assert(mpz_cmp(res, dp.x) == 0);

    mpz_clears(dp.h, dp.g, dp.p, dp.x, dp.q, res, NULL);
    return MUNIT_OK;
}

MunitResult test_baby_step_giant_step_with_neg(const MunitParameter params[], void *data) {
    dlog_params dp;
    random_dlog_params(&dp, 32);
    mpz_neg(dp.x, dp.x);
    mpz_invert(dp.h, dp.h, dp.p);
    mpz_t res, bound, bound_neg;
    mpz_inits(res, bound, bound_neg, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 15);
    mpz_neg(bound_neg, bound);

    cfe_uniform_sample_range(dp.x, bound_neg, bound);
    mpz_powm(dp.h, dp.g, dp.x, dp.p);

    cfe_error err = cfe_baby_giant_with_neg(res, dp.h, dp.g, dp.p, dp.q, bound);

    munit_assert(err == 0);
    munit_assert(mpz_cmp(res, dp.x) == 0);

    mpz_clears(dp.h, dp.g, dp.p, dp.x, dp.q, res, bound, bound_neg, NULL);
    return MUNIT_OK;
}

MunitResult test_pollard_rho_fixed(const MunitParameter params[], void *data) {
    dlog_params dp;
    fixed_dlog_params_small(&dp);

    mpz_t res;
    mpz_init(res);

    cfe_error err = cfe_pollard_rho(res, dp.h, dp.g, dp.p, dp.q);

    munit_assert(err == 0);
    munit_assert(mpz_cmp(res, dp.x) == 0);

    mpz_clears(dp.h, dp.g, dp.p, dp.x, dp.q, res, NULL);
    return MUNIT_OK;
}

MunitResult test_pollard_rho(const MunitParameter params[], void *data) {
    dlog_params dp;
    random_dlog_params(&dp, 32);

    mpz_t res;
    mpz_init(res);

    cfe_error err = cfe_pollard_rho(res, dp.h, dp.g, dp.p, dp.q);

    munit_assert(err == 0);
    munit_assert(mpz_cmp(res, dp.x) == 0);

    mpz_clears(dp.h, dp.g, dp.p, dp.x, dp.q, res, NULL);
    return MUNIT_OK;
}

void random_dlog_BN254_params(dlog_BN254_params *dp) {
    mpz_inits(dp->x, dp->q, dp->bound, NULL);
    mpz_from_BIG_256_56(dp->q, (int64_t *) CURVE_Order_BN254);
    ECP_BN254 g1;
    ECP2_BN254 g2;
    ECP_BN254_generator(&g1);
    ECP2_BN254_generator(&g2);
    PAIR_BN254_ate(&(dp->g), &g2, &g1);
    PAIR_BN254_fexp(&(dp->g));

    mpz_set_ui(dp->bound, 113);
    cfe_uniform_sample(dp->x, dp->bound);
    BIG_256_56 x_b;
    BIG_256_56_from_mpz(x_b, dp->x);
    FP12_BN254_pow(&(dp->h), &(dp->g), x_b);
}

MunitResult test_baby_step_giant_step_BN254(const MunitParameter params[], void *data) {
    dlog_BN254_params dp;
    random_dlog_BN254_params(&dp);

    mpz_t res;
    mpz_init(res);

    cfe_error err = cfe_baby_giant_BN256(res, &dp.h, &dp.g, dp.q, dp.bound);

    munit_assert(err == 0);
    munit_assert(mpz_cmp(res, dp.x) == 0);

    mpz_clears(dp.x, dp.q, res, NULL);
    return MUNIT_OK;
}


MunitTest dlog_tests[] = {
        {(char *) "/baby-giant-fixed",    test_baby_step_giant_step_fixed,    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/baby-giant-bounded",  test_baby_step_giant_step_bounded,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/baby-giant",          test_baby_step_giant_step,          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/baby-giant-with-neg", test_baby_step_giant_step_with_neg, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/pollard-rho-fixed",   test_pollard_rho_fixed,             NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/pollard-rho",         test_pollard_rho,                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/baby-giant-BN254",    test_baby_step_giant_step_BN254,    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                                          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite dlog_suite = {
        (char *) "/dlog", dlog_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
