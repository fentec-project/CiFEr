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

#include <gmp.h>
#include <internal/big.h>
#include <amcl/ecp_BN254.h>
#include <amcl/ecp2_BN254.h>
#include <amcl/pair_BN254.h>
#include "test.h"

MunitResult test_amcl(const MunitParameter *params, void *data) {
    // check if e(g1 + g1, g2) = e(g1, g2 + g2)
    ECP_BN254 G1, G1sum;
    ECP_BN254_generator(&G1);
    ECP_BN254_generator(&G1sum);
    ECP_BN254_add(&G1sum, &G1);

    ECP2_BN254 G2, G2sum;
    ECP2_BN254_generator(&G2);
    ECP2_BN254_generator(&G2sum);
    ECP2_BN254_add(&G2sum, &G2);

    FP12_BN254 GTsum1, GTsum2;
    PAIR_BN254_ate(&GTsum1, &G2, &G1sum);
    PAIR_BN254_fexp(&GTsum1);
    PAIR_BN254_ate(&GTsum2, &G2sum, &G1);
    PAIR_BN254_fexp(&GTsum2);

    int check = FP12_BN254_equals(&GTsum1, &GTsum2);
    munit_assert(check);

    // check if e(g1 + g1, g2) = e(g1, g2)^2
    FP12_BN254 GT_squared;
    PAIR_BN254_ate(&GT_squared, &G2, &G1);
    PAIR_BN254_fexp(&GT_squared);
    BIG_256_56 two;
    BIG_256_56_one(two);
    BIG_256_56_imul(two, two, 2);
    FP12_BN254_pow(&GT_squared, &GT_squared, two);
    check = FP12_BN254_equals(&GT_squared, &GTsum1);
    munit_assert(check);

    // check if g * CURVE_Order = identity in ECPNB254
    ECP_BN254 H, H_prime;
    ECP_BN254_generator(&H);
    ECP_BN254_generator(&H_prime);
    BIG_256_56 zero;
    BIG_256_56_zero(zero);
    ECP_BN254_mul(&H, (int64_t*) CURVE_Order_BN254);
    ECP_BN254_mul(&H_prime, zero);
    check = ECP_BN254_equals(&H, &H_prime);
    munit_assert(check);

    // check if e(g1*4, g2*4^-1) = e(g1, g2)
    FP12_BN254 GT, GT2;
    PAIR_BN254_ate(&GT, &G2, &G1);
    PAIR_BN254_fexp(&GT);
    mpz_t four, four_inv, p;
    mpz_init_set_ui(four, 4);
    mpz_inits(four_inv, p, NULL);
    mpz_from_BIG_256_56(p, (int64_t*) CURVE_Order_BN254);
    mpz_invert(four_inv, four, p);

    BIG_256_56 four_big, four_inv_big;
    BIG_256_56_from_mpz(four_big, four);
    BIG_256_56_from_mpz(four_inv_big, four_inv);

    ECP_BN254_mul(&G1, four_big);
    ECP2_BN254_mul(&G2, four_inv_big);
    PAIR_BN254_ate(&GT2, &G2, &G1);
    PAIR_BN254_fexp(&GT2);
    check = FP12_BN254_equals(&GT, &GT2);
    munit_assert(check);

    return MUNIT_OK;
}

MunitTest big_tests[] = {
        {(char *) "/test-amcl",  test_amcl,            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite big_suite = {
        (char *) "/abe/big", big_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
