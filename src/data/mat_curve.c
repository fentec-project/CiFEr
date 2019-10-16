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

#include "cifer/data/mat_curve.h"
#include <amcl/big_256_56.h>
#include <amcl/ecp2_BN254.h>
#include <amcl/ecp_BN254.h>
#include <amcl/pair_BN254.h>
#include <assert.h>
#include "cifer/internal/big.h"
#include "cifer/internal/common.h"

void cfe_mat_G1_init(cfe_mat_G1 *m, size_t rows, size_t cols) {
    m->rows = rows;
    m->cols = cols;
    m->mat = (cfe_vec_G1 *)cfe_malloc(rows * sizeof(cfe_vec_G1));

    for (size_t i = 0; i < rows; i++) {
        cfe_vec_G1_init(&(m->mat[i]), cols);
    }
}

void cfe_mat_mul_G1(cfe_mat_G1 *m, cfe_mat *u) {
    assert(m->cols == u->cols);
    assert(m->rows == u->rows);

    BIG_256_56 x;
    for (size_t i = 0; i < m->rows; i++) {
        for (size_t j = 0; j < m->cols; j++) {
            ECP_BN254_generator(&(m->mat[i].vec[j]));
            BIG_256_56_from_mpz(x, u->mat[i].vec[j]);
            ECP_BN254_mul(&(m->mat[i].vec[j]), x);
        }
    }
}

void cfe_mat_G1_transpose(cfe_mat_G1 *res, cfe_mat_G1 *m) {
    assert(res->rows == m->cols);
    assert(res->cols == m->rows);

    for (size_t i = 0; i < m->rows; i++) {
        for (size_t k = 0; k < m->cols; k++) {
            ECP_BN254_copy(&(res->mat[k].vec[i]), &(m->mat[i].vec[k]));
        }
    }
}

void cfe_mat_mul_G1_mat(cfe_mat_G1 *res, cfe_mat *mi, cfe_mat_G1 *m) {
    assert(m->rows == mi->cols);
    assert(res->cols == m->cols);
    assert(res->rows == mi->rows);

    ECP_BN254 g;
    BIG_256_56 x;

    for (size_t i = 0; i < m->rows; i++) {
        for (size_t j = 0; j < m->cols; j++) {
            ECP_BN254_inf(&(res->mat[i].vec[j]));
            for (size_t k = 0; k < m->rows; k++) {
                ECP_BN254_copy(&g, &(m->mat[k].vec[j]));
                BIG_256_56_from_mpz(x, mi->mat[i].vec[k]);
                ECP_BN254_mul(&g, x);
                ECP_BN254_add(&(res->mat[i].vec[j]), &g);
            }
        }
    }
}

void cfe_mat_G1_mul_vec(cfe_vec_G1 *res, cfe_mat_G1 *m, cfe_vec *u) {
    assert(m->rows == res->size);
    assert(m->cols == u->size);

    ECP_BN254 g;
    BIG_256_56 x;

    for (size_t i = 0; i < m->rows; i++) {
        ECP_BN254_inf(&(res->vec[i]));
        for (size_t k = 0; k < m->cols; k++) {
            ECP_BN254_copy(&g, &(m->mat[i].vec[k]));
            BIG_256_56_from_mpz(x, u->vec[k]);
            ECP_BN254_mul(&g, x);
            ECP_BN254_add(&(res->vec[i]), &g);
        }
    }
}

void cfe_mat_G1_free(cfe_mat_G1 *m) {
    for (size_t i = 0; i < m->rows; i++) {
        cfe_vec_G1_free(&(m->mat[i]));
    }
    free(m->mat);
}

void cfe_mat_G2_init(cfe_mat_G2 *m, size_t rows, size_t cols) {
    m->rows = rows;
    m->cols = cols;
    m->mat = (cfe_vec_G2 *)cfe_malloc(rows * sizeof(cfe_vec_G2));

    for (size_t i = 0; i < rows; i++) {
        cfe_vec_G2_init(&(m->mat[i]), cols);
    }
}

void cfe_mat_G2_transpose(cfe_mat_G2 *res, cfe_mat_G2 *m) {
    assert(res->rows == m->cols);
    assert(res->cols == m->rows);

    for (size_t i = 0; i < m->rows; i++) {
        for (size_t k = 0; k < m->cols; k++) {
            ECP2_BN254_copy(&(res->mat[k].vec[i]), &(m->mat[i].vec[k]));
        }
    }
}

void cfe_mat_mul_G2_mat(cfe_mat_G2 *res, cfe_mat *mi, cfe_mat_G2 *m) {
    assert(m->rows == mi->cols);
    assert(res->cols == m->cols);
    assert(res->rows == mi->rows);

    ECP2_BN254 g;
    BIG_256_56 x;

    for (size_t i = 0; i < m->rows; i++) {
        for (size_t j = 0; j < m->cols; j++) {
            ECP2_BN254_inf(&(res->mat[i].vec[j]));
            for (size_t k = 0; k < m->rows; k++) {
                ECP2_BN254_copy(&g, &(m->mat[k].vec[j]));
                BIG_256_56_from_mpz(x, mi->mat[i].vec[k]);
                ECP2_BN254_mul(&g, x);
                ECP2_BN254_add(&(res->mat[i].vec[j]), &g);
            }
        }
    }
}

void cfe_mat_G2_mul_vec(cfe_vec_G2 *res, cfe_mat_G2 *m, cfe_vec *u) {
    assert(m->rows == res->size);
    assert(m->cols == u->size);

    ECP2_BN254 g;
    BIG_256_56 x;

    for (size_t i = 0; i < m->rows; i++) {
        ECP2_BN254_inf(&(res->vec[i]));
        for (size_t k = 0; k < m->cols; k++) {
            ECP2_BN254_copy(&g, &(m->mat[i].vec[k]));
            BIG_256_56_from_mpz(x, u->vec[k]);
            ECP2_BN254_mul(&g, x);
            ECP2_BN254_add(&(res->vec[i]), &g);
        }
    }
}

void cfe_mat_mul_G2(cfe_mat_G2 *m, cfe_mat *u) {
    assert(m->cols == u->cols);
    assert(m->rows == u->rows);

    BIG_256_56 x;
    for (size_t i = 0; i < m->rows; i++) {
        for (size_t j = 0; j < m->cols; j++) {
            ECP2_BN254_generator(&(m->mat[i].vec[j]));
            BIG_256_56_from_mpz(x, u->mat[i].vec[j]);
            ECP2_BN254_mul(&(m->mat[i].vec[j]), x);
        }
    }
}

void cfe_mat_G2_free(cfe_mat_G2 *m) {
    for (size_t i = 0; i < m->rows; i++) {
        cfe_vec_G2_free(&(m->mat[i]));
    }
    free(m->mat);
}

void cfe_mat_GT_init(cfe_mat_GT *m, size_t rows, size_t cols) {
    m->rows = rows;
    m->cols = cols;
    m->mat = (cfe_vec_GT *)cfe_malloc(rows * sizeof(cfe_vec_GT));

    for (size_t i = 0; i < rows; i++) {
        cfe_vec_GT_init(&(m->mat[i]), cols);
    }
}

void cfe_mat_GT_transpose(cfe_mat_GT *res, cfe_mat_GT *m) {
    assert(res->rows == m->cols);
    assert(res->cols == m->rows);

    for (size_t i = 0; i < m->rows; i++) {
        for (size_t k = 0; k < m->cols; k++) {
            FP12_BN254_copy(&(res->mat[k].vec[i]), &(m->mat[i].vec[k]));
        }
    }
}

void cfe_mat_mul_GT_mat(cfe_mat_GT *res, cfe_mat *mi, cfe_mat_GT *m) {
    assert(m->rows == mi->cols);
    assert(res->cols == m->cols);
    assert(res->rows == mi->rows);

    FP12_BN254 g;
    BIG_256_56 x;

    for (size_t i = 0; i < m->rows; i++) {
        for (size_t j = 0; j < m->cols; j++) {
            FP12_BN254_one(&(res->mat[i].vec[j]));
            for (size_t k = 0; k < m->rows; k++) {
                BIG_256_56_from_mpz(x, mi->mat[i].vec[k]);
                FP12_BN254_pow(&g, &(m->mat[k].vec[j]), x);
                FP12_BN254_mul(&(res->mat[i].vec[j]), &g);
            }
        }
    }
}

void cfe_mat_GT_mul_vec(cfe_vec_GT *res, cfe_mat_GT *m, cfe_vec *u) {
    assert(m->rows == res->size);
    assert(m->cols == u->size);

    FP12_BN254 g;
    BIG_256_56 x;

    for (size_t i = 0; i < m->rows; i++) {
        FP12_BN254_one(&(res->vec[i]));
        for (size_t k = 0; k < m->cols; k++) {
            BIG_256_56_from_mpz(x, u->vec[k]);
            FP12_BN254_pow(&g, &(m->mat[i].vec[k]), x);
            FP12_BN254_mul(&(res->vec[i]), &g);
        }
    }
}

void cfe_mat_GT_pair_mat_G1(cfe_mat_GT *res, cfe_mat_G1 *m) {
    assert(res->rows == m->rows);
    assert(res->cols == m->cols);

    ECP2_BN254 g;
    ECP2_BN254_generator(&g);

    for (size_t i = 0; i < m->rows; i++) {
        for (size_t j = 0; j < m->cols; j++) {
            PAIR_BN254_ate(&(res->mat[i].vec[j]), &g, &(m->mat[i].vec[j]));
            PAIR_BN254_fexp(&(res->mat[i].vec[j]));
        }
    }
}

void cfe_mat_GT_pair_mat_G2(cfe_mat_GT *res, cfe_mat_G2 *m) {
    assert(res->rows == m->rows);
    assert(res->cols == m->cols);

    ECP_BN254 g;
    ECP_BN254_generator(&g);

    for (size_t i = 0; i < m->rows; i++) {
        for (size_t j = 0; j < m->cols; j++) {
            PAIR_BN254_ate(&(res->mat[i].vec[j]), &(m->mat[i].vec[j]), &g);
            PAIR_BN254_fexp(&(res->mat[i].vec[j]));
        }
    }
}

void cfe_mat_GT_free(cfe_mat_GT *m) {
    for (size_t i = 0; i < m->rows; i++) {
        cfe_vec_GT_free(&(m->mat[i]));
    }
    free(m->mat);
}
