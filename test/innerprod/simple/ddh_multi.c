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
#include "cifer/innerprod/simple/ddh_multi.h"
#include "cifer/sample/uniform.h"

MunitResult test_ddh_multi_end_to_end(const MunitParameter *params, void *data) {
    size_t l = 3;
    size_t slots = 2;

    mpz_t bound, xy_check, xy;
    mpz_inits(bound, xy_check, xy, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 10);

    cfe_ddh_multi m, decryptor;
    cfe_ddh_multi_enc encryptors[slots];

    cfe_error err;

    size_t modulus_len;
    const char *precomp = munit_parameters_get(params, "parameters");
    if (strcmp(precomp, "precomputed") == 0) {
        // modulus_len defines the security of the scheme, the higher the better
        modulus_len = 2048;
        err = cfe_ddh_multi_precomp_init(&m, slots, l, modulus_len, bound);
    } else {
        modulus_len = 512;
        err = cfe_ddh_multi_init(&m, slots, l, modulus_len, bound);
    }
    munit_assert(err == 0);

    cfe_mat x, y, ciphertext;
    cfe_mat_inits(slots, l, &x, &y, NULL);
    cfe_mat_init(&ciphertext, slots, l + 1);
    cfe_uniform_sample_mat(&y, bound);

    cfe_mat mpk;
    cfe_ddh_multi_sec_key msk;
    cfe_ddh_multi_fe_key fe_key;

    cfe_ddh_multi_master_keys_init(&mpk, &msk, &m);
    cfe_ddh_multi_generate_master_keys(&mpk, &msk, &m);

    for (size_t i = 0; i < slots; i++) {
        cfe_ddh_multi_enc_init(&encryptors[i], &m);
    }

    cfe_ddh_multi_fe_key_init(&fe_key, &m);
    err = cfe_ddh_multi_derive_fe_key(&fe_key, &m, &msk, &y);
    munit_assert(err == 0);

    for (size_t i = 0; i < slots; i++) {
        cfe_vec *x_vec = cfe_mat_get_row_ptr(&x, i);
        cfe_uniform_sample_vec(x_vec, bound);

        cfe_vec ct;
        cfe_vec *pub_key = cfe_mat_get_row_ptr(&mpk, i);
        cfe_vec *otp = cfe_mat_get_row_ptr(&msk.otp_key, i);
        cfe_ddh_multi_ciphertext_init(&ct, &encryptors[i]);
        err = cfe_ddh_multi_encrypt(&ct, &encryptors[i], x_vec, pub_key, otp);
        munit_assert(err == 0);

        cfe_mat_set_vec(&ciphertext, &ct, i);
        cfe_vec_free(&ct);
    }

    cfe_mat_dot(xy_check, &x, &y);
    mpz_mod(xy_check, xy_check, bound);

    cfe_ddh_multi_copy(&decryptor, &m);
    err = cfe_ddh_multi_decrypt(xy, &decryptor, &ciphertext, &fe_key, &y);
    munit_assert(err == 0);

    munit_assert(mpz_cmp(xy, xy_check) == 0);

    mpz_clears(bound, xy_check, xy, NULL);
    cfe_mat_frees(&x, &y, &ciphertext, &mpk, NULL);
    cfe_ddh_multi_sec_key_free(&msk);
    cfe_ddh_multi_fe_key_free(&fe_key);
    cfe_ddh_multi_free(&m);
    cfe_ddh_multi_free(&decryptor);
    for (size_t i = 0; i < slots; i++) {
        cfe_ddh_multi_enc_free(&encryptors[i]);
    }

    return MUNIT_OK;
}

char *ddh_multi_param[] = {
        (char *) "precomputed", (char *) "random", NULL
};

MunitParameterEnum ddh_multi_params[] = {
        {(char *) "parameters", ddh_multi_param},
        {NULL,                  NULL},
};

MunitTest ddh_multi_simple_tests[] = {
        {(char *) "/end-to-end", test_ddh_multi_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, ddh_multi_params},
        {NULL, NULL,                                        NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite ddh_multi_suite = {
        (char *) "/innerprod/simple/ddh-multi", ddh_multi_simple_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
