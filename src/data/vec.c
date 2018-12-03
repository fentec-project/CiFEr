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

#include "data/vec.h"
#include "data/mat.h"

// Initializes a vector.
void cfe_vec_init(cfe_vec *v, size_t size) {
    v->size = size;
    v->vec = (mpz_t *) malloc(size * sizeof(mpz_t));

    for (size_t i = 0; i < size; i++) {
        mpz_init(v->vec[i]);
    }
}

// Variadic version of cfe_vec_init.
// Initializes a NULL-terminated list of vectors.
void cfe_vec_inits(size_t size, cfe_vec *v, ...) {
    va_list ap;
    va_start (ap, v);

    while (v != NULL) {
        cfe_vec_init(v, size);
        v = va_arg (ap, cfe_vec*);
    }
    va_end (ap);
}

// Coordinate-wise addition.
void cfe_vec_add(cfe_vec *res, cfe_vec *v1, cfe_vec *v2) {
    assert(v1->size == v2->size);
    assert(res->size == v1->size);

    for (size_t i = 0; i < v1->size; i++) {
        mpz_add(res->vec[i], v1->vec[i], v2->vec[i]);
    }
}

// Calculates the inner product of two vectors.
void cfe_vec_dot(mpz_t res, cfe_vec *v1, cfe_vec *v2) {
    assert(v1->size == v2->size);

    mpz_t prod;
    mpz_init(prod);

    // set it to 0, in case it already holds some value != 0
    mpz_set_si(res, 0);

    for (size_t i = 0; i < v1->size; i++) {
        mpz_mul(prod, v1->vec[i], v2->vec[i]);
        mpz_add(res, res, prod);
    }

    mpz_clear(prod);
}

// Multiplication of a vector transposed by a matrix.
void cfe_vec_mul_matrix(cfe_vec *res, cfe_vec *v, cfe_mat *m) {
    assert(m->cols == res->size);
    assert(m->rows == v->size);

    cfe_vec col;
    cfe_vec_init(&col, m->rows);
    for (size_t i = 0; i < m->cols; i++) {
        cfe_mat_get_col(&col, m, i);
        cfe_vec_dot(res->vec[i], &col, v);
    }
    cfe_vec_free(&col);
}

// Coordinate-wise modulo.
void cfe_vec_mod(cfe_vec *res, cfe_vec *v, mpz_t modulo) {
    assert(res->size == v->size);

    for (size_t i = 0; i < v->size; i++) {
        mpz_mod(res->vec[i], v->vec[i], modulo);
    }
}

// Checks if all coordinates are < bound.
bool cfe_vec_check_bound(cfe_vec *v, mpz_t bound) {
    for (size_t i = 0; i < v->size; i++) {
        if (mpz_cmp(v->vec[i], bound) >= 0) {
            return false;
        }
    }

    return true;
}

// Sets res to the i-th element of v.
void cfe_vec_get(mpz_t res, cfe_vec *v, size_t i) {
    assert (i < v->size);
    mpz_set(res, v->vec[i]);
}

// Sets the i-th element of v to el.
void cfe_vec_set(cfe_vec *v, mpz_t el, size_t i) {
    assert (i < v->size);
    mpz_set(v->vec[i], el);
}

// Appends element el to the end of vector v, returning
// the result in vector res.
void cfe_vec_append(cfe_vec *res, cfe_vec *v, mpz_t el) {
    assert(res->size == v->size + 1);

    for (size_t i = 0; i < v->size; i++) {
        mpz_set(res->vec[i], v->vec[i]);
    }
    mpz_set(res->vec[res->size - 1], el);
}

// Joins vectors v1 and v2 into a single vector.
void cfe_vec_join(cfe_vec *res, cfe_vec *v1, cfe_vec *v2) {
    assert (res->size == v1->size + v2->size);

    for (size_t i = 0; i < v1->size; i++) {
        mpz_set(res->vec[i], v1->vec[i]);
    }

    for (size_t i = 0; i < v2->size; i++) {
        mpz_set(res->vec[v1->size + i], v2->vec[i]);
    }
}

// Extracts n elements of vector v starting at index from to vector res.
void cfe_vec_extract(cfe_vec *res, cfe_vec *v, size_t from, size_t n) {
    assert(from + n <= v->size);
    assert(res->size == n);

    for (size_t i = 0; i < n; i++) {
        mpz_set(res->vec[i], v->vec[from + i]);
    }
}

// Frees the space occupied by the vector and its elements.
void cfe_vec_free(cfe_vec *v) {
    for (size_t i = 0; i < v->size; i++) {
        mpz_clear(v->vec[i]);
    }
    free(v->vec);
}

// Variadic version of cfe_vec_free.
// Frees a NULL-terminated list of vectors.
void cfe_vec_frees(cfe_vec *v, ...) {
    va_list ap;
    va_start (ap, v);

    while (v != NULL) {
        cfe_vec_free(v);
        v = va_arg (ap, cfe_vec*);
    }
    va_end (ap);
}

// Prints a vector to standard output.
void cfe_vec_print(cfe_vec *v) {
    mpz_t el;
    mpz_init(el);

    gmp_printf("[");
    for (size_t i = 0; i < v->size; i++) {
        cfe_vec_get(el, v, i);
        gmp_printf(" %Zd ", el);
    }
    gmp_printf("]");

    mpz_clear(el);
}

// vector times scalar
void cfe_vec_mul_scalar(cfe_vec *res, cfe_vec *v, mpz_t s) {
    assert(res->size == v->size);

    for (size_t i = 0; i < v->size; i++) {
        mpz_mul(res->vec[i], v->vec[i], s);
    }
}

// multiplication of two vectors presenting two
// polynomials in Z[x] / (x^n + 1)
void cfe_vec_poly_mul(cfe_vec *res, cfe_vec *v1, cfe_vec *v2) {
    assert(v1->size == v2->size);
    assert(res->size == v1->size);
    mpz_t product;
    mpz_init(product);
    for (size_t i = 0; i < res->size; i++) {
        mpz_set_si(res->vec[i], 0);
        for (size_t j = 0; j <= i; j++) {
            mpz_mul(product, v1->vec[i - j], v2->vec[j]);
            mpz_add(res->vec[i], res->vec[i], product);
        }
        for (size_t j = (i + 1); j < v1->size; j++) {
            mpz_mul(product, v1->vec[(v1->size + i - j)], v2->vec[j]);
            mpz_neg(product, product);
            mpz_add(res->vec[i], res->vec[i], product);
        }
    }
    mpz_clear(product);
}

// saves vector -v to res
void cfe_vec_neg(cfe_vec *res, cfe_vec *v) {
    assert(res->size == v->size);

    for (size_t i = 0; i < v->size; i++) {
        mpz_neg(res->vec[i], v->vec[i]);
    }
}

// Coordinate-wise division with floor rounding
void cfe_vec_fdiv_q_scalar(cfe_vec *res, cfe_vec *v, mpz_t s) {
    assert(res->size == v->size);

    for (size_t i = 0; i < v->size; i++) {
        mpz_fdiv_q(res->vec[i], v->vec[i], s);
    }
}

// Coordinate-wise multiplication
void cfe_vec_mul(cfe_vec *res, cfe_vec *v1, cfe_vec *v2) {
    assert(v1->size == v2->size);
    assert(res->size == v1->size);

    for (size_t i = 0; i < v1->size; i++) {
        mpz_mul(res->vec[i], v1->vec[i], v2->vec[i]);
    }
}

// rescursive implementation of FFT, assuming the length
// of a is a power of 2. The vector y must have the same
// length n as a and root must be then n-th rooth of
// one in Z_q, or it can be double the length of a and
// the root must be 2n-th root of one
void cfe_vec_FFT(cfe_vec *y, cfe_vec *a, mpz_t root, mpz_t q) {
    assert(y->size == a->size || y->size == 2 * a->size);
    mpz_t check;
    mpz_init(check);
    mpz_pow_ui(check, root, y->size);
    mpz_mod(check, check, q);
    assert(mpz_cmp_si(check, 1) == 0);
    if (a->size == 1) {
        cfe_vec_set(y, a->vec[0], 0);
        if (y->size == 2) {
            cfe_vec_set(y, a->vec[0], 1);
        }
        mpz_clear(check);
        return;
    }

    mpz_t current_root;
    mpz_init_set_si(current_root, 1);

    cfe_vec a0, a1;
    cfe_vec_init(&a0, a->size / 2);
    cfe_vec_init(&a1, a->size / 2);
    for (size_t i = 0; i < (a->size / 2); i++) {
        cfe_vec_set(&a0, a->vec[2 * i], i);
        cfe_vec_set(&a1, a->vec[2 * i + 1], i);
    }

    cfe_vec y0, y1;
    cfe_vec_init(&y0, y->size / 2);
    cfe_vec_init(&y1, y->size / 2);

    mpz_t new_root;
    mpz_init(new_root);
    mpz_mul(new_root, root, root);
    cfe_vec_FFT(&y0, &a0, new_root, q);
    cfe_vec_FFT(&y1, &a1, new_root, q);

    mpz_t value;
    mpz_init(value);

    for (size_t i = 0; i < (y->size / 2); i++) {
        mpz_mul(value, current_root, y1.vec[i]);
        mpz_add(y->vec[i], y0.vec[i], value);

        mpz_sub(y->vec[(y->size / 2) + i], y0.vec[i], value);

        mpz_mul(current_root, current_root, root);
    }

    cfe_vec_mod(y, y, q);
    mpz_clears(check, current_root, new_root, value, NULL);
    cfe_vec_frees(&a0, &a1, &y0, &y1, NULL);
}

// multiplication of two vectors presenting two
// polynomials in Z_q[x] using FFT. Root must be a primitive n rooth of 1
// in Z_q, inv_root its inverse and inv_2n must be the inverse of 2n in Z_q
void cfe_vec_poly_mul_FFT(cfe_vec *res, cfe_vec *v1, cfe_vec *v2, mpz_t root, mpz_t inv_root, mpz_t inv_2n, mpz_t q) {
    assert(v1->size == v2->size);
    assert(res->size == v1->size);

    cfe_vec f1, f2, fr, prod;
    cfe_vec_inits(2 * v1->size, &f1, &f2, &fr, &prod, NULL);
    cfe_vec_FFT(&f1, v1, root, q);
    cfe_vec_FFT(&f2, v2, root, q);
    cfe_vec_mul(&fr, &f1, &f2);
    cfe_vec_mod(&fr, &fr, q);

    cfe_vec_FFT(&prod, &fr, inv_root, q);
    cfe_vec_mul_scalar(&prod, &prod, inv_2n);
    for (size_t i = 0; i < res->size; i++) {
        mpz_sub(res->vec[i], prod.vec[i], prod.vec[i + res->size]);
    }
    cfe_vec_mod(res, res, q);
    cfe_vec_frees(&f1, &f2, &fr, &prod, NULL);
}
