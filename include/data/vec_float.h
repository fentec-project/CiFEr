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
