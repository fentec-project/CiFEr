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

#ifndef CIFER_DDH_H
#define CIFER_DDH_H

#include "cifer/data/vec.h"
#include "cifer/internal/errors.h"

/**
 * \file
 * \ingroup simple
 * \brief DDH scheme.
 */

/**
 * cfe_ddh represents a scheme instantiated from the DDH assumption.
 */
typedef struct cfe_ddh {
    size_t l;
    mpz_t bound;
    mpz_t g;
    mpz_t p;
} cfe_ddh;

/**
 * Configures a new instance of the scheme.
 * It returns an error in case the scheme could not be properly configured, or
 * if precondition l * bound² is >= order of the cyclic group.
 *
 * @param s A pointer to an uninitialized struct representing the scheme
 * @param l The length of input vectors
 * @param modulus_len The bit length of the modulus
 * @param bound The bound by which coordinates of input vectors are bounded
 * @return Error code
 */
cfe_error cfe_ddh_init(cfe_ddh *s, size_t l, size_t modulus_len, mpz_t bound);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ddh struct)
 */
void cfe_ddh_free(cfe_ddh *s);

/**
 * Reconstructs the scheme with the same configuration parameters from
 * an already existing DDH scheme instance.
 *
 * @param res A pointer to an uninitialized ddh struct
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ddh struct)
 */
void cfe_ddh_copy(cfe_ddh *res, cfe_ddh *s);

/**
 * Initializes the vectors which represent the master secret key and master
 * public key.
 *
 * @param msk A pointer to an uninitialized vector
 * @param mpk A pointer to an uninitialized vector
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ddh struct)
 */
void cfe_ddh_master_keys_init(cfe_vec *msk, cfe_vec *mpk, cfe_ddh *s);

/**
 * Initializes the vector which represents the ciphertext.
 *
 * @param ciphertext A pointer to an uninitialized vector
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ddh struct)
 */
void cfe_ddh_ciphertext_init(cfe_vec *ciphertext, cfe_ddh *s);

/**
 * Generates a pair of master secret key and master public key for the scheme.
 * It returns an error in case master keys could not be generated.
 *
 * @param msk A pointer to a vector (master secret key will be stored here)
 * @param mpk A pointer to a vector (master public key will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ddh struct)
 */
void cfe_ddh_generate_master_keys(cfe_vec *msk, cfe_vec *mpk, cfe_ddh *s);

/**
 * Takes master secret key and input vector y, and returns the functional
 * encryption key. In case the key could not be derived, it returns an error.
 *
 * @param res The derived key (the value will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ddh struct)
 * @param msk A pointer to the master secret key
 * @param y A pointer to the input vector
 * @return Error code
 */
cfe_error cfe_ddh_derive_key(mpz_t res, cfe_ddh *s, cfe_vec *msk, cfe_vec *y);

/**
 * Encrypts input vector x with the provided master public key. It returns a
 * ciphertext vector. If encryption failed, an error is returned.
 *
 * @param ciphertext A pointer to a vector (the resulting ciphertext will be
 * stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ddh struct)
 * @param x A pointer to the input vector
 * @param mpk A pointer to the master public key
 * @return Error code
 */
cfe_error cfe_ddh_encrypt(cfe_vec *ciphertext, cfe_ddh *s, cfe_vec *x, cfe_vec *mpk);

/**
 * Accepts the encrypted vector x, functional encryption key, and a plaintext
 * vector y. It returns the inner product of x and y. If decryption failed, an
 * error is returned.
 *
 * @param res The result of the decryption (the value will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ddh struct)
 * @param ciphertext A pointer to the ciphertext vector
 * @param key The functional encryption key
 * @param y A pointer to the plaintext vector
 * @return Error code
 */
cfe_error cfe_ddh_decrypt(mpz_t res, cfe_ddh *s, cfe_vec *ciphertext, mpz_t key, cfe_vec *y);

#endif
