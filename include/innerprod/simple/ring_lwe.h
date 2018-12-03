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

#ifndef CIFER_RING_LWE_H
#define CIFER_RING_LWE_H

#include <math.h>
#include <gmp.h>

#include "data/vec.h"
#include "data/mat.h"
#include "internal/errors.h"
#include "sample/normal_cumulative.h"

/**
 * \file
 * \ingroup simple
 * \brief LWE scheme.
 */

/**
 * cfe_ring_lwe represents common properties of the scheme.
 */
typedef struct cfe_ring_lwe {
    size_t l;    // Length of data vectors for inner product

    size_t n;    // main security parameters of the scheme

    mpz_t bound; // Data vector coordinates should be strictly smaller than bound

    mpz_t p;  // Modulus for message space
    mpz_t q;  // Modulus for ciphertext and keys

    // vector a of dimension n representing the random polynomial for the scheme
    cfe_vec a;

    // sampler
    cfe_normal_cumulative sampler;
} cfe_ring_lwe;

// TODO: this scheme needs automatic parameters generation and the input should
// depend only on n, l, bound_x, bound_y (i.e. also bound x and bound_y should be separated),
// this needs some theoretical effort.

/**
 * Configures a new instance of the scheme.
 *
 * @param s A pointer to an uninitialized struct representing the scheme
 * @param l The length of input vectors
 * @param n The security parameter of the scheme
 * @param bound The bound by which coordinates of the input vectors are bounded
 * @param p Modulus for the inner product
 * @param q Modulus for ciphertext and keys
 * vectors are bounded
 * @param sigma Standard deviation
 * @return Error code
 */
cfe_error cfe_ring_lwe_init(cfe_ring_lwe *s, size_t l, size_t n, mpz_t bound, mpz_t p, mpz_t q, mpf_t sigma);

/**
 * Generates a private secret key for the scheme.
 *
 * @param SK A pointer to an uninitialized matrix (master
 * secret key will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ring_lwe
 * struct)
 */
void cfe_ring_lwe_generate_sec_key(cfe_mat *SK, cfe_ring_lwe *s);

/**
 * Generates a public key for the scheme.
 *
 * @param PK A pointer to an uninitialized instance of pub_key
 * struct (public key will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ring_lwe
 * struct)
 * @param SK A pointer to an initialized matrix representing the secret key.
 * @return Error code
 */
cfe_error cfe_ring_lwe_generate_pub_key(cfe_mat *PK, cfe_ring_lwe *s, cfe_mat *SK);

/**
 * Takes master secret key and inner product vector, and returns the functional
 * encryption key.
 *
 * @param sk_y A pointer to an uninitialized vector (the functional encryption
 * key will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized*
 * cfe_ring_lwe struct)
 * @param SK A pointer to the master secret key
 * @param y A pointer to the inner product vector
 * @return Error code
 */
cfe_error cfe_ring_lwe_derive_key(cfe_vec *sk_y, cfe_ring_lwe *s, cfe_mat *SK, cfe_vec *y);

/**
 * Encrypts input matrix x with the provided master public key.
 *
 * @param ct A pointer to an uninitialized matrix
 * (the resulting ciphertext will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ring_lwe
 * struct)
 * @param x A pointer to the input matrix
 * @param PK A pointer to the matrix representing the public key.
 * @return Error code
 */
cfe_error cfe_ring_lwe_encrypt(cfe_mat *ct, cfe_ring_lwe *s, cfe_mat *x, cfe_mat *PK);

/**
 * Accepts the encrypted matrix X, functional encryption key, and an inner
 * product vector y. It returns the product y^T * X. If decryption failed, an
 * error is returned.
 *
 * @param res An uninitialized vector (result of the decryption will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ring_lwe struct)
 * @param ct A pointer to the ciphertext matrix
 * @param sk_y The functional encryption key
 * @param y A pointer to the inner product vector
 * @return Error code
 */
cfe_error cfe_ring_lwe_decrypt(cfe_vec *res, cfe_ring_lwe *s, cfe_mat *ct, cfe_vec *sk_y, cfe_vec *y);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param s A pointer to an instance of the scheme (*initialized* cfe_ring_lwe
 * struct)
 */
void cfe_ring_lwe_free(cfe_ring_lwe *s);

#endif
