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

#ifndef CIFER_MATRIX_CURVE_H
#define CIFER_MATRIX_CURVE_H

#include <stdlib.h>
#include "cifer/data/mat.h"
#include "cifer/data/vec_curve.h"

/**
 * \file
 * \ingroup data
 * \brief Matrices of elements of an elliptic curve struct and operations on it.
 */

/**
 * Matrix of ECP_BN254 elements.
 */
typedef struct cfe_mat_G1 {
    cfe_vec_G1 *mat; /** A pointer to the first element */
    size_t rows;     /** The rows of the matrix */
    size_t cols;     /** The cols of the matrix */
} cfe_mat_G1;

/**
 * Matrix of ECP2_BN254 elements.
 */
typedef struct cfe_mat_G2 {
    cfe_vec_G2 *mat; /** A pointer to the first element */
    size_t rows;     /** The rows of the matrix */
    size_t cols;     /** The cols of the matrix */
} cfe_mat_G2;

/**
 * Matrix of FP12_BN254 elements.
 */
typedef struct cfe_mat_GT {
    cfe_vec_GT *mat; /** A pointer to the first element */
    size_t rows;     /** The rows of the matrix */
    size_t cols;     /** The cols of the matrix */
} cfe_mat_GT;

/**
 * Initializes a matrix of ECP_BN254 elements
 *
 * @param m A pointer to an uninitialized cfe_mat_G1 matrix
 * @param rows The number of rows
 * @param cols The number of columns
 */
void cfe_mat_G1_init(cfe_mat_G1 *m, size_t rows, size_t cols);

/**
 * Transposes the given cfe_mat_G1 matrix
 * 
 * @param res A pointer to an initialized cfe_mat_G1 matrix
 * @param m A pointer to a cfe_mat_G1 matrix
 */
void cfe_mat_G1_transpose(cfe_mat_G1 *res, cfe_mat_G1 *m);

/**
 * Creates a matrix whose i-th element equals u[i] * g,
 * where g is the generator of ECP_BN254. The sizes of
 * m and u must match.
 *
 * @param m A pointer to an initialized cfe_mat_G1 matrix
 * @param u A pointer to matrix of integers
 */
void cfe_mat_mul_G1(cfe_mat_G1 *m, cfe_mat *u);

/**
 * Multiplication of a matrix of integers by a matrix of elements of G1
 *
 * @param res A pointer to an initialized cfe_mat_G1 matrix
 * @param mi A pointer to matrix of integers
 * @param u A pointer to an initialized cfe_mat_G1 matrix
 */
void cfe_mat_mul_G1_mat(cfe_mat_G1 *res, cfe_mat *mi, cfe_mat_G1 *m);

/**
 * Multiplication of a matrix of elements of G1 by a vector of integers.
 * 
 * @param res A pointer to an initialized cfe_vec_G1 vector
 * @param m A pointer to a cfe_mat_G1 matrix
 * @param u A pointer to matrix of integers
 */
void cfe_mat_G1_mul_vec(cfe_vec_G1 *res, cfe_mat_G1 *m, cfe_vec *u);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param m A pointer to a matrix (*initialized*
 * cfe_mat_G1 struct)
 */
void cfe_mat_G1_free(cfe_mat_G1 *m);

/**
 * Initializes a matrix of ECP2_BN254 elements
 *
 * @param m A pointer to an uninitialized cfe_mat_G2 matrix
 * @param rows The number of rows
 * @param cols The number of columns
 */
void cfe_mat_G2_init(cfe_mat_G2 *m, size_t rows, size_t cols);

/**
 * Transposes the given cfe_mat_G2 matrix.
 * 
 * @param res A pointer to an initialized cfe_mat_G2 matrix
 * @param m A pointer to a cfe_mat_G2 matrix
 */
void cfe_mat_G2_transpose(cfe_mat_G2 *res, cfe_mat_G2 *m);

/**
 * Creates a matrix whose i-th element equals u[i] * g,
 * where g is the generator of ECP2_BN254. The sizes of
 * m and u must match.
 *
 * @param m A pointer to an initialized cfe_mat_G2 matrix
 * @param u A pointer to matrix of integers.
 */
void cfe_mat_mul_G2(cfe_mat_G2 *m, cfe_mat *u);

/**
 * Multiplication of a matrix of integers by a matrix of elements of G2
 *
 * @param res A pointer to an initialized cfe_mat_G2 matrix
 * @param mi A pointer to matrix of integers
 * @param u A pointer to an initialized cfe_mat_G2 matrix
 */
void cfe_mat_mul_G2_mat(cfe_mat_G2 *res, cfe_mat *mi, cfe_mat_G2 *m);

/**
 * Multiplication of a matrix of elements of G2 by a vector of integers.
 * 
 * @param res A pointer to an initialized cfe_vec_G2 vector
 * @param m A pointer to a cfe_mat_G2 matrix
 * @param u A pointer to matrix of integers.
 */
void cfe_mat_G2_mul_vec(cfe_vec_G2 *res, cfe_mat_G2 *m, cfe_vec *u);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param m A pointer to a matrix (*initialized*
 * cfe_mat_G2 struct)
 */
void cfe_mat_G2_free(cfe_mat_G2 *m);

/**
 * Initializes a matrix of FP12_BN254 elements
 *
 * @param m A pointer to an uninitialized cfe_mat_GT matrix
 * @param rows The number of rows
 * @param cols The number of columns
 */
void cfe_mat_GT_init(cfe_mat_GT *m, size_t rows, size_t cols);

/**
 * Transposes the given cfe_mat_GT matrix.
 * 
 * @param res A pointer to an initialized cfe_mat_GT matrix
 * @param m A pointer to a cfe_mat_GT matrix
 */
void cfe_mat_GT_transpose(cfe_mat_GT *res, cfe_mat_GT *m);

/**
 * Multiplication of a matrix of integers by a matrix of elements of GT
 *
 * @param res A pointer to an initialized cfe_mat_GT matrix
 * @param mi A pointer to matrix of integers
 * @param u A pointer to an initialized cfe_mat_GT matrix
 */
void cfe_mat_mul_GT_mat(cfe_mat_GT *res, cfe_mat *mi, cfe_mat_GT *m);

/**
 * Multiplication of a matrix of elements of GT by a vector of integers.
 * 
 * @param res A pointer to an initialized cfe_vec_GT vector
 * @param m A pointer to a cfe_mat_GT matrix
 * @param u A pointer to matrix of integers.
 */
void cfe_mat_GT_mul_vec(cfe_vec_GT *res, cfe_mat_GT *m, cfe_vec *u);

/**
 * Pairs a given cfe_mat_G1 matrix with the generator of G2
 * 
 * @param res A pointer to an initialized cfe_vec_GT matrix
 * @param m A pointer to a cfe_mat_G1 matrix
 */
void cfe_mat_GT_pair_mat_G1(cfe_mat_GT *res, cfe_mat_G1 *m);

/**
 * Pairs a given cfe_mat_G2 matrix with the generator of G1
 * 
 * @param res A pointer to an initialized cfe_vec_GT matrix
 * @param m A pointer to a cfe_mat_G2 matrix
 */
void cfe_mat_GT_pair_mat_G2(cfe_mat_GT *res, cfe_mat_G2 *m);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param m A pointer to a matrix (*initialized*
 * cfe_mat_GT struct)
 */
void cfe_mat_GT_free(cfe_mat_GT *m);

#endif
