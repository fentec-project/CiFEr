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
    cipher->c = cfe_malloc(msp->mat.rows * 3 *sizeof(ECP_BN254));
    cfe_mat_init(&(cipher->msp.mat), msp->mat.rows, msp->mat.cols);
    cipher->msp.row_to_attrib = cfe_malloc(msp->mat.rows * (sizeof(int)));
}

void cfe_fame_encrypt(cfe_fame_cipher *cipher, FP12_BN254 *msg, cfe_msp msp, cfe_fame_pub_key pk, cfe_fame *fame) {
    cfe_vec s;
    cfe_vec_init(&s, 2);
    cfe_uniform_sample_vec(&s, fame->p);
    BIG_256_56 tmp_big;
    mpz_t tmp;
    mpz_init(tmp);
    ECP2_BN254 gen2;
    ECP2_BN254_generator(&gen2);

    for (size_t i = 0; i < 2; i++) {
        ECP2_BN254_copy(&(cipher->ct0[i]), &(pk.part_G2[i]));
        BIG_256_56_from_mpz(tmp_big, s.vec[i]);
        ECP2_BN254_mul(&(cipher->ct0[i]), tmp_big);
    }
    mpz_add(tmp, s.vec[0], s.vec[1]);
    BIG_256_56_from_mpz(tmp_big, tmp);
    ECP2_BN254_generator(&(cipher->ct0[2]));
    ECP2_BN254_mul(&(cipher->ct0[2]), tmp_big);

    ECP_BN254 hs1, hs2;
    for (size_t i = 0; i < msp.mat.rows; i++) {
        for (size_t j = 0; j < 2; j++) {

        }
    }

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
    int len = (int) log10(i) + 1;
    char *result = cfe_malloc((len + 1) * sizeof(char));

    for (int j = 0; j < len; j++) {
        result[len - j - 1] = '0' + (i % 10);
        i = i / 10;
    }

    result[len] = '\0';

    return result;
}
