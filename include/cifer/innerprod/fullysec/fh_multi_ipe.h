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
 * \brief // FH-Multi-IPE represents a Function Hiding Multi-input Inner
 * Product Encryption scheme based on the paper by P. Datta, T. Okamoto, and
 * J. Tomida:
 * "Full-Hiding (Unbounded) Multi-Input Inner Product Functional Encryption
 * from the 𝒌-Linear Assumption".
 * It allows clients to encrypt vectors {x_1,...,x_m} and derive a secret key
 * based on an inner product vectors {y_1,...,y_m} so that a decryptor can
 * decrypt the sum of inner products <x_1,y_1> + ... + <x_m, y_m> without
 * revealing vectors x_i or y_i. The scheme is slightly modified from the
 * original one to achieve a better performance. The difference is in
 * storing the secret master key as matrices B_hat, B_hat_star, instead of matrices
 * of elliptic curve elements g_1^B_hat, g_2^B_hat_star. This replaces elliptic
 * curves operations with matrix multiplications.
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
 * Configures a new client for the fh_multi_ipe scheme. It returns an error if
 * the bounds and length of vectors are too high.
 *
 * @param c A pointer to an uninitialized struct representing the scheme
 * @param sec_level The parameter defines the security assumption of the scheme,
 * so called k-Lin assumption, where k is the specified sec_level
 * @param num_clients Number of clients participating in the scheme
 * @param vec_len Length of the vectors that each client will encrypt
 * @param bound_x Bound on the inputs of the vectors that will be encrypted
 * @param bound_y Bound on the inputs of the inner product vectors for which
 * the functional keys will be generated.
 * @return Error code
 */
cfe_error cfe_fh_multi_ipe_init(cfe_fh_multi_ipe *c, size_t sec_level, size_t num_clients,
                                size_t vec_len, mpz_t bound_x, mpz_t bound_y);

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
 * Generates a master secret key and a public key for the scheme. It returns an
 * error if generating one of the parts of the secret key failed.
 *
 * @param sec_key A pointer to a cfe_fh_multi_ipe_sec_key struct (the
 * master secret key will be stored here)
 * @param pub_key A pointer to a FH12_BN256 struct (the public key will
 * be stored here)
 * @param c A pointer to an instance of the scheme (*initialized*
 * cfe_fh_multi_ipe struct)
 * @return Error code
 */
cfe_error cfe_fh_multi_ipe_generate_keys(cfe_fh_multi_ipe_sec_key *sec_key,
                                         FP12_BN254 *pub_key, cfe_fh_multi_ipe *c);

/**
 * Initializes the struct which represents the functional encryption key
 * in fh_multi_ipe.
 *
 * @param fe_key A pointer to an uninitialized cfe_mat_G2 struct
 * @param c A pointer to an instance of the scheme (*initialized*
 * cfe_fh_multi_ipe struct)
 */
void cfe_fh_multi_ipe_fe_key_init(cfe_mat_G2 *fe_key, cfe_fh_multi_ipe *c);


/**
 * Takes a master secret key and input matrix y, and derives the functional
 * encryption key. In case the key could not be derived, it returns an error.
 *
 * @param fe_key A pointer to a cfe_mat_G2 struct (the functional
 * encryption key will be stored here)
 * @param y A pointer to the inner product matrix
 * @param sec_key A pointer to the master secret key
 * @param c A pointer to an instance of the scheme (*initialized* cfe_fh_multi_ipe
 * struct)
 * @return Error code
 */
cfe_error cfe_fh_multi_ipe_derive_fe_key(cfe_mat_G2 *fe_key, cfe_mat *y,
                                         cfe_fh_multi_ipe_sec_key *sec_key,
                                         cfe_fh_multi_ipe *c);

/**
 * Initializes the struct which represents the ciphertext.
 *
 * @param cipher A pointer to an uninitialized cfe_vec_G1 struct
 * @param c A pointer to an instance of the scheme (*initialized*
 * cfe_fh_multi_ipe struct)
 */
void cfe_fh_multi_ipe_ciphertext_init(cfe_vec_G1 *cipher, cfe_fh_multi_ipe *c);


/**
 * The function is called by a client that encrypts input vector x with
 * the provided part master secret key. It returns a ciphertext struct.
 * If encryption failed, an error is returned.
 *
 * @param cipher A pointer to an initialized cfe_vec_G1 struct
 * (the resulting ciphertext will be stored here)
 * @param x A pointer to the plaintext vector
 * @param part_sec_key A pointer to a matrix representing a part
 * of the master secret key (i-th client gets i-th matrix in array B_hat)
 * @param c A pointer to an instance of the scheme (*initialized*
 * cfe_fh_multi_ipe struct)
 * @return Error code
 */
cfe_error cfe_fh_multi_ipe_encrypt(cfe_vec_G1 *cipher, cfe_vec *x, cfe_mat *part_sec_key, cfe_fh_multi_ipe *c);

/**
 * Accepts the encrypted vectors and functional encryption key. It returns the
 * inner product of x and y, i.e. <x_1,y_1> + ... + <x_m, y_m> where x_i is i-th
 * encrypted vector and y_i is i-th inner product vector (i-th row of y).
 * If decryption failed, an error is returned.
 *
 * @param res The result of the decryption (the value will be stored here)
 * @param ciphers An array of the ciphertexts
 * @param fe_key A pointer to the functional encryption key
 * @param pub_key A pointer to the public key
 * @param c A pointer to an instance of the scheme (*initialized*
 * cfe_fh_multi_ipe struct)
 * @return Error code
 */
cfe_error cfe_fh_multi_ipe_decrypt(mpz_t res, cfe_vec_G1 *ciphers, cfe_mat_G2 *fe_key, FP12_BN254 *pub_key, cfe_fh_multi_ipe *c);

#endif
