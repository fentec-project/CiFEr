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

#ifndef CIFER_PAILLIER_H
#define CIFER_PAILLIER_H

#include "data/vec.h"
#include "internal/errors.h"

/**
 * \file
 * \ingroup fullysec
 * \brief Paillier scheme.
 */

/**
 * cfe_paillier represents common properties of the scheme.
 */
typedef struct cfe_paillier {
    size_t l;
    mpz_t n;
    mpz_t n_square;
    mpz_t bound_x;
    mpz_t bound_y;
    mpf_t sigma;
    size_t lambda;
    mpz_t g;
} cfe_paillier;

/**
* Initializes a new instance of the scheme.
* It accepts the length of input vectors l, security parameter lambda,
* the bit length of prime numbers (giving security to the scheme, it
* should be such that factoring two primes with such a bit length takes
* at least 2^lambda operations), and boundX and boundY by which
* coordinates of input vectors and inner product vectors are bounded.
* It returns an error in the case the scheme could not be properly
* configured, or if the precondition boundX, boundY < (n / l)^(1/2)
* is not satisfied.
 *
* @param s A pointer to an uninitialized struct representing the scheme
* @param l The length of input vectors
* @param lambda The security parameter
* @param bit_len Number of bits for generating prime numbers
* @param bound_x The bound by which the coordinates of encrypted vectors are bounded
* @param bound_y The bound by which the coordinates of inner product vectors are bounded
* @return Error code
*/
cfe_error cfe_paillier_init(cfe_paillier *s, size_t l, size_t lambda, size_t bit_len, mpz_t bound_x, mpz_t bound_y);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param s A pointer to an instance of the scheme (*initialized* cfe_paillier
 * struct)
 */
void cfe_paillier_free(cfe_paillier *s);

/**
 * Reconstructs the scheme with the same configuration parameters from
 * an already existing Paillier scheme instance.
 *
 * @param res A pointer to an uninitialized paillier struct
 * @param s A pointer to an instance of the scheme (*initialized* cfe_paillier
 * struct)
 */
void cfe_paillier_copy(cfe_paillier *res, cfe_paillier *s);

/**
 * Initializes the vectors which represent the master secret key and master
 * public key.
 *
 * @param msk A pointer to an uninitialized vector
 * @param mpk A pointer to an uninitialized vector
 * @param s A pointer to an instance of the scheme (*initialized* cfe_paillier
 * struct)
 */
void cfe_paillier_master_keys_init(cfe_vec *msk, cfe_vec *mpk, cfe_paillier *s);

/**
 * Generates a master secret key and master public key for the scheme.
 *
 * @param msk A pointer to a vector (master secret key will be stored here)
 * @param mpk A pointer to a vector (master public key will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_paillier
 * struct)
 * @return Error code
 */
cfe_error cfe_paillier_generate_master_keys(cfe_vec *msk, cfe_vec *mpk, cfe_paillier *s);

/**
 * Takes master secret key and input vector y, and returns the functional
 * encryption key. In case the key could not be derived, it returns an error.
 *
 * @param derived_key The resulting functional encryption key (the value will be
 * stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_paillier
 * struct)
 * @param msk A pointer to the master secret key
 * @param y A pointer to the inner product vector
 * @return Error code
 */
cfe_error cfe_paillier_derive_key(mpz_t derived_key, cfe_paillier *s, cfe_vec *msk, cfe_vec *y);

/**
 * Initializes the vector which represents the ciphertext.
 *
 * @param ciphertext A pointer to an uninitialized vector
 * @param s A pointer to an instance of the scheme (*initialized* cfe_paillier
 * struct)
 */
void cfe_paillier_ciphertext_init(cfe_vec *ciphertext, cfe_paillier *s);

/**
 * Encrypts input vector x with the provided master public key. It returns a
 * ciphertext vector. If encryption failed, an error is returned.
 *
 * @param ciphertext A pointer to a vector (the resulting ciphertext will be
 * stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_paillier
 * struct)
 * @param x A pointer to the input vector
 * @param mpk A pointer to the master public key
 * @return Error code
 */
cfe_error cfe_paillier_encrypt(cfe_vec *ciphertext, cfe_paillier *s, cfe_vec *x, cfe_vec *mpk);

/**
 * Accepts the encrypted vector, functional encryption key, and a plaintext
 * vector y. It returns the inner product of x and y. If decryption failed, an
 * error is returned.
 *
 * @param res The result of the decryption (the value will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_paillier
 * struct)
 * @param ciphertext A pointer to the ciphertext vector
 * @param key The functional encryption key
 * @param y A pointer to the inner product vector
 * @return Error code
 */
cfe_error cfe_paillier_decrypt(mpz_t res, cfe_paillier *s, cfe_vec *ciphertext, mpz_t key, cfe_vec *y);

#endif
