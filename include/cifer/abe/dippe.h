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

#include <gmp.h>
#include <amcl/fp12_BN254.h>
#include <amcl/ecp_BN254.h>
#include <amcl/ecp2_BN254.h>

#include "cifer/data/vec.h"
#include "cifer/internal/errors.h"

/**
* \file
* \ingroup abe
* \brief This is a decentralized inner-product predicate encryption
* scheme by Yan Michalevsky and Marc Joye: "Decentralized Policy-Hiding
* Attribute-Based Encryption with Receiver Privacy"
*/


/**
 * cfe_dippe_cipher represents the ciphertext of the DIPPE scheme;
 * Contains the global parameters for the scheme
 */
typedef struct cfe_dippe_cipher {
    FP12_BN254   C_prime;
    ECP_BN254*   C0;
    ECP_BN254*   Ci;
} cfe_dippe_cipher;

/**
 * Clears the cipher and frees allocated memory
 *
 * @param cipher A pointer to a cfe_dippe_cipher struct
 */
void cfe_dippe_cipher_clear(cfe_dippe_cipher *cipher);

/**
 * cfe_dippe_pub_key represents the public key of the DIPPE scheme
 */
typedef struct cfe_dippe_pub_key {
    ECP2_BN254   g2_sigma;
	ECP_BN254*   g1_W_A;
	FP12_BN254*  gt_alpha_A;
} cfe_dippe_pub_key;

/**
 * Clears the public key and frees allocated memory
 *
 * @param pk A pointer to a cfe_dippe_pub_key struct
 */
void cfe_dippe_pub_key_clear(cfe_dippe_pub_key *pk);

/**
 * cfe_dippe_sec_key represents the secret key of the DIPPE scheme
 */
typedef struct cfe_dippe_sec_key {
	mpz_t        sigma;
	cfe_vec      alpha;
	cfe_vec      W;
} cfe_dippe_sec_key;

/**
 * Clears the secret key and frees allocated memory
 *
 * @param sk A pointer to a cfe_dippe_sec_key struct
 */
void cfe_dippe_sec_key_clear(cfe_dippe_sec_key *sk);

/**
 * cfe_dippe_sec_key represents the user secret key of the DIPPE scheme
 */
typedef struct cfe_dippe_user_secret_key {
    ECP2_BN254*  Ki;
} cfe_dippe_user_secret_key;

/**
 * Clears the user secret key and frees allocated memory
 *
 * @param usk A pointer to a cfe_dippe_user_secret_key struct
 */
void cfe_dippe_user_secret_key_clear(cfe_dippe_user_secret_key *usk);

/**
 * cfe_dippe represents the DIPPE scheme
 */
typedef struct cfe_dippe {
    unsigned int assump_size;
    mpz_t        p;
    ECP_BN254*   g1_A;
    ECP_BN254*   g1_UA;
} cfe_dippe;

/**
 * Clears the scheme and frees allocated memory
 *
 * @param dippe A pointer to a cfe_dippe_clear struct
 */
void cfe_dippe_clear(cfe_dippe *dippe);

/**
 * Initializes the DIPPE scheme by generating the global parameters
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param assump_size The size of the underlying assumption
 */
void cfe_dippe_setup(cfe_dippe *dippe, unsigned int assump_size);

/**
 * Initializes an authority and returns its public and secret key pair
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param pk A pointer to a cfe_dippe_pub_key struct; Represents the public key
 * @param sk A pointer to a cfe_dippe_sec_key struct; Represents the secret key
 */
void cfe_dippe_authsetup(cfe_dippe *dippe, cfe_dippe_pub_key *pk, cfe_dippe_sec_key *sk);

/**
 * Encrypts the given messages under the given policy
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param cipher A pointer to a cfe_dippe_cipher struct; Represents the resulting cipher
 * @param pks An array containing references to cfe_dippe_pub_key structs, that are used for encryption
 * @param pks_len Length of the public key array
 * @param pol A reference to a cfe_vec struct; Represents the policy under which the message is encrypted
 * @param msg A reference to a FP12_BN254 struct; Represents the message that is about to be encrypted
 * @return Error code
 */
cfe_error cfe_dippe_encrypt(cfe_dippe *dippe, cfe_dippe_cipher *cipher, cfe_dippe_pub_key *pks[], size_t pks_len, cfe_vec *pol, FP12_BN254 *msg);

/**
 * Creates a user secret key
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param usk A pointer to a cfe_dippe_user_secret_key struct; Represents the resulting user secret key
 * @param usk_id Index of the given attribute vector for which a user secret key is about to be created
 * @param pks An array containing references to cfe_dippe_pub_key structs, that are required for masking
 * @param pks_len Length of the public key array
 * @param attrs A reference to a mpz_t struct; Represents the attribute vector
 * @param gid String that represents a unique user; Required for collusion prevention
 * @return Error code
 */
cfe_error cfe_dippe_keygen(cfe_dippe *dippe, cfe_dippe_user_secret_key *usk, unsigned int usk_id, cfe_dippe_pub_key *pks[], unsigned int pks_len, cfe_dippe_sec_key *sk, mpz_t *attrs, char gid[]);

/**
 * Restores the message of a given ciphertext
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param result A pointer to a FP12_BN254 struct; Represents the resulting clear text message
 * @param usks An array containing cfe_dippe_user_secret_key structs; Represents the set of user secretes key used for decryption
 * @param usks_len Length of the user secret key array
 * @param cipher A pointer to a cfe_dippe_cipher struct; Represents the cipher
 * @param attrs A pointer to a mpz_t struct; Represents the attribute vector
 * @param gid String that represents a unique user; Required for collusion prevention
 */
cfe_error cfe_dippe_decrypt(cfe_dippe *dippe, FP12_BN254 *result, cfe_dippe_user_secret_key *usks, unsigned int usks_len, cfe_dippe_cipher *cipher, mpz_t *attrs, char gid[]);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Helper function that build a conjunction policy vector out of a given pattern
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param pol A pointer to a cfe_vec struct; Represents the resulting policy vector
 * @param pattern String that represents the pattern for which the policy vector is about to be created
 * @return Error code
 */
cfe_error cfe_dippe_build_conjunction_policy_vector(cfe_dippe *dippe, cfe_vec *pol, const char pattern[]);

/**
 * Helper function that build a conjunction attribute vector out of a given pattern
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param attrs A pointer to a cfe_dippe_vec struct; Represents the resulting attribute vector
 * @param pattern String that represents the pattern for which the policy vector is about to be created
 * @return Error code
 */
cfe_error cfe_dippe_build_conjunction_attribute_vector(cfe_dippe *dippe, mpz_t *attrs, const char pattern[]);

/**
 * Helper function that build an exact threshold policy vector out of a given pattern
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param pol A pointer to a cfe_vec struct; Represents the resulting policy vector
 * @param pattern String that represents the pattern for which the policy vector is about to be created
 * @param threshold Threshold value
 * @return Error code
 */
cfe_error cfe_dippe_build_exact_threshold_policy_vector(cfe_dippe *dippe, cfe_vec *pol, const char pattern[], unsigned long threshold);

/**
 * Helper function that build an exact threshold attribute vector out of a given pattern
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param attrs A pointer to a cfe_dippe_vec struct; Represents the resulting attribute vector
 * @param pattern String that represents the pattern for which the policy vector is about to be created
 * @return Error code
 */
cfe_error cfe_dippe_build_exact_threshold_attribute_vector(cfe_dippe *dippe, mpz_t *attrs, const char pattern[]);


#endif
