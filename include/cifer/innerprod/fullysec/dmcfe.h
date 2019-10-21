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

#include "cifer/data/mat.h"
#include "cifer/data/vec_curve.h"

/**
 * \file
 * \ingroup fullysec
 * \brief Decentralized multi-client inner-product scheme based on the paper
 * "Decentralized Multi-Client Functional Encryption for Inner Product" by
 * Chotard, Dufour Sans, Gay, Phan, and Pointcheval.
 */

/**
 * cfe_dmcfe_client represents a client in a decentralized multi-client scheme.
 */
typedef struct cfe_dmcfe_client {
    size_t idx;
    BIG_256_56 client_sec_key_big;
    ECP_BN254 client_pub_key;
    cfe_mat share;
    cfe_vec s;
    mpz_t order;
    BIG_256_56 order_big;
} cfe_dmcfe_client;

/**
 * Configures a new client for the dmcfe scheme.
 *
 * @param c A pointer to an uninitialized struct representing the scheme
 * @param idx Identification value of the client; it is assumed that if there are
 * n clients, their identifications are from [0,n)
 */
void cfe_dmcfe_client_init(cfe_dmcfe_client *c, size_t idx);

/**
 * Frees the memory occupied by the struct members. It does not free the
 * memory occupied by the struct itself.
 *
 * @param c A pointer to an instance of the scheme (*initialized* cfe_dmcfe_client
 * struct)
 */
void cfe_dmcfe_client_free(cfe_dmcfe_client *c);

/**
 * Sets a secret key share for the client based on the public keys all the
 * participant. Note that the function assumes that if there is n clients,
 * their identifications are from [0,n).
 *
 * @param c A pointer to an initialized struct representing the scheme
 * @param pub_keys An array of public keys
 * @param num_clients Number of clients
 */
void cfe_dmcfe_set_share(cfe_dmcfe_client *c, ECP_BN254 *pub_keys, size_t num_clients);

/**
 * Client c encrypts input value x, a coordinate of a vector. All clients should use
 * the same label to encrypt a vector.
 *
 * @param cipher A pointer to ECP_BN254 struct (the resulting ciphertext will be
 * stored here)
 * @param c A pointer to an instance of the scheme (*initialized* cfe_dmcfe_client
 * @param x The input value
 * @param label A string label of the encrypted vector
 * struct)
 */
void cfe_dmcfe_encrypt(ECP_BN254 *cipher, cfe_dmcfe_client *c, mpz_t x, char *label);

/**
 * Sets a share of a decryption key needed for the decryption of an inner product
 * of encrypted vector and y.
 *
 * @param key_share A pointer to initialized cfe_vec_G2 of size 2 (the key share will
 * be save here)
 * @param c A pointer to an initialized struct representing the scheme
 * @param y A pointer to the inner-product vector
 */
void cfe_dmcfe_derive_key_share(cfe_vec_G2 *key_share, cfe_dmcfe_client *c, cfe_vec *y);

/**
 * Accepts an array of ciphers, i.e. the encrypted vector, an array of key shares for the
 * inner-product, a plaintext vector y, and a bound on the inputs of the encrypted and
 * inner-product vector. The bound is used to improve the computation of the discrete
 * logarithm needed for the decryption. It calculates the inner product of x and y.
 * If decryption failed, an error is returned.
 *
 * @param res The result of the decryption (the value will be stored here)
 * @param ciphers An array of the encrypted coordinates of the vector
 * @param key_shares An array of the decryption key shares
 * @param label A string label of the encrypted value
 * @param y A pointer to the inner-product vector
 * @param bound A bound on all the values of the encrypted vector and inner-product
 * vector
 * @return Error code
 */
cfe_error cfe_dmcfe_decrypt(mpz_t res, ECP_BN254 *ciphers, cfe_vec_G2 *key_shares,
                            char *label, cfe_vec *y, mpz_t bound);

#endif
