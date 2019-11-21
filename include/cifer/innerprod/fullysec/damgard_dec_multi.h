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

#ifndef CIFER_DAMGARD_DEC_MULTI_H
#define CIFER_DAMGARD_DEC_MULTI_H

#include "cifer/data/vec.h"
#include "cifer/internal/errors.h"
#include "cifer/innerprod/fullysec/damgard_multi.h"

/**
 * \file
 * \ingroup fullysec
 * \brief Damgard style decentralized multi input scheme. The decentralization
 * is based on paper by Abdalla, Benhamouda, Kohlweiss, and Waldner:
 * "Decentralizing Inner-Product Functional Encryption".
 * The participants in the scheme are clients without a central authority.
 * They interactively generate private keys for each client so that client i
 * can encrypt vector x_i. The scheme allows the clients to interactively
 * generate a key_Y, depending on a matrix Y with rows y_i, so that
 * given key_Y and the ciphertexts the decryptor can compute value
 * Σ_i <x_i, y_i> (sum of dot products).
 */

/**
 * cfe_damgard_dec_multi_client represents a client in a decentralized
 * multi input variant of the underlying Damgard scheme. Each client
 * has an identification idx, a Diffie-Hellman type of public
 * and secret key and a secret share. It is implicitly assumed
 * that if there are n clients, their idxs are in [0,n).
 */
typedef struct cfe_damgard_dec_multi_client {
    size_t idx;
    cfe_damgard_multi scheme;
    mpz_t client_pub_key;
    mpz_t client_sec_key;
    cfe_mat share;
} cfe_damgard_dec_multi_client;

/**
 * cfe_damgard_dec_multi_dec represents a decryptor in a decentralized
 * multi input Damgard style scheme.
 */
typedef struct cfe_damgard_dec_multi_dec {
    cfe_damgard_multi scheme;
} cfe_damgard_dec_multi_dec;

/**
 * cfe_damgard_dec_multi_sec_key represents a secret key that each client posses
 * and is needed for encryption, and derivation of functional encryption keys. This key
 * should be different for each new encryption.
 */
typedef struct cfe_damgard_dec_multi_sec_key {
    cfe_damgard_sec_key dam_sec_key;
    cfe_vec dam_pub_key;
    cfe_vec otp_key;
} cfe_damgard_dec_multi_sec_key;

/**
 * cfe_damgard_dec_multi_fe_key_part represents a functional encryption key
 * needed for the decryption.
 */
typedef struct cfe_damgard_dec_multi_fe_key_part {
    cfe_damgard_fe_key key_part;
    mpz_t otp_key_part;
} cfe_damgard_dec_multi_fe_key_part;

/**
 * Configures a new client for the cfe_damgard_dec_multi scheme.
 *
 * @param c A pointer to an uninitialized struct representing the scheme
 * @param damgard_multi A pointer to a damgard_multi scheme on top of which
 * the decentralized scheme is based.
 * @param idx Identification value of the client; it is assumed that if there are
 * n clients, their identifications are from [0,n)
 */
void cfe_damgard_dec_multi_client_init(cfe_damgard_dec_multi_client *c,
                                       cfe_damgard_multi *damgard_multi, size_t idx);

/**
 * Frees the memory occupied by the struct members. It does not free the
 * memory occupied by the struct itself.
 *
 * @param c A pointer to an instance of the scheme (*initialized* cfe_damgard_dec_multi_client
 * struct)
 */
void cfe_damgard_dec_multi_client_free(cfe_damgard_dec_multi_client *c);

/**
 * Sets a secret share for the client based on the public keys of all the
 * participant. Note that the function assumes that if there are n clients,
 * their identifications are from [0,n) and their public keys are given
 * by an array in the corresponding order.
 *
 * @param c A pointer to an initialized struct representing the scheme
 * @param pub_keys An array of public keys
 */
void cfe_damgard_dec_multi_client_set_share(cfe_damgard_dec_multi_client *c,
                                            mpz_t *pub_keys);

/**
 * Initializes the structs which represent the secret key of each client, needed
 * for the encryption and derivation of functional key.
 *
 * @param sec_key A pointer to an uninitialized cfe_damgard_dec_multi_sec_key struct
 * @param c A pointer to an instance of the scheme (*initialized*
 * cfe_damgard_dec_multi_client struct)
 */
void cfe_damgard_dec_multi_sec_key_init(cfe_damgard_dec_multi_sec_key *sec_key,
                                        cfe_damgard_dec_multi_client *c);

/**
 * Frees the memory occupied by the struct members. It does not free the
 * memory occupied by the struct itself.
 *
 * @param sec_key A pointer to an instance of the secret key
 * (*initialized* cfe_damgard_dec_multi_sec_key struct)
 */
void cfe_damgard_dec_multi_sec_key_free(cfe_damgard_dec_multi_sec_key *sec_key);

/**
 * Generates a secret key for a client.
 *
 * @param sec_key A pointer to an instance of the secret key
 * (*initialized* cfe_damgard_dec_multi_sec_key struct)
 * @param c A pointer to an instance of the scheme (*initialized*
 * cfe_damgard_dec_multi_client struct)
 */
void cfe_damgard_dec_multi_generate_keys(cfe_damgard_dec_multi_sec_key *sec_key,
                                         cfe_damgard_dec_multi_client *c);

/**
 * Initializes the struct which represents the ciphertext of each client.
 *
 * @param ciphertext A pointer to an uninitialized cfe_vec struct
 * @param c A pointer to an instance of the scheme (*initialized*
 * cfe_damgard_dec_multi_client struct)
 */
void cfe_damgard_dec_multi_ciphertext_init(cfe_vec *ciphertext,
                                           cfe_damgard_dec_multi_client *c);

/**
 * Each client encrypts an input vector x using its secret key.
 *
 * @param cipher A pointer to cfe_vec struct (the resulting ciphertext will be
 * stored here)
 * @param x The input value
 * @param sec_key A pointer to client's secret key
 * @param c A pointer to a struct representing a client
 * (*initialized* cfe_damgard_dec_multi_client)
 * @return Error code
 */
cfe_error cfe_damgard_dec_multi_encrypt(cfe_vec *cipher, cfe_vec *x,
                                        cfe_damgard_dec_multi_sec_key *sec_key,
                                        cfe_damgard_dec_multi_client *c);

/**
 * Initializes the structs which represent a part of functional encryption
 * key.
 *
 * @param fe_key_part A pointer to an uninitialized cfe_damgard_dec_multi_fe_key_part struct
 */
void cfe_damgard_dec_multi_fe_key_share_init(cfe_damgard_dec_multi_fe_key_part *fe_key_part);

/**
 * Frees the memory occupied by the struct members. It does not free the
 * memory occupied by the struct itself.
 *
 * @param fe_key_part A pointer to an instance of the functional
 *  encryption key.(*initialized* cfe_damgard_dec_multi_client
 * struct)
 */
void cfe_damgard_dec_multi_fe_key_free(cfe_damgard_dec_multi_fe_key_part *fe_key_part);

/**
 * Sets a part of a functional encryption key needed for the decryption of an inner
 * product of encrypted vectors.
 *
 * @param fe_key_part A pointer to initialized cfe_damgard_dec_multi_fe_key_part (the key part will
 * be save here)
 * @param y A pointer to the inner-product matrix, i.e. decryption of
 * Σ_i <x_i, y_i> will be possible where y_i is a row of y.
 * @param sec_key A client's secret key
 * @param c A pointer to an initialized struct representing the scheme
 * @return Error code
 */
cfe_error cfe_damgard_dec_multi_client_derive_fe_key_part(cfe_damgard_dec_multi_fe_key_part *fe_key_part,
                                                          cfe_mat *y, cfe_damgard_dec_multi_sec_key *sec_key,
                                                          cfe_damgard_dec_multi_client *c);

/**
 * Initializes the struct which represents a decryptor in a
 * in a decentralized multi input Damgard style scheme.
 *
 * @param d A pointer to an uninitialized cfe_damgard_dec_multi_dec struct
 * @param damgard_multi A pointer to the underlying damgard_multi scheme on
 * which the decentralization is based
 */
void cfe_damgard_dec_multi_dec_init(cfe_damgard_dec_multi_dec *d,
                                    cfe_damgard_multi *damgard_multi);

/**
 * Frees the memory occupied by the struct members. It does not free the
 * memory occupied by the struct itself.
 *
 * @param d A pointer to an instance of the decryptor.(*initialized* cfe_damgard_dec_multi_dec
 * struct)
 */
void cfe_damgard_dec_multi_dec_free(cfe_damgard_dec_multi_dec *d);

/**
 * Accepts an array of ciphers, i.e. the encrypted vectors, an array of functional
 * encryption key parts for the inner-product, a plaintext inner product matrix y,
 * and the decryptor instance. It calculates the value Σ_i <x_i, y_i> (sum
 * of inner products). If decryption failed, an error is returned.
 *
 * @param res The result of the decryption (the value will be stored here)
 * @param ciphers An array of the encrypted coordinates of the vector
 * @param fe_key_parts An array of the decryption key shares
 * @param y A pointer to the inner-product matrix
 * @param d A pointer to an instance of the decryptor.(*initialized* cfe_damgard_dec_multi_dec
 * struct)
 * @return Error code
 */
cfe_error cfe_damgard_dec_multi_decrypt(mpz_t res, cfe_vec *ciphers,
                                        cfe_damgard_dec_multi_fe_key_part *fe_key_parts, cfe_mat *y,
                                        cfe_damgard_dec_multi_dec *d);

#endif
