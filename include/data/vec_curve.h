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

#ifndef CIFER_VECTOR_CURVE_H
#define CIFER_VECTOR_CURVE_H

#include <amcl/ecp_BN254.h>
#include <amcl/ecp2_BN254.h>
#include "vec.h"

/**
 * \file
 * \ingroup data
 * \brief Vectors of elements of an elliptic curve struct and operations on it.
 */

/**
* Vector of ECP_BN254 elements.
*/
typedef struct cfe_vec_G1 {
    ECP_BN254 *vec; /** A pointer to the first element */
    size_t size; /** The size of the vector */
} cfe_vec_G1;

/**
 * Vector of ECP2_BN254 elements.
 */
typedef struct cfe_vec_G2 {
    ECP2_BN254 *vec; /** A pointer to the first element */
    size_t size; /** The size of the vector */
} cfe_vec_G2;

/**
 * Initializes a vector of ECP_BN254 elements of given size.
 *
 * @param v A pointer to an uninitialized cfe_vec_G1 vector
 * @param size Size of the vector
 */
void cfe_vec_G1_init(cfe_vec_G1 *v, size_t size);

/**
 * Creates a vector whose i-th element equals u[i] * g,
 * where g is the generator of ECP2_BN254. The sizes of
 * v and u must match.
 *
 * @param v A pointer to an initialized cfe_vec_G2 vector
 * @param u A pointer to vector of integers.
 */
void cfe_vec_mul_G1(cfe_vec_G1 *v, cfe_vec *u);

/**
 * Initializes a vector of ECP2_BN254 elements of given size.
 *
 * @param v A pointer to an uninitialized cfe_vec_G2 vector
 * @param size Size of the vector
 */
void cfe_vec_G2_init(cfe_vec_G2 *v, size_t size);

/**
 * Creates a vector whose i-th element equals u[i] * g,
 * where g is the generator of ECP2_BN254. The sizes of
 * v and u must match.
 *
 * @param v A pointer to an initialized cfe_vec_G2 vector
 * @param u A pointer to vector of integers.
 */
void cfe_vec_mul_G2(cfe_vec_G2 *v, cfe_vec *u);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param v A pointer to a vector (*initialized*
 * cfe_vec_G1 struct)
 */
void cfe_vec_G1_free(cfe_vec_G1 *v);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param v A pointer to a vector (*initialized*
 * cfe_vec_G2 struct)
 */
void cfe_vec_G2_free(cfe_vec_G2 *v);

#endif
