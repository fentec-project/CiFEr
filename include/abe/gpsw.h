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

#ifndef CIFER_GPSW_H
#define CIFER_GPSW_H

#include <gmp.h>
#include <amcl/fp12_BN254.h>

#include "data/vec_curve.h"
#include "data/vec.h"
#include "policy.h"
#include "internal/errors.h"

/**
 * \file
 * \ingroup abe
 * \brief Key policy attribute based scheme.
 *
 * This is a key policy (KP) attribute based (ABE) scheme based
 * on Goyal, Pandey, Sahai, Waters: "Attribute-Based Encryption for
 * Fine-Grained Access Control of Encrypted Data"
 * We abbreviated it GPSW scheme to honor the authors. This scheme
 * enables distribution of keys based on a boolean expression determining
 * which attributes are needed for an entity to be able to decrypt. Each
 * key is connected to some attribute, such that only a set of keys
 * whose attributes are sufficient can decrypt the massage. This scheme
 * is a PUBLIC-KEY scheme - no master secret key is needed to encrypt
 * the messages.
 */

/**
 * cfe_gpsw represents the GPSW scheme.
 */
typedef struct cfe_gpsw {
    size_t l;
    mpz_t p;
} cfe_gpsw;

/**
 * cfe_gpsw_pub_key represents the public key for the GPSW scheme.
 */
typedef struct cfe_gpsw_pub_key {
    cfe_vec_G2 t;
    FP12_BN254 y;
} cfe_gpsw_pub_key;

/**
 * cfe_gpsw_cipher represents the ciphertext structure for the GPSW scheme.
 */
typedef struct cfe_gpsw_cipher {
    int *gamma;
    FP12_BN254 e0;
    cfe_vec_G2 e;
} cfe_gpsw_cipher;

/**
 * cfe_gpsw_cipher represents the key structure with all the keys corresponding
 * owned attributes and is needed for the decryption in the GPSW scheme.
 */
typedef struct cfe_gpsw_keys {
    cfe_mat mat;
    cfe_vec_G1 d;
    int *row_to_attrib;
} cfe_gpsw_keys;

/**
 * Configures a new instance of the scheme.
 *
 * @param gpsw A pointer to an uninitialized struct representing the scheme
 * @param l The number of attributes that can be used in the scheme.
 */
void cfe_gpsw_init(cfe_gpsw *gpsw, size_t l);

/**
 * Initializes the structs which represent the master public and master secret key.
 *
 * @param pk A pointer to an uninitialized cfe_gpsw_pub_key struct
 * @param sk A pointer to an uninitialized vector
 * @param gpsw A pointer to an initialized struct representing the scheme
 */
void cfe_gpsw_master_keys_init(cfe_gpsw_pub_key *pk, cfe_vec *sk, cfe_gpsw *gpsw);

/**
 * Generates master secret and public key.
 *
 * @param pk A pointer to a cfe_gpsw_pub_key struct which represents the master
 * public key
 * @param sk A pointer to a vector which represents the master public key
 * @param gpsw A pointer to an initialized struct representing the scheme
 */
void cfe_gpsw_generate_master_keys(cfe_gpsw_pub_key *pk, cfe_vec *sk, cfe_gpsw *gpsw);

/**
 * Initializes the struct which represents the ciphertext.
 *
 * @param cipher A pointer to an uninitialized cfe_gpsw_cipher struct
 * @param num_attrib The number of attributes
 */
void cfe_gpsw_cipher_init(cfe_gpsw_cipher *cipher, size_t num_attrib);

// TODO: change message to be a string when mapping is defined
/**
 * The function takes as an input a message given as a string, gamma a set of
 * attributes that can be latter used in a decryption policy, and a public
 * key. It creates an encryption of the message.
 *
 * @param cipher A pointer to a cfe_gpsw_cipher struct, the ciphertext will be saved here
 * @param gpsw A pointer to an initialized struct representing the scheme
 * @param msg An element of FP12_BN254 representing the message
 * @param gamma A pointer to an array of integers defining which attributes can be
 * used for the decryption policy
 * @param num_attrib Length of gamma
 * @param pk A pointer to an initialized struct representing the public key
 */
void cfe_gpsw_encrypt(cfe_gpsw_cipher *cipher, cfe_gpsw *gpsw, FP12_BN254 *msg,
                      int *gamma, size_t num_attrib, cfe_gpsw_pub_key *pk);

/**
 * Initializes the vector of elements on the elliptic curve which represents the
 * policy keys.
 *
 * @param policy_keys A pointer to an uninitialized cfe_vec_G1 struct
 * @param msp A pointer to an initialized struct representing the MSP policy
 */
void cfe_gpsw_policy_keys_init(cfe_vec_G1 *policy_keys, cfe_msp *msp);

/**
 * The function given a monotone span program (MSP) and the vector of secret
 * keys produces a vector of keys needed for the decryption. In particular,
 * for each row of the MSP matrix it creates a corresponding key. Since
 * each row of the matrix has a corresponding key, this keys can be latter delegated
 * to entities with corresponding attributes.
 *
 * @param key A pointer to a vector of elements of the elliptic curve,
 * the keys will be saved here
 * @param gpsw A pointer to an initialized struct representing the scheme
 * @param msp A pointer to an initialized struct representing the MSP policy
 * @param sk A pointer to an initialized struct representing the secret key
 */
void cfe_gpsw_generate_policy_keys(cfe_vec_G1 *key, cfe_gpsw *gpsw, cfe_msp *msp, cfe_vec *sk);

/**
 * A helping function used in generate_policy_keys.
 */
void cfe_gpsw_rand_vec_const_sum(cfe_vec *v, mpz_t y, mpz_t p);

/**
 * Initializes the cfe_gpsw_keys struct which represents the keys for the
 * decryption.
 *
 * @param keys A pointer to an uninitialized cfe_gpsw_keys struct
 * @param msp A pointer to an initialized struct representing the MSP policy
 * @param attrib A pointer to an array of integers defining which attributes are
 * owned to join corresponding keys to the final key for the decryption
 * @param num_attrib The length of attrib
 */
void cfe_gpsw_keys_init(cfe_gpsw_keys *keys, cfe_msp *msp, int *attrib, size_t num_attrib);

/**
 * The function given the set of all keys produced from the MSP struct takes
 * those that are specified and creates keys for the decryption.
 *
 * @param keys A pointer to a cfe_gpsw_keys struct, the keys for the decryption
 * will be saved here
 * @param policy_keys A pointer to an initialized struct representing the keys of
 * all the attributes
 * @param msp A pointer to an initialized struct representing the MSP policy
 * @param attrib A pointer to an array of integers defining which attributes are
 * owned to join corresponding keys to the final key for the decryption
 * @param num_attrib The length of attrib
 */
void cfe_gpsw_delegate_keys(cfe_gpsw_keys *keys, cfe_vec_G1 *policy_keys,
                            cfe_msp *msp, int *attrib, size_t num_attrib);

// TODO: change decryption to be a string when mapping is defined
/**
 * The function takes as an input a cipher and keys and tries to decrypt
 * the cipher. If the keys were properly generated, this is possible if and only
 * if the rows of the matrix in the key span the vector [1, 1,..., 1]. If this
 * is not possible, i.e. keys are insufficient, the function returns the
 * corresponding error.
 *
 * @param res An element of FP12_BN254, the decryption will be saved here
 * @param cipher A pointer to an initialized struct representing the ciphertext
 * @param keys A pointer to an initialized struct representing the keys for the decryption
 * @param gpsw A pointer to an initialized struct representing the scheme
 * @return Error code
 */
cfe_error cfe_gpsw_decrypt(FP12_BN254 *res, cfe_gpsw_cipher *cipher, cfe_gpsw_keys *keys, cfe_gpsw *gpsw);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param gpsw A pointer to an instance of the scheme (*initialized* cfe_gpsw
 * struct)
 */
void cfe_gpsw_free(cfe_gpsw *gpsw);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param pk A pointer to an instance of the public key (*initialized*
 * cfe_gpsw_pub_key struct)
 */
void cfe_gpsw_pub_key_free(cfe_gpsw_pub_key *pk);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param cipher A pointer to an instance of the ciphertext (*initialized*
 * cfe_gpsw_cipher struct)
 */
void cfe_gpsw_cipher_free(cfe_gpsw_cipher *cipher);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param keys A pointer to an instance of the decryption keys (*initialized*
 * cfe_gpsw_keys struct)
 */
void cfe_gpsw_keys_free(cfe_gpsw_keys *keys);

#endif
