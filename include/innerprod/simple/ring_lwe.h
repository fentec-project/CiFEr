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

#ifndef CIFER_RING_LWE_H
#define CIFER_RING_LWE_H

#include <math.h>
#include <gmp.h>

#include "data/vec.h"
#include "data/mat.h"
#include "internal/errors.h"
#include "sample/normal_cumulative.h"

/**
 * \file
 * \ingroup simple
 * \brief LWE scheme.
 */

/**
 * cfe_ring_lwe represents common properties of the scheme.
 */
typedef struct cfe_ring_lwe {
    size_t l;    // Length of data vectors for inner product

    size_t n;    // main security parameters of the scheme

    mpz_t bound; // Data vector coordinates should be strictly smaller than bound

    mpz_t p;  // Modulus for message space
    mpz_t q;  // Modulus for ciphertext and keys

    // vector a of dimension n representing the random polynomial for the scheme
    cfe_vec a;

    // sampler
    cfe_normal_cumulative sampler;
} cfe_ring_lwe;

// TODO: this scheme needs automatic parameters generation and the input should
// depend only on n, l, bound_x, bound_y (i.e. also bound x and bound_y should be separated),
// this needs some theoretical effort.

/**
 * Configures a new instance of the scheme.
 *
 * @param s A pointer to an uninitialized struct representing the scheme
 * @param l The length of input vectors
 * @param n The security parameter of the scheme
 * @param bound The bound by which coordinates of the input vectors are bounded
 * @param p Modulus for the inner product
 * @param q Modulus for ciphertext and keys
 * vectors are bounded
 * @param sigma Standard deviation
 * @return Error code
 */
cfe_error cfe_ring_lwe_init(cfe_ring_lwe *s, size_t l, size_t n, mpz_t bound, mpz_t p, mpz_t q, mpf_t sigma);

/**
 * Initializes the matrix which represents the secret key.
 *
 * @param SK A pointer to an uninitialized matrix
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ring_lwe
 * struct)
 */
void cfe_ring_lwe_sec_key_init(cfe_mat *SK, cfe_ring_lwe *s);

/**
 * Generates a private secret key for the scheme.
 *
 * @param SK A pointer to a matrix (master secret key will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ring_lwe
 * struct)
 */
void cfe_ring_lwe_generate_sec_key(cfe_mat *SK, cfe_ring_lwe *s);

/**
 * Initializes the matrix which represents the public key.
 *
 * @param PK A pointer to an uninitialized matrix
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ring_lwe
 * struct)
 */
void cfe_ring_lwe_pub_key_init(cfe_mat *PK, cfe_ring_lwe *s);

/**
 * Generates a public key for the scheme.
 *
 * @param PK A pointer to a matrix (public key will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ring_lwe
 * struct)
 * @param SK A pointer to an initialized matrix representing the secret key.
 * @return Error code
 */
cfe_error cfe_ring_lwe_generate_pub_key(cfe_mat *PK, cfe_ring_lwe *s, cfe_mat *SK);

/**
 * Initializes the vector which represents the functional encryption key.
 *
 * @param sk_y A pointer to an uninitialized vector
 * @param s A pointer to an instance of the scheme (*initialized*
 * cfe_ring_lwe struct)
 */
void cfe_ring_lwe_fe_key_init(cfe_vec *sk_y, cfe_ring_lwe *s);

/**
 * Takes master secret key and inner product vector, and returns the functional
 * encryption key.
 *
 * @param sk_y A pointer to a vector (the functional encryption key will be
 * stored here)
 * @param s A pointer to an instance of the scheme (*initialized*
 * cfe_ring_lwe struct)
 * @param SK A pointer to the master secret key
 * @param y A pointer to the inner product vector
 * @return Error code
 */
cfe_error cfe_ring_lwe_derive_key(cfe_vec *sk_y, cfe_ring_lwe *s, cfe_mat *SK, cfe_vec *y);

/**
 * Initializes the matrix which represents the ciphertext.
 *
 * @param CT A pointer to an uninitialized matrix
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ring_lwe
 * struct)
 */
void cfe_ring_lwe_ciphertext_init(cfe_mat *CT, cfe_ring_lwe *s);

/**
 * Encrypts input matrix x with the provided master public key.
 *
 * @param CT A pointer to a matrix (the resulting ciphertext will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ring_lwe
 * struct)
 * @param x A pointer to the input matrix
 * @param PK A pointer to the matrix representing the public key.
 * @return Error code
 */
cfe_error cfe_ring_lwe_encrypt(cfe_mat *CT, cfe_ring_lwe *s, cfe_mat *x, cfe_mat *PK);

/**
 * Initialized the vector which represents the result of the decryption.
 *
 * @param res A pointer to an uninitialized vector
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ring_lwe
 * struct)
 */
void cfe_ring_lwe_decrypted_init(cfe_vec *res, cfe_ring_lwe *s);

/**
 * Accepts the encrypted matrix X, functional encryption key, and an inner
 * product vector y. It returns the product y^T * X. If decryption failed, an
 * error is returned.
 *
 * @param res A pointer to a vector (result of the decryption will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ring_lwe struct)
 * @param ct A pointer to the ciphertext matrix
 * @param sk_y The functional encryption key
 * @param y A pointer to the inner product vector
 * @return Error code
 */
cfe_error cfe_ring_lwe_decrypt(cfe_vec *res, cfe_ring_lwe *s, cfe_mat *ct, cfe_vec *sk_y, cfe_vec *y);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ring_lwe
 * struct)
 */
void cfe_ring_lwe_free(cfe_ring_lwe *s);

#endif
