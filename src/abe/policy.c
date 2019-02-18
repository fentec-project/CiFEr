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

#include <gmp.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <amcl/big_256_56.h>
#include "internal/common.h"
#include "abe/policy.h"


void boolean_to_msp(cfe_msp *msp, char *bool_exp, bool convert_to_ones) {
    char *bool_exp_trimmed = remove_spaces(bool_exp);
    cfe_vec vec;
    cfe_vec_init(&vec, 1);
    mpz_t zero, one;
    mpz_init_set_ui(zero, 0);
    mpz_init_set_ui(one, 1);

    cfe_vec_set(&vec, one, 0);
    boolean_to_msp_iterative(msp, bool_exp_trimmed, &vec, 1);

    if (convert_to_ones) {
        cfe_mat inv_mat, msp_mat;
        cfe_mat_const(&inv_mat, msp->mat->cols, msp->mat->cols, zero);

        for (size_t i = 0; i < inv_mat.rows; i++) {
            cfe_mat_set(&inv_mat, one, 0, i);
            cfe_mat_set(&inv_mat, one, i, i);
        }

        cfe_mat_copy(&msp_mat, msp->mat);
        cfe_mat_mul(msp->mat, &msp_mat, &inv_mat);
        cfe_mat_frees(&msp_mat, &inv_mat, NULL);
    }

    cfe_vec_free(&vec);
    mpz_clear(one);
}

size_t boolean_to_msp_iterative(cfe_msp *msp, char *bool_exp, cfe_vec *vec, size_t c) {
    size_t num_brc = 0;
    char *bool_exp1, *bool_exp2;
    size_t c1, c_out;
    cfe_msp msp1, msp2;
    bool found = false;

    for (size_t i = 0; i < strlen(bool_exp); i++) {
        if (bool_exp[i] == '(') {
            num_brc++;
            continue;
        }
        if (bool_exp[i] == ')') {
            num_brc--;
            continue;
        }
        if (num_brc == 0 && i < strlen(bool_exp) - 3 && bool_exp[i] == 'A' &&
                bool_exp[i+1] == 'N' && bool_exp[i+2] == 'D') {
            bool_exp1 = substring(bool_exp, 0, i);
            bool_exp2 = substring(bool_exp, i + 3, strlen(bool_exp));

            cfe_vec vec1, vec2;
            make_and_vecs(&vec1, &vec2, vec, c);
            c1 = boolean_to_msp_iterative(&msp1, bool_exp1, &vec1, c + 1);
            c_out = boolean_to_msp_iterative(&msp2, bool_exp2, &vec2, c1);
            found = true;
            break;
        }
        if (num_brc == 0 && i < strlen(bool_exp) - 2 && bool_exp[i] == 'O' &&
                bool_exp[i+1] == 'R') {
            bool_exp1 = substring(bool_exp, 0, i);
            bool_exp2 = substring(bool_exp, i + 2, strlen(bool_exp));
            c1 = boolean_to_msp_iterative(&msp1, bool_exp1, vec, c);
            c_out = boolean_to_msp_iterative(&msp2, bool_exp2, vec, c1);
            found = true;
            break;
        }

    }
    if (found == false) {
        if (bool_exp[0] == '(' && bool_exp[strlen(bool_exp) - 1] == ')') {
            bool_exp = substring(bool_exp, 1, strlen(bool_exp) - 1);
            return boolean_to_msp_iterative(msp, bool_exp, vec, c);
        }

        int attrib = atoi(bool_exp); // todo: error handling change to strtol

        msp->mat = (cfe_mat*) cfe_malloc(sizeof(cfe_mat));
        cfe_mat_init(msp->mat, 1, c);
        mpz_t zero;
        mpz_init_set_ui(zero, 0);
        for (size_t i = 0; i< c; i++) {
            if (i < vec->size) {
                cfe_mat_set(msp->mat, vec->vec[i], 0, i);
            } else {
                cfe_mat_set(msp->mat, zero, 0, i);
            }
        }

        msp->row_to_attrib = (int*) cfe_malloc(sizeof(int) * 1);
        msp->row_to_attrib[0] = attrib;
        return c;
    } else {
        msp->mat = (cfe_mat*) cfe_malloc(sizeof(cfe_mat));
        msp->row_to_attrib = (int*) cfe_malloc(sizeof(int) * (msp1.mat->rows + msp2.mat->rows));
        cfe_mat_init(msp->mat, msp1.mat->rows + msp2.mat->rows, c_out);
        mpz_t tmp;
        mpz_init(tmp);
        for (size_t i = 0; i < msp1.mat->rows; i++) {
            for (size_t j = 0; j < msp1.mat->cols; j++) {
                cfe_mat_get(tmp, msp1.mat, i, j);
                cfe_mat_set(msp->mat, tmp, i, j);
            }
            mpz_set_ui(tmp, 0);
            for (size_t j = msp->mat->cols; j < c_out; j++) {
                cfe_mat_set(msp->mat, tmp, i, j);
            }
            msp->row_to_attrib[i] = msp1.row_to_attrib[i];
        }
        for (size_t i = 0; i < msp2.mat->rows; i++) {
            for (size_t j = 0; j < c_out; j++) {
                cfe_mat_get(tmp, msp2.mat, i, j);
                cfe_mat_set(msp->mat, tmp, i + msp1.mat->rows, j);
            }
            msp->row_to_attrib[i + msp1.mat->rows] = msp2.row_to_attrib[i];
        }
        return c_out;
    }
}

void make_and_vecs(cfe_vec *vec1, cfe_vec *vec2, cfe_vec *vec, size_t c) {
    mpz_t zero;
    mpz_init_set_ui(zero, 0);
    cfe_vec_const(vec1, c + 1, zero);
    cfe_vec_const(vec2, c + 1, zero);
    for (size_t i = 0; i < vec->size; i++) {
        cfe_vec_set(vec2, vec->vec[i], i);
    }
    mpz_set_si(vec1->vec[c], -1);
    mpz_set_si(vec2->vec[c], 1);
    mpz_clear(zero);
}


char *substring(char *s, size_t start, size_t stop) {
    char *sub = (char*) cfe_malloc(sizeof(char)*(stop - start + 1));
    for (size_t i = start; i < stop; i++) {
        sub[i - start] = s[i];
    }
    sub[stop - start] = '\0';

    return sub;
}

char *remove_spaces(char* source) {
    size_t count = 0;
    for (size_t i = 0; i < strlen(source); i++) {
        if(source[i] != ' '){
            count++;
        }
    }

    char *res = (char*) cfe_malloc(sizeof(char)*(count + 1));
    count = 0;
    for (size_t i = 0; i < strlen(source); i++) {
        if(source[i] != ' '){
            res[count] = source[i];
            count++;
        }
    }
    res[count] = '\0';

    return res;
}

void cfe_msp_free(cfe_msp *msp) {
    cfe_mat_free(msp->mat);
    free(msp->row_to_attrib);
}

int gaussian_elimination(cfe_vec *res, cfe_mat *mat, cfe_vec *vec, mpz_t p) {
    int ret_error = 0;
    // copy mat and vec in m and v (mod p)
    cfe_mat m;
    cfe_vec v, vec_tmp;
    cfe_vec_init(&v, vec->size);
    cfe_mat_copy(&m, mat);
    cfe_vec_copy(&v, vec);
    cfe_mat_mod(&m, &m, p);
    cfe_vec_mod(&v, &v, p);

    mpz_t min_one, zero, tmp, tmp2, mhk_inv, lead_mul_inv;
    mpz_inits(min_one, zero, tmp, tmp2, mhk_inv, lead_mul_inv, NULL);

    mpz_set_si(min_one, -1);
    mpz_set_ui(zero, 0);
    cfe_vec_const(res, mat->cols, min_one);
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
            ret_error = 1;
            goto clearup;
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
    clearup:
    mpz_clears(min_one, zero, tmp, tmp2, mhk_inv, lead_mul_inv, NULL);
    cfe_vec_free(&v);
    cfe_mat_free(&m);

    return ret_error;
}
