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

#ifndef CIFER_POLICY_H
#define CIFER_POLICY_H

#include <stdbool.h>
#include <gmp.h>

#include "data/mat.h"
#include "data/vec.h"

/**
 * \file
 * \ingroup abe
 * \brief Functions for policy conversion needed for ABE schemes.
 */


/**
 * MSP represents a monotone span program (MSP) describing a policy defining which
 * attributes are needed to decrypt the ciphertext. It includes a matrix
 * mat and a mapping from the rows of the mat to attributes. A MSP policy
 * allows decryption of an entity with a set of attributes A if an only if all the
 * rows of the matrix mapped to an element of A span the vector [1, 0,..., 0] (or
 * vector [1, 1,..., 1] depending on the use case).
 */
typedef struct cfe_msp {
    cfe_mat *mat; /** A pointer to a matrix */
    int *row_to_attrib; /** Mapping from the rows of the matrix to attributes */
} cfe_msp;

void boolean_to_msp(cfe_msp *msp, char *bool_exp, bool convert_to_ones);

size_t boolean_to_msp_iterative(cfe_msp *msp, char *bool_exp, cfe_vec *vec, size_t c);

void make_and_vecs(cfe_vec *vec1, cfe_vec *vec2, cfe_vec *vec, size_t c);

char *substring(char *s, size_t start, size_t stop);

char *remove_spaces(char* source);

void cfe_msp_free(cfe_msp *msp);

int gaussian_elimination(cfe_vec *res, cfe_mat *mat, cfe_vec *vec, mpz_t p);

#endif
