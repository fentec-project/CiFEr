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

#ifndef CIFER_KEYGEN_H
#define CIFER_KEYGEN_H

#include <gmp.h>

#include "internal/errors.h"

/**
 * \file
 * \ingroup internal
 * \brief ElGamal key generation
 */

typedef struct cfe_elgamal {
    mpz_t p; // Modulus
    mpz_t g; // Generator of the cyclic group
    mpz_t q; // (p - 1) / 2, i.e. order of g
} cfe_elgamal;


/**
 * Initializes the cfe_elgamal struct.
 *
 * @param key A pointer to an uninitialized cfe_elgamal struct
 * @param modulus_len The bit length of the modulus
 * @return Error code
 */
cfe_error cfe_elgamal_init(cfe_elgamal *key, size_t modulus_len);

/**
 * Frees the memory occupied by the members of the struct.
 *
 * @param key A pointer to an initialized cfe_elgamal struct
 */
void cfe_elgamal_free(cfe_elgamal *key);

#endif
