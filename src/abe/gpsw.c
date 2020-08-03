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
#include <string.h>
#include <amcl/big_256_56.h>
#include <amcl/pair_BN254.h>

#include "cifer/internal/common.h"
#include "cifer/internal/big.h"
#include "cifer/sample/uniform.h"
#include "cifer/abe/policy.h"
#include "cifer/abe/gpsw.h"

void cfe_gpsw_init(cfe_gpsw *gpsw, size_t l) {
    gpsw->l = l;
    mpz_init(gpsw->p);
    mpz_from_BIG_256_56(gpsw->p, (int64_t *) CURVE_Order_BN254);
}

void cfe_gpsw_master_keys_init(cfe_gpsw_pub_key *pk, cfe_vec *sk, cfe_gpsw *gpsw) {
    cfe_vec_G2_init(&(pk->t), gpsw->l);
    cfe_vec_init(sk, gpsw->l + 1);
}

void cfe_gpsw_generate_master_keys(cfe_gpsw_pub_key *pk, cfe_vec *sk, cfe_gpsw *gpsw) {
    cfe_uniform_sample_vec(sk, gpsw->p);

    cfe_vec sub_sk;
    cfe_vec_init(&sub_sk, gpsw->l);
    cfe_vec_extract(&sub_sk, sk, 0, gpsw->l);
    cfe_vec_mul_G2(&(pk->t), &sub_sk);

    ECP_BN254 g1;
    ECP2_BN254 g2;
    FP12_BN254 gT;
    ECP_BN254_generator(&g1);
    ECP2_BN254_generator(&g2);
    PAIR_BN254_ate(&gT, &g2, &g1);
    PAIR_BN254_fexp(&gT);
    BIG_256_56 x;
    BIG_256_56_from_mpz(x, sk->vec[gpsw->l]);
    FP12_BN254_pow(&(pk->y), &gT, x);

    cfe_vec_free(&sub_sk);
}

void cfe_gpsw_cipher_init(cfe_gpsw_cipher *cipher, size_t num_attrib) {
    cfe_vec_G2_init(&(cipher->e), num_attrib);
    cipher->gamma = (int *) cfe_malloc(num_attrib * sizeof(int));
}

void cfe_gpsw_encrypt(cfe_gpsw_cipher *cipher, cfe_gpsw *gpsw, FP12_BN254 *msg,
                      int *gamma, size_t num_attrib, cfe_gpsw_pub_key *pk) {
    mpz_t s;
    mpz_init(s);
    cfe_uniform_sample(s, gpsw->p);
    BIG_256_56 s_big;
    BIG_256_56_from_mpz(s_big, s);
    FP12_BN254_pow(&(cipher->e0), &(pk->y), s_big);
    FP12_BN254_mul(&(cipher->e0), msg);

    for (size_t i = 0; i < num_attrib; i++) {
        cipher->gamma[i] = gamma[i];
        cipher->e.vec[i] = (pk->t).vec[gamma[i]];
        ECP2_BN254_mul(&(cipher->e.vec[i]), s_big);
    }

    mpz_clear(s);
}

void cfe_gpsw_rand_vec_const_sum(cfe_vec *v, mpz_t y, mpz_t p) {
    cfe_uniform_sample_vec(v, p);
    mpz_t sum;
    mpz_init_set_ui(sum, 0);
    for (size_t i = 0; i < v->size - 1; i++) {
        mpz_add(sum, sum, v->vec[i]);
    }
    mpz_sub(v->vec[v->size - 1], y, sum);
    mpz_mod(v->vec[v->size - 1], v->vec[v->size - 1], p);
    mpz_clear(sum);
}

void cfe_gpsw_key_init(cfe_gpsw_key *policy_key, cfe_msp *msp) {
    cfe_vec_G1_init(&(policy_key->d), msp->mat.rows);
    cfe_mat_init(&(policy_key->msp.mat), msp->mat.rows, msp->mat.cols);
    policy_key->msp.row_to_attrib = (int *) cfe_malloc(msp->mat.rows * sizeof(int));
}

void cfe_gpsw_generate_policy_key(cfe_gpsw_key *policy_key, cfe_gpsw *gpsw,
                                   cfe_msp *msp, cfe_vec *sk) {
    cfe_vec u;
    cfe_vec_init(&u, msp->mat.cols);
    cfe_gpsw_rand_vec_const_sum(&u, sk->vec[gpsw->l], gpsw->p);

    mpz_t t_map_i_inv, mat_times_u, pow;
    mpz_inits(t_map_i_inv, mat_times_u, pow, NULL);
    BIG_256_56 pow_big;
    cfe_mat_copy(&policy_key->msp.mat, &msp->mat);
    for (size_t i = 0; i < msp->mat.rows; i++) {
        mpz_invert(t_map_i_inv, sk->vec[msp->row_to_attrib[i]], gpsw->p);
        cfe_vec_dot(mat_times_u, &(msp->mat.mat[i]), &u);
        mpz_mul(pow, t_map_i_inv, mat_times_u);
        mpz_mod(pow, pow, gpsw->p);
        BIG_256_56_from_mpz(pow_big, pow);

        ECP_BN254_generator(&(policy_key->d.vec[i]));
        ECP_BN254_mul(&(policy_key->d.vec[i]), pow_big);

        policy_key->msp.row_to_attrib[i] = msp->row_to_attrib[i];
    }

    cfe_vec_free(&u);
    mpz_clears(t_map_i_inv, mat_times_u, pow, NULL);
}

cfe_error cfe_gpsw_decrypt(FP12_BN254 *res, cfe_gpsw_cipher *cipher, cfe_gpsw_key *key, cfe_gpsw *gpsw) {
    size_t count_attrib = 0;
    cfe_error err = CFE_ERR_NONE;

    for (size_t i = 0; i < key->msp.mat.rows; i++) {
        for (size_t j = 0; j < cipher->e.size; j++) {
            if (key->msp.row_to_attrib[i] == cipher->gamma[j]) {
                count_attrib++;
                break;
            }
        }
    }

    cfe_mat mat;
    cfe_mat_init(&mat, count_attrib, key->msp.mat.cols);
    cfe_vec_G1 d;
    cfe_vec_G1_init(&d, count_attrib);

    size_t counter = 0;
    size_t *positions = (size_t *) cfe_malloc(sizeof(size_t) * count_attrib);

    for (size_t i = 0; i < key->msp.mat.rows; i++) {
        for (size_t j = 0; j < cipher->e.size; j++) {
            if (key->msp.row_to_attrib[i] == cipher->gamma[j]) {
                cfe_mat_set_vec(&mat, &(key->msp.mat.mat[i]), counter);
                ECP_BN254_copy(&d.vec[counter], &(key->d.vec[i]));
                positions[counter] = j;
                counter++;
                break;
            }
        }
    }

    cfe_vec one_vec, alpha;
    mpz_t one;
    mpz_init_set_ui(one, 1);
    cfe_vec_init(&one_vec, mat.cols);
    cfe_vec_set_const(&one_vec, one);
    cfe_mat mat_transpose;
    cfe_mat_init(&mat_transpose, mat.cols, mat.rows);
    cfe_mat_transpose(&mat_transpose, &mat);
    cfe_error check = cfe_gaussian_elimination_solver(&alpha, &mat_transpose, &one_vec, gpsw->p);
    cfe_mat_free(&mat_transpose);
    cfe_vec_free(&one_vec);
    mpz_clear(one);
    if (check) {
        err = CFE_ERR_INSUFFICIENT_KEYS;
        goto cleanup;
    }

    FP12_BN254_copy(res, &(cipher->e0));
    FP12_BN254 pair;
    FP12_BN254 pair_pow;
    FP12_BN254 pair_pow_inv;

    BIG_256_56 alpha_i;
    for (size_t i = 0; i < mat.rows; i++) {
        if (mpz_cmp_ui(alpha.vec[i], 0) == 0) {
            continue;
        }
        PAIR_BN254_ate(&pair, &(cipher->e.vec[positions[i]]), &d.vec[i]);
        PAIR_BN254_fexp(&pair);
        BIG_256_56_from_mpz(alpha_i, alpha.vec[i]);
        FP12_BN254_pow(&pair_pow, &pair, alpha_i);

        FP12_BN254_inv(&pair_pow_inv, &pair_pow);
        FP12_BN254_mul(res, &pair_pow_inv);
    }

    cfe_vec_free(&alpha);

    cleanup:
    cfe_mat_free(&mat);
    cfe_vec_G1_free(&d);
    free(positions);

    return err;
}

void cfe_gpsw_free(cfe_gpsw *gpsw) {
    mpz_clear(gpsw->p);
}

void cfe_gpsw_pub_key_free(cfe_gpsw_pub_key *pk) {
    cfe_vec_G2_free(&(pk->t));
}

void cfe_gpsw_cipher_free(cfe_gpsw_cipher *cipher) {
    cfe_vec_G2_free(&(cipher->e));
    free(cipher->gamma);
}

void cfe_gpsw_key_free(cfe_gpsw_key *policy_key) {
    cfe_vec_G1_free(&policy_key->d);
    cfe_msp_free(&policy_key->msp);
}
