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

#include <sodium.h>
#include <stdint.h>

#include "cifer/sample/normal_cdt.h"

#define CDT_ENTRY_SIZE 16
uint64_t CDT_LOW_MASK = 0x7fffffffffffffff;
size_t CDT_LENGTH = 9;


/* CDT table */
uint64_t CDT[][2] = {{2200310400551559144, 3327841033070651387},
                     {7912151619254726620, 380075531178589176},
                     {5167367257772081627, 11604843442081400},
                     {5081592746475748971, 90134450315532},
                     {6522074513864805092, 175786317361},
                     {2579734681240182346, 85801740},
                     {8175784047440310133, 10472},
                     {2947787991558061753, 0},
                     {22489665999543,      0}};


/* constant time CDT sampler */
void cfe_normal_cdt_sample(mpz_t res) {
    uint64_t x = 0;
    uint64_t r1, r2;

    uint8_t r[16];
    randombytes_buf(r, 16);

    r1 = (*((uint64_t *) r)) & CDT_LOW_MASK;
    r2 = (*((uint64_t *) (r + 8))) & CDT_LOW_MASK;

    for (size_t i = 0; i < CDT_LENGTH; i++) {
        x += (((r1 - CDT[i][0]) & ((1LL << 63) ^ ((r2 - CDT[i][1]) | (CDT[i][1] - r2)))) | (r2 - CDT[i][1])) >> 63;
    }

    mpz_set_ui(res, x);
}
