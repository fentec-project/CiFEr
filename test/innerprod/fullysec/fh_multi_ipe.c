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
#include "cifer/innerprod/fullysec/fh_multi_ipe.h"
#include "cifer/sample/uniform.h"

MunitResult test_fh_multi_ipe_end_to_end(const MunitParameter *params, void *data) {
    // choose the parameters for the encryption and build the scheme
    size_t sec_level = 3;
    size_t num_clients = 10;
    size_t vec_len = 3;
    mpz_t bound, bound_neg, xy_check, xy;
    mpz_inits(bound, bound_neg, xy_check, xy, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 7);
    mpz_neg(bound_neg, bound);

    cfe_fh_multi_ipe fh_multi_ipe;
    cfe_error err= cfe_fh_multi_ipe_init(&fh_multi_ipe, sec_level, num_clients, vec_len, bound, bound);
    munit_assert(err == 0);

    // generate master key
    cfe_fh_multi_ipe_sec_key sec_key;
    cfe_fh_multi_ipe_master_key_init(&sec_key, &fh_multi_ipe);
    FP12_BN254 pub_key;
    err = cfe_fh_multi_ipe_generate_keys(&sec_key, &pub_key, &fh_multi_ipe);
    munit_assert(err == 0);

    // sample an inner product matrix
    cfe_mat y;
    cfe_mat_init(&y, num_clients, vec_len);
    cfe_uniform_sample_range_mat(&y, bound_neg, bound);

    // derive a functional key for vector y
    cfe_mat_G2 FE_key;
    cfe_fh_multi_ipe_fe_key_init(&FE_key, &fh_multi_ipe);
    err = cfe_fh_multi_ipe_derive_fe_key(&FE_key, &y, &sec_key, &fh_multi_ipe);
    munit_assert(err == 0);

    // we simulate the clients encrypting vectors; each client is given his
    // part of the secret key
    cfe_fh_multi_ipe clients[num_clients];
    cfe_vec x[num_clients];
    cfe_mat X_for_check;
    cfe_mat_init(&X_for_check, num_clients, vec_len);

    cfe_vec_G1 ciphers[num_clients];

    for (size_t i = 0; i < num_clients; i++) {
        cfe_fh_multi_ipe_copy(&clients[i], &fh_multi_ipe);

        cfe_vec_init(&(x[i]), vec_len);
        cfe_uniform_sample_vec(&x[i], bound);

        cfe_mat_set_vec(&X_for_check, &(x[i]), i);

        cfe_fh_multi_ipe_ciphertext_init(&ciphers[i], &clients[i]);
        err = cfe_fh_multi_ipe_encrypt(&ciphers[i], &x[i], &sec_key.B_hat[i], &clients[i]);
        munit_assert(err == 0);
    }


    // simulate a decryptor
    cfe_fh_multi_ipe decryptor;
    cfe_fh_multi_ipe_copy(&decryptor, &fh_multi_ipe);
    // decryptor decrypts the inner-product without knowing
    // vectors x and y
    err = cfe_fh_multi_ipe_decrypt(xy, ciphers, &FE_key, &pub_key, &decryptor);
    munit_assert(err == 0);

    // check correctness
    cfe_mat_dot(xy_check, &X_for_check, &y);
    munit_assert(mpz_cmp(xy, xy_check) == 0);
//
//    // clean up
//    mpz_clears(bound, bound_neg, xy_check, xy, NULL);
//    cfe_vec_frees(&x, &y, NULL);
    cfe_fh_multi_ipe_free(&fh_multi_ipe);
    cfe_fh_multi_ipe_free(&decryptor);
    cfe_fh_multi_ipe_master_key_free(&sec_key);
//    cfe_fh_multi_ipe_fe_key_free(&FE_key);
//    cfe_fh_multi_ipe_ciphertext_free(&cipher);

    return MUNIT_OK;
}

MunitTest fully_secure_fh_multi_ipe_tests[] = {
        {(char *) "/end-to-end", test_fh_multi_ipe_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite fh_multi_ipe_suite = {
        (char *) "/innerprod/fullysec/fh_multi_ipe", fully_secure_fh_multi_ipe_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

