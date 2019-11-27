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

#ifndef CIFER_MATRIX_H
#define CIFER_MATRIX_H

#include <gmp.h>

#include "cifer/internal/errors.h"
#include "cifer/data/vec.h"

/**
 * \file
 * \ingroup data
 * \brief Matrix struct and operations.
 *
 * As in GMP library, all functions except initialization functions presume that
 * all of the parameters are properly initialized.
 *
 * All functions (unless otherwise specified) store their results (either a GMP
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
 * Sets all the entries of a matrix to equal the given constant.
 *
 * @param m A pointer to an initialized matrix
 * @param c The constant that the entries equal to
 */
void cfe_mat_set_const(cfe_mat *m, mpz_t c);

/**
 * Copies all the entries of a matrix to another matrix. The
 * matrices must have equal dimensions.
 *
 * @param res A pointer to an initialized matrix, matrix will be
 * copied here
 * @param m A pointer to the matrix that will be copied
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
 * Negative of a matrix.
 */
void cfe_mat_neg(cfe_mat *res, cfe_mat *m);

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

/**
 * Returns a matrix obtained from m by removing row i and column j.
 * It returns an error if i >= number of rows of m, or if j >= number of
 * columns of m.
 */
void cfe_mat_extract_submatrix(cfe_mat *m, cfe_mat *min, size_t i, size_t j);

/**
 * Determinant of a square matrix.
 */
void cfe_mat_determinant(mpz_t det, cfe_mat *m);

/**
 * Inverse of a matrix over a modular field Z_mod.
 */
cfe_error cfe_mat_inverse_mod(cfe_mat *inverse_mat, cfe_mat *m, mpz_t mod);

/**
 * Calcualtes x^T * mat * y, for x, y vectors and mat a matrix.
 */
void cfe_mat_mul_x_mat_y(mpz_t res, cfe_mat *mat, cfe_vec *x, cfe_vec *y);

/**
 * Multiplication of a matrix mat and a scalar s.
 */
void cfe_mat_mul_scalar(cfe_mat *res, cfe_mat *mat, mpz_t s);

/**
 * cfe_gaussian_elimination transforms a matrix to an equivalent upper
 * triangular matrix over field Z_p, where p should be a prime number.
 *
 * @param res A pointer to an initialized matrix where the result will be saved
 * @param mat A pointer to a matrix to be transformed
 * @param p Modulus for the computations
 */
void cfe_mat_gaussian_elimination(cfe_mat *res, cfe_mat *mat, mpz_t p);

/**
 * cfe_mat_inverse_mod_gauss calculates the inverse of a matrix over field
 * Z_p, using Gaussian elimination. The latter is faster than the naive (analytic)
 * algorithm. Additionally the determinant of the matrix is returned.
 *
 * @param res A pointer to an initialized matrix where the result will be saved
 * @param det Determinant will be saved here; if not needed this should be NULL
 * @param mat A pointer to a matrix
 * @param p Modulus for the computations
 */
cfe_error cfe_mat_inverse_mod_gauss(cfe_mat *res, mpz_t det, cfe_mat *m, mpz_t p);

/**
 * cfe_mat_determinant_gauss calculates the determinant of a matrix over field
 * Z_p, using Gaussian elimination. The latter is faster than the naive (analytic)
 * algorithm.
 *
 * @param res The result will be saved
 * @param mat A pointer to a matrix
 * @param p Modulus for the computations
 */
void cfe_mat_determinant_gauss(mpz_t det, cfe_mat *m, mpz_t p);

/**
 * cfe_gaussian_elimination_solver solves a matrix vector equation mat * x = v and finds
 * vector x, using Gaussian elimination. Arithmetic operations are considered
 * to be over Z_p, where p should be a prime number. If such x does not exist,
 * then the function returns an 1, else 0.
 *
 * @param res A pointer to an uninitialized vector where the result will be saved
 * @param mat A pointer to the matrix for the equation
 * @param vec A pointer to the right-hand side vector in the equation
 * @param p Modulus for the computations
 * @return Returns CFE_ERR_NO_SOLUTION_EXISTS error if the solution does not
 * exist, else CFE_ERR_NONE for no error
 */
cfe_error cfe_gaussian_elimination_solver(cfe_vec *res, cfe_mat *mat, cfe_vec *vec, mpz_t p);

#endif
