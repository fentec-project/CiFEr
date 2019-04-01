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

#ifndef CIFER_KEYGEN_H
#define CIFER_KEYGEN_H

#include <gmp.h>

#include "internal/errors.h"

/**
 * \file
 * \ingroup internal
 * \brief ElGamal key generation
 */

typedef struct cfe_elgamal {
    mpz_t p; // Modulus
    mpz_t g; // Generator of the cyclic group
    mpz_t q; // (p - 1) / 2, i.e. order of g
} cfe_elgamal;


/**
 * Initializes the cfe_elgamal struct.
 *
 * @param key A pointer to an uninitialized cfe_elgamal struct
 * @param modulus_len The bit length of the modulus
 * @return Error code
 */
cfe_error cfe_elgamal_init(cfe_elgamal *key, size_t modulus_len);

/**
 * Frees the memory occupied by the members of the struct.
 *
 * @param key A pointer to an initialized cfe_elgamal struct
 */
void cfe_elgamal_free(cfe_elgamal *key);

#endif
