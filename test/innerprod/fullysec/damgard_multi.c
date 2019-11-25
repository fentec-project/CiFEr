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
#include "cifer/innerprod/fullysec/damgard_multi.h"
#include "cifer/sample/uniform.h"

MunitResult test_damgard_multi_end_to_end(const MunitParameter *params, void *data) {
    size_t l = 2;
    size_t num_clients = 6;
    mpz_t bound, xy_check, xy;
    mpz_inits(bound, xy_check, xy, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 10);

    // create a multi-client scheme
    cfe_damgard_multi m;
    cfe_error err;
    size_t modulus_len;
    const char *precomp = munit_parameters_get(params, "parameters");
    if (strcmp(precomp, "precomputed") == 0) {
        // modulus_len defines the security of the scheme, the higher the better
        modulus_len = 2048;
        err = cfe_damgard_multi_precomp_init(&m, num_clients, l, modulus_len, bound);
    } else if (strcmp(precomp, "random") == 0)  {
        modulus_len = 512;
        err = cfe_damgard_multi_init(&m, num_clients, l, modulus_len, bound);
    } else {
        err = CFE_ERR_INIT;
    }
    munit_assert(err == 0);


    cfe_mat x, y, mpk;
    cfe_mat_inits(num_clients, l, &x, &y, NULL);
    cfe_damgard_multi_sec_key msk;

    // generate secret an public key
    cfe_damgard_multi_master_keys_init(&mpk, &msk, &m);
    cfe_damgard_multi_generate_master_keys(&mpk, &msk, &m);

    // sample an inner product key and derive a corresponding
    // functional encryption key
    cfe_damgard_multi_fe_key fe_key;
    cfe_damgard_multi_fe_key_init(&fe_key, &m);
    cfe_uniform_sample_mat(&y, bound);
    err = cfe_damgard_multi_derive_fe_key(&fe_key, &m, &msk, &y);
    munit_assert(err == 0);

    // simulate clients so that each client encrypts its
    // vector x; collect encryptions in an array
    cfe_vec ciphertext[num_clients];

    cfe_damgard_multi_client clients[num_clients];
    for (size_t i = 0; i < num_clients; i++) {
        cfe_vec *x_vec = cfe_mat_get_row_ptr(&x, i);
        cfe_uniform_sample_vec(x_vec, bound);

        cfe_vec *pub_key = cfe_mat_get_row_ptr(&mpk, i);
        cfe_vec *otp = cfe_mat_get_row_ptr(&msk.otp, i);

        cfe_damgard_multi_client_init(&clients[i], &m);
        cfe_damgard_multi_ciphertext_init(&(ciphertext[i]), &clients[0]);

        err = cfe_damgard_multi_encrypt(&(ciphertext[i]), &clients[i], x_vec, pub_key, otp);
        munit_assert(err == 0);
    }

    // simulate decrytor that using FE key an an array of ciphertexts
    // decrypts a value Σ_i <x_i, y_i> (sum of inner products)
    cfe_damgard_multi decryptor;
    cfe_damgard_multi_copy(&decryptor, &m);
    err = cfe_damgard_multi_decrypt(xy, &m, ciphertext, &fe_key, &y);
    munit_assert(err == 0);

    // check correctness
    cfe_mat_dot(xy_check, &x, &y);
    munit_assert(mpz_cmp(xy, xy_check) == 0);

    // clear up
    mpz_clears(bound, xy_check, xy, NULL);
    cfe_mat_frees(&x, &y, &mpk, NULL);

    cfe_damgard_multi_sec_key_free(&msk);
    cfe_damgard_multi_fe_key_free(&fe_key);
    cfe_damgard_multi_free(&m);
    cfe_damgard_multi_free(&decryptor);
    for (size_t i = 0; i < num_clients; i++) {
        cfe_damgard_multi_client_free(&clients[i]);
        cfe_vec_free(&ciphertext[i]);
    }

    return MUNIT_OK;
}

char *damgard_multi_param[] = {
        (char *) "precomputed", (char *) "random", NULL
};

MunitParameterEnum damgard_multi_params[] = {
        {(char *) "parameters", damgard_multi_param},
        {NULL,                  NULL},
};

MunitTest damgard_multi_damgard_tests[] = {
        {(char *) "/end-to-end", test_damgard_multi_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, damgard_multi_params},
        {NULL,                   NULL,                          NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite damgard_multi_suite = {
        (char *) "/innerprod/fullysec/damgard-multi", damgard_multi_damgard_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

