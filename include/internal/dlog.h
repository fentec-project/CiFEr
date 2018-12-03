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

#ifndef CIFER_DLOG_H
#define CIFER_DLOG_H

#include <gmp.h>

#include "internal/errors.h"

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
