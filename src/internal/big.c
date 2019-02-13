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

#include "internal/big.h"

void BIG_256_56_from_mpz(BIG_256_56 a, mpz_t b) {
    int i;
    BIG_256_56_zero(a);
    mpz_t x, y;
    mpz_inits(x, y, NULL);
    mpz_set(y, b);
    size_t size = (MODBYTES_256_56 * 8) - ((MODBYTES_256_56 * 8) % BASEBITS_256_56);
    for (i=0; i<((MODBYTES_256_56 * 8) / BASEBITS_256_56) + 1; i++)
    {
        BIG_256_56_fshl(a, BASEBITS_256_56);
        mpz_fdiv_q_2exp(x, y, size);
        mpz_fdiv_r_2exp(y, y, size);
        a[0]+= mpz_get_ui(x);
        size = size - BASEBITS_256_56;
    }
    mpz_clears(x, y, NULL);
}

void mpz_from_BIG_256_56(mpz_t b, BIG_256_56 a) {
    mpz_import(b, NLEN_256_56, -1, sizeof(a[0]), 0, (8 * sizeof(a[0])) - BASEBITS_256_56, a);
}

