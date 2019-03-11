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

#include "data/vec_curve.h"
#include "data/vec.h"
#include "policy.h"
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
 * cfe_fame_pub_key represents the public key for the FAME scheme.
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
 * cfe_fame_attrib_keys represents the keys corresponding to owned
 * attributes used for the decryption.
 */
typedef struct cfe_fame_attrib_keys {
    ECP2_BN254 k0[3];
    ECP_BN254 (*k)[3];
    ECP_BN254 k_prime[3];
    int *row_to_attrib;
    size_t num_attrib;
} cfe_fame_attrib_keys;

void cfe_fame_init(cfe_fame *fame);

void cfe_fame_sec_key_init(cfe_fame_sec_key *sk);

void cfe_fame_generate_master_keys(cfe_fame_pub_key *pk,
        cfe_fame_sec_key *sk, cfe_fame *fame);

void cfe_fame_cipher_init(cfe_fame_cipher *cipher, cfe_msp *msp);

void cfe_fame_encrypt(cfe_fame_cipher *cipher, FP12_BN254 *msg,
        cfe_msp *msp, cfe_fame_pub_key *pk, cfe_fame *fame);

void cfe_fame_attrib_keys_init(cfe_fame_attrib_keys *keys, size_t num_attrib);

void cfe_fame_generate_attrib_keys(cfe_fame_attrib_keys *keys, int *gamma,
        size_t num_attrib, cfe_fame_sec_key *sk, cfe_fame *fame);

cfe_error cfe_fame_decrypt(FP12_BN254 *res, cfe_fame_cipher *cipher,
                           cfe_fame_attrib_keys *keys, cfe_fame *fame);

char *strings_concat_for_hash(char *str, ...);

char *int_to_str(int i);

void hash_G1(ECP_BN254 *g, char *str);

#endif
