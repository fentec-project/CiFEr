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

#ifndef CIFER_FHIPE_H
#define CIFER_FHIPE_H

#include "cifer/data/mat.h"
#include "cifer/data/vec_curve.h"

/**
 * \file
 * \ingroup fullysec
 * \brief // FHIPE represents a Function Hiding Inner Product Encryption scheme
 * based on the paper by Kim, Lewi, Mandal, Montgomery, Roy, Wu:
 * "Function-Hiding Inner Product Encryption is Practical".
 * It allows to encrypt a vector x and derive a secret key based
 * on an inner product vector y so that a deryptor can decrypt the
 * inner product <x,y> without revealing x or y.
 * The struct contains the shared choice for parameters on which
 * the functionality of the scheme depend.
 */

/**
 * cfe_fhipe represents a client in the function hiding scheme.
 */
typedef struct cfe_fhipe {
    size_t l;
    mpz_t bound_x;
    mpz_t bound_y;
    mpz_t order;
} cfe_fhipe;

/**
 * Configures a new client for the fhipe scheme. It returns an error if
 * the bounds and length of vectors are too high.
 *
 * @param c A pointer to an uninitialized struct representing the scheme
 * @param l Length of the vectors that will be encrypted
 * @param bound_x Bound on the inputs of the vectors that will be encrypted
 * @param bound_y Bound on the inputs of the inner product vectors for which
 * the functional keys will be generated.
 * @return Error code
 */
cfe_error cfe_fhipe_init(cfe_fhipe *c, size_t l, mpz_t bound_x, mpz_t bound_y);

/**
 * Reconstructs the scheme with the same configuration parameters from
 * an already existing fhipe scheme instance.
 *
 * @param res A pointer to an uninitialized cfe_fhipe struct
 * @param c A pointer to an instance of the scheme (*initialized* cfe_fhipe
 * struct)
 */
void cfe_fhipe_copy(cfe_fhipe *res, cfe_fhipe *c);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param c A pointer to an instance of the scheme (*initialized* cfe_fhipe
 * struct)
 */
void cfe_fhipe_free(cfe_fhipe *c);

/**
 * cfe_fhipe_sec_key represents a master secret key in fhipe scheme.
 */
typedef struct cfe_fhipe_sec_key {
    ECP_BN254 g1;
    ECP2_BN254 g2;
    cfe_mat B;
    cfe_mat B_star;
} cfe_fhipe_sec_key;

/**
 * Initializes the struct which represents the master secret key in fhipe.
 *
 * @param sec_key A pointer to an uninitialized cfe_fhipe_sec_key struct
 * @param c A pointer to an instance of the scheme (*initialized* cfe_fhipe
 * struct)
 */
void cfe_fhipe_master_key_init(cfe_fhipe_sec_key *sec_key, cfe_fhipe *c);

/**
 * Frees the memory occupied by the struct members. It does
 * not free memory occupied by the struct itself.
 *
 * @param sec_key A pointer to an *initialized* cfe_fhipe_sec_key struct
 */
void cfe_fhipe_master_key_free(cfe_fhipe_sec_key *sec_key);

/**
 * Generates a master secret key for the scheme. It returns an error if an error
 * occurred.
 *
 * @param sec_key A pointer to a cfe_fhipe_sec_key struct (the master secret key will
 * be stored here)
 * @param c A pointer to an instance of the scheme (*initialized* cfe_damgard
 * struct)
 * @return Error code
 */
cfe_error cfe_fhipe_generate_master_key(cfe_fhipe_sec_key *sec_key, cfe_fhipe *c);

/**
 * cfe_fhipe_FE_key represents a functional encryption key in fhipe scheme.
 */
typedef struct cfe_fhipe_fe_key {
    ECP_BN254 k1;
    cfe_vec_G1 k2;
} cfe_fhipe_fe_key;

/**
 * Initializes the struct which represents the function key in fhipe.
 *
 * @param fe_key A pointer to an uninitialized cfe_fhipe_FE_key struct
 * @param c A pointer to an instance of the scheme (*initialized* cfe_fhipe
 * struct)
 */
void cfe_fhipe_fe_key_init(cfe_fhipe_fe_key *fe_key, cfe_fhipe *c);

/**
 * Frees the memory occupied by the struct members. It does
 * not free memory occupied by the struct itself.
 *
 * @param fe_key A pointer to an *initialized* cfe_fhipe_FE_key struct
 */
void cfe_fhipe_fe_key_free(cfe_fhipe_fe_key *fe_key);

/**
 * Takes master secret key and input vector y, and derives the functional
 * encryption key. In case the key could not be derived, it returns an error.
 *
 * @param fe_key A pointer to a cfe_damgard_fe_key struct (the functional
 * encryption key will be stored here)
 * @param y A pointer to the inner product vector
 * @param msk A pointer to the master secret key
 * @param c A pointer to an instance of the scheme (*initialized* cfe_fhipe
 * struct)
 * @return Error code
 */
cfe_error cfe_fhipe_derive_fe_key(cfe_fhipe_fe_key *fe_key, cfe_vec *y, cfe_fhipe_sec_key *sec_key, cfe_fhipe *c);

/**
 * cfe_fhipe_ciphertext represents a client in a decentralized multi-client scheme.
 */
typedef struct cfe_fhipe_ciphertext {
    ECP2_BN254 c1;
    cfe_vec_G2 c2;
} cfe_fhipe_ciphertext;

/**
 * Initializes the struct which represents the ciphertext.
 *
 * @param cipher A pointer to an uninitialized cfe_fhipe_ciphertext struct
 * @param c A pointer to an instance of the scheme (*initialized* cfe_fhipe
 * struct)
 */
void cfe_fhipe_ciphertext_init(cfe_fhipe_ciphertext *cipher, cfe_fhipe *c);

/**
 * Frees the memory occupied by the struct members. It does
 * not free memory occupied by the struct itself.
 *
 * @param cipher A pointer to an *initialized* cfe_fhipe_ciphertext struct
 */
void cfe_fhipe_ciphertext_free(cfe_fhipe_ciphertext *cipher);

/**
 * Encrypts input vector x with the provided master secret key. It returns a
 * ciphertext struct. If encryption failed, an error is returned.
 *
 * @param cipher A pointer to an initialized cfe_fhipe_ciphertext struct
 * (the resulting ciphertext will be stored here)
 * @param x A pointer to the plaintext vector
 * @param sec_key A pointer to the master secret key
 * @param c A pointer to an instance of the scheme (*initialized* cfe_fhipe
 * struct)
 * @return Error code
 */
cfe_error cfe_fhipe_encrypt(cfe_fhipe_ciphertext *cipher, cfe_vec *x, cfe_fhipe_sec_key *sec_key, cfe_fhipe *c);

/**
 * Accepts the encrypted vector and functional encryption key. It returns the
 * inner product of x and y. If decryption failed, an error is returned.
 *
 * @param res The result of the decryption (the value will be stored here)
 * @param cipher A pointer to the ciphertext vector
 * @param fe_key The functional encryption key
 * @param c A pointer to an instance of the scheme (*initialized* cfe_damgard
 * struct)
 * @return Error code
 */
cfe_error cfe_fhipe_decrypt(mpz_t res, cfe_fhipe_ciphertext *cipher, cfe_fhipe_fe_key *fe_key, cfe_fhipe *c);

#endif
