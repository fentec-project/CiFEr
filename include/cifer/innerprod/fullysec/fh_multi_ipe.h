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

#ifndef CIFER_FH_MULTI_IPE_H
#define CIFER_FH_MULTI_IPE_H

#include "cifer/data/mat.h"
#include "cifer/data/mat_curve.h"

/**
 * \file
 * \ingroup fullysec
 * \brief // FHIPE represents a Function Hiding Inner Product Encryption scheme
 * based on the paper by Kim, Lewi, Mandal, Montgomery, Roy, Wu:
 * "Function-Hiding Inner Product Encryption is Practical".
 * It allows to encrypt a vector x and derive a secret key based
 * on an inner product vector y so that a deryptor can decrypt the
 * inner product <x,y> without revealing x or y.
 */


/**
 * cfe_fh_multi_ipe contains the shared choice for parameters on which
 * the functionality of the scheme depend.
 */
typedef struct cfe_fh_multi_ipe {
    size_t sec_level;
    size_t num_clients;
    size_t vec_len;
    mpz_t bound_x;
    mpz_t bound_y;
    mpz_t order;
} cfe_fh_multi_ipe;


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
cfe_error cfe_fh_multi_ipe_init(cfe_fh_multi_ipe *c, size_t sec_level, size_t num_clients, size_t vec_len, mpz_t bound_x, mpz_t bound_y);

/**
 * Reconstructs the scheme with the same configuration parameters from
 * an already existing fh_multi_ipe scheme instance.
 *
 * @param res A pointer to an uninitialized cfe_fh_multi_ipe struct
 * @param c A pointer to an instance of the scheme (*initialized* cfe_fh_multi_ipe
 * struct)
 */
void cfe_fh_multi_ipe_copy(cfe_fh_multi_ipe *res, cfe_fh_multi_ipe *c);

/**
 * Frees the memory occupied by the struct members. It does not free the
 * memory occupied by the struct itself.
 *
 * @param c A pointer to an instance of the scheme (*initialized* cfe_fh_multi_ipe
 * struct)
 */
void cfe_fh_multi_ipe_free(cfe_fh_multi_ipe *c);

/**
 * cfe_fh_multi_ipe_sec_key represents a master secret key in fh_multi_ipe scheme.
 */
typedef struct cfe_fh_multi_ipe_sec_key {
    cfe_mat *B_hat;
    cfe_mat *B_star_hat;
    size_t num_clients;
} cfe_fh_multi_ipe_sec_key;

/**
 * Initializes the struct which represents the master secret key in fh_multi_ipe.
 *
 * @param sec_key A pointer to an uninitialized cfe_fh_multi_ipe_sec_key struct
 * @param c A pointer to an instance of the scheme (*initialized* cfe_fh_multi_ipe
 * struct)
 */
void cfe_fh_multi_ipe_master_key_init(cfe_fh_multi_ipe_sec_key *sec_key, cfe_fh_multi_ipe *c);

/**
 * Frees the memory occupied by the struct members. It does
 * not free the memory occupied by the struct itself.
 *
 * @param sec_key A pointer to an *initialized* cfe_fh_multi_ipe_sec_key struct
 */
void cfe_fh_multi_ipe_master_key_free(cfe_fh_multi_ipe_sec_key *sec_key);

/**
 * Generates a master secret key for the scheme. It returns an error if generating
 * one of the parts of the secret key failed.
 *
 * @param sec_key A pointer to a cfe_fh_multi_ipe_sec_key struct (the master secret key will
 * be stored here)
 * @param c A pointer to an instance of the scheme (*initialized* cfe_damgard
 * struct)
 * @return Error code
 */
cfe_error cfe_fh_multi_ipe_generate_keys(cfe_fh_multi_ipe_sec_key *sec_key, FP12_BN254 *pub_key, cfe_fh_multi_ipe *c);

/**
 * Initializes the struct which represents the functional encryption key in fh_multi_ipe.
 *
 * @param fe_key A pointer to an uninitialized cfe_fh_multi_ipe_FE_key struct
 * @param c A pointer to an instance of the scheme (*initialized* cfe_fh_multi_ipe
 * struct)
 */
void cfe_fh_multi_ipe_fe_key_init(cfe_mat_G2 *fe_key, cfe_fh_multi_ipe *c);


/**
 * Takes a master secret key and input vector y, and derives the functional
 * encryption key. In case the key could not be derived, it returns an error.
 *
 * @param fe_key A pointer to a cfe_damgard_fe_key struct (the functional
 * encryption key will be stored here)
 * @param y A pointer to the inner product vector
 * @param sec_key A pointer to the master secret key
 * @param c A pointer to an instance of the scheme (*initialized* cfe_fh_multi_ipe
 * struct)
 * @return Error code
 */
cfe_error cfe_fh_multi_ipe_derive_fe_key(cfe_mat_G2 *fe_key, cfe_mat *y, cfe_fh_multi_ipe_sec_key *sec_key, cfe_fh_multi_ipe *c);

/**
 * Initializes the struct which represents the ciphertext.
 *
 * @param cipher A pointer to an uninitialized cfe_fh_multi_ipe_ciphertext struct
 * @param c A pointer to an instance of the scheme (*initialized* cfe_fh_multi_ipe
 * struct)
 */
void cfe_fh_multi_ipe_ciphertext_init(cfe_vec_G1 *cipher, cfe_fh_multi_ipe *c);


/**
 * Encrypts input vector x with the provided master secret key. It returns a
 * ciphertext struct. If encryption failed, an error is returned.
 *
 * @param cipher A pointer to an initialized cfe_fh_multi_ipe_ciphertext struct
 * (the resulting ciphertext will be stored here)
 * @param x A pointer to the plaintext vector
 * @param sec_key A pointer to the master secret key
 * @param c A pointer to an instance of the scheme (*initialized* cfe_fh_multi_ipe
 * struct)
 * @return Error code
 */
cfe_error cfe_fh_multi_ipe_encrypt(cfe_vec_G1 *cipher, cfe_vec *x, cfe_mat *part_sec_key, cfe_fh_multi_ipe *c);

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
cfe_error cfe_fh_multi_ipe_decrypt(mpz_t res, cfe_vec_G1 *cipher, cfe_mat_G2 *fe_key, FP12_BN254 *pub_key, cfe_fh_multi_ipe *c);

#endif
