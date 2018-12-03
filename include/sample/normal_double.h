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

#ifndef CIFER_NORMAL_DOUBLE_H
#define CIFER_NORMAL_DOUBLE_H

#include "sample/normal_cumulative.h"
#include "internal/errors.h"

/**
 * \file
 * \ingroup sample
 * \brief Normal double sampler.
 */

/**
 * Samples random values from the normal (Gaussian) probability distribution,
 * centered on 0. This sampler works in a way that first samples from a
 * normal_cumulative with some small sigma and then using another sampling from
 * uniform distribution creates a candidate for the output, which is accepted
 * or rejected with certain probability.
 */
typedef struct cfe_normal_double {
    cfe_normal nor;
    cfe_normal_cumulative sampler_cumu;    // normal_cumulative sampler used in the first part
    mpz_t k;                            // precomputed parameters used for sampling
    mpz_t twice_k;
} cfe_normal_double;

/**
 * Initializes an instance of cfe_normal_double sampler. It assumes mean = 0.
 * Values are precomputed when this function is called, so that
 * normal_double_sample merely returns a precomputed value.
 * sigma should be a multiple of first_sigma. Increasing first_sigma a bit
 * speeds up the algorithm but increases the size of the precomputed values.
 *
 * @param s A pointer to an uninitialized struct representing the sampler
 * @param sigma Standard deviation
 * @param n Precision parameter
 * @param first_sigma Standard deviation for the cumulative sampler
 * @return
 */
cfe_error cfe_normal_double_init(cfe_normal_double *s, mpf_t sigma, size_t n, mpf_t first_sigma);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param s A pointer to an instance of the sampler (*initialized*
 * cfe_normal_double struct)
 */
void cfe_normal_double_free(cfe_normal_double *s);

/**
 * Samples according to discrete Gauss distribution using normal_double and
 * second sampling.
 *
 * @param res The random number (result value will be stored here)
 * @param s A pointer to an instance of the sampler (*initialized*
 * cfe_normal_double struct)
 */
void cfe_normal_double_sample(mpz_t res, cfe_normal_double *s);

/**
 * Sets the elements of the vector to random numbers with the normal_double sampler.
 */
void cfe_normal_double_sample_vec(cfe_vec *res, cfe_normal_double *s);

/**
 * Sets the elements of a matrix to random numbers with the normal_double sampler.
 */
void cfe_normal_double_sample_mat(cfe_mat *res, cfe_normal_double *s);

#endif
