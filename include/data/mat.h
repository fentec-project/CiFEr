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

#ifndef CIFER_MATRIX_H
#define CIFER_MATRIX_H

#include <gmp.h>

#include "data/vec.h"

/**
 * \file
 * \ingroup data
 * \brief Matrix struct and operations.
 *
 * As in GMP library, all functions except initialization functions presume that
 * all of the parameters are properly initialized.
 *
 * All functions (unless othewise specified) store their results (either a GMP
 * integer, a vector or a matrix) to a parameter and do not modify the
 * original matrix.
 * Thus, the "result" passed as a parameter must also be properly initialized.
 */

/**
 * Matrix of arbitrary precision (GMP) integers.
 * It represents a row-major matrix. A matrix of dimensions i, j consists of i
 * vectors, each consisting of j elements.
 */
typedef struct cfe_mat {
    cfe_vec *mat; /** The pointer to the first vector */
    size_t rows; /** The number or rows (vectors) */
    size_t cols; /** The number of columns (size of vectors */
} cfe_mat;

/**
 * Initializes a matrix.
 *
 * @param m A pointer to the uninitialized matrix
 * @param rows The number of rows
 * @param cols The number of columns
 */
void cfe_mat_init(cfe_mat *m, size_t rows, size_t cols);

/**
 * Variadic version of cfe_mat_init.
 * Initializes a NULL-terminated list of matrices.
 */
void cfe_mat_inits(size_t rows, size_t cols, cfe_mat *m, ...);

/**
 * Creates a matrix with all its entries equal to a given constant.
 *
 * @param m A pointer to the uninitialized matrix
 * @param rows The number of rows
 * @param cols The number of columns
 * @param c The constant that the entries equal to
 */
void cfe_mat_const(cfe_mat *m, size_t rows, size_t cols, mpz_t c);

/**
 * Initializes a matrix with all the values set to
 * equal the values of another matrix.
 *
 * @param res A pointer to an uninitialized matrix
 * @param v A pointer to the matrix that will be copied
 */
void cfe_mat_copy(cfe_mat *res, cfe_mat *m);

/**
 * Frees the memory occupied by the contents of the matrix.
 */
void cfe_mat_free(cfe_mat *m);

/**
 * Variadic version of cfe_mat_free.
 * Frees a NULL-terminated list of matrices.
 */
void cfe_mat_frees(cfe_mat *m, ...);

/**
 * Prints a matrix to standard output.
 */
void cfe_mat_print(cfe_mat *m);

/**
 * Adds two matrices.
 */
void cfe_mat_add(cfe_mat *res, cfe_mat *m1, cfe_mat *m2);

/**
 * Calculates the dot (inner) product of matrices.
 */
void cfe_mat_dot(mpz_t res, cfe_mat *m1, cfe_mat *m2);

/**
 * Component-wise modulo.
 */
void cfe_mat_mod(cfe_mat *res, cfe_mat *m, mpz_t modulo);

/**
 * Multiplication of a matrix by a vector.
 */
void cfe_mat_mul_vec(cfe_vec *res, cfe_mat *m, cfe_vec *v);

/**
 * Matrix multiplication.
 */
void cfe_mat_mul(cfe_mat *res, cfe_mat *m1, cfe_mat *m2);

/**
 * Checks if all elements are < bound.
 * @return false if any element is >= bound, true otherwise
 */
bool cfe_mat_check_bound(cfe_mat *m, mpz_t bound);

/**
 * Sets res to the j-th element of the i-th row of the matrix.
 */
void cfe_mat_get(mpz_t res, cfe_mat *m, size_t i, size_t j);

/**
 * Sets res to the i-th column of the matrix.
 */
void cfe_mat_get_col(cfe_vec *res, cfe_mat *m, size_t i);

/**
 * Sets res to the i-th row of the matrix.
 */
void cfe_mat_get_row(cfe_vec *res, cfe_mat *m, size_t i);

/**
 * Returs the pointer to the i-th row of the matrix.
 */
cfe_vec *cfe_mat_get_row_ptr(cfe_mat *m, size_t i);

/**
 * Sets the j-th element of the i-th row of the matrix to el.
 */
void cfe_mat_set(cfe_mat *m, mpz_t el, size_t i, size_t j);

/**
 * Sets the i-th row of the matrix to v.
 */
void cfe_mat_set_vec(cfe_mat *m, cfe_vec *v, size_t i);

/**
 * Creates a vector from matrix elements by concatenating rows into a single
 * vector.
 */
void cfe_mat_to_vec(cfe_vec *res, cfe_mat *m);

/**
 * Constructs a matrix of dimensions rows * cols from v. The vector must hold
 * exactly rows * cols elements.
 */
void cfe_mat_from_vec(cfe_mat *m, cfe_vec *v);

/**
 * Transposes the matrix.
 */
void cfe_mat_transpose(cfe_mat *t, cfe_mat *m);

#endif
