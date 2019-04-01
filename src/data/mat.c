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

#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

#include "data/mat.h"
#include "internal/common.h"

// Initializes a matrix.
void cfe_mat_init(cfe_mat *m, size_t rows, size_t cols) {
    m->rows = rows;
    m->cols = cols;
    m->mat = (cfe_vec *) cfe_malloc(rows * sizeof(cfe_vec));

    for (size_t i = 0; i < rows; i++) {
        cfe_vec_init(&m->mat[i], cols);
    }
}

// Variadic version of cfe_mat_init.
// Initializes a NULL-terminated list of matrices.
void cfe_mat_inits(size_t rows, size_t cols, cfe_mat *m, ...) {
    va_list ap;

    va_start (ap, m);

    while (m != NULL) {
        cfe_mat_init(m, rows, cols);
        m = va_arg (ap, cfe_mat*);
    }
    va_end (ap);
}

// Set values of a matrix to a constant.
void cfe_mat_set_const(cfe_mat *m, mpz_t c) {
    for (size_t i = 0; i < m->rows; i++) {
        cfe_vec_set_const(&m->mat[i], c);
    }
}

// Copy of a matrix
void cfe_mat_copy(cfe_mat *res, cfe_mat *m) {
    assert(m->rows == res->rows && m->cols == res->cols);
    for (size_t i = 0; i < res->rows; i++) {
        cfe_vec_copy(&res->mat[i], &m->mat[i]);
    }
}

// Coordinate-wise addition.
void cfe_mat_add(cfe_mat *res, cfe_mat *m1, cfe_mat *m2) {
    assert(m1->rows == m2->rows);
    assert(res->rows == m1->rows);

    for (size_t i = 0; i < m1->rows; i++) {
        cfe_vec_add(&res->mat[i], &m1->mat[i], &m2->mat[i]);
    }
}

// Multiplication of a matrix by a vector.
void cfe_mat_mul_vec(cfe_vec *res, cfe_mat *m, cfe_vec *v) {
    assert(m->rows == res->size);
    assert(m->cols == v->size);

    for (size_t i = 0; i < m->rows; i++) {
        cfe_vec_dot(res->vec[i], &m->mat[i], v);
    }
}

// Dot (inner) product of matrices.
void cfe_mat_dot(mpz_t res, cfe_mat *m1, cfe_mat *m2) {
    assert(m1->rows == m2->rows);
    assert(m1->cols == m2->cols);

    mpz_t prod, sum;
    mpz_inits(prod, sum, NULL);

    for (size_t i = 0; i < m1->rows; i++) {
        cfe_vec_dot(prod, &m1->mat[i], &m2->mat[i]);
        mpz_add(sum, sum, prod);
    }

    mpz_set(res, sum);
    mpz_clears(prod, sum, NULL);
}

// Component-wise modulo.
void cfe_mat_mod(cfe_mat *res, cfe_mat *m, mpz_t modulo) {
    assert(res->rows == m->rows);

    for (size_t i = 0; i < m->rows; i++) {
        cfe_vec_mod(&res->mat[i], &m->mat[i], modulo);
    }
}

// Checks if all coordinates are < bound.
bool cfe_mat_check_bound(cfe_mat *m, mpz_t bound) {
    for (size_t i = 0; i < m->rows; i++) {
        if (!cfe_vec_check_bound(&m->mat[i], bound)) {
            return false;
        }
    }
    return true;
}

// Sets res to the j-th element of the i-th row of m.
void cfe_mat_get(mpz_t res, cfe_mat *m, size_t i, size_t j) {
    assert(i < m->rows);
    cfe_vec_get(res, &m->mat[i], j);
}

// Sets res to the i-th row of m.
void cfe_mat_get_row(cfe_vec *res, cfe_mat *m, size_t i) {
    assert(i < m->rows);
    for (size_t j = 0; j < m->cols; j++) {
        cfe_vec_get(res->vec[j], &m->mat[i], j);
    }
}

// Returns the pointer to the i-th row of m.
cfe_vec *cfe_mat_get_row_ptr(cfe_mat *m, size_t i) {
    assert(i < m->rows);
    return &m->mat[i];
}

// Sets res to the i-th col of m.
void cfe_mat_get_col(cfe_vec *res, cfe_mat *m, size_t i) {
    assert(i < m->cols);
    for (size_t j = 0; j < m->rows; j++) {
        cfe_vec_get(res->vec[j], &m->mat[j], i);
    }
}

// Sets the j-th element of the i-th row of m to el.
void cfe_mat_set(cfe_mat *m, mpz_t el, size_t i, size_t j) {
    assert(i < m->rows);
    cfe_vec_set(&m->mat[i], el, j);
}

// Sets the i-th row of m to v.
void cfe_mat_set_vec(cfe_mat *m, cfe_vec *v, size_t i) {
    assert(i < m->rows);
    assert(v->size == m->cols);
    for (size_t j = 0; j < m->cols; j++) {
        cfe_vec_set(&m->mat[i], v->vec[j], j);
    }
}

// Creates a vector from matrix elements by putting rows
// in a single long vector.
void cfe_mat_to_vec(cfe_vec *res, cfe_mat *m) {
    assert(res->size == m->rows * m->cols);

    mpz_t el;
    mpz_init(el);

    for (size_t i = 0; i < m->rows; i++) {
        for (size_t j = 0; j < m->cols; j++) {
            cfe_mat_get(el, m, i, j);
            cfe_vec_set(res, el, m->cols * i + j);
        }
    }

    mpz_clear(el);
}

// Constructs a matrix of dimensions rows*cols from the given vector.
// Vector must hold exactly rows*cols elements.
void cfe_mat_from_vec(cfe_mat *m, cfe_vec *v) {
    assert(v->size == m->rows * m->cols);

    mpz_t el;
    mpz_init(el);
    for (size_t i = 0; i < m->rows; i++) {
        for (size_t j = 0; j < m->cols; j++) {
            cfe_vec_get(el, v, m->cols * i + j);
            cfe_mat_set(m, el, i, j);
        }
    }
    mpz_clear(el);
}

// Transposes matrix m.
void cfe_mat_transpose(cfe_mat *mt, cfe_mat *m) {
    assert(mt->rows == m->cols);
    assert(mt->cols == m->rows);

    mpz_t el;
    mpz_init(el);
    for (size_t i = 0; i < mt->rows; i++) {
        for (size_t j = 0; j < mt->cols; j++) {
            cfe_mat_get(el, m, j, i);
            cfe_mat_set(mt, el, i, j);
        }
    }
    mpz_clear(el);
}

// Frees the space occupied by the matrix and its elements.
void cfe_mat_free(cfe_mat *m) {
    for (size_t i = 0; i < m->rows; i++) {
        cfe_vec_free(&m->mat[i]);
    }
    free(m->mat);
}

// Variadic version of cfe_mat_free.
// Frees a NULL-terminated list of matrices.
void cfe_mat_frees(cfe_mat *m, ...) {
    va_list ap;
    va_start (ap, m);

    while (m != NULL) {
        cfe_mat_free(m);
        m = va_arg (ap, cfe_mat*);
    }
    va_end (ap);
}

// Prints the matrix to standard output.
void cfe_mat_print(cfe_mat *m) {
    cfe_vec v;
    cfe_vec_init(&v, m->cols);

    gmp_printf("[\n");
    for (size_t i = 0; i < m->rows; i++) {
        cfe_mat_get_row(&v, m, i);
        gmp_printf("  ");
        cfe_vec_print(&v);
        gmp_printf("\n");
    }
    gmp_printf("]\n");

    cfe_vec_free(&v);
}

// Matrix multiplication. Returns m1 * m2.
void cfe_mat_mul(cfe_mat *res, cfe_mat *m1, cfe_mat *m2) {
    assert(m1->cols == m2->rows);
    assert(m1->rows == res->rows);
    assert(m2->cols == res->cols);

    mpz_t sum, prod, x, y;
    mpz_inits(sum, prod, x, y, NULL);

    for (size_t i = 0; i < m1->rows; i++) {
        for (size_t j = 0; j < m2->cols; j++) {
            mpz_set_ui(sum, 0);
            for (size_t k = 0; k < m1->cols; k++) {
                cfe_mat_get(x, m1, i, k);
                cfe_mat_get(y, m2, k, j);
                mpz_mul(prod, x, y);
                mpz_add(sum, sum, prod);
            }
            cfe_mat_set(res, sum, i, j);
        }
    }
    mpz_clears(sum, prod, x, y, NULL);
}
