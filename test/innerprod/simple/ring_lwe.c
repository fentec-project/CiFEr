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
#include "cifer/internal/keygen.h"
#include "cifer/innerprod/simple/ring_lwe.h"
#include "cifer/sample/uniform.h"


MunitResult test_ring_lwe(const MunitParameter *params, void *data) {
    // Length of data vectors x, y
    size_t l = 100;
    size_t n = 256;

    // message space size
    mpz_t B, B_neg;
    mpz_inits(B, B_neg, NULL);
    mpz_set_si(B, 1000000);
    mpz_neg(B_neg, B);

    // parameters for the sampling of small noise
    mpf_t sigma;
    mpf_init_set_ui(sigma, 20);

    // Create some mesage and vector for the product
    cfe_vec y;
    cfe_mat X;
    cfe_vec_init(&y, l);
    cfe_mat_init(&X, l, n);
    cfe_uniform_sample_range_vec(&y, B_neg, B);
    cfe_uniform_sample_range_mat(&X, B_neg, B);

    // TODO modify when code for generation of p, q is ready
    mpz_t p, q;
    mpz_init_set_str(p, "10000000000000000", 10);
    mpz_init_set_str(q, "903468688179973616387830299599", 10);

    cfe_vec expect, res;
    cfe_vec_init(&expect, n);
    // the correct result
    cfe_vec_mul_matrix(&expect, &y, &X);

    // initialize the scheme
    cfe_ring_lwe s;
    cfe_error err = cfe_ring_lwe_init(&s, l, n, B, p, q, sigma);
    munit_assert(!err);

    cfe_mat SK, PK; // secret and public keys
    cfe_ring_lwe_sec_key_init(&SK, &s);
    cfe_ring_lwe_generate_sec_key(&SK, &s);
    cfe_ring_lwe_pub_key_init(&PK, &s);
    cfe_ring_lwe_generate_pub_key(&PK, &s, &SK);

    cfe_vec sk_y;
    cfe_ring_lwe_fe_key_init(&sk_y, &s);
    err = cfe_ring_lwe_derive_key(&sk_y, &s, &SK, &y);
    munit_assert(!err);

    // encrypt the full mesage
    cfe_mat CT;
    cfe_ring_lwe_ciphertext_init(&CT, &s);
    err = cfe_ring_lwe_encrypt(&CT, &s, &X, &PK);
    munit_assert(!err);

    // decrypt the product y*X
    cfe_ring_lwe_decrypted_init(&res, &s);
    err = cfe_ring_lwe_decrypt(&res, &s, &CT, &sk_y, &y);
    munit_assert(!err);

    // check if the result is correct
    for (size_t i = 0; i < n; i++) {
        munit_assert(mpz_cmp(res.vec[i], expect.vec[i]) == 0);
    }

    cfe_mat_frees(&X, &CT, &SK, &PK, NULL);
    cfe_vec_frees(&y, &sk_y, &expect, &res, NULL);
    mpz_clears(B, B_neg, p, q, NULL);
    mpf_clear(sigma);
    cfe_ring_lwe_free(&s);

    return MUNIT_OK;
}

MunitTest ring_lwe_tests[] = {
        {(char *) "/end-to-end", test_ring_lwe, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite ring_lwe_suite = {
        (char *) "/innerprod/simple/ring-lwe", ring_lwe_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
