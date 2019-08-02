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

#include "cifer/test.h"
#include "cifer/innerprod/fullysec/dmcfe.h"
#include "cifer/sample/uniform.h"

MunitResult test_dmcfe_end_to_end(const MunitParameter *params, void *data) {
    size_t num_clients = 10;
    mpz_t bound, bound_neg, key1, key2, xy_check, xy;
    mpz_inits(bound, bound_neg, key1, key2, xy_check, xy, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 10);
    mpz_neg(bound_neg, bound);

    cfe_dmcfe_client clients[num_clients];
    ECP_BN254 pub_keys[num_clients];
    for (size_t i = 0; i < num_clients; i++) {
        cfe_dmcfe_client_init(&(clients[i]), i);
        pub_keys[i] = clients[i].client_pub_key;
    }

    for (size_t i = 0; i < num_clients; i++) {
        cfe_dmcfe_set_share(&(clients[i]), pub_keys, num_clients);
    }

    cfe_vec x, y;
    cfe_vec_inits(num_clients, &x, &y, NULL);
    cfe_uniform_sample_vec(&x, bound);
    cfe_uniform_sample_vec(&y, bound);
    char label[] = "some label";
    ECP_BN254 ciphers[num_clients];
    cfe_vec_G2 key_shares[num_clients];

    for (size_t i = 0; i < num_clients; i++) {
        cfe_dmcfe_encrypt(&(ciphers[i]), x.vec[i], label, &(clients[i]));
        cfe_vec_G2_init(&(key_shares[i]), 2);
        cfe_dmcfe_generate_key_share(&(key_shares[i]), &y, &(clients[i]));
    }

    cfe_dmcfe_decrypt(xy, ciphers, key_shares, label, &y, bound);

    cfe_vec_dot(xy_check, &x, &y);
    munit_assert(mpz_cmp(xy, xy_check) == 0);

    mpz_clears(bound, bound_neg, key1, key2, xy_check, xy, NULL);
    for (size_t i = 0; i < num_clients; i++) {
        cfe_dmcfe_client_free(&(clients[i]));
        cfe_vec_G2_free(&(key_shares[i]));
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

