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
    size_t l = 3;
    size_t slots = 2;
    size_t modulus_len = 64;
    mpz_t bound, xy_check, xy;
    mpz_inits(bound, xy_check, xy, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 10);

    cfe_damgard_multi m, decryptor;
    cfe_damgard_multi_enc encryptors[slots];
    cfe_error err = cfe_damgard_multi_init(&m, slots, l, modulus_len, bound);
    munit_assert(err == 0);

    cfe_mat x, y, ciphertext, mpk;
    cfe_mat_inits(slots, l, &x, &y, NULL);
    cfe_mat_init(&ciphertext, slots, l + 2);
    cfe_uniform_sample_mat(&y, bound);

    cfe_damgard_multi_sec_key msk;
    cfe_damgard_multi_master_keys_init(&mpk, &msk, &m);
    cfe_damgard_multi_generate_master_keys(&mpk, &msk, &m);

    for (size_t i = 0; i < slots; i++) {
        cfe_damgard_multi_enc_init(&encryptors[i], &m);
    }

    cfe_damgard_multi_fe_key key;
    cfe_damgard_multi_fe_key_init(&key, &m);
    err = cfe_damgard_multi_derive_key(&key, &m, &msk, &y);
    munit_assert(err == 0);

    cfe_vec ct;
    cfe_damgard_multi_ciphertext_init(&ct, &encryptors[0]);
    for (size_t i = 0; i < slots; i++) {
        cfe_vec *x_vec = cfe_mat_get_row_ptr(&x, i);
        cfe_uniform_sample_vec(x_vec, bound);

        cfe_vec *pub_key = cfe_mat_get_row_ptr(&mpk, i);
        cfe_vec *otp = cfe_mat_get_row_ptr(&msk.otp, i);
        err = cfe_damgard_multi_encrypt(&ct, &encryptors[i], x_vec, pub_key, otp);
        munit_assert(err == 0);

        cfe_mat_set_vec(&ciphertext, &ct, i);
    }
    cfe_vec_free(&ct);

    cfe_mat_dot(xy_check, &x, &y);
    mpz_mod(xy_check, xy_check, bound);

    cfe_damgard_multi_copy(&decryptor, &m);
    err = cfe_damgard_multi_decrypt(xy, &m, &ciphertext, &key, &y);
    munit_assert(err == 0);

    munit_assert(mpz_cmp(xy, xy_check) == 0);

    mpz_clears(bound, xy_check, xy, NULL);
    cfe_mat_frees(&x, &y, &ciphertext, &mpk, NULL);

    cfe_damgard_multi_sec_key_free(&msk);
    cfe_damgard_multi_fe_key_free(&key);
    cfe_damgard_multi_free(&m);
    cfe_damgard_multi_free(&decryptor);
    for (size_t i = 0; i < slots; i++) {
        cfe_damgard_multi_enc_free(&encryptors[i]);
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

