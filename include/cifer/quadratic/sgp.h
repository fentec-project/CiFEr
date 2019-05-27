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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CIFER_SGP_H
#define CIFER_SGP_H

#include "cifer/data/vec.h"
#include "cifer/internal/errors.h"
#include "cifer/data/vec_curve.h"

/**
 * \file
 * \ingroup quadratic
 * \brief SGP scheme.
 */

/**
 * cfe_sgp represents a scheme for quadratic multi-variate polynomials
// based on  Sans, Gay and Pointcheval:
// "Reading in the Dark: Classifying Encrypted Digits with
// Functional Encryption", see https://eprint.iacr.org/2018/206.pdf.
 */
typedef struct cfe_sgp {
    size_t l;
    mpz_t bound;
    mpz_t mod;
    BIG_256_56 mod_big;
} cfe_sgp;

/**
 * cfe_sgp_sec_key represents a secret key for SGP scheme.
 */
typedef struct cfe_sgp_sec_key {
    cfe_vec s;
    cfe_vec t;
} cfe_sgp_sec_key;

/**
 * cfe_sgp_sec_key represents a ciphertext in SGP scheme.
 */
typedef struct cfe_sgp_cipher {
    ECP_BN254 g1MulGamma;
    cfe_vec_G1 *a;
    cfe_vec_G2 *b;
    size_t l;
} cfe_sgp_cipher;

/**
 * Configures a new instance of the scheme.
 *
 * @param s A pointer to an uninitialized struct representing the scheme
 * @param l The length of input vectors
 * @param bound The bound by which coordinates of the encryption vectors
 * and matrix for quadratic polynomial are bounded
 * @return Error code
 */
cfe_error cfe_sgp_init(cfe_sgp *s, size_t l, mpz_t bound);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param s A pointer to an instance of the scheme (*initialized* cfe_sgp
 * struct)
 */
void cfe_sgp_free(cfe_sgp *s);

/**
 * Initializes the struct which represents the master secret key.
 *
 * @param msk A pointer to an uninitialized secret key
 * @param sgp A pointer to an instance of the scheme (*initialized* cfe_sgp
 * struct)
 */
void cfe_sgp_sec_key_init(cfe_sgp_sec_key *msk, cfe_sgp *sgp);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param msk A pointer to an instance of the secret key (*initialized*
 * cfe_sgp_sec_key struct)
 */
void cfe_sgp_sec_key_free(cfe_sgp_sec_key *msk);

/**
 * Generates a private secret key for the scheme.
 *
 * @param msk A pointer to a secret key (master secret key will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_sgp
 * struct)
 * @return Error code
 */
void cfe_sgp_generate_sec_key(cfe_sgp_sec_key *msk, cfe_sgp *s);

/**
 * Takes master secret key and a matrix, and returns the corresponding functional
 * encryption key.
 *
 * @param key A pointer to a ECP2_BN254 struct (the functional encryption
 * key will be stored here)
 * @param sgp A pointer to an instance of the scheme (*initialized*
 * cfe_sgp struct)
 * @param msk A pointer to the master secret key
 * @param f A pointer to the matrix for the quadratic polynomial
 * @return Error code
 */
cfe_error cfe_sgp_derive_key(ECP2_BN254 *key, cfe_sgp *sgp, cfe_sgp_sec_key *msk, cfe_mat *f);

/**
 * Initializes the struct which represents the ciphertext.
 *
 * @param cipher A pointer to an uninitialized ciphertext struct
 * @param s A pointer to an instance of the scheme (*initialized* cfe_sgp
 * struct)
 */
void cfe_sgp_cipher_init(cfe_sgp_cipher *cipher, cfe_sgp *s);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param cipher A pointer to an instance of the ciphertext (*initialized*
 * cfe_sgp_cipher struct)
 */
void cfe_sgp_cipher_free(cfe_sgp_cipher *cipher);

/**
 * Encrypts input vectors x and y with the provided master secret key.
 *
 * @param cipher A pointer to a vector (the resulting ciphertext will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_sgp
 * struct)
 * @param x A pointer to the input vector
 * @param y A pointer to the input vector
 * @param msk A pointer to a master secret key.
 * @return Error code
 */
cfe_error cfe_sgp_encrypt(cfe_sgp_cipher *cipher, cfe_sgp *s, cfe_vec *x, cfe_vec *y, cfe_sgp_sec_key *msk);

/**
 * Based on the encryption, functional encryption key, and a matrix,
 * it returns the value of the quadratic polynomial. If decryption failed, an
 * error is returned.
 *
 * @param res The result of the decryption (the value will be stored here)
 * @param s A pointer to an instance of the scheme (*initialized* cfe_sgp
 * struct)
 * @param cipher A pointer to the ciphertext
 * @param key A pointer to the functional encryption key
 * @param f A pointer to the matrix of the quadratic polynomial
 * @return Error code
 */
cfe_error cfe_sgp_decrypt(mpz_t res, cfe_sgp *s, cfe_sgp_cipher *cipher, ECP2_BN254 *key, cfe_mat *f);

#endif
