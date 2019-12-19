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
 * cfe_normal_cumulative samples random values from the cumulative normal (Gaussian)
 * probability distribution, centered on 0.
 * This sampler is the fastest, but is limited only to cases when sigma is not
 * too big, due to the sizes of the precomputed tables.
 */
typedef struct cfe_normal_double_constant {
    mpz_t k;
    mpf_t k_square_inv;
    mpz_t twice_k;
} cfe_normal_double_constant;

void cfe_normal_double_constant_init(cfe_normal_double_constant *s, mpz_t k);

void cfe_normal_double_constant_free(cfe_normal_double_constant *s);

void cfe_normal_double_constant_sample(mpz_t res, cfe_normal_double_constant *s);

#endif
