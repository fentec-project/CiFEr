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

#ifndef CIFER_PRIME_H
#define CIFER_PRIME_H

#include <stdbool.h>
#include <gmp.h>

#include "internal/errors.h"

/**
 * \file
 * \ingroup internal
 * \brief Prime number related functions.
 */

/**
 * Checks whether the number passed as argument is a safe prime.
 *
 * @param p The number to be checked
 * @return true if number is a safe prime, false otherwise
 */
bool cfe_is_safe_prime(mpz_t p);

/**
 * Sets the first argument to a prime with specified bit length.
 * The prime will be a safe prime if the safe parameter is true.
 *
 * @param res The safe prime (result value is stored here)
 * @param bits Bit length of the safe prime
 * @param safe Boolean value if the prime will be safe or not
 * @return Error code
 */
cfe_error cfe_get_prime(mpz_t res, size_t bits, bool safe);

#endif
