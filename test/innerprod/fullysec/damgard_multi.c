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
    size_t modulus_len = 512;
    mpz_t bound, xy_check, xy;
    mpz_inits(bound, xy_check, xy, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 10);

    cfe_damgard_multi m;

    cfe_error err = cfe_damgard_multi_init(&m, num_clients, l, modulus_len, bound);
    munit_assert(err == 0);


    cfe_mat x, y, mpk;
    cfe_mat_inits(num_clients, l, &x, &y, NULL);
    cfe_damgard_multi_sec_key msk;

    cfe_damgard_multi_master_keys_init(&mpk, &msk, &m);
    cfe_damgard_multi_generate_master_keys(&mpk, &msk, &m);

    cfe_damgard_multi_fe_key fe_key;
    cfe_damgard_multi_fe_key_init(&fe_key, &m);
    cfe_uniform_sample_mat(&y, bound);
    err = cfe_damgard_multi_derive_fe_key(&fe_key, &m, &msk, &y);
    munit_assert(err == 0);

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

    cfe_damgard_multi decryptor;
    cfe_damgard_multi_copy(&decryptor, &m);
    err = cfe_damgard_multi_decrypt(xy, &m, ciphertext, &fe_key, &y);
    munit_assert(err == 0);

    cfe_mat_dot(xy_check, &x, &y);
    munit_assert(mpz_cmp(xy, xy_check) == 0);

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

MunitTest damgard_multi_damgard_tests[] = {
        {(char *) "/end-to-end", test_damgard_multi_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite damgard_multi_suite = {
        (char *) "/innerprod/fullysec/damgard-multi", damgard_multi_damgard_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

