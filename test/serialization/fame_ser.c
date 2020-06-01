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

#include <cifer/internal/common.h>
#include <amcl/pair_BN254.h>
#include "cifer/test.h"
#include "cifer/abe/fame.h"
#include "cifer/serialization/fame_ser.h"
#include "cifer/serialization/data_ser.h"

MunitResult test_fame_ser(const MunitParameter *params, void *data) {
    // create FAME structs
    cfe_fame fame;
    cfe_fame_init(&fame);
    cfe_fame_pub_key pk;
    cfe_fame_sec_key sk;
    cfe_fame_sec_key_init(&sk);
    cfe_fame_generate_master_keys(&pk, &sk, &fame);
    FP12_BN254 msg;
    FP12_BN254_one(&msg);
    char bool_exp[] = "(5 OR 3) AND ((2 OR 4) OR (1 AND 6))";
    size_t bool_exp_len = 36;  // length of the boolean expression string
    cfe_msp msp;
    cfe_error err = cfe_boolean_to_msp(&msp, bool_exp, bool_exp_len, false);
    munit_assert(err == CFE_ERR_NONE);
    cfe_fame_cipher cipher;
    cfe_fame_cipher_init(&cipher, &msp);
    cfe_fame_encrypt(&cipher, &msg, &msp, &pk, &fame);
    int owned_attrib[] = {1, 3, 6};
    cfe_fame_attrib_keys keys;
    cfe_fame_attrib_keys_init(&keys, 3); // the number of attributes needs to be specified
    cfe_fame_generate_attrib_keys(&keys, owned_attrib, 3, &sk, &fame);
    FP12_BN254 decryption;
    err = cfe_fame_decrypt(&decryption, &cipher, &keys, &fame);
    munit_assert(err == CFE_ERR_NONE);
    munit_assert(FP12_BN254_equals(&msg, &decryption) == 1);

    // serialize structs, i.e. change them in a sequence of bytes and back to struct
    cfe_ser buf;
    // serialize pub key
    cfe_fame_pub_key pk2;
    cfe_fame_pub_key_ser(&pk, &buf);
    err = cfe_fame_pub_key_read(&pk2, &buf);
    munit_assert(err == CFE_ERR_NONE);
    // check if we get the same struct after serialization and reading
    int check = ECP2_BN254_equals(&pk.part_G2[0], &pk2.part_G2[0]);
    munit_assert(check == 1);
    check = ECP2_BN254_equals(&pk.part_G2[1], &pk2.part_G2[1]);
    munit_assert(check == 1);
    check = FP12_BN254_equals(&pk.part_GT[0], &pk2.part_GT[0]);
    munit_assert(check == 1);
    check = FP12_BN254_equals(&pk.part_GT[1], &pk2.part_GT[1]);
    munit_assert(check == 1);
    cfe_ser_free(&buf);

    // serialize pub key
    cfe_fame_sec_key sk2;
    cfe_fame_sec_key_ser(&sk, &buf);
    err = cfe_fame_sec_key_read(&sk2, &buf);
    munit_assert(err == CFE_ERR_NONE);
    // check if we get the same struct after serialization and reading
    for (int i =0; i<3; i++) {
        check = ECP_BN254_equals(&(sk.part_G1[i]), &(sk2.part_G1[i]));
        munit_assert(check == 1);
    }
    for (int i =0; i<4; i++) {
        munit_assert(mpz_cmp(sk.part_int[i], sk2.part_int[i]) == 0);
    }
    cfe_ser_free(&buf);

    // serialize attrib keys
    cfe_fame_attrib_keys keys2;
    cfe_fame_attrib_keys_ser(&keys, &buf);
    err = cfe_fame_attrib_keys_read(&keys2, &buf);
    munit_assert(err == CFE_ERR_NONE);
    // check if we get the same struct after serialization and reading
    for (int i =0; i<3; i++) {
        check = ECP2_BN254_equals(&(keys.k0[i]), &(keys2.k0[i]));
        munit_assert(check == 1);
    }
    for (int i =0; i<3; i++) {
        for (size_t j =0; j < keys.num_attrib; j++) {
            check = ECP_BN254_equals(&(keys.k[j][i]), &(keys2.k[j][i]));
            munit_assert(check == 1);
        }
    }
    for (int i =0; i<3; i++) {
        check = ECP_BN254_equals(&(keys.k_prime[i]), &(keys2.k_prime[i]));
        munit_assert(check == 1);
    }
    for (size_t j =0; j < keys.num_attrib; j++) {
        munit_assert(keys.row_to_attrib[j] == keys2.row_to_attrib[j]);
    }
    munit_assert(keys.num_attrib == keys2.num_attrib);
    cfe_ser_free(&buf);

    // serialize ciphertext
    cfe_fame_cipher cipher2;
    cfe_fame_cipher_ser(&cipher, &buf);
    err = cfe_fame_cipher_read(&cipher2, &buf);
    munit_assert(err == CFE_ERR_NONE);
    // check if we get the same struct after serialization and reading
    for (int i =0; i<3; i++) {
        check = ECP2_BN254_equals(&(cipher.ct0[i]), &(cipher2.ct0[i]));
        munit_assert(check == 1);
    }
    for (int i =0; i<3; i++) {
        for (size_t j =0; j < keys.num_attrib; j++) {
            check = ECP_BN254_equals(&(cipher.ct[j][i]), &(cipher2.ct[j][i]));
            munit_assert(check == 1);
        }
    }
    check = FP12_BN254_equals(&(cipher.ct_prime), &(cipher2.ct_prime));
    munit_assert(check == 1);
    cfe_ser_free(&buf);

    // use new structs in FAME scheme to make further tests
    cfe_fame_cipher cipher3;
    cfe_fame_encrypt(&cipher, &msg, &msp, &pk2, &fame);
    cfe_fame_cipher_ser(&cipher, &buf);
    err = cfe_fame_cipher_read(&cipher3, &buf);
    munit_assert(err == CFE_ERR_NONE);
    cfe_ser_free(&buf);

    cfe_fame_generate_attrib_keys(&keys, owned_attrib, 3, &sk2, &fame);
    cfe_fame_attrib_keys keys3;
    cfe_fame_attrib_keys_ser(&keys, &buf);
    err = cfe_fame_attrib_keys_read(&keys3, &buf);
    munit_assert(err == CFE_ERR_NONE);
    cfe_ser_free(&buf);

    err = cfe_fame_decrypt(&decryption, &cipher3, &keys3, &fame);
    munit_assert(err == CFE_ERR_NONE);
    munit_assert(FP12_BN254_equals(&msg, &decryption) == 1);

    // clear up
    cfe_fame_free(&fame);
    cfe_fame_sec_key_free(&sk);
    cfe_fame_sec_key_free(&sk2);
    cfe_fame_cipher_free(&cipher);
    cfe_fame_cipher_free(&cipher2);
    cfe_fame_cipher_free(&cipher3);
    cfe_fame_attrib_keys_free(&keys);
    cfe_fame_attrib_keys_free(&keys2);
    cfe_fame_attrib_keys_free(&keys3);
    cfe_msp_free(&msp);
    return MUNIT_OK;
}

MunitTest fame_ser_tests[] = {
        {(char *) "/all", test_fame_ser, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite fame_ser_suite = {
        (char *) "/ser/fame", fame_ser_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
