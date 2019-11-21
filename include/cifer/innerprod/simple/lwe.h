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

#ifndef CIFER_LWE_H
#define CIFER_LWE_H

#include <math.h>
#include <gmp.h>

#include "cifer/data/vec.h"
#include "cifer/data/mat.h"
#include "cifer/internal/errors.h"

/**
 * \file
 * \ingroup simple
 * \brief LWE scheme.
 */

/**
 * cfe_lwe represents common properties of the scheme.
 */
typedef struct cfe_lwe {
    size_t l; // Length of data vectors for inner product

    size_t n; // Main security parameters of the scheme
    size_t m; // Number of rows (samples) for the LWE problem

    mpz_t bound_x; // Bound for input vector coordinates (for x)
    mpz_t bound_y; // Bound for inner product vector coordinates (for y)

    mpz_t p; // Modulus for message space
    mpz_t q; // Modulus for ciphertext and keys

    mpf_t sigma_q;

    // Matrix A of dimensions m*n is a public parameter
    // of the scheme
    cfe_mat A;
} cfe_lwe;

/**
 * Configures a new instance of the scheme.
 * Security parameters are generated so
 * that they satisfy theoretical bounds provided in the phd thesis Functional
 * Encryption for Inner-Product Evaluations, see Section 8.3.1 in
 * https://www.di.ens.fr/~fbourse/publications/Thesis.pdf
 *
 * @param s A pointer to an uninitialized struct representing the scheme
 * @param l The length of input vectors
 * @param bound_x The bound by which coordinates of the encrypted vectors are bounded
 * @param bound_y The bound by which coordinates of the inner product
 * vectors are bounded
 * @param n The security parameter of the scheme
 * @return Error code
 */
cfe_error cfe_lwe_init(cfe_lwe *s, size_t l, mpz_t bound_x, mpz_t bound_y, size_t n);

/**
 * Initializes the matrix which represents the secret key.
 *
 * @param SK A pointer to an uninitialized matrix
 * @param s A pointer to an instance of the scheme (*initialized* cfe_lwe
 * struct)
 */
void cfe_lwe_sec_key_init(cfe_mat *SK, cfe_lwe *s);

/**
 * Generates a private secret key for the scheme.
 *
 * @param SK A pointer to a matrix (master secret key will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_lwe
 * struct)
 */
void cfe_lwe_generate_sec_key(cfe_mat *SK, cfe_lwe *s);

/**
 * Initializes the matrix which represents the public key.
 *
 * @param PK A pointer to an uninitialized matrix
 * @param s A pointer to an instance of the scheme (*initialized* cfe_lwe
 * struct)
 */
void cfe_lwe_pub_key_init(cfe_mat *PK, cfe_lwe *s);

/**
 * Generates a public key for the scheme.
 *
 * @param PK A pointer to a matrix (public key will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_lwe
 * struct)
 * @param SK A pointer to an initialized matrix representing the secret key.
 * @return Error code
 */
cfe_error cfe_lwe_generate_pub_key(cfe_mat *PK, cfe_lwe *s, cfe_mat *SK);

/**
 * Initializes the vector which represents the functional encryption key.
 *
 * @param sk_y A pointer to an uninitialized vector
 * @param s A pointer to an instance of the scheme (*initialized*
 * cfe_lwe struct)
 */
void cfe_lwe_fe_key_init(cfe_vec *sk_y, cfe_lwe *s);

/**
 * Takes master secret key and inner product vector, and returns the functional
 * encryption key.
 *
 * @param sk_y A pointer to a vector (the functional encryption key will be
 * stored here)
 * @param SK A pointer to the master secret key
 * @param s A pointer to an instance of the scheme (*initialized*
 * cfe_lwe struct)
 * @param y A pointer to the inner product vector
 * @return Error code
 */
cfe_error cfe_lwe_derive_fe_key(cfe_vec *sk_y, cfe_lwe *s, cfe_mat *SK, cfe_vec *y);

/**
 * Initializes the vector which represents the ciphertext.
 *
 * @param ct A pointer to an uninitialized vector
 * @param s A pointer to an instance of the scheme (*initialized* cfe_lwe
 * struct)
 */
void cfe_lwe_ciphertext_init(cfe_vec *ct, cfe_lwe *s);

/**
 * Encrypts input vector x with the provided master public key.
 *
 * @param ct A pointer to a vector (the resulting ciphertext will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_lwe
 * struct)
 * @param x A pointer to the input vector
 * @param PK A pointer to the matrix representing the public key.
 * @return Error code
 */
cfe_error cfe_lwe_encrypt(cfe_vec *ct, cfe_lwe *s, cfe_vec *x, cfe_mat *PK);

/**
 * Accepts the encrypted vector x, functional encryption key, and a plaintext
 * vector. It returns the inner product. If decryption failed, an
 * error is returned.
 *
 * @param res The result of the decryption (the value will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_lwe struct)
 * @param c A pointer to the ciphertext vector
 * @param sk_y The functional encryption key
 * @param y A pointer to the inner product vector
 * @return Error code
 */
cfe_error cfe_lwe_decrypt(mpz_t res, cfe_lwe *s, cfe_vec *c, cfe_vec *sk_y, cfe_vec *y);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param s A pointer to an instance of the scheme (*initialized* cfe_lwe
 * struct)
 */
void cfe_lwe_free(cfe_lwe *s);

#endif
