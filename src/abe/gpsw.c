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
#include <internal/common.h>
#include <internal/big.h>
#include <amcl/big_256_56.h>
#include <amcl/fp12_BN254.h>
#include <amcl/pair_BN254.h>

#include "sample/uniform.h"
#include "abe/policy.h"
#include "abe/gpsw.h"
#include "data/vec_curve.h"



void cfe_gpsw_init(cfe_gpsw *gpsw, size_t l) {
    gpsw->l = l;
    mpz_init(gpsw->p);
    mpz_from_BIG_256_56(gpsw->p, CURVE_Order_BN254);
}

void generate_master_keys(cfe_gpsw *gpsw, cfe_gpsw_pub_key *pk, cfe_vec *sk) {
    pk->t = (cfe_vec_G2 *) cfe_malloc(sizeof(cfe_vec_G2));
    pk->y = (FP12_BN254 *) cfe_malloc(sizeof(FP12_BN254));

    cfe_vec_init(sk, gpsw->l + 1);
    cfe_uniform_sample_vec(sk, gpsw->p);

    cfe_vec sub_sk;
    cfe_vec_init(&sub_sk, gpsw->l);
    cfe_vec_extract(&sub_sk, sk, 0, gpsw->l);

    cfe_vec_mul_G2(pk->t, &sub_sk);

    ECP_BN254 g1;
    ECP2_BN254 g2;
    FP12_BN254 gT;
    ECP_BN254_generator(&g1);
//    gmp_printf("generator 1\n");

//    ECP_BN254_output(&g1);


    ECP2_BN254_generator(&g2);
//    gmp_printf("generator 2\n");

//    ECP2_BN254_output(&g2);

//    gmp_printf("\n");

    PAIR_BN254_ate(&gT, &g2, &g1);
    PAIR_BN254_fexp(&gT);

//    gmp_printf("generator 3\n");

//    ECP2_BN254_output(&gT);

    BIG_256_56 x;
    BIG_256_56_from_mpz(x, sk->vec[gpsw->l]);
    FP12_BN254_pow(pk->y, &gT, x);
//    gmp_printf("generator 3a\n");

//    FP12_BN254_output(pk->y);
}

void gpsw_encrypt(cfe_gpsw_cipher *cipher, cfe_gpsw *gpsw, FP12_BN254 *msg,
        int *gamma, size_t num_attrib, cfe_gpsw_pub_key *pk) {
    mpz_t s;
    mpz_init(s);
    cfe_uniform_sample(s, gpsw->p);
//    mpz_set_ui(s, 1);
    BIG_256_56 s_big;
//    gmp_printf("aaa%Zd\n", s);
    BIG_256_56_from_mpz(s_big, s);
    FP12_BN254_pow(&(cipher->e0), pk->y, s_big);
//    gmp_printf("cipher1\n");
//    FP12_BN254_output(&(cipher->e0));

    FP12_BN254_mul(&(cipher->e0), msg);

//    gmp_printf("cipher2\n");
//    FP12_BN254_output(&(cipher->e0));

//    cipher->atrib_to_i = (int *) cfe_malloc(num_attrib * sizeof(int));
    cfe_vec_G2_init(&(cipher->e), num_attrib);
    cipher->gamma = (int *) cfe_malloc(num_attrib * sizeof(int));
    for (size_t i = 0; i < num_attrib; i++) {
        cipher->gamma[i] = gamma[i];
        cipher->e.vec[i] = (pk->t)->vec[gamma[i]];
        ECP2_BN254_mul(&(cipher->e.vec[i]), s_big);
    }
}

void get_sum(cfe_vec *v, mpz_t y, mpz_t p, size_t d) {
    cfe_vec_init(v, d);
    cfe_uniform_sample_vec(v, p);
    mpz_t sum;
    mpz_init_set_ui(sum, 0);
    for (size_t i = 0; i < d - 1; i++) {
        mpz_add(sum, sum, v->vec[i]);
    }
    mpz_sub(v->vec[d-1], y, sum);
    mpz_mod(v->vec[d-1], v->vec[d-1], p);
}


void generate_policy_keys(cfe_vec_G1 *policy_keys, cfe_gpsw *gpsw, cfe_msp *msp, cfe_vec *sk) {
    cfe_vec u;
    get_sum(&u, sk->vec[gpsw->l], gpsw->p, msp->mat->cols);

    mpz_t t_map_i_inv, mat_times_u, pow;
    mpz_inits(t_map_i_inv,mat_times_u, pow, NULL);
    BIG_256_56 pow_big;
    cfe_vec_G1_init(policy_keys, msp->mat->rows);
    for (size_t i = 0; i < msp->mat->rows; i++) {
        mpz_invert(t_map_i_inv, sk->vec[msp->row_to_attrib[i]], gpsw->p);
//        gmp_printf("uuuuuuuuuu%Zd\n", t_map_i_inv);

        cfe_vec_dot(mat_times_u, &(msp->mat->mat[i]), &u);
        mpz_mul(pow, t_map_i_inv, mat_times_u);
        mpz_mod(pow, pow, gpsw->p);
//        gmp_printf("uuuuuuuuuu%Zd\n", t_map_i_inv);

        ECP_BN254_generator(&(policy_keys->vec[i]));
//        gmp_printf("aaa%Zd\n", pow);
        BIG_256_56_from_mpz(pow_big, pow);
        ECP_BN254_mul(&(policy_keys->vec[i]), pow_big);
    }
}

void delegate_keys(cfe_gpsw_keys *keys, cfe_vec_G1 *policy_keys,
        cfe_msp *msp, int *atrib, size_t num_attrib) {

    size_t count_attrib = 0;
    size_t positions[num_attrib];
    for (size_t i = 0; i < msp->mat->rows; i++) {
        for (size_t j = 0; j < num_attrib; j++) {
//            gmp_printf("compare %d %d \n", msp->row_to_attrib[i], atrib[j]);
            if (msp->row_to_attrib[i] == atrib[j]) {

                positions[count_attrib] = i;
                count_attrib++;
                break;
            }
        }
    }

    cfe_mat_init(&(keys->mat), num_attrib, msp->mat->cols);
    cfe_vec_G1_init(&(keys->d), num_attrib);
    keys->row_to_attrib = (int *) cfe_malloc(sizeof(int) * count_attrib);
//    gmp_printf("num %d\n", count_attrib);

    for (size_t i = 0; i < count_attrib; i++) {
//        gmp_printf("here\n");
//        cfe_vec_print(&(msp->mat->mat[positions[i]]));
        cfe_mat_set_vec(&(keys->mat), &(msp->mat->mat[positions[i]]), i);
        ECP_BN254_copy(&(keys->d.vec[i]), &(policy_keys->vec[positions[i]]));
        keys->row_to_attrib[i] = msp->row_to_attrib[positions[i]];
    }

}


int gpsw_decrypt(FP12_BN254 *decryption, cfe_gpsw_cipher *cipher, cfe_gpsw_keys *keys, cfe_gpsw *gpsw) {
    cfe_vec one_vec, alpha;
    mpz_t one;
    mpz_init_set_ui(one, 1);
    cfe_vec_const(&one_vec, keys->mat.cols, one);

    cfe_mat mat_transpose;
    cfe_mat_init(&mat_transpose, keys->mat.cols, keys->mat.rows);
    cfe_mat_transpose(&mat_transpose, &(keys->mat));

//    cfe_mat_print(&mat_transpose);

    int check = gaussian_elimination(&alpha, &mat_transpose, &one_vec, gpsw->p);
    if (check) {
        return check;
    }
//    gmp_printf("alpha\n");

//    cfe_vec_print(&alpha);
//    gmp_printf("\n");

    size_t positions[keys->mat.rows];
    for (size_t i = 0; i < keys->mat.rows; i++) {
        for (size_t j = 0; j < cipher->e.size; j++) {
//            gmp_printf("compare2 %d %d \n", keys->row_to_attrib[i], cipher->gamma[j]);
            if (keys->row_to_attrib[i] == cipher->gamma[j]) {
                positions[i] = j;
                break;
            }
        }
    }

    FP12_BN254_copy(decryption, &(cipher->e0));

//    gmp_printf("cipher3\n");
//    FP12_BN254_output(&(cipher->e0));

//    gmp_printf("decryption\n");

//    FP12_BN254_output(decryption);

    FP12_BN254 pair;
    FP12_BN254 pair_pow;
    FP12_BN254 pair_pow_inv;

    BIG_256_56 alpha_i;
    for (size_t i = 0; i < keys->mat.rows; i++) {
//        gmp_printf("position%d\n", positions[i]);

        PAIR_BN254_ate(&pair, &(cipher->e.vec[positions[i]]), &(keys->d.vec[i]));
        PAIR_BN254_fexp(&pair);
//        gmp_printf("aaa%Zd\n", alpha.vec[i]);
        BIG_256_56_from_mpz(alpha_i, alpha.vec[i]);
        FP12_BN254_pow(&pair_pow, &pair, alpha_i);

//        gmp_printf("cipher4\n");
//        FP12_BN254_output(&pair_pow_inv);

        FP12_BN254_inv(&pair_pow_inv, &pair_pow);
        FP12_BN254_mul(decryption, &pair_pow_inv);
    }

    return 0;
}


















