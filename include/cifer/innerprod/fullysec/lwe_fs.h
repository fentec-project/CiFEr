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

#ifndef CIFER_LWE_FS_H
#define CIFER_LWE_FS_H

#include <math.h>
#include <gmp.h>

#include "cifer/data/vec.h"
#include "cifer/data/mat.h"
#include "cifer/internal/errors.h"

/**
 * \file
 * \ingroup fullysec
 * \brief LWE scheme.
 */

/**
 * lwe_fs represents common properties of the scheme.
 */
typedef struct cfe_lwe_fs {
    size_t l; // Length of data vectors for inner product

    size_t n; // Main security parameters of the scheme
    size_t m; // Number of samples

    // Message space size
    mpz_t bound_x;
    // Inner product vector space size
    mpz_t bound_y;
    // Modulus for the resulting inner product.
    // K depends on the parameters l, P and V and is computed by the scheme.
    mpz_t K;
    // Modulus for ciphertext and keys.
    // Must be significantly larger than K.
    mpz_t q;
    // standard deviation for the noise terms in the encryption process
    mpf_t sigma_q;
    mpz_t k_sigma_q;
    // standard deviation for first half of the matrix for sampling private key
    mpf_t sigma1;
    mpz_t k_sigma1;
    // standard deviation for second half of the matrix for sampling private key
    mpf_t sigma2;
    mpz_t k_sigma2;

    // Matrix A of dimensions m*n is a public parameter
    // of the scheme
    cfe_mat A;
} cfe_lwe_fs;

/**
 * Configures a new instance of the scheme.
 *
 * @param s A pointer to an uninitialized struct representing the scheme
 * @param l The length of input vectors
 * @param n The security parameter of the scheme
 * @param bound_x The bound by which coordinates of the encrypted vectors are bounded
 * @param bound_y The bound by which coordinates of the inner product
 * vectors are bounded
 * @return Error code
 */
cfe_error cfe_lwe_fs_init(cfe_lwe_fs *s, size_t l, size_t n, mpz_t bound_x, mpz_t bound_y);

/**
 * Initializes the matrix which represents the secret key.
 *
 * @param SK A pointer to an uninitialized matrix
 * @param s A pointer to an instance of the scheme (*initialized* cfe_lwe_fs
 * struct)
 */
void cfe_lwe_fs_sec_key_init(cfe_mat *SK, cfe_lwe_fs *s);

/**
 * Generates a private secret key for the scheme.
 *
 * @param SK A pointer to a matrix (master secret key will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_lwe_fs
 * struct)
 */
void cfe_lwe_fs_generate_sec_key(cfe_mat *SK, cfe_lwe_fs *s);

/**
 * Initializes the matrix which represents the public key.
 *
 * @param PK A pointer to an uninitialized matrix
 * @param s A pointer to an instance of the scheme (*initialized* cfe_lwe_fs
 * struct)
 */
void cfe_lwe_fs_pub_key_init(cfe_mat *PK, cfe_lwe_fs *s);

/**
 * Generates a public key for the scheme.
 *
 * @param PK A pointer to a matrix (master public key will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_lwe_fs
 * struct)
 * @param SK A pointer to an initialized matrix representing the secret key.
 * @return Error code
 */
cfe_error cfe_lwe_fs_generate_pub_key(cfe_mat *PK, cfe_lwe_fs *s, cfe_mat *SK);

/**
 * Initializes the vector which represents the functional encryption key.
 *
 * @param z_y A pointer to an uninitialized vector
 * @param s A pointer to an instance of the scheme (*initialized*
 * cfe_lwe_fs struct)
 */
void cfe_lwe_fs_fe_key_init(cfe_vec *z_y, cfe_lwe_fs *s);

/**
 * Takes master secret key and inner product vector, and returns the functional
 * encryption key.
 *
 * @param z_y A pointer to an initialized functional encryption
 * key, which will be stored here
 * @param s A pointer to an instance of the scheme (*initialized*
 * cfe_lwe_fs struct)
 * @param y A pointer to the input vector
 * @param SK A pointer to the master secret key
 * @return Error code
 */
cfe_error cfe_lwe_fs_derive_fe_key(cfe_vec *z_y, cfe_lwe_fs *s, cfe_vec *y, cfe_mat *SK);

/**
 * Initializes the vector which represents the ciphertext.
 *
 * @param ct A pointer to an uninitialized vector
 * @param s A pointer to an instance of the scheme (*initialized*
 * cfe_lwe_fs struct)
 */
void cfe_lwe_fs_ciphertext_init(cfe_vec *ct, cfe_lwe_fs *s);

/**
 * Encrypts input vector x with the provided master public key.
 *
 * @param ct A pointer to a vector (the resulting ciphertext will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_lwe_fs
 * struct)
 * @param x A pointer to the input vector
 * @param PK A pointer to a matrix - master public key.
 * @return Error code
 */
cfe_error cfe_lwe_fs_encrypt(cfe_vec *ct, cfe_lwe_fs *s, cfe_vec *x, cfe_mat *PK);

/**
 * Accepts the encrypted vector, functional encryption key, and a plaintext
 * vector. It returns the inner product. If decryption failed, an
 * error is returned.
 *
 * @param res The result of the decryption (the value will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_lwe_fs
 * struct)
 * @param ct A pointer to the vector representing the ciphertext
 * @param z_y A pointer to the functional encryption key vector
 * @param y A pointer to the inner product vector
 * @return Error code
 */
cfe_error cfe_lwe_fs_decrypt(mpz_t res, cfe_lwe_fs *s, cfe_vec *ct, cfe_vec *z_y, cfe_vec *y);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param s A pointer to an instance of the scheme (*initialized* cfe_lwe_fs
 * struct)
 */
void cfe_lwe_fs_free(cfe_lwe_fs *s);

#endif
