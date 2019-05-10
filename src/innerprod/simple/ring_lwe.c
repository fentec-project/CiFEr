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

#include "cifer/innerprod/simple/ring_lwe.h"

#include "cifer/sample/uniform.h"

// This version of the scheme provides a speedup in comparison to
// ordinary LWE scheme. Another difference is that the encypted measage
// is now a matrix X of size l*n and the decryption is a vector whose value
// is y*X for a vector y of length l. This is a generalization of encrypting vector
// x of length l and decrypting <y, x> since vector x can be presented as
// a column of X (other values being zero).


// Calculates the center function t(x) = floor(x*q/p) % q for a matrix x
// componentwise
void ring_lwe_center(cfe_ring_lwe *s, cfe_mat *T, cfe_mat *X) {
    mpz_t t_i, x_i;
    mpz_inits(t_i, x_i, NULL);

    for (size_t i = 0; i < T->rows; i++) {
        for (size_t j = 0; j < T->cols; j++) {
            cfe_mat_get(x_i, X, i, j);
            mpz_mul(t_i, x_i, s->q);
            mpz_fdiv_q(t_i, t_i, s->p);
            mpz_mod(t_i, t_i, s->q);
            cfe_mat_set(T, t_i, i, j);
        }
    }

    mpz_clears(t_i, x_i, NULL);
}

// Initializes scheme struct with the desired confifuration
// and configures public parameters for the scheme.
cfe_error cfe_ring_lwe_init(cfe_ring_lwe *s, size_t l, size_t n, mpz_t bound, mpz_t p, mpz_t q, mpf_t sigma) {
    // Ensure that p >= 2 * l * B² holds
    mpz_t b_squared, two_l_times_b_squared;
    mpz_inits(b_squared, two_l_times_b_squared, NULL);
    mpz_pow_ui(b_squared, bound, 2);
    mpz_mul_ui(two_l_times_b_squared, b_squared, l * 2);

    bool cond = mpz_cmp(p, two_l_times_b_squared) < 0;
    mpz_clears(b_squared, two_l_times_b_squared, NULL);
    if (cond) {
        return CFE_ERR_PRECONDITION_FAILED;
    }

    // n has to be a power of 2
    if ((n & (n - 1)) != 0) {
        return CFE_ERR_PRECONDITION_FAILED;
    }

    s->l = l;
    s->n = n;
    mpz_init_set(s->bound, bound);

    // TODO p, q should be generated within this function?
    // TODO currently we merely set the value passed to it
    mpz_init_set(s->p, p);
    mpz_init_set(s->q, q);

    cfe_vec_init(&s->a, s->n);
    cfe_uniform_sample_vec(&s->a, s->q);

    cfe_normal_cumulative_init(&s->sampler, sigma, n, true);

    return CFE_ERR_NONE;
}

void cfe_ring_lwe_sec_key_init(cfe_mat *SK, cfe_ring_lwe *s) {
    cfe_mat_init(SK, s->l, s->n);
}

// Generates a secret key for the scheme.
// The key is represented by a matrix with dimensions l*n whose
// elements are small values sampled as discrete Gaussian.
void cfe_ring_lwe_generate_sec_key(cfe_mat *SK, cfe_ring_lwe *s) {
    cfe_normal_cumulative_sample_mat(SK, &s->sampler);
}

void cfe_ring_lwe_pub_key_init(cfe_mat *PK, cfe_ring_lwe *s) {
    cfe_mat_init(PK, s->l, s->n);
}

// Generates a public key PK for the scheme.
// Public key is a matrix of l*n elements.
cfe_error cfe_ring_lwe_generate_pub_key(cfe_mat *PK, cfe_ring_lwe *s, cfe_mat *SK) {
    if (SK->rows != s->l || SK->cols != s->n) {
        return CFE_ERR_MALFORMED_SEC_KEY;
    }

    // Initialize and fill noise matrix E with l*n samples
    cfe_mat E;
    cfe_mat_init(&E, s->l, s->n);
    cfe_normal_cumulative_sample_mat(&E, &s->sampler);

    // Calculate public key row by row as PK_i = (a * SK_i + E_i) % q
    // where operations of multiplication and addition are in the ring of
    // polynomials
    cfe_vec pk_i, sk_i, e_i;
    cfe_vec_inits(s->n, &pk_i, &sk_i, &e_i, NULL);

    for (size_t i = 0; i < s->l; i++) {
        cfe_mat_get_row(&sk_i, SK, i);
        cfe_vec_poly_mul(&pk_i, &sk_i, &s->a);
        cfe_mat_get_row(&e_i, &E, i);
        cfe_vec_add(&pk_i, &pk_i, &e_i);
        cfe_mat_set_vec(PK, &pk_i, i);
    }
    cfe_mat_mod(PK, PK, s->q);

    cfe_vec_frees(&pk_i, &sk_i, &e_i, NULL);
    cfe_mat_free(&E);
    return CFE_ERR_NONE;
}

void cfe_ring_lwe_fe_key_init(cfe_vec *sk_y, cfe_ring_lwe *s) {
    cfe_vec_init(sk_y, s->n);
}

// Derives a secret key sk_y for decryption of inner product of y and
// a secret operand.
// Secret key is a linear combination of input vector y and master secret keys.
cfe_error cfe_ring_lwe_derive_key(cfe_vec *sk_y, cfe_ring_lwe *s, cfe_mat *SK, cfe_vec *y) {
    if (!cfe_vec_check_bound(y, s->bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }
    if (SK->rows != s->l || SK->cols != s->n) {
        return CFE_ERR_MALFORMED_SEC_KEY;
    }

    cfe_vec_mul_matrix(sk_y, y, SK);
    cfe_vec_mod(sk_y, sk_y, s->q);
    return CFE_ERR_NONE;
}

void cfe_ring_lwe_ciphertext_init(cfe_mat *CT, cfe_ring_lwe *s) {
    cfe_mat_init(CT, s->l + 1, s->n);
}

// Encrypts matrix X using public key PK.
// The resulting ciphertext is stored in a matrix CT of
// size (l + 1) * n
cfe_error cfe_ring_lwe_encrypt(cfe_mat *CT, cfe_ring_lwe *s, cfe_mat *X, cfe_mat *PK) {
    if (!cfe_mat_check_bound(X, s->bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }
    if (PK->rows != s->l || PK->cols != s->n) {
        return CFE_ERR_MALFORMED_PUB_KEY;
    }
    if (X->rows != s->l || X->cols != s->n) {
        return CFE_ERR_MALFORMED_INPUT;
    }
    // Create a random small vector as the randomness for the encryption
    cfe_vec r;
    cfe_vec_init(&r, s->n);
    cfe_normal_cumulative_sample_vec(&r, &s->sampler);

    // create noise to secure the encryption
    cfe_mat E;
    cfe_mat_init(&E, s->l, s->n);
    cfe_normal_cumulative_sample_mat(&E, &s->sampler);

    // Calculate ciphertext row by row as CT_i = (PK_i * r + E_i) % q,
    //  where operations of multiplication and addition are in the ring of
    // polynomials
    // The last row of CT is set at the end
    cfe_vec v_ct, v_e;
    cfe_vec_inits(s->n, &v_ct, &v_e, NULL);

    for (size_t i = 0; i < s->l; i++) {
        cfe_vec *v_pk = cfe_mat_get_row_ptr(PK, i);
        cfe_vec_poly_mul(&v_ct, v_pk, &r);
        cfe_mat_get_row(&v_e, &E, i);
        cfe_vec_add(&v_ct, &v_ct, &v_e);
        cfe_mat_set_vec(CT, &v_ct, i);
    }
    cfe_vec_free(&v_e);

    cfe_mat_mod(CT, CT, s->q);

    // include the message in the encryption
    cfe_mat T;
    cfe_mat_init(&T, s->l, s->n);
    ring_lwe_center(s, &T, X);

    // do not use cfe_mat_add as the dimensions do not match
    // CT has one more row than T
    // we do this to save memory and time by preventing copying
    for (size_t i = 0; i < s->l; i++) {
        cfe_vec *ct_i = cfe_mat_get_row_ptr(CT, i);
        cfe_vec *t_i = cfe_mat_get_row_ptr(&T, i);
        cfe_vec_add(ct_i, ct_i, t_i);
    }

    cfe_mat_mod(CT, CT, s->q);

    // A vector comprising the last row of the cipher
    cfe_vec CT_last, e;
    cfe_vec_init(&CT_last, s->n);
    cfe_vec_poly_mul(&CT_last, &(s->a), &r);

    // create the last part of the encryption, needed for the decryption
    cfe_vec_init(&e, s->n);
    cfe_normal_cumulative_sample_vec(&e, &s->sampler);

    cfe_vec_add(&CT_last, &CT_last, &e);
    cfe_vec_mod(&CT_last, &CT_last, s->q);

    // Construct the final ciphertext matrix by joining both parts
    cfe_mat_set_vec(CT, &CT_last, s->l);

    // Cleanup
    cfe_vec_frees(&CT_last, &e, &r, &v_ct, NULL);
    cfe_mat_frees(&T, &E, NULL);

    return CFE_ERR_NONE;
}

void cfe_ring_lwe_decrypted_init(cfe_vec *res, cfe_ring_lwe *s) {
    cfe_vec_init(res, s->n);
}

// Decrypts the ciphertext CT.
// res will hold the decrypted product y*X
// sk_y is the derived secret key for decryption of y*X
// y is plaintext input vector
cfe_error cfe_ring_lwe_decrypt(cfe_vec *res, cfe_ring_lwe *s, cfe_mat *CT, cfe_vec *sk_y, cfe_vec *y) {
    if (!cfe_vec_check_bound(y, s->bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }
    if (sk_y->size != s->n) {
        return CFE_ERR_MALFORMED_FE_KEY;
    }
    if (y->size != s->l) {
        return CFE_ERR_MALFORMED_INPUT;
    }
    if (CT->rows != s->l + 1 || CT->cols != s->n) {
        return CFE_ERR_MALFORMED_CIPHER;
    }

    // Break down the ciphertext vector into
    // CT_first which holds the matrix of the cipher, and
    // CT_last  which holds the vector beeing the last row of the cipher
    cfe_mat CT_first;
    cfe_vec CT_last;
    cfe_mat_init(&CT_first, s->l, s->n);
    cfe_vec_init(&CT_last, s->n);

    for (size_t i = 0; i < s->l; i++) {
        cfe_vec *ct_i = cfe_mat_get_row_ptr(CT, i);
        cfe_mat_set_vec(&CT_first, ct_i, i);
    }
    cfe_mat_get_row(&CT_last, CT, s->l);

    // decrypt the centered value of y*X
    cfe_vec ct_prod;
    cfe_vec_init(&ct_prod, s->n);
    cfe_vec_mul_matrix(&ct_prod, y, &CT_first);
    cfe_vec_mod(&ct_prod, &ct_prod, s->q);
    cfe_vec_poly_mul(res, &CT_last, sk_y);
    // cfe_vec_poly_mul_FFT(res, &CT_last, sk_y, s->root, s->inv_root, s->inv_2n, s->q);

    cfe_vec_neg(res, res);
    cfe_vec_add(res, &ct_prod, res);
    cfe_vec_mod(res, res, s->q);

    // Return the plaintext res, where res is such that
    // d - center(m) % q is closest to 0.
    // half_q = floor(q/2)
    mpz_t half_q, res_i;
    mpz_inits(half_q, res_i, NULL);
    mpz_fdiv_q_ui(half_q, s->q, 2);
    cfe_vec half_q_vec;
    cfe_vec_init(&half_q_vec, s->n);
    for (size_t i = 0; i < s->n; i++) {
        cfe_vec_get(res_i, res, i);
        if (mpz_cmp(res_i, half_q) == 1) {
            mpz_sub(res_i, res_i, s->q);
            cfe_vec_set(res, res_i, i);
        }
        cfe_vec_set(&half_q_vec, half_q, i);
    }

    cfe_vec_mul_scalar(res, res, s->p);
    cfe_vec_add(res, res, &half_q_vec);
    cfe_vec_fdiv_q_scalar(res, res, s->q);

    // Cleanup
    mpz_clears(half_q, res_i, NULL);
    cfe_vec_frees(&CT_last, &ct_prod, &half_q_vec, NULL);
    cfe_mat_free(&CT_first);

    return CFE_ERR_NONE;
}

// Frees the memory allocated for configuration of the scheme.
void cfe_ring_lwe_free(cfe_ring_lwe *s) {
    mpz_clear(s->p);
    mpz_clear(s->q);
    mpz_clear(s->bound);
    cfe_vec_free(&s->a);
    cfe_normal_cumulative_free(&s->sampler);
}
