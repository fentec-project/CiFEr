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

#include <stdlib.h>
#include <assert.h>

#include "cifer/data/vec_float.h"
#include "cifer/internal/common.h"

// Initializes a vector.
void cfe_vec_float_init(cfe_vec_float *v, size_t size, size_t prec) {
    v->size = size;
    v->vec = (mpf_t *) cfe_malloc(size * sizeof(mpf_t));

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
