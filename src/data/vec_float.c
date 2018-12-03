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

#include <stdlib.h>
#include <assert.h>
#include "data/vec_float.h"

// Initializes a vector.
void cfe_vec_float_init(cfe_vec_float *v, size_t size, size_t prec) {
    v->size = size;
    v->vec = (mpf_t *) malloc(size * sizeof(mpf_t));

    for (size_t i = 0; i < size; i++) {
        mpf_init2(v->vec[i], prec);
    }
}

// Frees the contents of a vector.
void cfe_vec_float_free(cfe_vec_float *v) {
    for (size_t i = 0; i < v->size; i++) {
        mpf_clear(v->vec[i]);
    }
    free(v->vec);
}

// Sets res to the i-th element of v.
void cfe_vec_float_get(mpf_t res, cfe_vec_float *v, size_t i) {
    assert (i < v->size);
    mpf_set(res, v->vec[i]);
}
