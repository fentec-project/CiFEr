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

#ifndef CIFER_HASH_H
#define CIFER_HASH_H

#include <amcl/ecp_BN254.h>
#include <amcl/ecp2_BN254.h>

#include "cifer/data/vec.h"

/**
 * \file
 * \ingroup internal
 * \brief Definitions of functions related to hashing.
 */

/**
 * This function hashes a string into an element of the elliptic
 * group G1 represented by ECP_BN254.
 *
 * @param g Hash will be saved here
 * @param str A string to be hashed
 */
void cfe_hash_G1(ECP_BN254 *g, char *str);

/**
 * This function hashes a string into an element of the elliptic
 * group G2 represented by ECP2_BN254.
 *
 * @param g Hash will be saved here
 * @param str A string to be hashed
 */
void cfe_hash_G2(ECP2_BN254 *g, char *str);

/**
 * The function converts a vector represented by cfe_vec
 * into a string. This is needed for hashing a vector.
 *
 * @param v A vector to be converted
 * @return String representation
 */
char *cfe_vec_to_string(cfe_vec *v);

/**
 * The function concatenates given strings. This is needed
 * to build a string to be hashed.
 *
 * @param start Strings to bo joined
 * @return Concatenation of strings
 */
char *cfe_strings_concat(char *start, ...);

/**
 * The function converts a non-negative int into a string of its
 * decimal representation that can be used for hashing.
 *
 * @param i Non-negative integer
 * @return String representation of the integer
 */
char *cfe_int_to_str(int i);

#endif
