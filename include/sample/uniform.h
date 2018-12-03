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

#ifndef CIFER_UNIFORM_H
#define CIFER_UNIFORM_H

#include <stdbool.h>
#include <gmp.h>

#include "data/vec.h"
#include "data/mat.h"

/**
 * \file
 * \ingroup sample
 * \brief Uniform samplers
 */

/**
 * Returns a random number from the range [0, upper). Relies on OS's entropy
 * source and is cryptographically secure.
 *
 * @param res The random number (result value will be stored here)
 * @param upper Upper bound for sampling
 */
void cfe_uniform_sample(mpz_t res, mpz_t upper);

/**
 * Integer version of uniform_sample.
 */
void cfe_uniform_sample_i(mpz_t res, size_t upper);

/**
 * Returns a random number from the range [min, max). Relies on OS's entropy
 * source and is cryptographically secure.
 *
 * @param res The random number (result value will be stored here)
 * @param min Minimum value for sampling
 * @param max Maximum value for sampling
 */
void cfe_uniform_sample_range(mpz_t res, mpz_t min, mpz_t max);

/**
 * Integer version of uniform_sample_range.
 */
void cfe_uniform_sample_range_i_mpz(mpz_t res, int min, mpz_t max);

/**
 * Integer version of uniform_sample_range.
 */
void cfe_uniform_sample_range_i_i(mpz_t res, int min, int max);

/**
 * Sets the elements of a vector to uniform random integers < max.
 */
void cfe_uniform_sample_vec(cfe_vec *res, mpz_t max);

/**
 * Sets the elements of a vector to uniform random integers in [lower, upper).
 */
void cfe_uniform_sample_range_vec(cfe_vec *res, mpz_t lower, mpz_t upper);

/**
 * Sets the elements of a matrix to uniform random integers < max.
 */
void cfe_uniform_sample_mat(cfe_mat *res, mpz_t max);

/**
 * Sets the elements of a matrix to uniform random integers in [lower, upper).
 */
void cfe_uniform_sample_range_mat(cfe_mat *res, mpz_t lower, mpz_t upper);

/**
 * Returns a random boolean value.
 *
 * @return Random boolean value
 */
bool cfe_bit_sample(void);

#endif
