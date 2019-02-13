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

#ifndef CIFER_BIG_H
#define CIFER_BIG_H

/**
 * \file
 * \ingroup internal
 * \brief Functionalities for transformating mpz_t and
 * BIG_256_56 integers.
 */

#include <gmp.h>
#include <amcl/big_256_56.h>

/**
 * This function transforms an integer of type mpz_t to
 * type BIG_256_56.
 *
 * @param a Integer will be saved here
 * @param b Integer will be copied from
 */
void BIG_256_56_from_mpz(BIG_256_56 a, mpz_t b);

/**
 * This function transforms an integer of type BIG_256_56
 * to type mpz_t.
 *
 * @param b Integer will be saved here
 * @param a Integer will be copied from
 */void mpz_from_BIG_256_56(mpz_t b, BIG_256_56 a);

#endif
