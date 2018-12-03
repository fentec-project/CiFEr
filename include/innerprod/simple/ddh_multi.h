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

#ifndef CIFER_DDH_MULTI_H
#define CIFER_DDH_MULTI_H

#include "data/mat.h"
#include "innerprod/simple/ddh.h"

/**
 * \file
 * \ingroup simple
 * \brief DDH multi input scheme.
 */

/**
 * cfe_ddh_multi represents a multi input variant of the underlying DDH scheme.
 */
typedef struct cfe_ddh_multi {
    size_t slots;
    cfe_ddh scheme;
} cfe_ddh_multi;

/**
 * cfe_ddh_multi_sec_key is a secret key for DDH multi input scheme.
 */
typedef struct cfe_ddh_multi_sec_key {
    cfe_mat msk;
    cfe_mat otp_key;
} cfe_ddh_multi_sec_key;

/**
 * cfe_ddh_multi_fe_key is a functional encryption key for DDH multi input
 * scheme.
 */
typedef struct cfe_ddh_multi_fe_key {
    cfe_vec keys;
    mpz_t otp_key;
} cfe_ddh_multi_fe_key;

/**
 * cfe_ddh_multi_enc represents a single encryptor for the ddh_multi scheme.
 */
typedef struct cfe_ddh_multi_enc {
    cfe_ddh scheme;
} cfe_ddh_multi_enc;

/**
 * Configures a new instance of the scheme. It accepts the number of slots
 * (encryptors), the length of input vectors l, the bit length of the modulus
 * (we are operating in the Z_p group), and a bound by which coordinates of
 * input vectors are bounded.
 *
 * @param m A pointer to an uninitialized struct representing the scheme
 * @param slots The number of slots (encryptors)
 * @param l The length of input vectors
 * @param modulus_len The bit length of the modulus (we are operating in the Z_p
 * group)
 * @param bound The bound by which coordinates of input vectors are bounded.
 * @return Error code
 */
cfe_error cfe_ddh_multi_init(cfe_ddh_multi *m, size_t slots, size_t l, size_t modulus_len, mpz_t bound);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param m A pointer to an instance of the scheme (*initialized* cfe_ddh_multi
 * struct)
 */
void cfe_ddh_multi_free(cfe_ddh_multi *m);

/**
* Reconstructs the scheme with the same configuration parameters from
* an already existing DDH multi input scheme instance.
*
* @param res A pointer to an uninitialized cfe_ddh_multi struct
* @param m A pointer to an instance of the scheme (*initialized* cfe_ddh_multi
 * struct)
*/
void cfe_ddh_multi_copy(cfe_ddh_multi *res, cfe_ddh_multi *m);

/**
 * Takes configuration parameters of a scheme instance,
 * and instantiates a new ddh_multi_enc.
 *
 * @param e A pointer to an uninitialized struct representing the encryptor for
 * the scheme.
 * @param m A pointer to an instance of the scheme (*initialized* cfe_ddh_multi
 * struct)
 */
void cfe_ddh_multi_enc_init(cfe_ddh_multi_enc *e, cfe_ddh_multi *m);

/**
 * Frees the memory occupied by the struct members. It does not
 * free memory occupied by the struct itself.
 *
 * @param e A pointer to an instance of the encryptor (*initialized*
 * cfe_ddh_multi_enc struct)
 */
void cfe_ddh_multi_enc_free(cfe_ddh_multi_enc *e);

/**
 * Frees the memory occupied by the struct members. It does
 * not free memory occupied by the struct itself.
 *
 * @param key A pointer to an *initialized* cfe_ddh_multi_sec_key struct
 */
void cfe_ddh_multi_sec_key_free(cfe_ddh_multi_sec_key *key);

/**
 * Frees the memory occupied by the struct members. It
 * does not free memory occupied by the struct itself.
 *
 * @param key A pointer to an *initialized* cfe_ddh_multi_fe_key struct
 */
void cfe_ddh_multi_fe_key_free(cfe_ddh_multi_fe_key *key);

/**
 * Generates a matrix comprised of master public keys and a struct encapsulating
 * master secret keys for the scheme.
 * It returns an error in case master keys could not be generated.
 *
 * @param mpk A pointer to an uninitialized matrix (master public key will be
 * stored here)
 * @param msk A pointer to an uninitialized cfe_ddh_multi_sec_key struct (master
 * secret key will be stored here)
 * @param m A pointer to an instance of the scheme (*initialized* cfe_ddh_multi
 * struct)
 */
void cfe_ddh_multi_generate_master_keys(cfe_mat *mpk, cfe_ddh_multi_sec_key *msk, cfe_ddh_multi *m);

/**
 * Takes master secret key and a matrix y comprised of input vectors, and
 * returns the functional encryption key.
 * In case the key could not be derived, it returns an error.

 * @param res A pointer to an uninitialized cfe_ddh_multi_fe_key struct (the
 * functional encryption key's value will be stored here)
 * @param m A pointer to an instance of the scheme (*initialized* cfe_ddh_multi
 * struct)
 * @param msk A pointer to the master secret key
 * @param y A pointer to the matrix comprised of input vectors
 * @return Error code
 */
cfe_error cfe_ddh_multi_derive_key(cfe_ddh_multi_fe_key *res, cfe_ddh_multi *m, cfe_ddh_multi_sec_key *msk, cfe_mat *y);

/**
 * Generates a ciphertext from the input vector x with the provided public key
 * and one-time pad otp (which is a part of the secret key). It returns the
 * ciphertext vector. If encryption failed, an error is returned.
 *
 * @param ciphertext A pointer to an uninitialized vector (the resulting
 * ciphertext will be stored here)
 * @param e A pointer to an instance of the encryptor (*initialized*
 * cfe_ddh_multi_enc struct)
 * @param x A pointer to the input vector
 * @param pub_key A pointer to the public key vector
 * @param otp A pointer to the one-time pad vector
 * @return Error code
 */
cfe_error cfe_ddh_multi_encrypt(cfe_vec *ciphertext, cfe_ddh_multi_enc *e, cfe_vec *x, cfe_vec *pub_key, cfe_vec *otp);

/**
 * Accepts the matrix cipher comprised of encrypted vectors, functional
 * encryption key, and a matrix y comprised of plaintext vectors. It returns
 * the sum of inner products. If decryption failed, an error is returned.
 *
 * @param res The result of the decryption (the value will be stored here)
 * @param m A pointer to an instance of the scheme (*initialized* cfe_ddh_multi
 * struct)
 * @param ciphertext A pointer to the matrix comprised of encrypted vectors
 * @param key A pointer to the functional encryption key
 * @param y A pointer to the matrix comprised of plaintext vectors
 * @return Error code
 */
cfe_error
cfe_ddh_multi_decrypt(mpz_t res, cfe_ddh_multi *m, cfe_mat *ciphertext, cfe_ddh_multi_fe_key *key, cfe_mat *y);

#endif
