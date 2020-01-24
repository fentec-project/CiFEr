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

#ifndef CIFER_POLICY_H
#define CIFER_POLICY_H

#include <stdbool.h>
#include <gmp.h>

#include "cifer/data/mat.h"
#include "cifer/data/vec.h"
#include "cifer/internal/errors.h"
#include "cifer/internal/str.h"

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
cfe_error cfe_boolean_to_msp(cfe_msp *msp, char *bool_exp,
                             size_t bool_exp_len, bool convert_to_ones);

/**
 * A helping function used in boolean_to_msp.
 */
cfe_error cfe_boolean_to_msp_iterative(cfe_msp *msp, cfe_string *bool_exp, cfe_vec *vec, size_t c);

/**
 * A helping function used in boolean_to_msp_iterative.
 */
void cfe_init_set_vecs_and(cfe_vec *vec1, cfe_vec *vec2, cfe_vec *vec, size_t c);

/**
 * Frees the memory occupied by the struct members. It does not free
 * memory occupied by the struct itself.
 *
 * @param msp A pointer to a MSP (*initialized* cfe_msp
 * struct)
 */
void cfe_msp_free(cfe_msp *msp);

#endif
