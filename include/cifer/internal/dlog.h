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

#ifndef CIFER_DLOG_H
#define CIFER_DLOG_H

#include <gmp.h>

#include "cifer/internal/errors.h"

/**
 * \file
 * \ingroup internal
 * \brief Algorithms for computing discrete logarithms.
 *
 * FE schemes instantiated from the Discrete Diffie-Hellman assumption (DDH)
 * all rely on efficient algorithms for calculating discrete logarithms.
 */

/**
 * @brief Baby-step giant-step method for computing the discrete logarithm in
 * the Zp group.
 *
 * It searches for a solution <= bound. If bound argument is nil,
 * the bound is automatically set to p-1.
 * The function returns x, where h = g^x mod p. If the solution was not found
 * within the provided bound, it returns an error.
 *
 * @param res Discrete logarithm (the result value placeholder)
 * @param h Element
 * @param g Generator
 * @param p Modulus
 * @param order Order
 * @param bound Bound for solution
 * @return Error code
 */
cfe_error cfe_baby_giant(mpz_t res, mpz_t h, mpz_t g, mpz_t p, mpz_t order, mpz_t bound);

/**
 * @brief Baby-step giant-step method for computing the discrete logarithm in
 * the Zp group finding also negative solutions.
 *
 * It searches for a solution (-bound, bound). If bound argument is nil,
 * the bound is automatically set to p-1 and it works identically than
 * function baby_step_giant_step.
 * The function returns x, where h = g^x mod p. If the solution was not found
 * within the provided bound, it returns an error.
 *
 * @param res Discrete logarithm (the result value placeholder)
 * @param h Element
 * @param g Generator
 * @param p Modulus
 * @param _order Order
 * @param bound Bound for solution
 * @return Error code
 */
cfe_error cfe_baby_giant_with_neg(mpz_t res, mpz_t h, mpz_t g, mpz_t p, mpz_t _order, mpz_t bound);

/**
 * @brief  Pollard's rho algorithm - simple, non-parallel version.
 *
 * @param res Discrete logarithm (the result value placeholder)
 * @param h Element
 * @param g Generator
 * @param p Modulus
 * @param n Order
 * @return Error code
 */
cfe_error cfe_pollard_rho(mpz_t res, mpz_t h, mpz_t g, mpz_t p, mpz_t n);

#endif
