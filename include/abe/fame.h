/* *
 * Copyright (C) 2018 XLAB d.o.o.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of either:
 *
 *     * the GNU Lesser General Public License as published by the Free
 *     Software Foundation; either version 3 of the License, or (at your
 *     option) any later version.
 *
 * or
 *
 *     * the GNU General Public License as published by the Free Software
 *     Foundation; either version 2 of the License, or (at your option) any
 *     later version.
 *
 * or both in parallel, as here.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CIFER_FAME_H
#define CIFER_FAME_H

#include <gmp.h>
#include <amcl/fp12_BN254.h>
#include <amcl/ecp_BN254.h>
#include <amcl/ecp2_BN254.h>

#include "data/vec.h"
#include "abe/policy.h"
#include "internal/errors.h"

/**
* \file
* \ingroup abe
* \brief This is a ciphertext policy (CP) attribute based (ABE) scheme based
* on Shashank Agrawal and Melissa Chase: "FAME: Fast Attribute-based Message
* Encryption"
* This scheme enables encrypting a message based on a boolean expression
* determining which attributes are needed for an entity to be able to decrypt.
* Each key is connected to some attribute, such that only a set of keys
* whose attributes are sufficient can decrypt the massage. This scheme
* is a PUBLIC-KEY scheme - no master secret key is needed to encrypt
* the messages.
*/

/**
 * cfe_fame represents the FAME scheme.
 */
typedef struct cfe_fame {
    mpz_t p;
} cfe_fame;

/**
 * cfe_fame_pub_key represents the public key for the FAME scheme.It does
 * not need to be manually initialized by an initialization function.
 */
typedef struct cfe_fame_pub_key {
    ECP2_BN254 part_G2[2];
    FP12_BN254 part_GT[2];
} cfe_fame_pub_key;

/**
 * cfe_fame_sec_key represents the secret key for the FAME scheme.
 */
typedef struct cfe_fame_sec_key {
    mpz_t part_int[4];
    ECP_BN254 part_G1[3];
} cfe_fame_sec_key;

/**
 * cfe_fame_cipher represents the cipher for the FAME scheme.
 */
typedef struct cfe_fame_cipher {
    ECP2_BN254 ct0[3];
    ECP_BN254 (*ct)[3];
    FP12_BN254 ct_prime;
    cfe_msp msp;
} cfe_fame_cipher;

/**
 * cfe_fame_attrib_keys represents the keys corresponding to
 * attributes needed for the decryption.
 */
typedef struct cfe_fame_attrib_keys {
    ECP2_BN254 k0[3];
    ECP_BN254 (*k)[3];
    ECP_BN254 k_prime[3];
    int *row_to_attrib;
    size_t num_attrib;
} cfe_fame_attrib_keys;

/**
 * Configures a new instance of the scheme.
 *
 * @param fame A pointer to an uninitialized struct representing the scheme
 */
void cfe_fame_init(cfe_fame *fame);

/**
 * Frees the memory occupied by the struct members. It does not free
 * the memory occupied by the struct itself.
 *
 * @param fame A pointer to an instance of the scheme (*initialized* cfe_fame
 * struct)
 */
void cfe_fame_free(cfe_fame *fame);

/**
 * Initializes a secret key.
 *
 * @param sk A pointer to an uninitialized struct representing the secret key
 */
void cfe_fame_sec_key_init(cfe_fame_sec_key *sk);

/**
 * Frees the memory occupied by the struct members. It does not free
 * the memory occupied by the struct itself.
 *
 * @param sk A pointer to an instance of the secret key (*initialized*
 * cfe_fame_sec_key struct)
 */
void cfe_fame_sec_key_free(cfe_fame_sec_key *sk);

/**
 * Generates a master secret key and a public key.
 *
 * @param pk A pointer to a cfe_fame_pub_key struct, the public key will be saved here;
 * pk does not need to be manually initialized by an initialization function
 * @param sk A pointer to an initialized cfe_fame_sec_key struct, the secret key will be saved here
 * @param fame A pointer to an initialized struct representing the scheme
 */
void cfe_fame_generate_master_keys(cfe_fame_pub_key *pk,
                                   cfe_fame_sec_key *sk, cfe_fame *fame);

/**
 * Initializes the ciphertext that will be generated given a
 * MSP structure. A MSP structure is needed to decide how much
 * memory needs to be allocated.
 *
 * @param cipher A pointer to an uninitialized struct representing the ciphertext
 * @param msp A pointer to an initialized struct representing the MSP policy
 */
void cfe_fame_cipher_init(cfe_fame_cipher *cipher, cfe_msp *msp);

/**
 * Frees the memory occupied by the struct members. It does not free
 * the memory occupied by the struct itself.
 *
 * @param cipher A pointer to an instance of the ciphertext (*initialized*
 * cfe_fame_cipher struct)
 */
void cfe_fame_cipher_free(cfe_fame_cipher *cipher);

// TODO: change message to be a string when mapping is defined
/**
 * The function takes as an input a message, a MSP structure describing a
 * policy which attributes are needed for the decryption, and a public
 * key. It creates an encryption of the message.
 *
 * @param cipher A pointer to an initialized cfe_fame_cipher struct, the
 * encryption will be saved here
 * @param msg An element of FP12_BN254 representing the message
 * @param msp A pointer to an initialized struct representing the MSP policy
 * @param pk A pointer to an initialized struct representing the public key
 * @param fame A pointer to an initialized struct representing the scheme
 */
void cfe_fame_encrypt(cfe_fame_cipher *cipher, FP12_BN254 *msg,
                      cfe_msp *msp, cfe_fame_pub_key *pk, cfe_fame *fame);

/**
 * Initializes attribute keys needed for the decryption. The number of attributes
 * needs to be specified to decide how much memory needs to be allocated.
 *
 * @param keys A pointer to an uninitialized struct representing the keys
 * @param num_attrib The number of attributes that the keys will contain
 */
void cfe_fame_attrib_keys_init(cfe_fame_attrib_keys *keys, size_t num_attrib);

/**
 * Frees the memory occupied by the struct members. It does not free
 * the memory occupied by the struct itself.
 *
 * @param keys A pointer to an instance of attribute keys (*initialized*
 * cfe_fame_attrib_keys struct)
 */
void cfe_fame_attrib_keys_free(cfe_fame_attrib_keys *keys);

/**
 * The function given an array of attributes and master secret keys creates
 * keys corresponding to attributes that can be used for the decryption.
 *
 * @param keys A pointer to an initialized cfe_fame_attrib_keys struct,
 * the keys for the decryption will be saved here
 * @param gamma An array of attributes for which keys will be generated
 * @param num_attrib Length of the array gamma, i.e. the number of attributes
 * @param sk A pointer to a struct representing the secret keys
 * @param fame A pointer to an initialized struct representing the scheme
 */
void cfe_fame_generate_attrib_keys(cfe_fame_attrib_keys *keys, int *gamma,
                                   size_t num_attrib, cfe_fame_sec_key *sk, cfe_fame *fame);

// TODO: change decryption to be a string when mapping is defined
/**
 * The function takes as an input a cipher and keys and tries to decrypt
 * the cipher. If the keys were properly generated, this is possible if and only
 * if the rows of the MSP matrix corresponding to owned attributes span the
 * vector [1, 0,..., 0]. If this is not possible, i.e. keys are insufficient,
 * the function returns the corresponding error.
 *
 * @param res An element of FP12_BN254, the decryption will be saved here
 * @param cipher A pointer to an initialized struct representing the ciphertext
 * @param keys A pointer to an initialized struct representing the keys for the decryption
 * @param fame A pointer to an initialized struct representing the scheme
 * @return Error code
 */
cfe_error cfe_fame_decrypt(FP12_BN254 *res, cfe_fame_cipher *cipher,
                           cfe_fame_attrib_keys *keys, cfe_fame *fame);

/**
 * A helping function used in cfe_fame_encrypt and cfe_fame_generate_attrib_keys.
 */
char *cfe_strings_concat_for_hash(char *str, ...);

/**
 * A helping function used in cfe_fame_encrypt and cfe_fame_generate_attrib_keys.
 */
char *cfe_int_to_str(int i);

/**
 * A helping function used in cfe_fame_encrypt and cfe_fame_generate_attrib_keys.
 */
void cfe_hash_G1(ECP_BN254 *g, char *str);

#endif
