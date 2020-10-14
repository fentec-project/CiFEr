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
#include <amcl/pair_BN254.h>
#include "cifer/internal/big.h"

#include "cifer/data/vec.h"
#include "cifer/data/vec_curve.h"
#include "cifer/internal/common.h"

void cfe_vec_G1_init(cfe_vec_G1 *v, size_t size) {
    v->size = size;
    v->vec = (ECP_BN254 *) cfe_malloc(size * sizeof(ECP_BN254));
}

void cfe_vec_G1_inf(cfe_vec_G1 *v) {
    for (size_t i = 0; i < v->size; i++) {
        ECP_BN254_inf(&(v->vec[i]));
    }
}

void cfe_vec_mul_G1(cfe_vec_G1 *v, cfe_vec *u) {
    assert(v->size == u->size);
    mpz_t neg_x;
    mpz_init(neg_x);

    BIG_256_56 x;
    for (size_t i = 0; i < u->size; i++) {
        if (mpz_cmp_si(u->vec[i], 0) > 0) {
            ECP_BN254_generator(&(v->vec[i]));
            BIG_256_56_from_mpz(x, u->vec[i]);
            ECP_BN254_mul(&(v->vec[i]), x);
        }
        if (mpz_cmp_si(u->vec[i], 0) < 0) {
            ECP_BN254_generator(&(v->vec[i]));
            ECP_BN254_neg(&(v->vec[i]));
            mpz_neg(neg_x, u->vec[i]);
            BIG_256_56_from_mpz(x, neg_x);
            ECP_BN254_mul(&(v->vec[i]), x);
        }

        if (mpz_cmp_si(u->vec[i], 0) == 0) {
            ECP_BN254_inf(&(v->vec[i]));
        }
    }

    mpz_clear(neg_x);
}

void cfe_vec_mul_vec_G1(cfe_vec_G1 *res, cfe_vec *u, cfe_vec_G1 *v) {
    assert(v->size == u->size);
    assert(v->size == res->size);

    BIG_256_56 x;
    for (size_t i = 0; i < res->size; i++) {
        ECP_BN254_copy(&(res->vec[i]), &(v->vec[i]));
        BIG_256_56_from_mpz(x, u->vec[i]);
        ECP_BN254_mul(&(res->vec[i]), x);
    }
}

void cfe_vec_G2_init(cfe_vec_G2 *v, size_t size) {
    v->size = size;
    v->vec = (ECP2_BN254 *) cfe_malloc(size * sizeof(ECP2_BN254));
}

void cfe_vec_G2_inf(cfe_vec_G2 *v) {
    for (size_t i = 0; i < v->size; i++) {
        ECP2_BN254_inf(&(v->vec[i]));
    }
}

void cfe_vec_mul_G2(cfe_vec_G2 *v, cfe_vec *u) {
    assert(v->size == u->size);
    BIG_256_56 x;
    mpz_t neg_x;
    mpz_init(neg_x);

    for (size_t i = 0; i < u->size; i++) {
        if (mpz_cmp_si(u->vec[i], 0) > 0) {
            ECP2_BN254_generator(&(v->vec[i]));
            BIG_256_56_from_mpz(x, u->vec[i]);
            ECP2_BN254_mul(&(v->vec[i]), x);
        }
        if (mpz_cmp_si(u->vec[i], 0) < 0) {
            ECP2_BN254_generator(&(v->vec[i]));
            ECP2_BN254_neg(&(v->vec[i]));
            mpz_neg(neg_x, u->vec[i]);
            BIG_256_56_from_mpz(x, neg_x);
            ECP2_BN254_mul(&(v->vec[i]), x);
        }

        if (mpz_cmp_si(u->vec[i], 0) == 0) {
            ECP2_BN254_inf(&(v->vec[i]));
        }
    }

    mpz_clear(neg_x);
}

void cfe_vec_mul_vec_G2(cfe_vec_G2 *res, cfe_vec *u, cfe_vec_G2 *v) {
    assert(v->size == u->size);
    assert(v->size == res->size);

    BIG_256_56 x;
    for (size_t i = 0; i < u->size; i++) {
        ECP2_BN254_copy(&(res->vec[i]), &(v->vec[i]));
        BIG_256_56_from_mpz(x, u->vec[i]);
        ECP2_BN254_mul(&(res->vec[i]), x);
    }
}

void cfe_vec_GT_init(cfe_vec_GT *v, size_t size) {
    v->size = size;
    v->vec = (FP12_BN254 *) cfe_malloc(size * sizeof(FP12_BN254));
}

void cfe_vec_GT_inf(cfe_vec_GT *v) {
    for (size_t i = 0; i < v->size; i++) {
        FP12_BN254_one(&(v->vec[i]));
    }
}

void cfe_vec_mul_GT(cfe_vec_GT *v, cfe_vec *u) {
    assert(v->size == u->size);
    BIG_256_56 x;

    ECP_BN254 g1;
    ECP_BN254_generator(&g1);

    ECP2_BN254 g2;
    ECP2_BN254_generator(&g2);

    FP12_BN254 gt;
    PAIR_BN254_ate(&gt, &g2, &g1);
    PAIR_BN254_fexp(&gt);

    for (size_t i = 0; i < u->size; i++) {
        BIG_256_56_from_mpz(x, u->vec[i]);
        FP12_BN254_pow(&(v->vec[i]), &gt, x);
    }
}

void cfe_vec_G1_free(cfe_vec_G1 *v) {
    free(v->vec);
}

void cfe_vec_G2_free(cfe_vec_G2 *v) {
    free(v->vec);
}

void cfe_vec_GT_free(cfe_vec_GT *v) {
    free(v->vec);
}
