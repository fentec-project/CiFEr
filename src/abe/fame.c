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
#include <math.h>
#include <string.h>

#include "internal/common.h"
#include "internal/big.h"
#include "sample/uniform.h"
#include "abe/fame.h"


void cfe_fame_init(cfe_fame *fame) {
    mpz_init(fame->p);
    mpz_from_BIG_256_56(fame->p, (int64_t *) CURVE_Order_BN254);
}

void cfe_fame_free(cfe_fame *fame) {
    mpz_clear(fame->p);
}

void cfe_fame_sec_key_init(cfe_fame_sec_key *sk) {
    for (size_t i = 0; i < 4; i++) {
        mpz_init(sk->part_int[i]);
    }
}

void cfe_fame_sec_key_free(cfe_fame_sec_key *sk) {
    for (size_t i = 0; i < 4; i++) {
        mpz_clear(sk->part_int[i]);
    }
}

void cfe_fame_generate_master_keys(cfe_fame_pub_key *pk, cfe_fame_sec_key *sk, cfe_fame *fame) {
    // prepare variables
    BIG_256_56 tmp_big;
    mpz_t tmp;
    mpz_init(tmp);
    cfe_vec val;
    cfe_vec_init(&val, 7);

    // sample randomness
    cfe_uniform_sample_vec(&val, fame->p);

    // set int part of the secret key
    for (size_t i = 0; i < 4; i++) {
        mpz_set(sk->part_int[i], val.vec[i]);
    }

    // set the elliptic curve part of the secret key
    for (size_t i = 0; i < 3; i++) {
        BIG_256_56_from_mpz(tmp_big, val.vec[i + 4]);
        ECP_BN254_generator(&(sk->part_G1[i]));
        ECP_BN254_mul(&(sk->part_G1[i]), tmp_big);
    }

    // set the elliptic curve part of the public key
    for (size_t i = 0; i < 2; i++) {
        BIG_256_56_from_mpz(tmp_big, val.vec[i]);
        ECP2_BN254_generator(&(pk->part_G2[i]));
        ECP2_BN254_mul(&(pk->part_G2[i]), tmp_big);
    }

    // set the field part of the public key
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
    cipher->ct = (ECP_BN254 (*)[3]) cfe_malloc(msp->mat.rows * 3 * sizeof(ECP_BN254));
    cfe_mat_init(&(cipher->msp.mat), msp->mat.rows, msp->mat.cols);
    cipher->msp.row_to_attrib = (int *) cfe_malloc(msp->mat.rows * (sizeof(int)));
}

void cfe_fame_cipher_free(cfe_fame_cipher *cipher) {
    free(cipher->ct);
    cfe_msp_free(&(cipher->msp));
}

void cfe_fame_encrypt(cfe_fame_cipher *cipher, FP12_BN254 *msg, cfe_msp *msp, cfe_fame_pub_key *pk, cfe_fame *fame) {
    // prepare the variables
    cfe_vec s, msp_row;
    cfe_vec_init(&s, 2);
    cfe_vec_init(&msp_row, msp->mat.cols);
    mpz_t tmp;
    mpz_init(tmp);
    BIG_256_56 tmp_big;
    char *for_hash, *str_attrib, *str_l, *str_k, *str_j;
    ECP_BN254 hs;
    ECP_BN254 hs_pow_m;

    // sample the randomness
    cfe_uniform_sample_vec(&s, fame->p);
    BIG_256_56 s_big[2];
    for (size_t i = 0; i < 2; i++) {
        BIG_256_56_from_mpz(s_big[i], s.vec[i]);
    }

    // calculate ciphertext
    for (size_t i = 0; i < 2; i++) {
        ECP2_BN254_copy(&(cipher->ct0[i]), &(pk->part_G2[i]));
        ECP2_BN254_mul(&(cipher->ct0[i]), s_big[i]);
    }
    mpz_add(tmp, s.vec[0], s.vec[1]);
    BIG_256_56_from_mpz(tmp_big, tmp);
    ECP2_BN254_generator(&(cipher->ct0[2]));
    ECP2_BN254_mul(&(cipher->ct0[2]), tmp_big);

    for (size_t i = 0; i < msp->mat.rows; i++) {
        cipher->msp.row_to_attrib[i] = msp->row_to_attrib[i];
        cfe_mat_get_row(&msp_row, &(msp->mat), i);
        cfe_mat_set_vec(&(cipher->msp.mat), &msp_row, i);

        str_attrib = cfe_int_to_str(msp->row_to_attrib[i]);
        for (int l = 0; l < 3; l++) {
            str_l = cfe_int_to_str(l);
            ECP_BN254_inf(&(cipher->ct[i][l]));
            for (int k = 0; k < 2; k++) {
                str_k = cfe_int_to_str(k);
                for_hash = cfe_strings_concat_for_hash(str_attrib, " ", str_l, " ", str_k, NULL);
                cfe_hash_G1(&hs, for_hash);
                ECP_BN254_mul(&hs, s_big[k]);
                ECP_BN254_add(&(cipher->ct[i][l]), &hs);
                free(str_k);
                free(for_hash);
            }

            for (int j = 0; j < (int) msp->mat.cols; j++) {
                str_j = cfe_int_to_str(j);
                ECP_BN254_inf(&hs_pow_m);
                for (int k = 0; k < 2; k++) {
                    str_k = cfe_int_to_str(k);
                    for_hash = cfe_strings_concat_for_hash((char *) "0 ", str_j, " ", str_l, " ", str_k, NULL);
                    cfe_hash_G1(&(hs), for_hash);
                    ECP_BN254_mul(&(hs), s_big[k]);
                    ECP_BN254_add(&hs_pow_m, &(hs));
                    free(str_k);
                    free(for_hash);
                }
                free(str_j);

                if (mpz_sgn(msp->mat.mat[i].vec[j]) == -1) {
                    mpz_neg(tmp, msp->mat.mat[i].vec[j]);
                    BIG_256_56_from_mpz(tmp_big, tmp);
                    ECP_BN254_mul(&hs_pow_m, tmp_big);
                    ECP_BN254_neg(&hs_pow_m);
                } else {
                    BIG_256_56_from_mpz(tmp_big, msp->mat.mat[i].vec[j]);
                    ECP_BN254_mul(&hs_pow_m, tmp_big);
                }
                ECP_BN254_add(&(cipher->ct[i][l]), &hs_pow_m);
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

    // clear up
    cfe_vec_frees(&s, &msp_row, NULL);
    mpz_clear(tmp);
}

void cfe_fame_attrib_keys_init(cfe_fame_attrib_keys *keys, size_t num_attrib) {
    keys->k = (ECP_BN254 (*)[3]) cfe_malloc(num_attrib * 3 * sizeof(ECP_BN254));
    keys->row_to_attrib = (int *) cfe_malloc(num_attrib * sizeof(int));
    keys->num_attrib = num_attrib;
}

void cfe_fame_attrib_keys_free(cfe_fame_attrib_keys *keys) {
    free(keys->k);
    free(keys->row_to_attrib);
}

void cfe_fame_generate_attrib_keys(cfe_fame_attrib_keys *keys, int *gamma,
                                   size_t num_attrib, cfe_fame_sec_key *sk, cfe_fame *fame) {
    // prepare variables
    cfe_vec r, sigma;
    cfe_vec_init(&r, 2);
    cfe_vec_init(&sigma, num_attrib);
    BIG_256_56 tmp_big, a_inv_big[2];
    mpz_t pow[3], a_inv[2], sigma_prime;
    mpz_inits(pow[0], pow[1], pow[2], a_inv[0], a_inv[1], sigma_prime, NULL);
    ECP_BN254 g_sigma, hs, g_sigma_prime;
    char *str_attrib, *str_t, *str_j, *for_hash;

    // sample randomness
    cfe_uniform_sample_vec(&r, fame->p);
    cfe_uniform_sample_vec(&sigma, fame->p);

    // prepare values
    for (size_t j = 0; j < 2; j++) {
        mpz_mul(pow[j], sk->part_int[2 + j], r.vec[j]);
        mpz_mod(pow[j], pow[j], fame->p);
        mpz_invert(a_inv[j], sk->part_int[j], fame->p);
        BIG_256_56_from_mpz(a_inv_big[j], a_inv[j]);
    }
    mpz_add(pow[2], r.vec[0], r.vec[1]);
    mpz_mod(pow[2], pow[2], fame->p);

    // calculate keys
    for (size_t j = 0; j < 3; j++) {
        BIG_256_56_from_mpz(tmp_big, pow[j]);
        ECP2_BN254_generator(&(keys->k0[j]));
        ECP2_BN254_mul(&(keys->k0[j]), tmp_big);
    }

    for (size_t i = 0; i < num_attrib; i++) {
        ECP_BN254_generator(&g_sigma);
        BIG_256_56_from_mpz(tmp_big, sigma.vec[i]);
        ECP_BN254_mul(&g_sigma, tmp_big);

        str_attrib = cfe_int_to_str(gamma[i]);
        for (int t = 0; t < 2; t++) {
            str_t = cfe_int_to_str(t);

            ECP_BN254_copy(&(keys->k[i][t]), &g_sigma);
            for (int j = 0; j < 3; j++) {
                str_j = cfe_int_to_str(j);
                for_hash = cfe_strings_concat_for_hash(str_attrib, " ", str_j, " ", str_t, NULL);
                cfe_hash_G1(&hs, for_hash);

                BIG_256_56_from_mpz(tmp_big, pow[j]);
                ECP_BN254_mul(&hs, tmp_big);
                ECP_BN254_add(&(keys->k[i][t]), &hs);

                free(str_j);
                free(for_hash);
            }
            ECP_BN254_mul(&(keys->k[i][t]), a_inv_big[t]);

            free(str_t);
        }

        ECP_BN254_copy(&(keys->k[i][2]), &g_sigma);
        ECP_BN254_neg(&(keys->k[i][2]));
        keys->row_to_attrib[i] = gamma[i];

        free(str_attrib);
    }

    cfe_uniform_sample(sigma_prime, fame->p);
    BIG_256_56_from_mpz(tmp_big, sigma_prime);
    ECP_BN254_generator(&g_sigma_prime);
    ECP_BN254_mul(&g_sigma_prime, tmp_big);

    for (int t = 0; t < 2; t++) {
        ECP_BN254_copy(&(keys->k_prime[t]), &g_sigma_prime);
        str_t = cfe_int_to_str(t);
        for (int j = 0; j < 3; j++) {
            str_j = cfe_int_to_str(j);
            for_hash = cfe_strings_concat_for_hash((char *) "0 0 ", str_j, " ", str_t, NULL);
            cfe_hash_G1(&hs, for_hash);

            BIG_256_56_from_mpz(tmp_big, pow[j]);
            ECP_BN254_mul(&hs, tmp_big);
            ECP_BN254_add(&(keys->k_prime[t]), &hs);

            free(str_j);
            free(for_hash);
        }
        ECP_BN254_mul(&(keys->k_prime[t]), a_inv_big[t]);
        ECP_BN254_add(&(keys->k_prime[t]), &(sk->part_G1[t]));

        free(str_t);
    }
    ECP_BN254_copy(&(keys->k_prime[2]), &g_sigma_prime);
    ECP_BN254_neg(&(keys->k_prime[2]));
    ECP_BN254_add(&(keys->k_prime[2]), &(sk->part_G1[2]));

    // clear up
    mpz_clears(pow[0], pow[1], pow[2], a_inv[0], a_inv[1], sigma_prime, NULL);
    cfe_vec_frees(&sigma, &r, NULL);
}

cfe_error cfe_fame_decrypt(FP12_BN254 *res, cfe_fame_cipher *cipher,
                           cfe_fame_attrib_keys *keys, cfe_fame *fame) {
    // determine the intersection between owned attributes and specified in encryption
    size_t count_attrib = 0;
    size_t positions_keys[keys->num_attrib];
    size_t positions_msp[keys->num_attrib];
    for (size_t i = 0; i < keys->num_attrib; i++) {
        for (size_t j = 0; j < cipher->msp.mat.rows; j++) {
            if (keys->row_to_attrib[i] == cipher->msp.row_to_attrib[j]) {
                positions_keys[count_attrib] = i;
                positions_msp[count_attrib] = j;
                count_attrib++;
                break;
            }
        }
    }

    // prepare all the variables
    cfe_mat mat_for_keys, mat_for_keys_trans;
    cfe_mat_init(&mat_for_keys, count_attrib, cipher->msp.mat.cols);
    cfe_mat_init(&mat_for_keys_trans, cipher->msp.mat.cols, count_attrib);
    cfe_vec tmp, one_vec, alpha;
    cfe_vec_init(&tmp, cipher->msp.mat.cols);
    mpz_t zero;
    mpz_init_set_si(zero, 0);
    ECP_BN254 ct_prod[3], key_prod[3], x_pow_alpha;
    BIG_256_56 alpha_big;
    FP12_BN254 ct_pairing, key_pairing, key_pairing_inv;


    // determine needed attributes
    for (size_t i = 0; i < count_attrib; i++) {
        cfe_mat_get_row(&tmp, &(cipher->msp.mat), positions_msp[i]);
        cfe_mat_set_vec(&mat_for_keys, &tmp, i);
    }
    cfe_mat_transpose(&mat_for_keys_trans, &mat_for_keys);
    cfe_vec_init(&one_vec, cipher->msp.mat.cols);
    cfe_vec_set_const(&one_vec, zero);
    mpz_set_ui(one_vec.vec[0], 1);

    cfe_error check = cfe_gaussian_elimination(&alpha, &mat_for_keys_trans, &one_vec, fame->p);
    if (check) {
        return CFE_ERR_INSUFFICIENT_KEYS;
    }

    // calculate decryption
    FP12_BN254_copy(res, &(cipher->ct_prime));
    for (size_t j = 0; j < 3; j++) {
        ECP_BN254_inf(&(ct_prod[j]));
        ECP_BN254_inf(&(key_prod[j])); //check inf
        for (size_t i = 0; i < count_attrib; i++) {
            BIG_256_56_from_mpz(alpha_big, alpha.vec[i]);

            ECP_BN254_copy(&x_pow_alpha, &(cipher->ct[positions_msp[i]][j]));
            ECP_BN254_mul(&x_pow_alpha, alpha_big);
            ECP_BN254_add(&(ct_prod[j]), &(x_pow_alpha));

            ECP_BN254_copy(&x_pow_alpha, &(keys->k[positions_keys[i]][j]));
            ECP_BN254_mul(&x_pow_alpha, alpha_big);
            ECP_BN254_add(&(key_prod[j]), &(x_pow_alpha));
        }

        ECP_BN254_add(&(key_prod[j]), &(keys->k_prime[j]));
        PAIR_BN254_ate(&ct_pairing, &(keys->k0[j]), &(ct_prod[j]));
        PAIR_BN254_fexp(&ct_pairing);
        PAIR_BN254_ate(&key_pairing, &(cipher->ct0[j]), &(key_prod[j]));
        PAIR_BN254_fexp(&key_pairing);

        FP12_BN254_inv(&key_pairing_inv, &key_pairing);
        FP12_BN254_mul(res, &ct_pairing);
        FP12_BN254_mul(res, &key_pairing_inv);
    }

    // clear up
    cfe_mat_frees(&mat_for_keys_trans, &mat_for_keys, NULL);
    cfe_vec_frees(&one_vec, &alpha, &tmp, NULL);
    mpz_clear(zero);

    return CFE_ERR_NONE;
}

// cfe_hash_G1 hashes a string of length MODBYTES_256_56
// into the elliptic group represented by ECP_BN254.
void cfe_hash_G1(ECP_BN254 *g, char *str) {
    octet tmp;
    tmp.val = str;
    ECP_BN254_mapit(g, &tmp);
}

// cfe_strings_concat_for_hash joins the given strings to create
// a string of length MODBYTES_256_56 needed for the hashing function.
char *cfe_strings_concat_for_hash(char *start, ...) {
    // allocate memory for the result
    char *res = (char *) cfe_malloc((MODBYTES_256_56 + 1) * sizeof(char));

    // set the string
    va_list ap;
    char *str = start;
    va_start(ap, start);
    size_t j = 0;
    while (str != NULL) {
        for (size_t i = 0; i < strlen(str); i++) {
            res[j] = str[i];
            j++;
        }
        str = va_arg(ap, char*);
    }

    // pad the string
    for (size_t i = j; i < MODBYTES_256_56 + 1; i++) {
        res[i] = '\0';
    }
    va_end(ap);

    return res;
}

// cfe_int_to_str changes a non-negative int into a string of its
// decimal representation
char *cfe_int_to_str(int i) {
    int len;
    if (i == 0) {
        len = 1;
    } else {
        len = (int) log10(i) + 1;
    }
    char *result = (char *) cfe_malloc((len + 1) * sizeof(char));

    sprintf(result, "%d", i);

    return result;
}
