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
#include <stdarg.h>
#include <amcl/big_256_56.h>
#include <amcl/pair_BN254.h>
#include <memory.h>
#include <math.h>

#include "internal/common.h"
#include "internal/big.h"
#include "sample/uniform.h"
#include "abe/policy.h"
#include "abe/fame.h"
#include "data/vec_curve.h"


void cfe_fame_init(cfe_fame *fame) {
    mpz_init(fame->p);
    mpz_from_BIG_256_56(fame->p, (int64_t *) CURVE_Order_BN254);
}

void cfe_fame_sec_key_init(cfe_fame_sec_key *sk) {
    for (size_t i = 0; i < 4; i++) {
        mpz_init(sk->part_int[i]);
    }
}

void cfe_fame_generate_master_keys(cfe_fame_pub_key *pk, cfe_fame_sec_key *sk, cfe_fame *fame) {
    BIG_256_56 tmp_big;
    mpz_t tmp;
    mpz_init(tmp);
    cfe_vec val;
    cfe_vec_init(&val, 7);
    cfe_uniform_sample_vec(&val, fame->p);

    for (size_t i = 0; i < 4; i++) {
        mpz_set(sk->part_int[i], val.vec[i]);
    }

    for (size_t i = 0; i < 3; i++) {
        ECP_BN254_generator(&(sk->part_G1[i]));
        BIG_256_56_from_mpz(tmp_big, val.vec[i + 4]);
        ECP_BN254_mul(&(sk->part_G1[i]), tmp_big);
    }

    for (size_t i = 0; i < 2; i++) {
        ECP2_BN254_generator(&(pk->part_G2[i]));
        BIG_256_56_from_mpz(tmp_big, val.vec[i]);
        ECP2_BN254_mul(&(pk->part_G2[i]), tmp_big);
    }

    ECP_BN254 gen1;
    ECP2_BN254 gen2;
    FP12_BN254 pair;
    ECP_BN254_generator(&gen1);
    ECP2_BN254_generator(&gen2);
    PAIR_BN254_ate(&pair, &gen2, &gen1);
    PAIR_BN254_fexp(&pair);
    for (size_t i = 0; i < 2; i++) {
        mpz_mul(tmp, val.vec[i], val.vec[i + 4]);
        mpz_add(tmp, tmp, val.vec[6]);
        mpz_mod(tmp, tmp, fame->p);
        BIG_256_56_from_mpz(tmp_big, tmp);
        FP12_BN254_pow(&(pk->part_GT[i]), &pair, tmp_big);
    }

    mpz_clear(tmp);
    cfe_vec_free(&val);
}

void cfe_fame_cipher_init(cfe_fame_cipher *cipher, cfe_msp *msp) {
    cipher->ct = cfe_malloc(msp->mat.rows * 3 *sizeof(ECP_BN254));
    cfe_mat_init(&(cipher->msp.mat), msp->mat.rows, msp->mat.cols);
    cipher->msp.row_to_attrib = cfe_malloc(msp->mat.rows * (sizeof(int)));
}

void cfe_fame_encrypt(cfe_fame_cipher *cipher, FP12_BN254 *msg, cfe_msp *msp, cfe_fame_pub_key *pk, cfe_fame *fame) {
    cfe_vec s;
    cfe_vec_init(&s, 2);
    cfe_uniform_sample_vec(&s, fame->p);
    BIG_256_56 s_big[2];
    for (size_t i = 0; i < 2; i++) {
        BIG_256_56_from_mpz(s_big[i], s.vec[i]);
    }

    mpz_t tmp;
    mpz_init(tmp);
    BIG_256_56 tmp_big;
    ECP2_BN254 gen2;
    ECP2_BN254_generator(&gen2);

    for (size_t i = 0; i < 2; i++) {
        ECP2_BN254_copy(&(cipher->ct0[i]), &(pk->part_G2[i]));
        ECP2_BN254_mul(&(cipher->ct0[i]), s_big[i]);
    }
    mpz_add(tmp, s.vec[0], s.vec[1]);
    BIG_256_56_from_mpz(tmp_big, tmp);
    ECP2_BN254_generator(&(cipher->ct0[2]));
    ECP2_BN254_mul(&(cipher->ct0[2]), tmp_big);

    char *for_hash, *str_attrib, *str_l, *str_k, *str_j;
    ECP_BN254 hs[2];
    ECP_BN254 hs_to_m;
    for (size_t i = 0; i < msp->mat.rows; i++) {
        str_attrib = int_to_str(msp->row_to_attrib[i]);
        for (int l = 0; l < 3; l++) {
            str_l = int_to_str(l);
            for (int k = 0; k < 2; k++) {
                str_k = int_to_str(k);
                for_hash = strings_concat(str_attrib, " ", str_l, str_k, NULL);
                hash_G1(&(hs[k]), for_hash);
                ECP_BN254_mul(&(hs[k]), s_big[k]);
                free(str_k);
                free(for_hash);
            }

            ECP_BN254_copy(&(cipher->ct[i][l]), &(hs[0]));
            ECP_BN254_add(&(cipher->ct[i][l]), &(hs[1]));

            for (int j = 0; j < (int) msp->mat.cols; j++) {
                str_j = int_to_str(j);
                for (int k = 0; k < 2; k++) {
                    str_k = int_to_str(k);
                    for_hash = strings_concat((char *) "0", str_j, " ", str_l, str_k, NULL);
                    hash_G1(&(hs[k]), for_hash);
                    ECP_BN254_mul(&(hs[k]), s_big[k]);
                    free(str_k);
                    free(for_hash);
                }
                free(str_j);

                ECP_BN254_copy(&hs_to_m, &(hs[0]));
                if (mpz_sgn(msp->mat.mat[i].vec[j]) == -1) {
                    mpz_neg(tmp, msp->mat.mat[i].vec[j]);
                    BIG_256_56_from_mpz(tmp_big, tmp);
                    ECP_BN254_mul(&hs_to_m, tmp_big);
                    ECP_BN254_neg(&hs_to_m);
                } else {
                    BIG_256_56_from_mpz(tmp_big, msp->mat.mat[i].vec[j]);
                    ECP_BN254_mul(&hs_to_m, tmp_big);
                }
            }
            free(str_l);
        }
        free(str_attrib);
    }
    FP12_BN254 tmp_GT;
    FP12_BN254_pow(&(cipher->ct_prime), &(pk->part_GT[0]), s_big[0]);
    FP12_BN254_pow(&tmp_GT, &(pk->part_GT[1]), s_big[1]);
    FP12_BN254_mul(&(cipher->ct_prime), &tmp_GT);
    FP12_BN254_mul(&(cipher->ct_prime), msg);

    mpz_clear(tmp);
}

void hash_G1(ECP_BN254 *g, char *str) {
    octet tmp;
    tmp.val = str;
    ECP_BN254_mapit(g, &tmp);
}

char *strings_concat(char *start, ...) {
    // sum the length of all the strings
    va_list ap;
    size_t len = 0;
    va_start(ap, start);
    char *str = start;
    while (str != NULL) {
        len += strlen(str);
        str = va_arg(ap, char*);
    }
    va_end(ap);
    char *res = cfe_malloc((len + 1) * sizeof(char));

    // set the string
    va_start(ap, start);
    str = start;
    size_t j = 0;
    while (str != NULL) {
        for (size_t i = 0; i < strlen(str); i++) {
            res[j] = str[i];
            j++;
        }
        str = va_arg(ap, char*);
    }
    res[j] = '\0';
    va_end(ap);
    return res;
}

char *int_to_str(int i) {
    int len;
    if (i == 0) {
        len = 1;
    } else {
        len = (int) log10(i) + 1;
    }
    char *result = cfe_malloc((len + 1) * sizeof(char));

    for (int j = 0; j < len; j++) {
        result[len - j - 1] = '0' + (i % 10);
        i = i / 10;
    }

    result[len] = '\0';

    return result;
}

void cfe_fame_attrib_keys_init(cfe_fame_attrib_keys *keys, size_t num_attrib) {
    keys->k = cfe_malloc(num_attrib * 3 * sizeof(ECP_BN254));
    keys->row_to_attrib = cfe_malloc(num_attrib * sizeof(int));
}

void cfe_fame_generate_attrib_keys(cfe_fame_attrib_keys *keys, int *gamma, size_t num_attrib, cfe_fame_sec_key *sk, cfe_fame *fame) {
    cfe_vec r, sigma;
    cfe_vec_init(&r, 2);
    cfe_vec_init(&sigma, num_attrib);
    cfe_uniform_sample_vec(&r, fame->p);
    cfe_uniform_sample_vec(&sigma, fame->p);

    BIG_256_56 tmp_big, a_inv_big[2];
    mpz_t pow[3], a_inv[2];
    mpz_inits(pow[0], pow[1], pow[2], a_inv[0], a_inv[1], NULL);
    for (size_t j = 0; j < 2; j++) {
        mpz_mul(pow[j], sk->part_int[2 + j], r.vec[j]);
        mpz_mod(pow[j], pow[j], fame->p);
        mpz_invert(a_inv[j], sk->part_int[j], fame->p);
        BIG_256_56_from_mpz(a_inv_big[j], a_inv[j]);
    }
    mpz_add(pow[2], r.vec[0], r.vec[1]);
    mpz_mod(pow[2], pow[2], fame->p);

    for (size_t j = 0; j < 3; j++) {
        BIG_256_56_from_mpz(tmp_big, pow[j]);
        ECP2_BN254_generator(&(keys->k0[j]));
        ECP2_BN254_mul(&(keys->k0[j]), tmp_big);
    }

    ECP_BN254 g_sigma, hs;
    char *str_attrib, *str_t, *str_j, *for_hash;
    for (size_t i = 0; i < num_attrib; i++) {
        ECP_BN254_generator(&g_sigma);
        BIG_256_56_from_mpz(tmp_big, sigma.vec[i]);
        ECP_BN254_mul(&g_sigma, tmp_big);

        str_attrib = int_to_str(gamma[i]);

        for (int t = 0; t < 2; t++) {
            str_t = int_to_str(t);

            ECP_BN254_copy(&(keys->k[i][t]), &g_sigma);
            for (int j = 0; j < 3; j++) {
                str_j = int_to_str(j);

                for_hash = strings_concat(str_attrib, " ", str_j, " ", str_t, NULL);
                hash_G1(&hs, for_hash);
                ECP_BN254_add(&(keys->k[i][t]), &hs);
            }

            ECP_BN254_mul(&(keys->k[i][t]), a_inv_big[t]);
        }

        ECP_BN254_copy(&(keys->k[i][2]), &g_sigma);
        ECP_BN254_neg(&(keys->k[i][2]));

        keys->row_to_attrib[i] = gamma[i];
    }



}































