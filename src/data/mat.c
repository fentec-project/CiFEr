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

#include "cifer/data/mat.h"
#include "cifer/internal/common.h"

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

// Sets res to -m.
void cfe_mat_neg(cfe_mat *res, cfe_mat *m) {
    assert(res->rows == m->rows);

    for (size_t i = 0; i < m->rows; i++) {
        cfe_vec_neg(&res->mat[i], &m->mat[i]);
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

void cfe_mat_extract_submatrix(cfe_mat *min, cfe_mat *m, size_t i, size_t j) {
    assert(i < m->rows && j < m->cols);
    assert(min->rows == m->rows -1 && min->cols == m->cols -1);

    size_t ind1 = 0;
    mpz_t val;
    mpz_init(val);
    for (size_t k = 0; k < m->rows - 1; k++) {
        if (k == i) {
            ind1++;
        }
        size_t ind2 = 0;
        for (size_t l = 0; l < m->cols - 1; l++) {
            if (l == j) {
                ind2++;
            }
            cfe_mat_get(val, m, ind1, ind2);
            cfe_mat_set(min, val, k, l);
            ind2++;
        }
        ind1++;
    }
    mpz_clear(val);
}

void cfe_mat_determinant(mpz_t det, cfe_mat *m) {
    assert(m->rows == m->cols);

    if (m->rows == 1) {
        cfe_mat_get(det, m, 0, 0);
        return;
    }
    mpz_set_si(det, 0);
    mpz_t sign, minus, val, minor;
    mpz_inits(sign, minus, val, minor, NULL);
    mpz_set_ui(sign, 1);
    mpz_set_si(minus, -1);

    cfe_mat min;
    cfe_mat_init(&min, m->rows - 1, m->cols - 1);

    for (size_t i = 0; i < m->rows; i++) {
        cfe_mat_extract_submatrix(&min, m, 0, i);
        cfe_mat_get(val, m, 0, i);
        cfe_mat_determinant(minor, &min);

        mpz_mul(minor, minor, val);
        mpz_mul(minor, minor, sign);
        mpz_mul(sign, sign, minus);
        mpz_add(det, det, minor);
    }
    cfe_mat_free(&min);
    mpz_clears(sign, minus, val, minor, NULL);
}

cfe_error cfe_mat_inverse_mod(cfe_mat *inverse_mat, cfe_mat *m, mpz_t mod) {
    cfe_error err = CFE_ERR_NONE;
    mpz_t det, det_inv, sign, minus, minor, val;
    mpz_inits(det, det_inv, sign, minus, minor, val, NULL);
    cfe_mat_determinant(det, m);
    mpz_mod(det, det, mod);
    if(mpz_cmp_si(det, 0) == 0) {
        err = CFE_ERR_NO_INVERSE;
        goto cleanup;
    }
    mpz_invert(det_inv, det, mod);

    mpz_set_si(minus, -1);
    cfe_mat min, transposed;
    cfe_mat_init(&min, m->rows - 1, m->cols - 1);
    cfe_mat_init(&transposed, m->rows, m->cols);
    for (size_t i = 0; i < m->rows; i++) {
        for (size_t j = 0; j < m->cols; j++) {
            cfe_mat_extract_submatrix(&min, m, i, j);
            cfe_mat_determinant(minor, &min);

            mpz_mod(minor, minor, mod);
            mpz_pow_ui(sign, minus, i+j);

            mpz_mul(val, minor, det_inv);
            mpz_mul(val, val, sign);
            mpz_mod(val, val, mod);

            cfe_mat_set(&transposed, val, i, j);
        }
    }

    cfe_mat_transpose(inverse_mat, &transposed);
    cfe_mat_frees(&min, &transposed, NULL);
    cleanup:
    mpz_clears(det, det_inv, sign, minus, minor, val, NULL);

    return err;
}

void cfe_mat_mul_x_mat_y(mpz_t res, cfe_mat *mat, cfe_vec *x, cfe_vec *y) {
    cfe_vec t;
    cfe_vec_init(&t, x->size);
    cfe_mat_mul_vec(&t, mat, y);
    cfe_vec_dot(res, &t, x);
    cfe_vec_free(&t);
}

void cfe_mat_mul_scalar(cfe_mat *res, cfe_mat *mat, mpz_t s) {
    assert(res->rows == mat->rows);
    assert(res->cols == mat->cols);
    for (size_t j = 0; j < mat->cols; j++) {
        cfe_vec_mul_scalar(&(res->mat[j]), &(mat->mat[j]), s);
    }
}


void cfe_mat_gaussian_elimination(cfe_mat *res, cfe_mat *mat, mpz_t p) {
    cfe_mat_copy(res, mat);
    cfe_mat_mod(res, res, p);

    mpz_t zero, tmp, tmp2, mhk_inv, lead_mul_inv;
    mpz_inits(zero, tmp, tmp2, mhk_inv, lead_mul_inv, NULL);
    cfe_vec vec_tmp;

    mpz_set_ui(zero, 0);
    size_t h = 0, k = 0;
    while (h < mat->rows && k < mat->cols) {
        bool is_zero = true;
        for (size_t i = h; i < mat->rows; i++) {
            cfe_mat_get(tmp, res, i, k);
            if (mpz_cmp_ui(tmp, 0) != 0) {
                vec_tmp = res->mat[i];
                res->mat[i] = res->mat[h];
                res->mat[h] = vec_tmp;

                is_zero = false;
                break;
            }
        }

        if (is_zero) {
            k++;
            continue;
        }
        cfe_mat_get(tmp, res, h, k);
        mpz_invert(mhk_inv, tmp, p);
        for (size_t i = h + 1; i < res->rows; i++) {
            cfe_mat_get(tmp, res, i, k);
            mpz_mul(lead_mul_inv, mhk_inv, tmp);
            cfe_mat_set(res, zero, i, k);
            for (size_t j = k + 1; j < res->cols; j++) {
                cfe_mat_get(tmp, res, i, j);
                cfe_mat_get(tmp2, res, h, j);

                mpz_mul(tmp2, tmp2, lead_mul_inv);
                mpz_sub(tmp, tmp, tmp2);
                mpz_mod(tmp, tmp, p);

                cfe_mat_set(res, tmp, i, j);
            }
        }
        k++;
        h++;
    }

    mpz_clears(zero, tmp, tmp2, mhk_inv, lead_mul_inv, NULL);
}

cfe_error cfe_mat_inverse_mod_gauss(cfe_mat *res, mpz_t det, cfe_mat *m, mpz_t p) {
    cfe_error err = CFE_ERR_NONE;
    cfe_mat m_ext;
    cfe_mat_init(&m_ext, m->rows, 2*m->cols);

    mpz_t tmp, tmp_sum, one, zero;
    mpz_inits(tmp, tmp_sum, one, zero, NULL);
    mpz_set_ui(one, 1);
    mpz_set_ui(zero, 0);

    // we copy matrix m into m_ext and extend it with identity
    for (size_t i = 0; i < m->rows; i++) {
        for (size_t j = 0; j < m->cols; j++) {
            cfe_mat_get(tmp, m, i, j);
            cfe_mat_set(&m_ext, tmp, i, j);

        }
        for (size_t j = m->cols; j < 2 * m->cols; j++) {
            if (i+m->cols == j) {
                cfe_mat_set(&m_ext, one, i, j);
            } else {
                cfe_mat_set(&m_ext, zero, i, j);
            }
        }
    }

    // transform m into upper triangular matrix
    cfe_mat triang;
    cfe_mat_init(&triang, m->rows, 2*m->cols);
    cfe_mat_gaussian_elimination(&triang, &m_ext, p);

    // check if the inverse can be computed
    mpz_set(det, one);
    for (size_t i = 0; i < m->rows; i++) {
        cfe_mat_get(tmp, &triang, i, i);
        mpz_mul(det, det, tmp);
        mpz_mod(det, det, p);
    }
    if (mpz_cmp(det, zero) == 0) {
        err = CFE_ERR_NO_INVERSE;
        goto cleanup;
    }

    // use the upper triangular form to obtain the solution
    for (size_t k = 0; k < m->rows; k++) {
        size_t i = m->rows - 1;
        while (true) {
            mpz_set(tmp_sum, zero);
            for (size_t l = i + 1; l < m->cols; l++) {
                mpz_mul(tmp, triang.mat[i].vec[l], res->mat[l].vec[k]);
                mpz_add(tmp_sum, tmp_sum, tmp);
            }

            mpz_sub(res->mat[i].vec[k], triang.mat[i].vec[m->cols + k], tmp_sum);
            mpz_invert(tmp, triang.mat[i].vec[i], p);
            mpz_mul(res->mat[i].vec[k], res->mat[i].vec[k], tmp);
            mpz_mod(res->mat[i].vec[k], res->mat[i].vec[k], p);

            if (i == 0) {
                break;
            }
            i--;
        }
    }

    cleanup:
    cfe_mat_frees(&m_ext, &triang, NULL);
    mpz_clears(tmp, tmp_sum, one, zero, NULL);

    return err;
}

void cfe_mat_determinant_gauss(mpz_t det, cfe_mat *m, mpz_t p) {
    mpz_t tmp;
    mpz_init(tmp);

    // transform m into upper triangular matrix
    cfe_mat triang;
    cfe_mat_init(&triang, m->rows, m->cols);
    cfe_mat_gaussian_elimination(&triang, m, p);

    // compute the determinant
    mpz_set_ui(det, 1);
    for (size_t i = 0; i < m->rows; i++) {
        cfe_mat_get(tmp, &triang, i, i);
        mpz_mul(det, det, tmp);
        mpz_mod(det, det, p);
    }

    cfe_mat_free(&triang);
    mpz_clear(tmp);
}

cfe_error cfe_gaussian_elimination_solver(cfe_vec *res, cfe_mat *mat, cfe_vec *vec, mpz_t p) {
    cfe_error ret_error = CFE_ERR_NONE;

    cfe_mat m;
    cfe_vec v, vec_tmp;
    cfe_mat_init(&m, mat->rows, mat->cols);
    cfe_mat_copy(&m, mat);
    cfe_vec_init(&v, vec->size);
    cfe_vec_copy(&v, vec);
    cfe_mat_mod(&m, &m, p);
    cfe_vec_mod(&v, &v, p);

    mpz_t min_one, zero, tmp, tmp2, mhk_inv, lead_mul_inv;
    mpz_inits(min_one, zero, tmp, tmp2, mhk_inv, lead_mul_inv, NULL);

    mpz_set_si(min_one, -1);
    mpz_set_ui(zero, 0);
    cfe_vec_init(res, mat->cols);
    cfe_vec_set_const(res, min_one);
    size_t h = 0, k = 0;
    while (h < m.rows && k < m.cols) {
        bool is_zero = true;
        for (size_t i = h; i < m.rows; i++) {
            cfe_mat_get(tmp, &m, i, k);
            if (mpz_cmp_ui(tmp, 0) != 0) {
                vec_tmp = m.mat[i];
                m.mat[i] = m.mat[h];
                m.mat[h] = vec_tmp;

                mpz_set(tmp, v.vec[i]);
                mpz_set(v.vec[i], v.vec[h]);
                mpz_set(v.vec[h], tmp);
                is_zero = false;
                break;
            }
        }

        if (is_zero) {
            cfe_vec_set(res, zero, k);
            k++;
            continue;
        }
        cfe_mat_get(tmp, &m, h, k);
        mpz_invert(mhk_inv, tmp, p);
        for (size_t i = h + 1; i < m.rows; i++) {
            cfe_mat_get(tmp, &m, i, k);
            mpz_mul(lead_mul_inv, mhk_inv, tmp);
            cfe_mat_set(&m, zero, i, k);
            for (size_t j = k + 1; j < m.cols; j++) {
                cfe_mat_get(tmp, &m, i, j);
                cfe_mat_get(tmp2, &m, h, j);

                mpz_mul(tmp2, tmp2, lead_mul_inv);
                mpz_sub(tmp, tmp, tmp2);
                mpz_mod(tmp, tmp, p);

                cfe_mat_set(&m, tmp, i, j);
            }
            cfe_vec_get(tmp2, &v, h);
            cfe_vec_get(tmp, &v, i);

            mpz_mul(tmp2, tmp2, lead_mul_inv);
            mpz_sub(tmp, tmp, tmp2);
            mpz_mod(tmp, tmp, p);

            cfe_vec_set(&v, tmp, i);
        }
        k++;
        h++;
    }

    for (size_t i = h; i < m.rows; i++) {
        if (mpz_cmp_ui(v.vec[i], 0) != 0) {
            ret_error = CFE_ERR_NO_SOLUTION_EXISTS;
            cfe_vec_free(res);
            goto cleanup;
        }
    }

    for (size_t j = k; j < m.cols; j++) {
        cfe_vec_set(res, zero, j);
    }

    h--;
    k--;
    while (true) {
        if (mpz_cmp_si(res->vec[k], -1) == 0) {
            mpz_set_ui(tmp, 0);
            for (size_t l = k + 1; l < m.cols; l++) {
                cfe_mat_get(tmp2, &m, h, l);
                mpz_mul(tmp2, tmp2, res->vec[l]);
                mpz_add(tmp, tmp, tmp2);
            }
            mpz_sub(tmp, v.vec[h], tmp);
            cfe_mat_get(tmp2, &m, h, k);
            mpz_invert(tmp2, tmp2, p);
            mpz_mul(tmp, tmp, tmp2);
            mpz_mod(tmp, tmp, p);
            cfe_vec_set(res, tmp, k);
            h--;
        }
        if (k == 0) {
            break;
        }
        k--;
    }
    cleanup:
    mpz_clears(min_one, zero, tmp, tmp2, mhk_inv, lead_mul_inv, NULL);
    cfe_vec_free(&v);
    cfe_mat_free(&m);

    return ret_error;
}
