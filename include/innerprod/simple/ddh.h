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

#ifndef CIFER_DDH_H
#define CIFER_DDH_H

#include "data/vec.h"
#include "internal/errors.h"

/**
 * \file
 * \ingroup simple
 * \brief DDH scheme.
 */

/**
 * cfe_ddh represents a scheme instantiated from the DDH assumption.
 */
typedef struct cfe_ddh {
    size_t l;
    mpz_t bound;
    mpz_t g;
    mpz_t p;
} cfe_ddh;

/**
 * Configures a new instance of the scheme.
 * It returns an error in case the scheme could not be properly configured, or
 * if precondition l * bound² is >= order of the cyclic group.
 *
 * @param s A pointer to an uninitialized struct representing the scheme
 * @param l The length of input vectors
 * @param modulus_len The bit length of the modulus
 * @param bound The bound by which coordinates of input vectors are bounded
 * @return Error code
 */
cfe_error cfe_ddh_init(cfe_ddh *s, size_t l, size_t modulus_len, mpz_t bound);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ddh struct)
 */
void cfe_ddh_free(cfe_ddh *s);

/**
 * Reconstructs the scheme with the same configuration parameters from
 * an already existing DDH scheme instance.
 *
 * @param res A pointer to an uninitialized ddh struct
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ddh struct)
 */
void cfe_ddh_copy(cfe_ddh *res, cfe_ddh *s);

/**
 * Generates a pair of master secret key and master public key for the scheme.
 * It returns an error in case master keys could not be generated.
 *
 * @param msk A pointer to an uninitialized vector (master secret key will be
 * stored here)
 * @param mpk A pointer to an uninitialized vector (master public key will be
 * stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ddh struct)
 */
void cfe_ddh_generate_master_keys(cfe_vec *msk, cfe_vec *mpk, cfe_ddh *s);

/**
 * Takes master secret key and input vector y, and returns the functional
 * encryption key. In case the key could not be derived, it returns an error.
 *
 * @param res The derived key (the value will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ddh struct)
 * @param msk A pointer to the master secret key
 * @param y A pointer to the input vector
 * @return Error code
 */
cfe_error cfe_ddh_derive_key(mpz_t res, cfe_ddh *s, cfe_vec *msk, cfe_vec *y);

/**
 * Encrypts input vector x with the provided master public key. It returns a
 * ciphertext vector. If encryption failed, an error is returned.
 *
 * @param ciphertext A pointer to an uninitialized vector (the resulting
 * ciphertext will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ddh struct)
 * @param x A pointer to the input vector
 * @param mpk A pointer to the master public key
 * @return Error code
 */
cfe_error cfe_ddh_encrypt(cfe_vec *ciphertext, cfe_ddh *s, cfe_vec *x, cfe_vec *mpk);

/**
 * Accepts the encrypted vector x, functional encryption key, and a plaintext
 * vector y. It returns the inner product of x and y. If decryption failed, an
 * error is returned.
 *
 * @param res The result of the decryption (the value will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ddh struct)
 * @param ciphertext A pointer to the ciphertext vector
 * @param key The functional encryption key
 * @param y A pointer to the plaintext vector
 * @return Error code
 */
cfe_error cfe_ddh_decrypt(mpz_t res, cfe_ddh *s, cfe_vec *ciphertext, mpz_t key, cfe_vec *y);

#endif
