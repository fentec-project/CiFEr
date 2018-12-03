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

#ifndef CIFER_NORMAL_H
#define CIFER_NORMAL_H

#include <gmp.h>
#include "data/vec.h"
#include "data/vec_float.h"

/**
 * \file
 * \ingroup sample
 * \brief Normal sampler
 */

/**
 * normal samples random values from the Normal (Gaussian) probability
 * distribution, centered on 0.
 */
typedef struct cfe_normal {
    mpf_t sigma;            // Standard deviation
    size_t n;               // Precision parameter
    cfe_vec_float pre_exp;  // Precomputed values of exponential function with precision n
    mpz_t pow_n;            // Precomputed values for quicker sampling
    mpf_t pow_nf;
} cfe_normal;

/**
 * Initializes an instance of cfe_normal sampler. It assumes mean = 0.
 */
void cfe_normal_init(cfe_normal *s, mpf_t sigma, size_t n);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 */
void cfe_normal_free(cfe_normal *s);

/**
 * Precomputes the values of exp(-2^i / 2 * sigma^2) needed for sampling
 * discrete Gauss distribution with standard deviation sigma to arbitrary
 * precision. This is needed since such computations present one of the
 * bottlenecks of the computation. Values are precomputed in the interval
 * 0 <= i < sigma^2 * sqrt(n) since for greater i the results are negligible.
 */
void cfe_normal_precomp_exp(cfe_normal *s);

/**
 * Outputs if y > exp(-x/(2*sigma^2)) with minimal calculation of
 * exp(-x/(2*sigma^2)) based on the precomputed values. Sigma is implicit in
 * the precomputed values saved in s.
 */
bool cfe_normal_is_exp_greater(cfe_normal *s, mpf_t y, mpz_t x);

/**
 * Approximates exp(-x/alpha) with taylor polynomial of degree k, precise at
 * least up to 2^-n.
 */
void cfe_taylor_exp(mpf_t res, mpz_t x, mpf_t alpha, size_t k, size_t n);

/**
 * Calculates the mean of a vector of integers.
 */
void cfe_mean(mpf_t res, cfe_vec *vec);

/**
 * Calculates the variance of a vector of integers.
 */
void cfe_variance(mpf_t res, cfe_vec *vec);

#endif
