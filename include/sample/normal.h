/*
 * Copyright (c) 2018 XLAB d.o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
