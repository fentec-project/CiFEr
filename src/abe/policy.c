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

#include <gmp.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "cifer/internal/common.h"
#include "cifer/abe/policy.h"
#include "cifer/internal/str.h"

cfe_error cfe_boolean_to_msp(cfe_msp *msp, char *bool_exp,
        size_t bool_exp_len, bool convert_to_ones) {
    cfe_string bool_exp_trimmed, bool_exp_basic;
    bool_exp_basic.str = bool_exp;
    bool_exp_basic.str_len = bool_exp_len;

    cfe_remove_spaces(&bool_exp_trimmed, &bool_exp_basic);
    cfe_vec vec;
    cfe_vec_init(&vec, 1);
    mpz_t zero, one;
    mpz_init_set_ui(zero, 0);
    mpz_init_set_ui(one, 1);

    cfe_vec_set(&vec, one, 0);
    cfe_error err = cfe_boolean_to_msp_iterative(msp, &bool_exp_trimmed, &vec, 1);
    cfe_string_free(&bool_exp_trimmed);
    if (err) {
        goto cleanup;
    }
    if (convert_to_ones) {
        cfe_mat inv_mat, msp_mat;
        cfe_mat_init(&inv_mat, msp->mat.cols, msp->mat.cols);
        cfe_mat_set_const(&inv_mat, zero);

        for (size_t i = 0; i < inv_mat.rows; i++) {
            cfe_mat_set(&inv_mat, one, 0, i);
            cfe_mat_set(&inv_mat, one, i, i);
        }
        cfe_mat_init(&msp_mat, msp->mat.rows, msp->mat.cols);
        cfe_mat_copy(&msp_mat, &(msp->mat));
        cfe_mat_mul(&(msp->mat), &msp_mat, &inv_mat);
        cfe_mat_frees(&msp_mat, &inv_mat, NULL);
    }
    cleanup:
    cfe_vec_free(&vec);
    mpz_clears(one, zero, NULL);
    return err;

}

cfe_error cfe_boolean_to_msp_iterative(cfe_msp *msp, cfe_string *bool_exp, cfe_vec *vec, size_t c) {
    size_t num_brc = 0;
    cfe_string bool_exp1, bool_exp2;
    cfe_error err;
    cfe_msp msp1, msp2;
    cfe_vec vec1, vec2;
    bool found = false;

    for (size_t i = 0; i < bool_exp->str_len; i++) {
        if (bool_exp->str[i] == '(') {
            num_brc++;
            continue;
        }
        if (bool_exp->str[i] == ')') {
            num_brc--;
            continue;
        }
        if (num_brc == 0 && i < bool_exp->str_len - 3 && bool_exp->str[i] == 'A' &&
            bool_exp->str[i + 1] == 'N' && bool_exp->str[i + 2] == 'D') {
            cfe_substring(&bool_exp1, bool_exp, 0, i);
            cfe_init_set_vecs_and(&vec1, &vec2, vec, c);

            err = cfe_boolean_to_msp_iterative(&msp1, &bool_exp1, &vec1, c + 1);
            cfe_string_free(&bool_exp1);
            cfe_vec_free(&vec1);
            if (err) {
                cfe_vec_free(&vec2);
                return err;
            }
            cfe_substring(&bool_exp2, bool_exp, i + 3, bool_exp->str_len);
            err = cfe_boolean_to_msp_iterative(&msp2, &bool_exp2, &vec2, msp1.mat.cols);
            cfe_string_free(&bool_exp2);
            cfe_vec_free(&vec2);
            if (err) {
                cfe_msp_free(&msp1);
                return err;
            }
            found = true;
            break;
        }
        if (num_brc == 0 && i < bool_exp->str_len - 2 && bool_exp->str[i] == 'O' &&
            bool_exp->str[i + 1] == 'R') {
            cfe_substring(&bool_exp1, bool_exp, 0, i);
            err = cfe_boolean_to_msp_iterative(&msp1, &bool_exp1, vec, c);
            cfe_string_free(&bool_exp1);
            if (err) {
                return err;
            }
            cfe_substring(&bool_exp2, bool_exp, i + 2, bool_exp->str_len);
            err = cfe_boolean_to_msp_iterative(&msp2, &bool_exp2, vec, msp1.mat.cols);
            cfe_string_free(&bool_exp2);
            if (err) {
                cfe_msp_free(&msp1);
                return err;
            }
            found = true;
            break;
        }

    }
    if (found == false) {
        if (bool_exp->str[0] == '(' && bool_exp->str[bool_exp->str_len - 1] == ')') {
            cfe_substring(&bool_exp1, bool_exp, 1, bool_exp->str_len - 1);
            err = cfe_boolean_to_msp_iterative(msp, &bool_exp1, vec, c);
            cfe_string_free(&bool_exp1);
            return err;
        }

        int attrib = cfe_str_to_int(bool_exp);
        if (attrib == -1) {
            return CFE_ERR_CORRUPTED_BOOL_EXPRESSION;
        }

        cfe_mat_init(&(msp->mat), 1, c);
        mpz_t zero;
        mpz_init_set_ui(zero, 0);
        for (size_t i = 0; i < c; i++) {
            if (i < vec->size) {
                cfe_mat_set(&(msp->mat), vec->vec[i], 0, i);
            } else {
                cfe_mat_set(&(msp->mat), zero, 0, i);
            }
        }
        mpz_clear(zero);

        msp->row_to_attrib = (int *) cfe_malloc(sizeof(int) * 1);
        msp->row_to_attrib[0] = attrib;
        return CFE_ERR_NONE;
    } else {
        msp->row_to_attrib = (int *) cfe_malloc(sizeof(int) * (msp1.mat.rows + msp2.mat.rows));
        cfe_mat_init(&(msp->mat), msp1.mat.rows + msp2.mat.rows, msp2.mat.cols);
        mpz_t tmp;
        mpz_init(tmp);
        for (size_t i = 0; i < msp1.mat.rows; i++) {
            for (size_t j = 0; j < msp1.mat.cols; j++) {
                cfe_mat_get(tmp, &(msp1.mat), i, j);
                cfe_mat_set(&(msp->mat), tmp, i, j);
            }
            mpz_set_ui(tmp, 0);
            for (size_t j = msp->mat.cols; j < msp2.mat.cols; j++) {
                cfe_mat_set(&(msp->mat), tmp, i, j);
            }
            msp->row_to_attrib[i] = msp1.row_to_attrib[i];
        }
        for (size_t i = 0; i < msp2.mat.rows; i++) {
            for (size_t j = 0; j < msp2.mat.cols; j++) {
                cfe_mat_get(tmp, &(msp2.mat), i, j);
                cfe_mat_set(&(msp->mat), tmp, i + msp1.mat.rows, j);
            }
            msp->row_to_attrib[i + msp1.mat.rows] = msp2.row_to_attrib[i];
        }
        mpz_clear(tmp);
        cfe_msp_free(&msp1);
        cfe_msp_free(&msp2);
        return CFE_ERR_NONE;
    }
}

// cfe_init_set_vecs_and is a helping function that given a vector and a counter
// creates two new vectors used whenever an AND gate is found in an iterative
// step of boolean_to_msp
void cfe_init_set_vecs_and(cfe_vec *vec1, cfe_vec *vec2, cfe_vec *vec, size_t c) {
    mpz_t zero;
    mpz_init_set_ui(zero, 0);
    cfe_vec_inits(c + 1, vec1, vec2, NULL);
    cfe_vec_set_const(vec1, zero);
    cfe_vec_set_const(vec2, zero);
    for (size_t i = 0; i < vec->size; i++) {
        cfe_vec_set(vec2, vec->vec[i], i);
    }
    mpz_set_si(vec1->vec[c], -1);
    mpz_set_si(vec2->vec[c], 1);
    mpz_clear(zero);
}

void cfe_msp_free(cfe_msp *msp) {
    cfe_mat_free(&(msp->mat));
    free(msp->row_to_attrib);
}
