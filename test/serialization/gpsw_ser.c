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
#include "cifer/abe/gpsw.h"
#include "cifer/serialization/gpsw_ser.h"

MunitResult test_gpsw_pub_key_ser(const MunitParameter *params, void *data) {
    cfe_ser buf;
    cfe_gpsw gpsw;
    cfe_gpsw_pub_key pk, pk2;
    cfe_vec sk;

    cfe_gpsw_init(&gpsw, 10);
    cfe_gpsw_master_keys_init(&pk, &sk, &gpsw);
    cfe_gpsw_generate_master_keys(&pk, &sk, &gpsw);

    cfe_gpsw_pub_key_ser(&pk, &buf);
    cfe_gpsw_pub_key_read(&pk2, &buf);

    munit_assert(FP12_BN254_equals(&pk.y, &pk2.y) == 1);
    munit_assert(pk.t.size == pk2.t.size);
    for (size_t i = 0; i < pk.t.size; i++) {
        int check = ECP2_BN254_equals(&pk.t.vec[i], &pk2.t.vec[i]);
        munit_assert(check == 1);
    }

    cfe_gpsw_free(&gpsw);
    cfe_vec_free(&sk);
    cfe_gpsw_pub_key_free(&pk);
    cfe_ser_free(&buf);

    return MUNIT_OK;
}

MunitResult test_gpsw_key_ser(const MunitParameter *params, void *data) {
    cfe_ser buf;
    cfe_gpsw gpsw;
    cfe_gpsw_pub_key pk;
    cfe_vec sk;

    cfe_gpsw_init(&gpsw, 10);
    cfe_gpsw_master_keys_init(&pk, &sk, &gpsw);
    cfe_gpsw_generate_master_keys(&pk, &sk, &gpsw);

    // define a boolean expression and make a corresponding msp structure
    char bool_exp[] = "(5 OR 3) AND ((2 OR 4) OR (1 AND 6))";
    size_t bool_exp_len = 36;  // length of the boolean expression string
    cfe_msp msp;
    cfe_error check = cfe_boolean_to_msp(&msp, bool_exp, bool_exp_len, true);
    munit_assert(check == CFE_ERR_NONE);

    cfe_gpsw_key policy_key;
    cfe_gpsw_key_init(&policy_key, &msp);
    cfe_gpsw_generate_policy_key(&policy_key, &gpsw, &msp, &sk);

    cfe_gpsw_key key1, key2;
    cfe_gpsw_key_init(&key1, &msp);

    cfe_gpsw_key_ser(&key1, &buf);
    cfe_gpsw_key_read(&key2, &buf);

    munit_assert(key1.msp.mat.rows == key2.msp.mat.rows);
    munit_assert(key1.msp.mat.cols == key2.msp.mat.cols);
    for (size_t i =0; i<key1.msp.mat.rows; i++) {
        munit_assert(key1.msp.row_to_attrib[i] == key2.msp.row_to_attrib[i]);
        for (size_t j =0; j < key1.msp.mat.cols; j++) {
            munit_assert(mpz_cmp(key1.msp.mat.mat[i].vec[j], key2.msp.mat.mat[i].vec[j]) == 0);
        }
    }
    munit_assert(key1.d.size == key2.d.size);
    for (size_t i = 0; i < key1.d.size; i++) {
        munit_assert(ECP_BN254_equals(&key1.d.vec[i], &key2.d.vec[i]) == 1);
    }

    cfe_gpsw_free(&gpsw);
    cfe_vec_free(&sk);
    cfe_gpsw_pub_key_free(&pk);
    cfe_ser_free(&buf);

    return MUNIT_OK;
}

MunitResult test_gpsw_cipher_ser(const MunitParameter *params, void *data) {
    cfe_ser buf;
    cfe_gpsw gpsw;
    cfe_gpsw_pub_key pk;
    cfe_vec sk;

    cfe_gpsw_init(&gpsw, 10);
    cfe_gpsw_master_keys_init(&pk, &sk, &gpsw);
    cfe_gpsw_generate_master_keys(&pk, &sk, &gpsw);

    FP12_BN254 msg;
    FP12_BN254_one(&msg);

    // define a set of attributes (a subset of the universe of attributes)
    // that will later be used in the decryption policy of the message
    int gamma[] = {2, 1, 3, 4, 5, 6, 7};

    // encrypt the message
    cfe_gpsw_cipher cipher, cipher2;
    cfe_gpsw_cipher_init(&cipher, 7);
    cfe_gpsw_encrypt(&cipher, &gpsw, &msg, gamma, 7, &pk);

    cfe_gpsw_cipher_ser(&cipher, &buf);
    cfe_gpsw_cipher_read(&cipher2, &buf);

    munit_assert(FP12_BN254_equals(&cipher.e0, &cipher2.e0) == 1);
    munit_assert(cipher.e.size == cipher2.e.size);
    for (size_t i = 0; i < cipher.e.size; i++) {
        munit_assert(cipher.gamma[i] == cipher2.gamma[i]);
        int check = ECP2_BN254_equals(&cipher.e.vec[i], &cipher2.e.vec[i]);
        munit_assert(check == 1);
    }

    cfe_gpsw_free(&gpsw);
    cfe_vec_free(&sk);
    cfe_gpsw_cipher_free(&cipher);
    cfe_gpsw_cipher_free(&cipher2);
    cfe_ser_free(&buf);

    return MUNIT_OK;
}


MunitTest gpsw_ser_tests[] = {
        {(char *) "/pub_key", test_gpsw_pub_key_ser, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/keys", test_gpsw_key_ser, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/cipher", test_gpsw_cipher_ser, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite gpsw_ser_suite = {
        (char *) "/ser/gpsw", gpsw_ser_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
