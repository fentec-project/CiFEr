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

#ifndef CIFER_DIPPE_H
#define CIFER_DIPPE_H

#include <amcl/ecp2_BN254.h>
#include <amcl/fp12_BN254.h>
#include <gmp.h>
#include <stdlib.h>

#include "cifer/data/mat.h"
#include "cifer/data/mat_curve.h"
#include "cifer/data/vec.h"
#include "cifer/data/vec_curve.h"
#include "cifer/internal/errors.h"

/**
* \file
* \ingroup abe
* \brief This is a decentralized inner-product predicate encryption
* scheme by Yan Michalevsky and Marc Joye: "Decentralized Policy-Hiding
* Attribute-Based Encryption with Receiver Privacy"
*/

/**
 * cfe_dippe represents the DIPPE scheme
 */
typedef struct cfe_dippe {
    size_t assump_size;
    mpz_t p;
    cfe_mat_G1 g1_A;
    cfe_mat_G1 g1_UA;
} cfe_dippe;

/**
 * Initializes the DIPPE scheme and generates the global parameters
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param assump_size The size of the underlying assumption
 */
void cfe_dippe_init(cfe_dippe *dippe, size_t assump_size);

/**
 * Clears the scheme and frees allocated memory
 *
 * @param dippe A pointer to a cfe_dippe struct
 */
void cfe_dippe_free(cfe_dippe *dippe);

/**
 * Prepares an attribute vector
 *
 * @param av A pointer to a cfe_vec struct; Represents the resulting attribute vector
 * @param num_attrib Number of attributes
 * @param pattern Array of indices to define the attribute vector
 * @param pat_len Length of the pattern array
 * @return Error code
 */
cfe_error cfe_dippe_attribute_vector_init(cfe_vec *av, size_t num_attrib, size_t pattern[], size_t pat_len);

/**
 * Prepares an exact threshold policy vector
 *
 * @param pv A pointer to a cfe_vec struct; Represents the resulting policy vector
 * @param dippe A pointer to a cfe_dippe struct
 * @param num_attrib Number of attributes
 * @param pattern Array of indices to define the policy vector
 * @param pat_len Length of the pattern array
 * @param threshold Threshold value
 * @return Error code
 */
cfe_error cfe_dippe_exact_threshold_policy_vector_init(cfe_vec *pv, cfe_dippe *dippe, size_t num_attrib, size_t pattern[], size_t pat_len, size_t threshold);

/**
 * Prepares a conjunction policy vector
 *
 * @param pv A pointer to a cfe_vec struct; Represents the resulting policy vector
 * @param dippe A pointer to a cfe_dippe struct
 * @param num_attrib Number of attributes
 * @param pattern Array of indices to define the policy vector
 * @param pat_len Length of the pattern array
 * @return Error code
 */
cfe_error cfe_dippe_conjunction_policy_vector_init(cfe_vec *pv, cfe_dippe *dippe, size_t num_attrib, size_t pattern[], size_t pat_len);

/**
 * cfe_dippe_cipher represents the ciphertext of the DIPPE scheme
 */
typedef struct cfe_dippe_cipher {
    FP12_BN254 C_prime;
    cfe_vec_G1 C0;
    cfe_mat_G1 Ci;
} cfe_dippe_cipher;

/**
 * Prepares a cipher struct
 *
 * @param pk A pointer to a cfe_dippe_cipher struct
 * @param dippe A pointer to a cfe_dippe struct
 */
void cfe_dippe_cipher_init(cfe_dippe_cipher *cipher, cfe_dippe *dippe, size_t pol_size);

/**
 * Clears the ciphertext and frees allocated memory
 *
 * @param cipher A pointer to a cfe_dippe_cipher struct
 */
void cfe_dippe_cipher_free(cfe_dippe_cipher *cipher);

/**
 * cfe_dippe_pub_key represents the public key of the DIPPE scheme
 */
typedef struct cfe_dippe_pub_key {
    ECP2_BN254 g2_sigma;
    cfe_mat_G1 g1_W_A;
    cfe_vec_GT gt_alpha_A;
} cfe_dippe_pub_key;

/**
 * Prepares a public key struct
 *
 * @param pk A pointer to a cfe_dippe_pub_key struct
 * @param dippe A pointer to a cfe_dippe struct
 */
void cfe_dippe_pub_key_init(cfe_dippe_pub_key *pk, cfe_dippe *dippe);

/**
 * Clears the public key and frees allocated memory
 *
 * @param pk A pointer to a cfe_dippe_pub_key struct
 */
void cfe_dippe_pub_key_free(cfe_dippe_pub_key *pk);

/**
 * cfe_dippe_sec_key represents the secret key of the DIPPE scheme
 */
typedef struct cfe_dippe_sec_key {
    mpz_t sigma;
    cfe_vec alpha;
    cfe_mat W;
} cfe_dippe_sec_key;

/**
 * Prepares a secret key struct
 *
 * @param pk A pointer to a cfe_dippe_sec_key struct
 * @param dippe A pointer to a cfe_dippe struct
 */
void cfe_dippe_sec_key_init(cfe_dippe_sec_key *sk, cfe_dippe *dippe);

/**
 * Clears the secret key and frees allocated memory
 *
 * @param sk A pointer to a cfe_dippe_sec_key struct
 */
void cfe_dippe_sec_key_free(cfe_dippe_sec_key *sk);

/**
 * cfe_dippe_sec_key represents the user secret key of the DIPPE scheme
 */
typedef struct cfe_dippe_user_sec_key {
    cfe_vec_G2 Ki;
} cfe_dippe_user_sec_key;

/**
 * Prepares a user secret key struct
 *
 * @param usk A pointer to a cfe_dippe_user_sec_key struct
 * @param dippe A pointer to a cfe_dippe struct
 */
void cfe_dippe_user_sec_key_init(cfe_dippe_user_sec_key *usk, cfe_dippe *dippe);

/**
 * Clears the user secret key and frees allocated memory
 *
 * @param usk A pointer to a cfe_dippe_user_sec_key struct
 */
void cfe_dippe_user_sec_key_free(cfe_dippe_user_sec_key *usk);

/**
 * Initializes an authority and generates its public and private key
 *
 * @param pk A pointer to a cfe_dippe_pub_key struct; Represents the public key that is about to be populated
 * @param sk A pointer to a cfe_dippe_sec_key struct; Represents the secret key that is about to be populated
 * @param dippe A pointer to a cfe_dippe struct
 */
void cfe_dippe_generate_master_keys(cfe_dippe_pub_key *pk, cfe_dippe_sec_key *sk, cfe_dippe *dippe);

/**
 * Encrypts a given message under the provided policy
 *
 * @param cipher A pointer to a cfe_dippe_cipher struct; Represents the ciphertext that is about to be populated
 * @param dippe A pointer to a cfe_dippe struct
 * @param pks An array containing references to cfe_dippe_pub_key structs;
 * @param pks_len Length of the public key array
 * @param pv A reference to a cfe_vec struct; Represents the policy under which the message will be encrypted
 * @param msg A reference to a FP12_BN254 struct; Represents the message that is about to be encrypted
 * @return Error code
 */
cfe_error cfe_dippe_encrypt(cfe_dippe_cipher *cipher, cfe_dippe *dippe, cfe_dippe_pub_key *pks[], size_t pks_len, cfe_vec *pv, FP12_BN254 *msg);

/**
 * Used with an authority's secret key to create a new user secret key for a given attribute vector
 *
 * @param usk A pointer to a cfe_dippe_user_sec_key struct; Represents the user secret key that is about to be populated
 * @param dippe A pointer to a cfe_dippe struct
 * @param usk_id Index of the given attribute vector for which a user secret key will be created
 * @param pks An array containing references to cfe_dippe_pub_key structs
 * @param pks_len Length of the public key array
 * @param av A pointer to a cfe_vec struct; Represents the attribute vector
 * @param gid String that represents a unique user; Required for collusion prevention
 * @return Error code
 */
cfe_error cfe_dippe_keygen(cfe_dippe_user_sec_key *usk, cfe_dippe *dippe, size_t usk_id, cfe_dippe_pub_key *pks[], size_t pks_len, cfe_dippe_sec_key *sk, cfe_vec *av, char gid[]);

/**
 * Restores the underlying message of a given ciphertext
 *
 * @param result A pointer to a FP12_BN254 struct; Represents the resulting message
 * @param dippe A pointer to a cfe_dippe struct
 * @param usks An array containing cfe_dippe_user_sec_key structs; Represents the set of user secretes key used for decryption
 * @param usks_len Length of the user secret key array
 * @param cipher A pointer to a cfe_dippe_cipher struct; Represents the ciphertext that is about to be decrypted
 * @param av A pointer to a cfe_vec struct; Represents the attribute vector
 * @param gid String that represents a unique user; Required for collusion prevention
 */
cfe_error cfe_dippe_decrypt(FP12_BN254 *result, cfe_dippe *dippe, cfe_dippe_user_sec_key *usks, size_t usks_len, cfe_dippe_cipher *cipher, cfe_vec *av, char gid[]);

#endif
