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

#ifndef CIFER_VECTOR_H
#define CIFER_VECTOR_H

#include <stdbool.h>
#include <gmp.h>

/**
 * \file
 * \ingroup data
 * \brief Vector struct and operations.
 *
 * As in GMP library, all functions except initialization functions presume that
 * all of the parameters are properly initialized.
 *
 * All functions (unless othewise specified) store their results (either a GMP
 * integer or a vector) to a parameter and do not modify the original vector.
 * Thus, the "result" passed as a parameter must also be properly initialized.
 */

typedef struct cfe_mat cfe_mat;

/**
 * Vector of arbitrary precision (GMP) integers.
 */
typedef struct cfe_vec {
    mpz_t *vec; /** A pointer to the first integer */
    size_t size; /** The size of the vector */
} cfe_vec;

/**
 * Initializes a vector.
 *
 * @param v A pointer to an uninitialized vector
 * @param size The size of the vector
 */
void cfe_vec_init(cfe_vec *v, size_t size);

/**
 * Variadic version of cfe_vec_init.
 * Initializes a NULL-terminated list of vectors.
 */
void cfe_vec_inits(size_t size, cfe_vec *v, ...);

/**
 * Initializes a vector with all the values set to
 * equal a constant.
 *
 * @param vec A pointer to an uninitialized vector
 * @param size The size of the vector
 * @param c The constant that the entries equal to
 */
void cfe_vec_const(cfe_vec *vec, size_t size, mpz_t c);

/**
 * Initializes a vector with all the values set to
 * equal the values of another vector.
 *
 * @param res A pointer to an uninitialized vector
 * @param v A pointer to the vector that will be copied
 */
void cfe_vec_copy(cfe_vec *res, cfe_vec *v);

/**
 * Frees the memory occupied by the contents of the vector.
 */
void cfe_vec_free(cfe_vec *v);

/**
 * Variadic version of cfe_vec_free.
 * Frees a NULL-terminated list of vectors.
 */
void cfe_vec_frees(cfe_vec *v, ...);

/**
 * Prints a vector to standard output.
 */
void cfe_vec_print(cfe_vec *v);

/**
 * Adds two vectors.
 */
void cfe_vec_add(cfe_vec *res, cfe_vec *v1, cfe_vec *v2);

/**
 * Calculates the dot product (inner product) of two vectors.
 */
void cfe_vec_dot(mpz_t res, cfe_vec *v1, cfe_vec *v2);

/**
 * Coordinate-wise modulo.
 */
void cfe_vec_mod(cfe_vec *res, cfe_vec *v, mpz_t modulo);

/**
 * Coordinate-wise multiplication.
 */
void cfe_vec_mul(cfe_vec *res, cfe_vec *v1, cfe_vec *v2);

/**
 * Checks if all coordinates are < bound.
 * @return false if any coordinate is >= bound, true otherwise
 */
bool cfe_vec_check_bound(cfe_vec *v, mpz_t bound);

/**
 * Sets res to the i-th element of the vector.
 */
void cfe_vec_get(mpz_t res, cfe_vec *v, size_t i);

/**
 * Sets the i-th element of the vector to el.
 */
void cfe_vec_set(cfe_vec *v, mpz_t el, size_t i);

/**
 * Joins two vectors into a single vector.
 */
void cfe_vec_join(cfe_vec *res, cfe_vec *v1, cfe_vec *v2);

/**
 * Extract n elements of the vector starting at index from.
 */
void cfe_vec_extract(cfe_vec *res, cfe_vec *v, size_t from, size_t n);

/**
 * Appends element el to the end of the vector.
 */
void cfe_vec_append(cfe_vec *res, cfe_vec *v, mpz_t el);

/**
 * Multiplication of a vector transposed by a matrix.
 */
void cfe_vec_mul_matrix(cfe_vec *res, cfe_vec *v, cfe_mat *m);

/**
 * Multiplication of a vector with a scalar.
 */
void cfe_vec_mul_scalar(cfe_vec *res, cfe_vec *v, mpz_t s);

/**
 * Multiplication of two vectors representing polynomials in Z[x] / (x^n + 1)
 */
void cfe_vec_poly_mul(cfe_vec *res, cfe_vec *v1, cfe_vec *v2);

/**
 * Coordinate-wise negation.
 */
void cfe_vec_neg(cfe_vec *res, cfe_vec *v);

/**
 * Coordinate-wise division with floor rounding.
 */
void cfe_vec_fdiv_q_scalar(cfe_vec *res, cfe_vec *v, mpz_t s);

/**
 * Recursive implementation of FFT, assuming the length of a is a power of 2.
 */
void cfe_vec_FFT(cfe_vec *y, cfe_vec *a, mpz_t root, mpz_t q);

/**
 * Multiplication of two vectors representing polynomials in Z_q[x] using FFT.
 */
void cfe_vec_poly_mul_FFT(cfe_vec *res, cfe_vec *v1, cfe_vec *v2, mpz_t root, mpz_t inv_root, mpz_t inv_n, mpz_t q);

#endif
