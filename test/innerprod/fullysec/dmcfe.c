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
#include "cifer/innerprod/fullysec/dmcfe.h"
#include "cifer/sample/uniform.h"

MunitResult test_dmcfe_end_to_end(const MunitParameter *params, void *data) {
    size_t num_clients = 10;
    mpz_t bound, bound_neg, xy_check, xy;
    mpz_inits(bound, bound_neg, xy_check, xy, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 10);
    mpz_neg(bound_neg, bound);

    // create clients and make an array of their public keys
    cfe_dmcfe_client clients[num_clients];
    ECP_BN254 pub_keys[num_clients];
    for (size_t i = 0; i < num_clients; i++) {
        cfe_dmcfe_client_init(&(clients[i]), i);
        pub_keys[i] = clients[i].client_pub_key;
    }

    // based on public values of each client create private matrices T_i summing to 0
    for (size_t i = 0; i < num_clients; i++) {
        cfe_dmcfe_set_share(&(clients[i]), pub_keys, num_clients);
    }

    // now that the clients have agreed on secret keys they can encrypt a vector in
    // a decentralized way and create partial keys such that only with all of them
    // the decryption of the inner product is possible
    cfe_vec x, y;
    cfe_vec_inits(num_clients, &x, &y, NULL);
    cfe_uniform_sample_vec(&x, bound);
    cfe_uniform_sample_range_vec(&y, bound_neg, bound);
    char label[] = "some label";
    size_t label_len = 10; // length of the label string
    ECP_BN254 ciphers[num_clients];
    cfe_vec_G2 fe_key[num_clients];

    for (size_t i = 0; i < num_clients; i++) {
        cfe_dmcfe_encrypt(&(ciphers[i]), &(clients[i]), x.vec[i], label, label_len);
        cfe_dmcfe_fe_key_part_init(&(fe_key[i]));
        cfe_dmcfe_derive_fe_key_part(&(fe_key[i]), &(clients[i]), &y);
    }

    // decrypt the inner product with the corresponding label
    cfe_error err = cfe_dmcfe_decrypt(xy, ciphers, fe_key, label, label_len, &y, bound);
    munit_assert(err == CFE_ERR_NONE);

    // check correctness
    cfe_vec_dot(xy_check, &x, &y);
    munit_assert(mpz_cmp(xy, xy_check) == 0);

    // free the memory
    mpz_clears(bound, bound_neg, xy_check, xy, NULL);
    for (size_t i = 0; i < num_clients; i++) {
        cfe_dmcfe_client_free(&(clients[i]));
        cfe_vec_G2_free(&(fe_key[i]));
    }
    cfe_vec_frees(&x, &y, NULL);

    return MUNIT_OK;
}

MunitTest fully_secure_dmcfe_tests[] = {
        {(char *) "/end-to-end", test_dmcfe_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                      NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite dmcfe_suite = {
        (char *) "/innerprod/fullysec/dmcfe", fully_secure_dmcfe_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

