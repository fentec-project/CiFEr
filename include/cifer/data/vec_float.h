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

#ifndef CIFER_VECTOR_FLOAT_H
#define CIFER_VECTOR_FLOAT_H

#include <gmp.h>

/**
 * \file
 * \ingroup data
 * \brief Vector float struct
 *
 * Similar to the vector struct, but for floating point numbers.
 */


/**
 * Vector of arbitrary precision (GMP) floats.
 */
typedef struct cfe_vec_float {
    mpf_t *vec; /** A pointer to the first float */
    size_t size; /** The size of the vector */
} cfe_vec_float;

/**
 * Initializes a vector_float.
 *
 * @param v A pointer to an uninitialized vector_float
 * @param size The size of the vector_float
 * @param prec Precision
 */
void cfe_vec_float_init(cfe_vec_float *v, size_t size, size_t prec);

/**
 * Frees the memory occupied by the contents of the vector_float.
 */
void cfe_vec_float_free(cfe_vec_float *v);

/**
 * Sets res to the i-th element of the vector.
 */
void cfe_vec_float_get(mpf_t res, cfe_vec_float *v, size_t i);

#endif
