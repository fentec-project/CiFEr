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

#ifndef CIFER_NORMAL_NEGATIVE_H
#define CIFER_NORMAL_NEGATIVE_H

#include "sample/normal.h"

/**
 * \file
 * \ingroup sample
 * \brief Normal negative sampler.
 */

/**
 * Samples random values from the possible outputs of normal (Gaussian)
 * probability distribution centered on 0 and accepts or denies each sample
 * with probability defined by the distribution.
 */
typedef struct cfe_normal_negative {
    cfe_normal nor;
    mpz_t cut;                  // cut defines from which interval we sample
    mpz_t twice_cut_plus_one;   // precomputed value so we do not need to calculate it each time
} cfe_normal_negative;

/**
 * Initializes an instance of cfe_normal_negative sampler. It assumes mean = 0.
 *
 * @param s A pointer to an uninitialized struct representing the sampler
 * @param sigma Standard deviation
 * @param n Precision parameter
 */
void cfe_normal_negative_init(cfe_normal_negative *s, mpf_t sigma, size_t n);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param s A pointer to an instance of the sampler (*initialized*
 * cfe_normal_negative struct)
 */
void cfe_normal_negative_free(cfe_normal_negative *s);

/**
 * Samples the discrete distribution.
 *
 * @param res The random number (result value will be stored here)
 * @param s A pointer to an instance of the sampler (*initialized*
 * cfe_normal_negative struct)
 */
void cfe_normal_negative_sample(mpz_t res, cfe_normal_negative *s);

/**
 * Sets the elements of the vector to random numbers with the normal_negative sampler.
 */
void cfe_normal_negative_sample_vec(cfe_vec *res, cfe_normal_negative *s);

/**
 * Sets the elements of a matrix to random numbers with the normal_negative sampler.
 */
void cfe_normal_negative_sample_mat(cfe_mat *res, cfe_normal_negative *s);

#endif
