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
#include "cifer/innerprod/simple/ddh.h"
#include "cifer/sample/uniform.h"

MunitResult test_ddh_end_to_end(const MunitParameter *params, void *data) {
    size_t l = 3;
    size_t modulus_len = 128;

    mpz_t bound, bound_neg, func_key, xy_check, xy;
    mpz_inits(bound, bound_neg, func_key, xy_check, xy, NULL);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 15);
    mpz_neg(bound_neg, bound);

    cfe_ddh s, encryptor, decryptor;
    cfe_error err = cfe_ddh_init(&s, l, modulus_len, bound);
    munit_assert(err == 0);

    cfe_vec msk, mpk, ciphertext, x, y;
    cfe_vec_inits(l, &x, &y, NULL);
    cfe_uniform_sample_range_vec(&x, bound_neg, bound);
    cfe_uniform_sample_range_vec(&y, bound_neg, bound);
    cfe_vec_dot(xy_check, &x, &y);

    cfe_ddh_master_keys_init(&msk, &mpk, &s);
    cfe_ddh_generate_master_keys(&msk, &mpk, &s);

    err = cfe_ddh_derive_key(func_key, &s, &msk, &y);
    munit_assert(err == 0);

    cfe_ddh_copy(&encryptor, &s);
    cfe_ddh_ciphertext_init(&ciphertext, &encryptor);
    err = cfe_ddh_encrypt(&ciphertext, &encryptor, &x, &mpk);
    munit_assert(err == 0);

    cfe_ddh_copy(&decryptor, &s);
    err = cfe_ddh_decrypt(xy, &decryptor, &ciphertext, func_key, &y);
    munit_assert(err == 0);

    munit_assert(mpz_cmp(xy, xy_check) == 0);

    mpz_clears(bound, bound_neg, func_key, xy_check, xy, NULL);
    cfe_vec_frees(&x, &y, &msk, &mpk, &ciphertext, NULL);

    cfe_ddh_free(&s);
    cfe_ddh_free(&encryptor);
    cfe_ddh_free(&decryptor);

    return MUNIT_OK;
}

MunitTest simple_ip_tests[] = {
        {(char *) "/end-to-end", test_ddh_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite ddh_suite = {
        (char *) "/innerprod/simple/ddh", simple_ip_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

