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

#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include "internal/big.h"

#include "data/vec.h"
#include "data/vec_curve.h"
#include "data/mat.h"
#include "internal/common.h"

void cfe_vec_G1_init(cfe_vec_G1 *v, size_t size) {
    v->size = size;
    v->vec = (ECP_BN254 *) cfe_malloc(size * sizeof(ECP_BN254));
}

void cfe_vec_mul_G1(cfe_vec_G1 *v, cfe_vec *u) {
    v->size = u->size;
    v->vec = (ECP_BN254 *) cfe_malloc(u->size * sizeof(ECP_BN254));
    BIG_256_56 x;

    for (size_t i = 0; i < u->size; i++) {
        ECP_BN254_generator(&(v->vec[i]));
        BIG_256_56_from_mpz(x, u->vec[i]);
        ECP_BN254_mul(&(v->vec[i]), x);
    }
}

void cfe_vec_G2_init(cfe_vec_G2 *v, size_t size) {
    v->size = size;
    v->vec = (ECP2_BN254 *) cfe_malloc(size * sizeof(ECP2_BN254));
}

void cfe_vec_mul_G2(cfe_vec_G2 *v, cfe_vec *u) {
    v->size = u->size;
    v->vec = (ECP2_BN254 *) cfe_malloc(u->size * sizeof(ECP2_BN254));
    BIG_256_56 x;

    for (size_t i = 0; i < u->size; i++) {
        ECP2_BN254_generator(&(v->vec[i]));
        BIG_256_56_from_mpz(x, u->vec[i]);
        ECP2_BN254_mul(&(v->vec[i]), x);
    }
}
