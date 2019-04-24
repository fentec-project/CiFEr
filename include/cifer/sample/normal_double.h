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

#ifndef CIFER_NORMAL_DOUBLE_H
#define CIFER_NORMAL_DOUBLE_H

#include "cifer/sample/normal_cumulative.h"
#include "cifer/internal/errors.h"

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
