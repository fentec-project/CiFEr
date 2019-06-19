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


#ifndef CIFER_DMCFE_H
#define CIFER_DMCFE_H

#include <amcl/big_256_56.h>
#include <amcl/fp12_BN254.h>
#include <amcl/ecp_BN254.h>
#include <amcl/ecp2_BN254.h>
#include "cifer/data/mat.h"

#include <cifer/data/vec_curve.h>

/**
 * \file
 * \ingroup fullysec
 * \brief Damgard multi input scheme.
 */

/**
 * cfe_damgard_multi represents a multi input variant of the underlying Damgard
 * scheme.
 */
typedef struct cfe_dmcfe_client {
    size_t idx;
    cfe_mat share_sec;
    cfe_mat share_pub;
    cfe_mat share;
    cfe_vec s;
    mpz_t order;
    BIG_256_56 order_big;
} cfe_dmcfe_client;

void cfe_dmcfe_client_init(cfe_dmcfe_client *c, size_t idx);

void cfe_dmcfe_set_share(cfe_dmcfe_client *c, cfe_mat **pub_t, size_t num_clients);

void cfe_dmcfe_encrypt(ECP_BN254 *cipher, mpz_t x, char *label, cfe_dmcfe_client *c);

char *cfe_int_to_str2(int i);

void cfe_dmcfe_generate_key_share(cfe_vec_G2 *key_share, cfe_vec *y, cfe_dmcfe_client *c);

cfe_error cfe_dmcfe_decrypt(mpz_t res, ECP_BN254 *ciphers, cfe_vec_G2 *key_shares,
                            char *label, cfe_vec *y, mpz_t bound, size_t num_clients);

///**
// * cfe_damgard_multi_sec_key is a secret key for Damgard multi input scheme.
// */
//typedef struct cfe_damgard_multi_sec_key {
//    size_t num_keys;
//    cfe_damgard_sec_key *msk;
//    cfe_mat otp;
//} cfe_damgard_multi_sec_key;
//
///**
// * cfe_damgard_multi_fe_key is a functional encryption key for Damgard multi
// * input scheme.
// */
//typedef struct cfe_damgard_multi_fe_key {
//    cfe_vec keys1;
//    cfe_vec keys2;
//    mpz_t z;
//} cfe_damgard_multi_fe_key;
//
///**
// * cfe_damgard_multi_enc represents a single encryptor for the damgard_multi scheme.
// */
//typedef struct cfe_damgard_multi_enc {
//    cfe_damgard scheme;
//} cfe_damgard_multi_enc;
//
///**
// * Configures a new instance of the scheme. It accepts the number of slots
// * (encryptors), the length of input vectors l, the bit length of the modulus
// * (we are operating in the Z_p group), and a bound by which coordinates of
// * input vectors are bounded.
// *
// * @param m A pointer to an uninitialized struct representing the scheme
// * @param slots The number of slots (encryptors)
// * @param l The length of input vectors
// * @param modulus_len The bit length of the modulus (we are operating in the Z_p
// * group)
// * @param bound The bound by which coordinates of input vectors are bounded.
// * @return Error code
// */
//cfe_error cfe_damgard_multi_init(cfe_damgard_multi *m, size_t slots, size_t l, size_t modulus_len, mpz_t bound);
//
///**
// * Frees the memory occupied by the struct members. It does not free
// * memory occupied by the struct itself.
// *
// * @param m A pointer to an instance of the scheme (*initialized* cfe_damgard_multi
// * struct)
// */
//void cfe_damgard_multi_free(cfe_damgard_multi *m);
//
///**
//* Reconstructs the scheme with the same configuration parameters from
//* an already existing Damgard multi input scheme instance.
//*
//* @param res A pointer to an uninitialized damgard_multi struct
//* @param m A pointer to an instance of the scheme (*initialized* cfe_damgard_multi
// * struct)
//*/
//void cfe_damgard_multi_copy(cfe_damgard_multi *res, cfe_damgard_multi *m);
//
///**
// * Takes configuration parameters of a scheme instance,
// * and instantiates a new cfe_damgard_multi_enc.
// *
// * @param e A pointer to an uninitialized struct representing the encryptor for
// * the scheme.
// * @param m A pointer to an instance of the scheme (*initialized* cfe_damgard_multi
// * struct)
// */
//void cfe_damgard_multi_enc_init(cfe_damgard_multi_enc *e, cfe_damgard_multi *m);
//
///**
// * Frees the memory occupied by the struct members. It does not
// * free memory occupied by the struct itself.
// *
// * @param e A pointer to an instance of the encryptor (*initialized*
// * cfe_damgard_multi_enc struct)
// */
//void cfe_damgard_multi_enc_free(cfe_damgard_multi_enc *e);
//
///**
// * Frees the memory occupied by the struct members. It does
// * not free memory occupied by the struct itself.
// *
// * @param key A pointer to an *initialized* cfe_damgard_multi_sec_key struct
// */
//void cfe_damgard_multi_sec_key_free(cfe_damgard_multi_sec_key *key);
//
///**
// * Frees the memory occupied by the struct members. It
// * does not free memory occupied by the struct itself.
// *
// * @param key A pointer to an *initialized* cfe_damgard_multi_fe_key struct
// */
//void cfe_damgard_multi_fe_key_free(cfe_damgard_multi_fe_key *key);
//
///**
// * Initializes the structs which represent the master public key and master
// * secret key.
// *
// * @param mpk A pointer to an uninitialized matrix
// * @param msk A pointer to an uninitialized cfe_damgard_multi_sec_key struct
// * @param m A pointer to an instance of the scheme (*initialized* cfe_damgard_multi
// * struct)
// */
//void cfe_damgard_multi_master_keys_init(cfe_mat *mpk, cfe_damgard_multi_sec_key *msk, cfe_damgard_multi *m);
//
///**
// * Generates a matrix comprised of master public keys and a struct encapsulating
// * master secret keys for the scheme.
// * It returns an error in case master keys could not be generated.
// *
// * @param mpk A pointer to a matrix (master public key will be stored here)
// * @param msk A pointer to a cfe_damgard_multi_sec_key struct (master secret
// * key will be stored here)
// * @param m A pointer to an instance of the scheme (*initialized* cfe_damgard_multi
// * struct)
// */
//void cfe_damgard_multi_generate_master_keys(cfe_mat *mpk, cfe_damgard_multi_sec_key *msk, cfe_damgard_multi *m);
//
///**
// * Initializes the struct which represents the functional encryption key.
// *
// * @param fe_key A pointer to an uninitialized cfe_damgard_multi_fe_key struct
// * @param m A pointer to an instance of the scheme (*initialized* cfe_damgard_multi
// * struct)
// */
//void cfe_damgard_multi_fe_key_init(cfe_damgard_multi_fe_key *fe_key, cfe_damgard_multi *m);
//
///**
// * Takes master secret key and a matrix y comprised of input vectors, and
// * returns the functional encryption key.
// * In case the key could not be derived, it returns an error.
//
// * @param fe_key A pointer to a cfe_damgard_multi_fe_key struct (the functional
// * encryption key will be stored here)
// * @param m A pointer to an instance of the scheme (*initialized* cfe_damgard_multi
// * struct)
// * @param msk A pointer to the master secret key
// * @param y A pointer to the matrix comprised of input vectors
// * @return Error code
// */
//cfe_error
//cfe_damgard_multi_derive_key(cfe_damgard_multi_fe_key *fe_key, cfe_damgard_multi *m, cfe_damgard_multi_sec_key *msk,
//                             cfe_mat *y);
//
///**
// * Initializes the vector which represents the ciphertext.
// *
// * @param ciphertext A pointer to an uninitialized vector
// * @param e A pointer to an instance of the encryptor (*initialized*
// * cfe_damgard_multi_enc struct)
// */
//void cfe_damgard_multi_ciphertext_init(cfe_vec *ciphertext, cfe_damgard_multi_enc *e);
//
///**
// * Generates a ciphertext from the input vector x with the provided public key
// * and one-time pad otp (which is a part of the secret key). It returns the
// * ciphertext vector. If encryption failed, an error is returned.
// *
// * @param ciphertext A pointer to a vector (the resulting ciphertext will be
// * stored here)
// * @param e A pointer to an instance of the encryptor (*initialized*
// * cfe_damgard_multi_enc struct)
// * @param x A pointer to the input vector
// * @param pub_key A pointer to the public key vector
// * @param otp A pointer to the one-time pad vector
// * @return Error code
// */
//cfe_error
//cfe_damgard_multi_encrypt(cfe_vec *ciphertext, cfe_damgard_multi_enc *e, cfe_vec *x, cfe_vec *pub_key, cfe_vec *otp);
//
///**
// * Accepts the matrix cipher comprised of encrypted vectors, functional
// * encryption key, and a matrix y comprised of plaintext vectors. It returns
// * the sum of inner products. If decryption failed, an error is returned.
// *
// * @param res The result of the decryption (the value will be stored here)
// * @param m A pointer to an instance of the scheme (*initialized* cfe_damgard_multi
// * struct)
// * @param ciphertext A pointer to the matrix comprised of encrypted vectors
// * @param key A pointer to the functional encryption key
// * @param y A pointer to the matrix comprised of plaintext vectors
// * @return Error code
// */
//cfe_error cfe_damgard_multi_decrypt(mpz_t res, cfe_damgard_multi *m, cfe_mat *ciphertext, cfe_damgard_multi_fe_key *key,
//                                    cfe_mat *y);

#endif