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
#include <uthash.h>
#include <gmp.h>
#include <amcl/fp12_BN254.h>
#include <cifer/internal/big.h>

#include "cifer/internal/common.h"
#include "cifer/internal/dlog.h"

typedef struct bigint_hash {
    mpz_t key;
    mpz_t val;
    UT_hash_handle hh;
} bigint_hash;

cfe_error cfe_baby_giant(mpz_t res, mpz_t h, mpz_t g, mpz_t p, mpz_t _order, mpz_t bound) {
    if (_order == NULL && mpz_probab_prime_p(p, 20) == 0) {
        return CFE_ERR_DLOG_CALC_FAILED;
    }

    mpz_t order, m, x, i, z, tmp;
    mpz_inits(order, m, x, i, z, tmp, NULL);
    cfe_error err = CFE_ERR_DLOG_NOT_FOUND;

    if (_order == NULL) {
        mpz_sub_ui(order, p, 1);
    } else {
        mpz_set(order, _order);
    }

    if (bound != NULL) {
        mpz_sqrt(m, bound);
    } else {
        mpz_sqrt(m, order);
    }
    mpz_add_ui(m, m, 1);

    mpz_set_ui(x, 1);

    // the hash table
    bigint_hash *T = NULL;

    // reusable pointer for hash table entries + a temporary variable for clearing later
    bigint_hash *t, *u;

    for (mpz_set_ui(i, 0); mpz_cmp(i, m) < 0; mpz_add_ui(i, i, 1)) {
        // store T[x] = i
        // create a struct t and store the key and value
        // the key is actually the internal contents of a mpz_t, the array and the minimal possible length
        t = (bigint_hash *) cfe_malloc(sizeof(bigint_hash));
        mpz_init_set(t->key, x);
        mpz_init_set(t->val, i);

        // the key is a pointer the the array and the length in bytes
        HASH_ADD_KEYPTR(hh, T, t->key->_mp_d, t->key->_mp_alloc * sizeof(mp_limb_t), t);

        mpz_mul(x, x, g);
        mpz_mod(x, x, p);
    }

    mpz_invert(z, g, p);
    mpz_powm(z, z, m, p);

    mpz_set(x, h);

    for (mpz_set_ui(i, 0); mpz_cmp(i, m) < 0; mpz_add_ui(i, i, 1)) {
        // get T[x]
        // the value in X needs to be assigned to a temporary variable
        // when setting a mpz_t's value, the minimal possible amount of memory will be allocated,
        // which ensures that we get the same amount of bytes read from both arrays, which is needed for a match
        mpz_set(tmp, x);
        HASH_FIND(hh, T, tmp->_mp_d, tmp->_mp_alloc * sizeof(mp_limb_t), t);

        if (t != NULL) {
            mpz_mul(res, i, m);
            mpz_add(res, res, t->val);
            err = CFE_ERR_NONE;
            break;
        }

        mpz_mul(x, x, z);
        mpz_mod(x, x, p);
    }

    // cleanup
    // iterate through all entries, clear the mpz_t variables, delete the entries and free their allocated memory
    HASH_ITER(hh, T, t, u) {
        mpz_clear(t->val);
        mpz_clear(t->key);
        HASH_DEL(T, t);
        free(t);
    }

    free(T);
    mpz_clears(order, m, x, i, z, tmp, NULL);

    return err;
}

// TODO: parallel version?
cfe_error cfe_baby_giant_with_neg(mpz_t res, mpz_t h, mpz_t g, mpz_t p, mpz_t _order, mpz_t bound) {
    cfe_error err = cfe_baby_giant(res, h, g, p, _order, bound);
    if (err) {
        mpz_t g_inv;
        mpz_init(g_inv);
        mpz_invert(g_inv, g, p);
        err = cfe_baby_giant(res, h, g_inv, p, _order, bound);
        if (err == 0) {
            mpz_neg(res, res);
        }
        mpz_clear(g_inv);
    }

    return err;
}

void iterate(mpz_t x, mpz_t a, mpz_t b, mpz_t h, mpz_t g, mpz_t p, mpz_t n, mpz_t r) {
    switch (mpz_mod_ui(r, x, 3)) {
        case 0:
            mpz_mul(x, x, x);
            mpz_mul_ui(a, a, 2);
            mpz_mul_ui(b, b, 2);
            break;
        case 1:
            mpz_mul(x, x, g);
            mpz_add_ui(a, a, 1);
            break;
        case 2:
            mpz_mul(x, x, h);
            mpz_add_ui(b, b, 1);
            break;
    }
    mpz_mod(x, x, p);
    mpz_mod(a, a, n);
    mpz_mod(b, b, n);
}

cfe_error cfe_pollard_rho(mpz_t res, mpz_t h, mpz_t g, mpz_t p, mpz_t n) {
    mpz_t x1, a1, b1, x2, a2, b2, r, q, t, d, n_div_d, j, h_check;
    mpz_inits(x1, a1, b1, x2, a2, b2, r, q, t, d, n_div_d, j, h_check, NULL);
    cfe_error err = CFE_ERR_DLOG_NOT_FOUND;

    mpz_set_ui(x1, 1);
    mpz_set_ui(x2, 1);

    uint64_t iterations = (uint64_t) 2 << 31;

    for (uint64_t i = 0; i < iterations; i++) {
        iterate(x1, a1, b1, h, g, p, n, r);
        iterate(x2, a2, b2, h, g, p, n, r);
        iterate(x2, a2, b2, h, g, p, n, r);

        if (mpz_cmp(x1, x2) == 0) {
            mpz_sub(r, b2, b1);
            mpz_mod(r, r, n);
            mpz_sub(t, a1, a2);
            mpz_mod(t, t, n);

            if (mpz_cmp_ui(r, 0) == 0) {
                goto cleanup;
            }

            mpz_gcd(d, r, n);

            // if r and n are coprime the algorithm is simple
            if (mpz_cmp_ui(d, 1) == 0) {
                mpz_invert(q, r, n);
                mpz_mul(q, q, t);
                mpz_mod(res, q, n);
                err = CFE_ERR_NONE;
                goto cleanup;
            }

            // in case r and n are not coprime additional computations are needed
            mpz_div(r, r, d);
            mpz_div(t, t, d);
            mpz_div(n_div_d, n, d);
            mpz_invert(q, r, n_div_d);
            mpz_mul(q, q, t);
            mpz_mod(q, q, n_div_d);

            for (mpz_set_ui(j, 0); mpz_cmp(j, d) < 0; mpz_add_ui(j, j, 1)) {
                mpz_powm(h_check, g, q, p);

                if (mpz_cmp(h, h_check) == 0) {
                    mpz_set(res, q);
                    err = CFE_ERR_NONE;
                    goto cleanup;
                }

                mpz_add(q, q, n_div_d);
            }
        }
    }

    cleanup:
    mpz_clears(x1, a1, b1, x2, a2, b2, r, q, t, d, n_div_d, j, h_check, NULL);

    return err;
}

typedef struct BN254_hash {
    FP12_BN254 key;
    mpz_t val;
    UT_hash_handle hh;
} BN254_hash;

cfe_error cfe_baby_giant_BN256(mpz_t res, FP12_BN254 *h, FP12_BN254 *g, mpz_t order, mpz_t bound) {
    mpz_t m, i;
    mpz_inits(m, i, NULL);
    FP12_BN254 x, x_neg, z;
    cfe_error err = CFE_ERR_DLOG_NOT_FOUND;

    if (bound != NULL) {
        mpz_sqrt(m, bound);
    } else {
        mpz_sqrt(m, order);
    }
    mpz_add_ui(m, m, 1);

    FP12_BN254_one(&x);

    // the hash table
    struct BN254_hash *T = NULL;

    // reusable pointer for hash table entries + a temporary variable for clearing later
    BN254_hash *t, *u;
    octet oct;
    oct.val = cfe_malloc(12 * MODBYTES_256_56 * sizeof(char));
//    FP12_BN254_toOctet(&oct, &x);
    for (mpz_set_ui(i, 0); mpz_cmp(i, m) < 0; mpz_add_ui(i, i, 1)) {
        // store T[x] = i
        // create a struct t and store the key and value
        // the key is actually the internal contents of a mpz_t, the array and the minimal possible length
        t = (BN254_hash *) cfe_malloc(sizeof(BN254_hash));

        FP12_BN254_copy(&(t->key), &x);
        mpz_init_set(t->val, i);

        FP12_BN254_toOctet(&oct, &x);
        // the key is a pointer the the array and the length in bytes
        HASH_ADD_KEYPTR(hh, T, oct.val, oct.len * sizeof(char), t);

        FP12_BN254_mul(&x, g);
    }

    FP12_BN254_inv(&z, g);
    BIG_256_56 m_b;
    BIG_256_56_from_mpz(m_b, m);
    FP12_BN254_pow(&z, &z, m_b);
    FP12_BN254_copy(&x, h);
    FP12_BN254_inv(&x_neg, h);

    for (mpz_set_ui(i, 0); mpz_cmp(i, m) < 0; mpz_add_ui(i, i, 1)) {
        // get T[x]
        // the value in X needs to be assigned to a temporary variable
        // TODO: check this solution
        FP12_BN254_toOctet(&oct, &x);
        HASH_FIND(hh, T, oct.val, oct.len * sizeof(char), t);

        if (t != NULL) {
            mpz_mul(res, i, m);
            mpz_add(res, res, t->val);
            err = CFE_ERR_NONE;
            break;
        }

        FP12_BN254_mul(&x, &z);

        FP12_BN254_toOctet(&oct, &x_neg);
        HASH_FIND(hh, T, oct.val, oct.len * sizeof(char), t);

        if (t != NULL) {
            mpz_mul(res, i, m);
            mpz_add(res, res, t->val);
            mpz_neg(res, res);
            err = CFE_ERR_NONE;
            break;
        }

        FP12_BN254_mul(&x_neg, &z);
    }

    // cleanup
    // iterate through all entries, clear the mpz_t variables, delete the entries and free their allocated memory
    HASH_ITER(hh, T, t, u) {
        mpz_clear(t->val);
        HASH_DEL(T, t);
        free(t);
    }

    free(T);
    mpz_clears(m, i, NULL);
    free(oct.val);
    return err;
}
