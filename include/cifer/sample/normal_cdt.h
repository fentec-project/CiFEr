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

#ifndef CIFER_NORMAL_CDT_H
#define CIFER_NORMAL_CDT_H

#include <gmp.h>

/**
 * \file
 * \ingroup sample
 * \brief Normal sampler that samples from small fixed half-Gaussian distribution.
 */

/**
 * cfe_sigma_cdt is a constant 1/(2ln(2)).
 */
static const double cfe_sigma_cdt = 0.84932180028801904272150283410;

/**
* cfe_normal_cdt_sample samples random values from the discrete Normal (Gaussian)
* probability distribution, limited to non-negative values (half-Gaussian).
* In particular each value x from Z^+ is sampled with probability proportional to
* exp(-x^2/sigma^2) where sigma = 1/2ln(2).
* The implementation is based on paper:
* "FACCT: FAst, Compact, and Constant-Time Discrete Gaussian
* Sampler over Integers" by R. K. Zhao, R. Steinfeld, and A. Sakzad
* (https://eprint.iacr.org/2018/1234.pdf). See the above paper where
* it is argued that such a sampling achieves a relative error at most
* 2^{-46} with the chosen parameters.
*
* @param res The result of sampling will be saved here
*/
void cfe_normal_cdt_sample(mpz_t res);

#endif
