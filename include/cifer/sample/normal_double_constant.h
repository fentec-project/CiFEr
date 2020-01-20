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

#ifndef CIFER_NORMAL_DOUBLE_CONSTANT_H
#define CIFER_NORMAL_DOUBLE_CONSTANT_H

#include "normal.h"

/**
 * \file
 * \ingroup sample
 * \brief Normal constant time sampler based on double sampling.
 */

/**
 * cfe_normal_double_constant samples random values from the normal (Gaussian)
 * probability distribution, centered on 0. This sampler works in a way that it
 * first samples from a cfe_normal_cdt - normal distribution with fixed sigma -
 * and then using another sampling from uniform distribution creates a
 * candidate for the output, which is accepted or rejected with certain
 * probability. The sampler algorithm is constant time in the
 * sense that the sampled value is independent of the time needed.
 * The implementation is based on paper:
 * "FACCT: FAst, Compact, and Constant-Time Discrete Gaussian Sampler
 * over Integers" by R. K. Zhao, R. Steinfeld, and A. Sakzad,
 * see https://eprint.iacr.org/2018/1234.pdf.
 * See the above paper for the argumentation of the choice of
 * parameters and proof of precision and security.
 */
typedef struct cfe_normal_double_constant {
    mpz_t k;
    mpf_t k_square_inv;
    mpz_t twice_k;
} cfe_normal_double_constant;

/**
 * Initializes an instance of cfe_normal_double_constant sampler. It assumes
 * mean = 0. Parameter k needs to be given, such that sigma = k * sqrt(1/(2ln(2))).
 *
 * @param s A pointer to an uninitialized struct representing the sampler
 * @param k The value determining sigma = k * sqrt(1/(2ln(2)))
 */
void cfe_normal_double_constant_init(cfe_normal_double_constant *s, mpz_t k);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param s A pointer to an instance of the sampler (*initialized*
 * cfe_normal_double_constant struct)
 */
void cfe_normal_double_constant_free(cfe_normal_double_constant *s);

/**
 * Samples according to discrete Gaussian distribution using
 * cfe_normal_double_constant.
 *
 * @param res The sampling value (result value will be stored here)
 * @param s A pointer to an instance of the sampler (*initialized*
 * cfe_normal_double_constant struct)
 */
void cfe_normal_double_constant_sample(mpz_t res, cfe_normal_double_constant *s);

/**
 * Sets the elements of the vector to random numbers with the cfe_normal_double_constant sampler.
 */
void cfe_normal_double_constant_sample_vec(cfe_vec *res, cfe_normal_double_constant *s);

/**
 * Sets the elements of a matrix to random numbers with the cfe_normal_double_constant sampler.
 */
void cfe_normal_double_constant_sample_mat(cfe_mat *res, cfe_normal_double_constant *s);

#endif
