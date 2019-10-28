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

#ifndef CIFER_DAMGARD_H
#define CIFER_DAMGARD_H

#include "cifer/data/vec.h"
#include "cifer/internal/errors.h"

/**
 * \file
 * \ingroup fullysec
 * \brief Damgard scheme.
 */

/**
 * cfe_damgard represents a scheme instantiated from the DDH assumption.
 */
typedef struct cfe_damgard {
    size_t l;
    mpz_t bound;
    mpz_t g;
    mpz_t h;
    mpz_t p;
    mpz_t q;
} cfe_damgard;

/**
 * cfe_damgard_sec_key is a secret key for Damgard scheme.
 */
typedef struct cfe_damgard_sec_key {
    cfe_vec s;
    cfe_vec t;
} cfe_damgard_sec_key;

/**
 * cfe_damgard_fe_key is a functional encryption key for Damgard scheme.
 */
typedef struct cfe_damgard_fe_key {
    mpz_t key1;
    mpz_t key2;
} cfe_damgard_fe_key;

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
cfe_error cfe_damgard_init(cfe_damgard *s, size_t l, size_t modulus_len, mpz_t bound);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param s A pointer to an instance of the scheme (*initialized* cfe_damgard
 * struct)
 */
void cfe_damgard_free(cfe_damgard *s);

/**
 * Reconstructs the scheme with the same configuration parameters from
 * an already existing Damgard scheme instance.
 *
 * @param res A pointer to an uninitialized cfe_damgard struct
 * @param s A pointer to an instance of the scheme (*initialized* cfe_damgard
 * struct)
 */
void cfe_damgard_copy(cfe_damgard *res, cfe_damgard *s);

/**
 * Frees the memory occupied by the struct members. It does
 * not free memory occupied by the struct itself.
 *
 * @param key A pointer to an *initialized* cfe_damgard_sec_key struct
 */
void cfe_damgard_sec_key_free(cfe_damgard_sec_key *key);

/**
 * Frees the memory occupied by the struct members. It does
 * not free memory occupied by the struct itself.
 *
 * @param key A pointer to an *initialized* cfe_damgard_fe_key struct
 */
void cfe_damgard_fe_key_free(cfe_damgard_fe_key *key);

/**
 * Initializes the struct which represents the master secret key.
 *
 * @param msk A pointer to an uninitialized cfe_damgard_sec_key struct
 * @param s A pointer to an instance of the scheme (*initialized* cfe_damgard
 * struct)
 */
void cfe_damgard_sec_key_init(cfe_damgard_sec_key *msk, cfe_damgard *s);

/**
 * Initializes the vector which represents the master public key.
 *
 * @param mpk A pointer to an uninitialized vector
 * @param s A pointer to an instance of the scheme (*initialized* cfe_damgard
 * struct)
 */
void cfe_damgard_pub_key_init(cfe_vec *mpk, cfe_damgard *s);

/**
 * Generates a master secret key and master public key for the scheme.
 *
 * @param msk A pointer to a cfe_damgard_sec_key struct (master secret key will
 * be stored here)
 * @param mpk A pointer to a vector (master public key will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_damgard
 * struct)
 */
void cfe_damgard_generate_master_keys(cfe_damgard_sec_key *msk, cfe_vec *mpk, cfe_damgard *s);

/**
 * Initializes the struct which represents the functional encryption key.
 *
 * @param fe_key A pointer to an uninitialized cfe_damgard_fe_key struct
 */
void cfe_damgard_fe_key_init(cfe_damgard_fe_key *fe_key);

/**
 * Takes master secret key and input vector y, and returns the functional
 * encryption key. In case the key could not be derived, it returns an error.
 *
 * @param fe_key A pointer to a cfe_damgard_fe_key struct (the functional
 * encryption key will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_damgard
 * struct)
 * @param msk A pointer to the master secret key
 * @param y A pointer to the inner product vector
 * @return Error code
 */
cfe_error cfe_damgard_derive_fe_key(cfe_damgard_fe_key *fe_key, cfe_damgard *s, cfe_damgard_sec_key *msk, cfe_vec *y);

/**
 * Initializes the vector which represents the ciphertext.
 *
 * @param ciphertext A pointer to an uninitialized vector
 * @param s A pointer to an instance of the scheme (*initialized* cfe_damgard
 * struct)
 */
void cfe_damgard_ciphertext_init(cfe_vec *ciphertext, cfe_damgard *s);

/**
 * Encrypts input vector x with the provided master public key. It returns a
 * ciphertext vector. If encryption failed, an error is returned.
 *
 * @param ciphertext A pointer to a vector (the resulting ciphertext will be
 * stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_damgard
 * struct)
 * @param x A pointer to the input vector
 * @param mpk A pointer to the master public key
 * @return Error code
 */
cfe_error cfe_damgard_encrypt(cfe_vec *ciphertext, cfe_damgard *s, cfe_vec *x, cfe_vec *mpk);

/**
 * Accepts the encrypted vector, functional encryption key, and a plaintext
 * vector y. It returns the inner product of x and y. If decryption failed, an
 * error is returned.
 *
 * @param res The result of the decryption (the value will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_damgard
 * struct)
 * @param ciphertext A pointer to the ciphertext vector
 * @param key The functional encryption key
 * @param y A pointer to the inner product vector
 * @return Error code
 */
cfe_error cfe_damgard_decrypt(mpz_t res, cfe_damgard *s, cfe_vec *ciphertext, cfe_damgard_fe_key *key, cfe_vec *y);

#endif
