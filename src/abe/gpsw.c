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
#include <string.h>
#include <amcl/big_256_56.h>
#include <amcl/pair_BN254.h>

#include "internal/common.h"
#include "internal/big.h"
#include "sample/uniform.h"
#include "abe/policy.h"
#include "abe/gpsw.h"

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

void cfe_gpsw_policy_keys_init(cfe_vec_G1 *policy_keys, cfe_msp *msp) {
    cfe_vec_G1_init(policy_keys, msp->mat.rows);
}

void cfe_gpsw_generate_policy_keys(cfe_vec_G1 *policy_keys, cfe_gpsw *gpsw,
                                   cfe_msp *msp, cfe_vec *sk) {
    cfe_vec u;
    cfe_vec_init(&u, msp->mat.cols);
    cfe_gpsw_rand_vec_const_sum(&u, sk->vec[gpsw->l], gpsw->p);

    mpz_t t_map_i_inv, mat_times_u, pow;
    mpz_inits(t_map_i_inv, mat_times_u, pow, NULL);
    BIG_256_56 pow_big;
    for (size_t i = 0; i < msp->mat.rows; i++) {
        mpz_invert(t_map_i_inv, sk->vec[msp->row_to_attrib[i]], gpsw->p);
        cfe_vec_dot(mat_times_u, &(msp->mat.mat[i]), &u);
        mpz_mul(pow, t_map_i_inv, mat_times_u);
        mpz_mod(pow, pow, gpsw->p);
        BIG_256_56_from_mpz(pow_big, pow);

        ECP_BN254_generator(&(policy_keys->vec[i]));
        ECP_BN254_mul(&(policy_keys->vec[i]), pow_big);
    }

    cfe_vec_free(&u);
    mpz_clears(t_map_i_inv, mat_times_u, pow, NULL);
}

void cfe_gpsw_keys_init(cfe_gpsw_keys *keys, cfe_msp *msp, int *attrib, size_t num_attrib) {
    size_t count_attrib = 0;

    for (size_t i = 0; i < msp->mat.rows; i++) {
        for (size_t j = 0; j < num_attrib; j++) {
            if (msp->row_to_attrib[i] == attrib[j]) {
                count_attrib++;
                break;
            }
        }
    }

    cfe_mat_init(&keys->mat, count_attrib, msp->mat.cols);
    cfe_vec_G1_init(&keys->d, count_attrib);
    keys->row_to_attrib = (int *) cfe_malloc(sizeof(int) * count_attrib);

}

void cfe_gpsw_delegate_keys(cfe_gpsw_keys *keys, cfe_vec_G1 *policy_keys,
                            cfe_msp *msp, int *attrib, size_t num_attrib) {
    size_t count_attrib = 0;
    size_t *positions = (size_t *) cfe_malloc(sizeof(size_t) * keys->mat.rows);

    for (size_t i = 0; i < msp->mat.rows; i++) {
        for (size_t j = 0; j < num_attrib; j++) {
            if (msp->row_to_attrib[i] == attrib[j]) {
                positions[count_attrib] = i;
                count_attrib++;
                break;
            }
        }
    }

    for (size_t i = 0; i < count_attrib; i++) {
        cfe_mat_set_vec(&(keys->mat), &(msp->mat.mat[positions[i]]), i);
        ECP_BN254_copy(&(keys->d.vec[i]), &(policy_keys->vec[positions[i]]));
        keys->row_to_attrib[i] = msp->row_to_attrib[positions[i]];
    }

    free(positions);
}


cfe_error cfe_gpsw_decrypt(FP12_BN254 *res, cfe_gpsw_cipher *cipher, cfe_gpsw_keys *keys, cfe_gpsw *gpsw) {
    cfe_vec one_vec, alpha;
    mpz_t one;
    mpz_init_set_ui(one, 1);
    cfe_vec_init(&one_vec, keys->mat.cols);
    cfe_vec_set_const(&one_vec, one);
    cfe_mat mat_transpose;
    cfe_mat_init(&mat_transpose, keys->mat.cols, keys->mat.rows);
    cfe_mat_transpose(&mat_transpose, &(keys->mat));
    cfe_error check = cfe_gaussian_elimination(&alpha, &mat_transpose, &one_vec, gpsw->p);
    cfe_mat_free(&mat_transpose);
    cfe_vec_free(&one_vec);
    mpz_clear(one);
    if (check) {
        return CFE_ERR_INSUFFICIENT_KEYS;
    }

    size_t *positions = (size_t *) cfe_malloc(sizeof(size_t) * keys->mat.rows);

    for (size_t i = 0; i < keys->mat.rows; i++) {
        for (size_t j = 0; j < cipher->e.size; j++) {
            if (keys->row_to_attrib[i] == cipher->gamma[j]) {
                positions[i] = j;
                break;
            }
        }
    }

    FP12_BN254_copy(res, &(cipher->e0));
    FP12_BN254 pair;
    FP12_BN254 pair_pow;
    FP12_BN254 pair_pow_inv;

    BIG_256_56 alpha_i;
    for (size_t i = 0; i < keys->mat.rows; i++) {
        PAIR_BN254_ate(&pair, &(cipher->e.vec[positions[i]]), &(keys->d.vec[i]));
        PAIR_BN254_fexp(&pair);
        BIG_256_56_from_mpz(alpha_i, alpha.vec[i]);
        FP12_BN254_pow(&pair_pow, &pair, alpha_i);
        FP12_BN254_inv(&pair_pow_inv, &pair_pow);
        FP12_BN254_mul(res, &pair_pow_inv);
    }

    cfe_vec_free(&alpha);
    free(positions);

    return CFE_ERR_NONE;
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

void cfe_gpsw_keys_free(cfe_gpsw_keys *keys) {
    cfe_mat_free(&(keys->mat));
    cfe_vec_G1_free(&(keys->d));
    free(keys->row_to_attrib);
}
