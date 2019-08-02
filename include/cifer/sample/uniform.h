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

#ifndef CIFER_UNIFORM_H
#define CIFER_UNIFORM_H

#include <stdbool.h>
#include <gmp.h>

#include "cifer/data/vec.h"
#include "cifer/data/mat.h"

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

void cfe_uniform_sample_vec_det(cfe_vec *res, mpz_t max, unsigned char *key);

void cfe_uniform_sample_mat_det(cfe_mat *res, mpz_t max, unsigned char *key);

#endif
