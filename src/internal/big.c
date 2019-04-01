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

#include "internal/big.h"

void BIG_256_56_from_mpz(BIG_256_56 dst, mpz_t src) {
    BIG_256_56_zero(dst);
    mpz_t x, y;
    mpz_inits(x, y, NULL);
    mpz_set(y, src);
    size_t size = (MODBYTES_256_56 * 8) - ((MODBYTES_256_56 * 8) % BASEBITS_256_56);
    for (int i = 0; i < ((MODBYTES_256_56 * 8) / BASEBITS_256_56) + 1; i++) {
        BIG_256_56_fshl(dst, BASEBITS_256_56);
        mpz_fdiv_q_2exp(x, y, size);
        mpz_fdiv_r_2exp(y, y, size);
        dst[0] += mpz_get_ui(x);
        size = size - BASEBITS_256_56;
    }
    mpz_clears(x, y, NULL);
}

void mpz_from_BIG_256_56(mpz_t dst, BIG_256_56 src) {
    mpz_import(dst, NLEN_256_56, -1, sizeof(src[0]), 0, (8 * sizeof(src[0])) - BASEBITS_256_56, src);
}

