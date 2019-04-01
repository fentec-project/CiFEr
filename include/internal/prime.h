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

#ifndef CIFER_PRIME_H
#define CIFER_PRIME_H

#include <stdbool.h>
#include <gmp.h>

#include "internal/errors.h"

/**
 * \file
 * \ingroup internal
 * \brief Prime number related functions.
 */

/**
 * Checks whether the number passed as argument is a safe prime.
 *
 * @param p The number to be checked
 * @return true if number is a safe prime, false otherwise
 */
bool cfe_is_safe_prime(mpz_t p);

/**
 * Sets the first argument to a prime with specified bit length.
 * The prime will be a safe prime if the safe parameter is true.
 *
 * @param res The safe prime (result value is stored here)
 * @param bits Bit length of the safe prime
 * @param safe Boolean value if the prime will be safe or not
 * @return Error code
 */
cfe_error cfe_get_prime(mpz_t res, size_t bits, bool safe);

#endif
