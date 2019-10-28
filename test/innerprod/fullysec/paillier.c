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
#include "cifer/data/vec.h"
#include "cifer/innerprod/fullysec/paillier.h"
#include "cifer/sample/uniform.h"

MunitResult test_paillier_end_to_end(const MunitParameter *params, void *data) {
    size_t l = 50;
    size_t lambda = 128;
    size_t bit_len = 512;
    mpz_t bound_x, bound_y, fe_key, xy_check, xy, bound_x_neg, bound_y_neg;
    mpz_inits(bound_x, bound_y, fe_key, xy_check, xy, bound_x_neg, bound_y_neg, NULL);
    mpz_set_ui(bound_x, 2);
    mpz_pow_ui(bound_x, bound_x, 10);
    mpz_set(bound_y, bound_x);
    mpz_neg(bound_x_neg, bound_x);
    mpz_add_ui(bound_x_neg, bound_x_neg, 1);
    mpz_neg(bound_y_neg, bound_y);
    mpz_add_ui(bound_y_neg, bound_y_neg, 1);

    cfe_paillier s, encryptor;
    cfe_error err = cfe_paillier_init(&s, l, lambda, bit_len, bound_x, bound_y);
    munit_assert(err == 0);

    cfe_vec msk, mpk, ciphertext, x, y;
    cfe_vec_inits(l, &x, &y, NULL);
    cfe_uniform_sample_range_vec(&x, bound_x_neg, bound_x);
    cfe_uniform_sample_range_vec(&y, bound_y_neg, bound_y);
    cfe_vec_dot(xy_check, &x, &y);

    cfe_paillier_master_keys_init(&msk, &mpk, &s);
    err = cfe_paillier_generate_master_keys(&msk, &mpk, &s);
    munit_assert(err == 0);

    err = cfe_paillier_derive_fe_key(fe_key, &s, &msk, &y);
    munit_assert(err == 0);

    cfe_paillier_copy(&encryptor, &s);
    cfe_paillier_ciphertext_init(&ciphertext, &encryptor);
    err = cfe_paillier_encrypt(&ciphertext, &encryptor, &x, &mpk);
    munit_assert(err == 0);

    err = cfe_paillier_decrypt(xy, &s, &ciphertext, fe_key, &y);
    munit_assert(err == 0);

    munit_assert(mpz_cmp(xy, xy_check) == 0);

    mpz_clears(bound_x, bound_y, fe_key, xy_check, xy, bound_x_neg, bound_y_neg, NULL);
    cfe_vec_frees(&x, &y, &msk, &mpk, &ciphertext, NULL);

    cfe_paillier_free(&s);
    cfe_paillier_free(&encryptor);

    return MUNIT_OK;
}

MunitTest simple_ip_paillier_tests[] = {
        {(char *) "/end-to-end", test_paillier_end_to_end, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                       NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite paillier_suite = {
        (char *) "/innerprod/fullysec/paillier", simple_ip_paillier_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

