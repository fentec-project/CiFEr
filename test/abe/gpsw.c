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
    // that will later be used in the decryption policy of the message
    int gamma[] = {2, 1, 3, 4, 5, 6, 7};

    // encrypt the message
    cfe_gpsw_cipher cipher;
    cfe_gpsw_cipher_init(&cipher, 7);
    cfe_gpsw_encrypt(&cipher, &gpsw, &msg, gamma, 7, &pk);

    // create a msp struct out of a boolean expression  representing the
    // policy specifying which attributes are needed to decrypt the ciphertext
    char bool_exp[] = "(5 OR 3) AND ((2 OR 4) OR (1 AND 6))";
    size_t bool_exp_len = 36;  // length of the boolean expression string
    cfe_msp msp;
    cfe_error err = cfe_boolean_to_msp(&msp, bool_exp, bool_exp_len, true);
    munit_assert(err == CFE_ERR_NONE);

    // generate keys for decryption that correspond to provided msp struct,
    // i.e. a vector of keys, for each row in the msp matrix one key, having
    // the property that a subset of keys can decrypt a message iff the
    // corresponding rows span the vector of ones (which is equivalent to
    // corresponding attributes satisfy the boolean expression)
    cfe_vec_G1 policy_keys;
    cfe_gpsw_policy_keys_init(&policy_keys, &msp);
    cfe_gpsw_generate_policy_keys(&policy_keys, &gpsw, &msp, &sk);

    // produce a set of keys that are given to an entity with a set
    // of attributes in owned_attrib
    int owned_attrib[] = {1, 3, 6};
    cfe_gpsw_keys keys;
    cfe_gpsw_keys_init(&keys, &msp, owned_attrib, 3);
    cfe_gpsw_delegate_keys(&keys, &policy_keys, &msp, owned_attrib, 3);

    // decrypt the message with owned keys
    FP12_BN254 decrypted;
    cfe_error check = cfe_gpsw_decrypt(&decrypted, &cipher, &keys, &gpsw);
    munit_assert(check == CFE_ERR_NONE);

    // check if the decrypted message equals the starting message
    munit_assert(FP12_BN254_equals(&msg, &decrypted) == 1);

    // produce a insufficient set of keys that are given to an entity with
    // a set of attributes in insuff_attrib
    int insuff_attrib[] = {1, 2, 6};
    cfe_gpsw_keys insuff_keys;
    cfe_gpsw_keys_init(&insuff_keys, &msp, insuff_attrib, 3);
    cfe_gpsw_delegate_keys(&insuff_keys, &policy_keys, &msp, insuff_attrib, 3);

    // check if the decryption is denied
    check = cfe_gpsw_decrypt(&decrypted, &cipher, &insuff_keys, &gpsw);
    munit_assert(check == CFE_ERR_INSUFFICIENT_KEYS);

    // cleanup
    cfe_gpsw_free(&gpsw);
    cfe_vec_free(&sk);
    cfe_gpsw_pub_key_free(&pk);
    cfe_gpsw_cipher_free(&cipher);
    cfe_vec_G1_free(&policy_keys);
    cfe_gpsw_keys_free(&keys);
    cfe_gpsw_keys_free(&insuff_keys);
    cfe_msp_free(&msp);

    return MUNIT_OK;
}

MunitTest gpsw_tests[] = {
        {(char *) "/end-to-end", test_gpsw_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite gpsw_suite = {
        (char *) "/abe/gpsw", gpsw_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
