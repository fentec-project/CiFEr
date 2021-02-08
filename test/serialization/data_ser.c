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

#include <amcl/pair_BN254.h>
#include <cifer/sample/uniform.h>
#include "cifer/test.h"
#include "cifer/serialization/data_ser.h"

MunitResult test_ec_ser(const MunitParameter *params, void *data) {
    cfe_ser buf;
    ECP_BN254 gen, out;
    ECP_BN254_generator(&gen);
    cfe_ECP_BN254_ser(&gen, &buf);
    cfe_error err = cfe_ECP_BN254_read(&out, &buf);
    int check =ECP_BN254_equals(&gen, &out);
    munit_assert(err == CFE_ERR_NONE);
    munit_assert(check == 1);
    cfe_ser_free(&buf);

    ECP2_BN254 gen2, out2;
    ECP2_BN254_generator(&gen2);
    cfe_ECP2_BN254_ser(&gen2, &buf);
    err = cfe_ECP2_BN254_read(&out2, &buf);
    check =ECP2_BN254_equals(&gen2, &out2);
    munit_assert(err == CFE_ERR_NONE);
    munit_assert(check == 1);
    cfe_ser_free(&buf);

    FP12_BN254 gen3, out3;
    PAIR_BN254_ate(&gen3, &gen2, &gen);
    PAIR_BN254_fexp(&gen3);
    cfe_FP12_BN254_ser(&gen3, &buf);
    err = cfe_FP12_BN254_read(&out3, &buf);
    check =FP12_BN254_equals(&gen3, &out3);
    munit_assert(err == CFE_ERR_NONE);
    munit_assert(check == 1);
    cfe_ser_free(&buf);

    return MUNIT_OK;
}

MunitResult test_mpz_ser(const MunitParameter *params, void *data) {
    cfe_ser buf;
    mpz_t a, b;
    mpz_inits(a, b, NULL);
    mpz_set_str(a, "-12341234212431111111111111111111111111111", 10);
    cfe_mpz_ser(a, &buf);
    cfe_mpz_read(b, &buf);
    munit_assert(mpz_cmp(a, b) == 0);
    cfe_ser_free(&buf);

    mpz_clears(a, b, NULL);

    return MUNIT_OK;
}

MunitResult test_mat_ser(const MunitParameter *params, void *data) {
    cfe_ser buf;
    mpz_t a;
    mpz_init(a);
    mpz_set_str(a, "11", 10);

    cfe_mat m, m2;
    cfe_mat_init(&m, 2, 3);
    cfe_uniform_sample_mat(&m, a);

    cfe_mat_ser(&m, &buf);
    cfe_mat_read(&m2, &buf);

    for (size_t i =0; i<m.rows; i++) {
        for (size_t j =0; j < m.cols; j++) {
            munit_assert(mpz_cmp(m.mat[i].vec[j], m2.mat[i].vec[j]) == 0);
        }
    }

    cfe_ser_free(&buf);
    cfe_mat_free(&m);
    cfe_mat_free(&m2);
    mpz_clear(a);

    return MUNIT_OK;
}

MunitResult test_msp_ser(const MunitParameter *params, void *data) {
    cfe_ser buf;

    // define a boolean expression and make a corresponding msp structure
    char bool_exp[] = "(5 OR 3) AND ((2 OR 4) OR (1 AND 6))";
    size_t bool_exp_len = 36;  // length of the boolean expression string
    cfe_msp msp, msp2;
    cfe_error check = cfe_boolean_to_msp(&msp, bool_exp, bool_exp_len, true);
    munit_assert(check == CFE_ERR_NONE);

    cfe_msp_ser(&msp, &buf);
    cfe_msp_read(&msp2, &buf);

    for (size_t i =0; i<msp.mat.rows; i++) {
        munit_assert(msp.row_to_attrib[i] == msp2.row_to_attrib[i]);
        for (size_t j =0; j < msp.mat.cols; j++) {
            munit_assert(mpz_cmp(msp.mat.mat[i].vec[j], msp2.mat.mat[i].vec[j]) == 0);
        }
    }

    cfe_ser_free(&buf);
    cfe_msp_free(&msp);
    cfe_msp_free(&msp2);

    return MUNIT_OK;
}

MunitResult test_vec_octet_ser(const MunitParameter *params, void *data) {
    cfe_ser buf;
    mpz_t a;
    mpz_init(a);
    mpz_set_str(a, "1100", 10);

    cfe_vec exponents;
    cfe_vec_init(&exponents, 5);
    cfe_uniform_sample_vec(&exponents, a);

    cfe_vec_G2 vec, vec2;
    cfe_vec_G2_init(&vec, 5);

    cfe_vec_mul_G2(&vec, &exponents);

    cfe_vec_ECP2_BN254_ser(&vec, &buf);
    cfe_vec_ECP2_BN254_read(&vec2, &buf);

    for (size_t i =0; i<vec.size; i++) {
        munit_assert(ECP2_BN254_equals(&vec.vec[i], &vec2.vec[i]) == 1);
    }

    cfe_ser_free(&buf);
    cfe_vec_G2_free(&vec);
    cfe_vec_G2_free(&vec2);
    cfe_vec_free(&exponents);
    mpz_clear(a);

    return MUNIT_OK;
}

MunitTest data_ser_tests[] = {
        {(char *) "/ec", test_ec_ser, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/mpz", test_mpz_ser, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/mat", test_mat_ser, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/msp", test_msp_ser, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/ec_vec", test_vec_octet_ser, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite data_ser_suite = {
        (char *) "/ser/data", data_ser_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
