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
#include <stdarg.h>
#include <gmp.h>
#include <cifer/internal/big.h>
#include <amcl/pair_BN254.h>
#include <cifer/internal/common.h>
#include <math.h>
#include <string.h>

#include "cifer/innerprod/fullysec/dmcfe.h"
#include "cifer/internal/keygen.h"
#include "cifer/internal/dlog.h"
#include "cifer/sample/uniform.h"

// TODO: hash functions need to be chosen
void cfe_dmcfe_client_init(cfe_dmcfe_client *c, size_t idx) {
    mpz_t el1, el2, three;
    mpz_inits(c->order, el1, el2, three, NULL);
    mpz_set_ui(three, 3);
    c->idx = idx;
    BIG_256_56_rcopy(c->order_big, CURVE_Order_BN254);
    mpz_from_BIG_256_56(c->order, c->order_big);
    cfe_vec_init(&(c->s), 2);
    cfe_uniform_sample_vec(&(c->s), c->order);
    cfe_mat_init(&(c->share_sec), 2, 2);
    cfe_uniform_sample_mat(&(c->share_sec), c->order);
    cfe_mat_init(&(c->share_pub), 2, 2);
    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < 2; j++) {
            cfe_mat_get(el1, &(c->share_sec), i, j);
            mpz_powm(el2, three, el1, c->order);
            cfe_mat_set( &(c->share_pub), el2, i, j);
        }
    }
}

void cfe_dmcfe_set_share(cfe_dmcfe_client *c, cfe_mat **pub_t, size_t num_clients) {
    mpz_t el1, el2, el3;
    mpz_inits(el1, el2, el3, NULL);
    cfe_mat add;
    cfe_mat_inits(2, 2, &(c->share), &add, NULL);
    for (size_t k = 0; k < num_clients; k++) {
        if (k == c->idx) {
            continue;
        }
        for (size_t i = 0; i < 2; i++) {
            for (size_t j = 0; j < 2; j++) {
                cfe_mat_get(el1, pub_t[k], i, j);
                cfe_mat_get(el2, &(c->share_sec), i, j);
                mpz_powm(el3, el1, el2, c->order);
                cfe_mat_set( &add, el3, i, j);
            }
        }
        if (k > c->idx) {
            cfe_mat_neg(&add, &add);
        }
        cfe_mat_add(&(c->share), &(c->share), &add);
        cfe_mat_mod(&(c->share), &(c->share), c->order);
    }
}

// cfe_hash_G1 hashes a string of length MODBYTES_256_56
// into the elliptic group represented by ECP_BN254.
void cfe_hash_G12(ECP_BN254 *g, char *str) {
    octet tmp;
    tmp.val = str;
    ECP_BN254_mapit(g, &tmp);
}

// cfe_hash_G1 hashes a string of length MODBYTES_256_56
// into the elliptic group element represented by ECP2_BN254.
void cfe_hash_G22(ECP2_BN254 *g, char *str) {
    octet tmp;
    tmp.val = str;
    ECP2_BN254_mapit(g, &tmp);
}

char *cfe_vec_to_string(cfe_vec *v) {
    size_t alloc_len = 0;
    for (size_t i = 0; i < v->size; i++) {
        alloc_len += strlen(mpz_get_str(NULL, 62, v->vec[i])) + 1;
    }

    // allocate memory for the result
    char *res = (char *) cfe_malloc((alloc_len) * sizeof(char));

    // set the string
    size_t j = 0;
    char *tmp_string;
    for (size_t i = 0; i < v->size; i++) {
        tmp_string = mpz_get_str(NULL, 62, v->vec[i]);
        for (size_t i = 0; i < strlen(tmp_string); i++) {
            res[j] = tmp_string[i];
            j++;
        }
        res[j] = ' ';
        j++;
    }

    res[alloc_len] = '\0';

    return res;
}

// cfe_strings_concat_for_hash joins the given strings to create
// a string of length MODBYTES_256_56 needed for the hashing function.
char *cfe_strings_concat_for_hash2(char *start, ...) {
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
char *cfe_int_to_str2(int i) {
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

void cfe_dmcfe_encrypt(ECP_BN254 *cipher, mpz_t x, char *label, cfe_dmcfe_client *c) {
    char *label_for_hash;
    ECP_BN254 h;
    BIG_256_56 tmp_big;
    ECP_BN254_inf(cipher);
    for (int i = 0; i < 2; i++) {
        label_for_hash = cfe_strings_concat_for_hash2(cfe_int_to_str2(i), label, NULL);
        cfe_hash_G12(&h, label_for_hash);
        BIG_256_56_from_mpz(tmp_big, c->s.vec[i]);
        ECP_BN254_mul(&h, tmp_big);
        ECP_BN254_add(cipher, &h);
    }

    BIG_256_56_from_mpz(tmp_big, x);
    ECP_BN254_generator(&h);
    ECP_BN254_mul(&h, tmp_big);
    ECP_BN254_add(cipher, &h);
}

void cfe_dmcfe_generate_key_share(cfe_vec_G2 *key_share, cfe_vec *y, cfe_dmcfe_client *c) {
    char *str = cfe_vec_to_string(y);
    char *for_hash;
    ECP2_BN254 hash[2];
    for (int i = 0; i < 2; i++) {
        for_hash = cfe_strings_concat_for_hash2(cfe_int_to_str2(i), str, NULL);
        cfe_hash_G22(&(hash[i]), for_hash);
    }

    mpz_t tmp;
    mpz_init(tmp);
    BIG_256_56 tmp_big;
    ECP2_BN254 h;
    for (size_t k = 0; k < 2; k++) {
        ECP2_BN254_inf(&(key_share->vec[k]));
        for (size_t i = 0; i < 2; i++) {
            ECP2_BN254_copy(&h, &(hash[i]));
            cfe_mat_get(tmp, &(c->share), k, i);
            BIG_256_56_from_mpz(tmp_big, tmp);
            ECP2_BN254_mul(&(h), tmp_big);
            ECP2_BN254_add(&(key_share->vec[k]), &h);
        }

        mpz_mul(tmp, y->vec[c->idx], c->s.vec[k]);
        BIG_256_56_from_mpz(tmp_big, tmp);
        ECP2_BN254_generator(&h);
        ECP2_BN254_mul(&h, tmp_big);
        ECP2_BN254_add(&(key_share->vec[k]), &h);
    }
}

cfe_error cfe_dmcfe_decrypt(mpz_t res, ECP_BN254 *ciphers, cfe_vec_G2 *key_shares,
        char *label, cfe_vec *y, mpz_t bound, size_t num_clients) {
    cfe_vec_G2 keys_sum;
    cfe_vec_G2_init(&keys_sum, 2);
    for (size_t i = 0; i < 2; i++) {
        ECP2_BN254_inf(&(keys_sum.vec[i]));
    }
    for (size_t k = 0; k < num_clients; k++) {
        for (size_t i = 0; i < 2; i++) {
            ECP2_BN254_add(&(keys_sum.vec[i]), &(key_shares[k].vec[i]));
        }
    }
    ECP_BN254 ciphers_sum, cipher_i, gen1;
    ECP2_BN254 gen2;
    ECP_BN254_generator(&gen1);
    ECP2_BN254_generator(&gen2);
    ECP_BN254_inf(&ciphers_sum);
    BIG_256_56 y_i;
    for (size_t i = 0; i < num_clients; i++) {
        ECP_BN254_copy(&cipher_i, &(ciphers[i]));
        BIG_256_56_from_mpz(y_i, y->vec[i]);
        ECP_BN254_mul(&cipher_i, y_i);
        ECP_BN254_add(&ciphers_sum, &cipher_i);
    }
    FP12_BN254 s;
    PAIR_BN254_ate(&s, &gen2, &ciphers_sum);
    PAIR_BN254_fexp(&s);

    char *label_for_hash;
    ECP_BN254 h;
    FP12_BN254 t, pair;
    FP12_BN254_one(&t);
    for (int i = 0; i < 2; i++) {
        label_for_hash = cfe_strings_concat_for_hash2(cfe_int_to_str2(i), label, NULL);
        cfe_hash_G12(&h, label_for_hash);
        PAIR_BN254_ate(&pair, &(keys_sum.vec[i]), &h);
        PAIR_BN254_fexp(&pair);
        FP12_BN254_mul(&t, &pair);
    }
    FP12_BN254_inv(&t, &t);
    FP12_BN254_mul(&s, &t);

    PAIR_BN254_ate(&pair, &gen2, &gen1);
    PAIR_BN254_fexp(&pair);

    mpz_t res_bound;
    mpz_init(res_bound);
    mpz_pow_ui(res_bound, bound, 2);
    mpz_mul_ui(res_bound, res_bound, num_clients);

    cfe_error err;
    err = cfe_baby_giant_FP12_BN256_with_neg(res, &s, &pair, res_bound);
//    mpz_clears(el, res_bound, NULL);
    return err;
}
