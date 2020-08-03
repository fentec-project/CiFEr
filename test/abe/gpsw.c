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

#include <gmp.h>
#include <amcl/ecp_BN254.h>
#include <amcl/pair_BN254.h>

#include "cifer/internal/common.h"
#include "cifer/test.h"
#include "cifer/abe/gpsw.h"

MunitResult test_gpsw_end_to_end(const MunitParameter *params, void *data) {
    // create a new GPSW struct with the universe of l possible
    // attributes (attributes are denoted by the integers in [0, l)
    cfe_gpsw gpsw;
    cfe_gpsw_init(&gpsw, 10);

    // generate a public key and a secret key for the scheme
    cfe_gpsw_pub_key pk;
    cfe_vec sk;
    cfe_gpsw_master_keys_init(&pk, &sk, &gpsw);
    cfe_gpsw_generate_master_keys(&pk, &sk, &gpsw);

    // create a message to be encrypted
    FP12_BN254 msg;
    FP12_BN254_one(&msg);

    // define a set of attributes (a subset of the universe of attributes)
    // that will be associated to the ciphertext
    int gamma[] = {3, 1, 6, 2};

    // encrypt the message
    cfe_gpsw_cipher cipher;
    cfe_gpsw_cipher_init(&cipher, 4);
    cfe_gpsw_encrypt(&cipher, &gpsw, &msg, gamma, 4, &pk);

    // create a msp struct out of a boolean expression  representing the
    // policy specifying which attributes are needed to decrypt the ciphertext
    char bool_exp[] = "(5 OR 3) AND ((2 OR 4) OR (1 AND 6))";
    size_t bool_exp_len = 36;  // length of the boolean expression string
    cfe_msp msp;
    cfe_error err = cfe_boolean_to_msp(&msp, bool_exp, bool_exp_len, true);
    munit_assert(err == CFE_ERR_NONE);

    // generate keys for decryption that correspond to provided msp struct,
    // which is equivalent the boolean expression
    cfe_gpsw_key key;
    cfe_gpsw_key_init(&key, &msp);
    cfe_gpsw_generate_policy_key(&key, &gpsw, &msp, &sk);

    // decrypt the message with policy key, which is only possible if the
    // attributes associated to the ciphertext satisfy the policy associated
    // to the key
    FP12_BN254 decrypted;
    cfe_error check = cfe_gpsw_decrypt(&decrypted, &cipher, &key, &gpsw);
    munit_assert(check == CFE_ERR_NONE);

    // check if the decrypted message equals the starting message
    munit_assert(FP12_BN254_equals(&msg, &decrypted) == 1);

    // produce a policy that is the attributes of the ciphertext do not
    // satisfy
    char insuff_bool_exp[] = "(5 AND 3) AND ((2 OR 4) OR (1 AND 6))";
    cfe_msp insuff_msp;
    err = cfe_boolean_to_msp(&insuff_msp, insuff_bool_exp, 37, true);
    munit_assert(err == CFE_ERR_NONE);
    cfe_gpsw_key insuff_key;
    cfe_gpsw_key_init(&insuff_key, &insuff_msp);
    cfe_gpsw_generate_policy_key(&insuff_key, &gpsw, &insuff_msp, &sk);

    // check if the decryption is denied
    check = cfe_gpsw_decrypt(&decrypted, &cipher, &insuff_key, &gpsw);
    munit_assert(check == CFE_ERR_INSUFFICIENT_KEYS);

    // cleanup
    cfe_gpsw_free(&gpsw);
    cfe_vec_free(&sk);
    cfe_gpsw_pub_key_free(&pk);
    cfe_gpsw_cipher_free(&cipher);
    cfe_gpsw_key_free(&key);
    cfe_gpsw_key_free(&insuff_key);
    cfe_msp_free(&msp);
    cfe_msp_free(&insuff_msp);

    return MUNIT_OK;
}

MunitTest gpsw_tests[] = {
        {(char *) "/end-to-end", test_gpsw_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite gpsw_suite = {
        (char *) "/abe/gpsw", gpsw_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
