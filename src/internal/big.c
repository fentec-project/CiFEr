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

#include "internal/big.h"


void BIG_256_56_from_mpz(BIG_256_56 a, mpz_t b) {
    int i;
    BIG_256_56_zero(a);
    mpz_t x, y;
    mpz_inits(x, y, NULL);
    mpz_set(y, b);
    size_t size = (MODBYTES_256_56 * 8) - ((MODBYTES_256_56 * 8) % BASEBITS_256_56);
    for (i=0; i<((MODBYTES_256_56 * 8) / BASEBITS_256_56) + 1; i++)
    {
        BIG_256_56_fshl(a, BASEBITS_256_56);
        mpz_fdiv_q_2exp(x, y, size);
        mpz_fdiv_r_2exp(y, y, size);
        a[0]+= mpz_get_ui(x);
        size = size - BASEBITS_256_56;
    }
    mpz_clears(x, y, NULL);
}

void mpz_from_BIG_256_56(mpz_t b, BIG_256_56 a) {
    mpz_import(b, NLEN_256_56, -1, sizeof(a[0]), 0, (8 * sizeof(a[0])) - BASEBITS_256_56, a);
}

//void mpz_from_BIG_256_56_2(mpz_t c, BIG_256_56 a)
//{
//    BIG_256_56 b;
//    int i,len;
//    len=BIG_256_56_nbits(a);
//    if (len%4==0) len/=4;
//    else
//    {
//        len/=4;
//        len++;
//    }
//    if (len<MODBYTES_256_56*2) len=MODBYTES_256_56*2;
//
//    unsigned int s[len];
//    char s2[len];
//
//    for (i=len-1; i>=0; i--)
//    {
//        BIG_256_56_copy(b,a);
//        BIG_256_56_shr(b,i*4);
//        s[len-1-i] = (unsigned int) b[0]&15;
////        char u = (char) s[i];
////        printf("%d", s[len-1-i]);
////        printf("%c", '0' + s[i]);
//        sprintf(&s2[len-1-i], "%x", s[len-1-i]);
////        s2[len-1-i] = '0' + s[len-1-i];
//
////        printf("%01x",(unsigned int) b[0]&15);
//    }
////    mpz_import(c, len, -1, sizeof(s[0]), 0, sizeof(s[0]) - 3, a);
////    gmp_printf("\n%Zd \n", c);
////    printf("\n%s", s2);
////    mpz_import(c, len, 1, sizeof(s[0]), 0, sizeof(s[0]) - 4, s);
//    mpz_set_str(c, s2, 16);
////    gmp_printf("\nmpz %Zd \n", c);
//
//}
