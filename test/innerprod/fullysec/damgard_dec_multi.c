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
#include "cifer/innerprod/fullysec/damgard_dec_multi.c"
#include "cifer/sample/uniform.h"

MunitResult test_damgard_dec_multi_end_to_end(const MunitParameter *params, void *data) {
    size_t num_clients = 1;
    size_t l = 1;
    size_t modulus_len = 64;
    mpz_t bound, bound_neg, key1, key2, xy_check, xy;
    mpz_inits(bound, bound_neg, key1, key2, xy_check, xy, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 10);
    mpz_neg(bound_neg, bound);

    // create a underlying multi-client scheme
    cfe_damgard_multi damgard_multi;
    cfe_error err = cfe_damgard_multi_init(&damgard_multi, num_clients, l, modulus_len, bound);
    munit_assert(err == 0);

    // create clients and make an array of their public keys
    cfe_damgard_dec_multi_client clients[num_clients];
    mpz_t pub_keys[num_clients];
    for (size_t i = 0; i < num_clients; i++) {
        cfe_damgard_dec_multi_client_init(&(clients[i]), &damgard_multi, i);
        mpz_init(pub_keys[i]);
        mpz_set(pub_keys[i], clients[i].client_pub_key);
    }

    // based on public values of each client create private shares summing to 0
    for (size_t i = 0; i < num_clients; i++) {
        cfe_damgard_dec_multi_client_set_share(&(clients[i]), pub_keys);
    }

    // independently each client creates its private key
    cfe_damgard_dec_multi_sec_key sec_keys[num_clients];
    for (size_t i = 0; i < num_clients; i++) {
        cfe_damgard_dec_multi_sec_key_init(&(sec_keys[i]), &(clients[i]));
        cfe_damgard_dec_multi_generate_keys(&(sec_keys[i]), &(clients[i]));
    }

    // now that the clients have  generated secret keys and agreed on shared secret
    // they can encrypt a vector in a decentralized way and create partial keys such
    // that only with all of them the decryption of the inner product is possible
    cfe_vec x[num_clients];
    cfe_mat X_for_check;
    cfe_mat_init(&X_for_check, num_clients, l);

    cfe_vec ciphers[num_clients];

    for (size_t i = 0; i < num_clients; i++) {
        cfe_vec_init(&(x[i]), l);
        cfe_uniform_sample_vec(&x[i], bound);
        cfe_vec_print(&x[i]);

        cfe_mat_set_vec(&X_for_check, &(x[i]), i);


        cfe_damgard_dec_multi_ciphertext_init(&ciphers[i], &clients[i]);
        cfe_damgard_dec_multi_encrypt(&ciphers[i], &x[i], &sec_keys[i], &clients[i]);
    }
    cfe_mat_print(&X_for_check);

    cfe_mat y;
    cfe_mat_init(&y, num_clients, l);
    cfe_uniform_sample_mat(&y, bound);
    cfe_mat_print(&y);


    cfe_damgard_dec_multi_derived_key_part derived_key_shares[num_clients];
    for (size_t i = 0; i < num_clients; i++) {
        cfe_damgard_dec_multi_derived_key_init(&derived_key_shares[i]);
        cfe_damgard_dec_multi_derive_key_share(&(derived_key_shares[i]), &y, &(sec_keys[i]), &(clients[i]));
    }

    cfe_damgard_dec_multi_dec decryptor;
    cfe_damgard_dec_multi_dec_init(&decryptor, &damgard_multi);

    err = cfe_damgard_dec_multi_decrypt(xy, ciphers, derived_key_shares, &y, &decryptor);
    munit_assert(err == 0);


    // check correctness
//    cfe_mat_print(&X_for_check);
    cfe_mat_dot(xy_check, &X_for_check, &y);
    munit_assert(mpz_cmp(xy, xy_check) == 0);

//    // free the memory
//    mpz_clears(bound, bound_neg, key1, key2, xy_check, xy, NULL);
//    for (size_t i = 0; i < num_clients; i++) {
//        cfe_dmcfe_client_free(&(clients[i]));
//        cfe_vec_G2_free(&(key_shares[i]));
//    }
//    cfe_vec_frees(&x, &y, NULL);

    return MUNIT_OK;
}

MunitTest fully_secure_damgard_dec_multi_tests[] = {
        {(char *) "/end-to-end", test_damgard_dec_multi_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite damgard_dec_multi_suite = {
        (char *) "/innerprod/fullysec/damgard_dec_multi", fully_secure_damgard_dec_multi_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
