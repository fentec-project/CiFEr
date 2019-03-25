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
#include "internal/errors.h"

/**
 * \file
 * \ingroup abe
 * \brief Functions for policy conversion needed for ABE schemes.
 */


/**
 * cfe_msp represents a monotone span program (MSP) describing a policy defining which
 * attributes are needed to decrypt the ciphertext. It includes a matrix
 * and a mapping from the rows of the matrix to attributes. A MSP policy
 * allows decryption of an entity with a set of attributes A if an only if all the
 * rows of the matrix mapped to an element of A span the vector [1, 0,..., 0] (or
 * vector [1, 1,..., 1] depending on the use case).
 */
typedef struct cfe_msp {
    cfe_mat mat; /** A matrix representing the policy */
    int *row_to_attrib; /** Mapping from the rows of the matrix to attributes */
} cfe_msp;

/**
 * cfe_boolean_to_msp takes as an input a boolean expression (without a NOT gate) and
 * outputs a msp structure representing the expression, i.e. a matrix whose rows
 * correspond to attributes used in the expression and with the property that a
 * boolean expression assigning 1 to some attributes is satisfied iff the
 * corresponding rows span a vector [1, 1,..., 1] or vector [1, 0,..., 0]
 * depending if parameter convert_to_ones is set to true or false. Additionally a
 * vector is produced whose i-th entry indicates to which attribute the i-th row
 * corresponds. The boolean expression is a string where attributes are written as
 * integers (hence they must not be greater than what int can hold) separated
 * with AND and OR gates, with brackets defining the order of the gates.
 *
 * @param msp A pointer to an uninitialized cfe_msp struct representing the
 * MSP structure; the result will be saved here
 * @param bool_exp A string with the boolean expression
 * @param convert_to_ones A boolean value defining which vector must the MSP matrix
 * span, a vector [1, 1,..., 1] if set to true or vector [1, 0,..., 0] if set to false
 * @return Returns an error if the boolean expression is not in the proper form and
 * cannot be transformed.
 */
cfe_error cfe_boolean_to_msp(cfe_msp *msp, char *bool_exp, bool convert_to_ones);

/**
 * A helping function used in boolean_to_msp.
 */
cfe_error cfe_boolean_to_msp_iterative(cfe_msp *msp, char *bool_exp, cfe_vec *vec, size_t c);

/**
 * A helping function used in boolean_to_msp_iterative.
 */
void cfe_init_set_vecs_and(cfe_vec *vec1, cfe_vec *vec2, cfe_vec *vec, size_t c);

/**
 * A helping function used in boolean_to_msp_iterative.
 */
int cfe_str_to_int(char *str);

/**
 * A helping function used in boolean_to_msp_iterative.
 */
char *cfe_substring(char *s, size_t start, size_t stop);

/**
 * A helping function used in boolean_to_msp_iterative.
 */
char *cfe_remove_spaces(char *source);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param msp A pointer to a MSP (*initialized* cfe_msp
 * struct)
 */
void cfe_msp_free(cfe_msp *msp);

/**
 * cfe_gaussian_elimination solves a matrix vector equation mat * x = v and finds
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
cfe_error cfe_gaussian_elimination(cfe_vec *res, cfe_mat *mat, cfe_vec *vec, mpz_t p);

#endif
