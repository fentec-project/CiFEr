
#include <amcl/big_256_56.h>
#include <amcl/pair_BN254.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include "cifer/abe/dippe.h"
#include "cifer/data/mat.h"
#include "cifer/internal/big.h"
#include "cifer/internal/common.h"
#include "cifer/sample/uniform.h"

int cfe_dippe_int_len(int i) {
    return i ? (((int)log10((double)i)) + 1) : 1;
}

void cfe_dippe_hash_G2(ECP2_BN254 *out, char *in_buf, size_t in_buf_size) {
    // rounds
    size_t r = (in_buf_size / MODBYTES_256_56) + ((in_buf_size % MODBYTES_256_56) != 0);
    // hash buffer
    char buf[MODBYTES_256_56];

    octet tmp;
    tmp.val = (char *)&buf;
    tmp.len = MODBYTES_256_56;
    tmp.max = MODBYTES_256_56;

    ECP2_BN254_inf(out);
    ECP2_BN254 tmp_g2;

    for (size_t i = 0; i < r; i++) {
        for (size_t j = 0; j < MODBYTES_256_56; j++) {
            if (((i * MODBYTES_256_56) + j) < in_buf_size)
                buf[j] = in_buf[(i * MODBYTES_256_56) + j];
            else
                buf[j] = '\0';
            if ((((i * MODBYTES_256_56) + j + 1) % MODBYTES_256_56) == 0) {
                ECP2_BN254_mapit(&tmp_g2, &tmp);
                ECP2_BN254_add(out, &tmp_g2);
            }
        }
    }
}

void cfe_dippe_init(cfe_dippe *dippe, size_t assump_size) {
    dippe->assump_size = assump_size;
    mpz_init(dippe->p);
    mpz_from_BIG_256_56(dippe->p, (int64_t *)CURVE_Order_BN254);

    // A (k+1 x k)
    cfe_mat A;
    cfe_mat_init(&A, (assump_size + 1), assump_size);
    cfe_uniform_sample_mat(&A, dippe->p);

    // g1^A (k+1 x k)
    cfe_mat_G1_init(&(dippe->g1_A), (assump_size + 1), assump_size);
    cfe_mat_mul_G1(&(dippe->g1_A), &A);

    // U(T) (k+1 x k+1)
    cfe_mat U;
    cfe_mat_init(&U, (assump_size + 1), (assump_size + 1));
    cfe_uniform_sample_mat(&U, dippe->p);

    // U(T)A  (k+1 x k)
    cfe_mat UA;
    cfe_mat_init(&UA, (assump_size + 1), assump_size);
    cfe_mat_mul(&UA, &U, &A);
    cfe_mat_mod(&UA, &UA, dippe->p);

    // g1^U(T)A (k+1 x k)
    cfe_mat_G1_init(&(dippe->g1_UA), (assump_size + 1), assump_size);
    cfe_mat_mul_G1(&(dippe->g1_UA), &UA);

    // cleanup
    cfe_mat_free(&A);
    cfe_mat_free(&U);
    cfe_mat_free(&UA);
}

void cfe_dippe_free(cfe_dippe *dippe) {
    mpz_clear(dippe->p);
    cfe_mat_G1_free(&(dippe->g1_A));
    cfe_mat_G1_free(&(dippe->g1_UA));
}

void cfe_dippe_pub_key_init(cfe_dippe_pub_key *pk, cfe_dippe *dippe) {
    cfe_mat_G1_init(&(pk->g1_W_A), (dippe->assump_size + 1), dippe->assump_size);
    cfe_vec_GT_init(&(pk->gt_alpha_A), dippe->assump_size);
}

void cfe_dippe_pub_key_free(cfe_dippe_pub_key *pk) {
    cfe_mat_G1_free(&(pk->g1_W_A));
    cfe_vec_GT_free(&(pk->gt_alpha_A));
}

void cfe_dippe_sec_key_init(cfe_dippe_sec_key *sk, cfe_dippe *dippe) {
    mpz_init(sk->sigma);
    cfe_vec_init(&(sk->alpha), (dippe->assump_size + 1));
    cfe_mat_init(&(sk->W), (dippe->assump_size + 1), (dippe->assump_size + 1));
}

void cfe_dippe_sec_key_free(cfe_dippe_sec_key *sk) {
    mpz_clear(sk->sigma);
    cfe_vec_free(&(sk->alpha));
    cfe_mat_free(&(sk->W));
}

void cfe_dippe_cipher_init(cfe_dippe_cipher *cipher, cfe_dippe *dippe, size_t pol_size) {
    cfe_vec_G1_init(&(cipher->C0), (dippe->assump_size + 1));
    cfe_mat_G1_init(&(cipher->Ci), pol_size, (dippe->assump_size + 1));
}

void cfe_dippe_cipher_free(cfe_dippe_cipher *cipher) {
    cfe_vec_G1_free(&(cipher->C0));
    cfe_mat_G1_free(&(cipher->Ci));
}

void cfe_dippe_user_sec_key_init(cfe_dippe_user_sec_key *usk, cfe_dippe *dippe) {
    cfe_vec_G2_init(&(usk->Ki), (dippe->assump_size + 1));
}

void cfe_dippe_user_sec_key_free(cfe_dippe_user_sec_key *usk) {
    cfe_vec_G2_free(&(usk->Ki));
}

void cfe_dippe_generate_master_keys(cfe_dippe_pub_key *pk, cfe_dippe_sec_key *sk, cfe_dippe *dippe) {
    // temp vars
    BIG_256_56 tmp_big;
    cfe_mat tmp_mat;
    cfe_mat_GT tmp_mat_gt;

    ///////////////////////////////////////////////
    // Secret key section
    ///////////////////////////////////////////////

    // sigma
    cfe_uniform_sample(sk->sigma, dippe->p);

    // alpha (k+1 x 1)
    cfe_uniform_sample_vec(&(sk->alpha), dippe->p);

    // W (k+1 x k+1)
    cfe_uniform_sample_mat(&(sk->W), dippe->p);

    ///////////////////////////////////////////////
    // Public key section
    ///////////////////////////////////////////////

    // g2^sigma
    ECP2_BN254_generator(&(pk->g2_sigma));
    BIG_256_56_from_mpz(tmp_big, sk->sigma);
    ECP2_BN254_mul(&(pk->g2_sigma), tmp_big);

    // g1^W(T)A (k+1 x k)
    cfe_mat_init(&tmp_mat, sk->W.cols, sk->W.rows);
    cfe_mat_transpose(&tmp_mat, &(sk->W));
    cfe_mat_mul_G1_mat(&(pk->g1_W_A), &tmp_mat, &(dippe->g1_A));

    // gt^A (k+1 x k)
    cfe_mat_GT gt_A;
    cfe_mat_GT_init(&gt_A, dippe->g1_A.rows, dippe->g1_A.cols);
    cfe_mat_GT_pair_mat_G1(&gt_A, &(dippe->g1_A));

    // gt^(alpha(T)A) (k x 1)
    cfe_mat_GT_init(&tmp_mat_gt, gt_A.cols, gt_A.rows);
    cfe_mat_GT_transpose(&tmp_mat_gt, &gt_A);
    cfe_mat_GT_mul_vec(&(pk->gt_alpha_A), &tmp_mat_gt, &(sk->alpha));

    // cleanup
    cfe_mat_free(&tmp_mat);
    cfe_mat_GT_free(&gt_A);
    cfe_mat_GT_free(&tmp_mat_gt);
}

cfe_error cfe_dippe_encrypt(cfe_dippe_cipher *cipher, cfe_dippe *dippe, cfe_dippe_pub_key *pks[], size_t pks_len, cfe_dippe_policy_vector *pv, FP12_BN254 *msg) {
    // check whether there is the same number of keys as policy vector components
    if (pks_len != pv->len) {
        return CFE_ERR_INSUFFICIENT_KEYS;
    }

    // temp vars
    BIG_256_56 tmp_big;
    ECP_BN254 tmp_g1;
    FP12_BN254 tmp_gt;

    // s (k x 1)
    cfe_vec s;
    cfe_vec_init(&s, dippe->assump_size);
    cfe_uniform_sample_vec(&s, dippe->p);

    // C0 = g1^As (k+1 x 1)
    cfe_mat_G1_mul_vec(&(cipher->C0), &(dippe->g1_A), &s);

    // g1^(W_A_s) (k+1 x 1)
    cfe_vec_G1 g1_W_A_s;
    cfe_vec_G1_init(&g1_W_A_s, (dippe->assump_size + 1));

    // g1^(x_UA_s) (k+1 x 1)
    cfe_vec_G1 g1_x_UA_s;
    cfe_vec_G1_init(&g1_x_UA_s, (dippe->assump_size + 1));

    // Ci = g1^(xU(T)+W(T))As (pol x k+1)
    for (size_t m = 0; m < pks_len; m++) {
        // g1^(W(T)As)
        cfe_mat_G1_mul_vec(&g1_W_A_s, &(pks[m]->g1_W_A), &s);

        // g1^(W(T)As)
        for (size_t i = 0; i < (dippe->assump_size + 1); i++) {
            ECP_BN254_inf(&(g1_x_UA_s.vec[i]));
            for (size_t k = 0; k < dippe->assump_size; k++) {
                ECP_BN254_copy(&tmp_g1, &(dippe->g1_UA.mat[i].vec[k]));
                BIG_256_56_from_mpz(tmp_big, s.vec[k]);
                ECP_BN254_mul(&tmp_g1, tmp_big);
                ECP_BN254_add(&(g1_x_UA_s.vec[i]), &tmp_g1);
            }
            BIG_256_56_from_mpz(tmp_big, pv->pol.vec[m]);
            ECP_BN254_mul(&(g1_x_UA_s.vec[i]), tmp_big);
        }

        for (size_t i = 0; i < (dippe->assump_size + 1); i++) {
            ECP_BN254_copy(&(cipher->Ci.mat[m].vec[i]), &(g1_x_UA_s.vec[i]));
            ECP_BN254_add(&(cipher->Ci.mat[m].vec[i]), &(g1_W_A_s.vec[i]));
        }
    }

    // C_prime = m * gt^alpha(T)As
    FP12_BN254_one(&(cipher->C_prime));
    for (size_t m = 0; m < pks_len; m++) {
        for (size_t k = 0; k < dippe->assump_size; k++) {
            BIG_256_56_from_mpz(tmp_big, s.vec[k]);
            FP12_BN254_pow(&tmp_gt, &(pks[m]->gt_alpha_A.vec[k]), tmp_big);
            FP12_BN254_mul(&(cipher->C_prime), &tmp_gt);
        }
    }
    FP12_BN254_mul(&(cipher->C_prime), msg);

    // cleanup
    cfe_vec_free(&s);
    cfe_vec_G1_free(&g1_W_A_s);
    cfe_vec_G1_free(&g1_x_UA_s);

    return CFE_ERR_NONE;
}

cfe_error cfe_dippe_keygen(cfe_dippe_user_sec_key *usk, cfe_dippe *dippe, size_t usk_id, cfe_dippe_pub_key *pks[], size_t pks_len, cfe_dippe_sec_key *sk, cfe_dippe_attribute_vector *av, char gid[]) {
    // check whether there is the same number of keys as attribute vector components
    if ((av->len != pks_len) || (pks_len <= usk_id)) {
        return CFE_ERR_INSUFFICIENT_KEYS;
    }

    // temp vars
    BIG_256_56 tmp_big;
    ECP2_BN254 tmp_g2;
    ECP2_BN254 hashed;

    // char buffer that is about to be hashed into a group element
    int buf_len = (4 * MODBYTES_256_56) + strlen(gid) + strlen(av->s) + 3;
    char buf[buf_len];
    octet oct;
    oct.val = (char *)&buf;

    // mü
    ECP2_BN254 mue;
    ECP2_BN254_inf(&mue);
    for (size_t i = 0; i < pks_len; i++) {
        ECP2_BN254_copy(&tmp_g2, &(pks[i]->g2_sigma));
        BIG_256_56_from_mpz(tmp_big, sk->sigma);
        ECP2_BN254_mul(&tmp_g2, tmp_big);

        // hash element
        ECP2_BN254_toOctet(&oct, &tmp_g2);
        sprintf(buf + (4 * MODBYTES_256_56), "|%s|%s", gid, av->s);
        cfe_dippe_hash_G2(&hashed, buf, buf_len);

        if (i < usk_id) {
            ECP2_BN254_add(&mue, &hashed);
        } else if (i > usk_id) {
            ECP2_BN254_sub(&mue, &hashed);
        }
    }

    char buf2[cfe_dippe_int_len(dippe->assump_size + 1) + strlen(gid) + strlen(av->s) + 3];
    // g2^h (k+1 x 1)
    cfe_vec_G2 g2_h;
    cfe_vec_G2_init(&g2_h, (dippe->assump_size + 1));
    for (size_t i = 0; i < (dippe->assump_size + 1); i++) {
        sprintf(buf2, "%zu|%s|%s", i, gid, av->s);
        cfe_dippe_hash_G2(&(g2_h.vec[i]), buf2, strlen(buf2));
    }

    // Ki = g2^(α−vWh+μ) (k+1 x 1)
    for (size_t i = 0; i < (dippe->assump_size + 1); i++) {
        ECP2_BN254_inf(&(usk->Ki.vec[i]));
        // add vi
        if (mpz_tstbit(av->attrs, usk_id) == 1) {
            for (size_t k = 0; k < dippe->assump_size + 1; k++) {
                ECP2_BN254_copy(&tmp_g2, &(g2_h.vec[k]));
                BIG_256_56_from_mpz(tmp_big, sk->W.mat[i].vec[k]);
                ECP2_BN254_mul(&tmp_g2, tmp_big);
                ECP2_BN254_add(&(usk->Ki.vec[i]), &tmp_g2);
            }
            // negate
            ECP2_BN254_neg(&(usk->Ki.vec[i]));
        }

        // add alpha
        BIG_256_56_from_mpz(tmp_big, sk->alpha.vec[i]);
        ECP2_BN254_generator(&tmp_g2);
        ECP2_BN254_mul(&tmp_g2, tmp_big);
        ECP2_BN254_add(&(usk->Ki.vec[i]), &tmp_g2);

        // add mue
        ECP2_BN254_add(&(usk->Ki.vec[i]), &mue);
    }

    // cleanup
    cfe_vec_G2_free(&g2_h);

    return CFE_ERR_NONE;
}

cfe_error cfe_dippe_decrypt(FP12_BN254 *result, cfe_dippe *dippe, cfe_dippe_user_sec_key *usks, size_t usks_len, cfe_dippe_cipher *cipher, cfe_dippe_attribute_vector *av, char gid[]) {
    // check whether there is the same number of keys as attribute vector components
    if (av->len != usks_len) {
        return CFE_ERR_INSUFFICIENT_KEYS;
    }

    // temp vars
    ECP_BN254 tmp_g1;
    ECP2_BN254 tmp_g2;
    FP12_BN254 tmp_gt;

    // C0_K
    FP12_BN254 C0_K;
    FP12_BN254_one(&C0_K);
    for (size_t i = 0; i < (dippe->assump_size + 1); i++) {
        ECP2_BN254_inf(&tmp_g2);
        for (size_t j = 0; j < usks_len; j++) {
            ECP2_BN254_add(&tmp_g2, &((usks + j)->Ki.vec[i]));
        }
        PAIR_BN254_ate(&tmp_gt, &tmp_g2, &(cipher->C0.vec[i]));
        PAIR_BN254_fexp(&tmp_gt);
        FP12_BN254_mul(&C0_K, &tmp_gt);
    }

    // Ci_H
    FP12_BN254 Ci_H;
    FP12_BN254_one(&Ci_H);
    char buf[cfe_dippe_int_len(dippe->assump_size + 1) + strlen(gid) + strlen(av->s) + 3];
    for (size_t i = 0; i < (dippe->assump_size + 1); i++) {
        ECP_BN254_inf(&tmp_g1);
        for (size_t j = 0; j < usks_len; j++) {
            if (mpz_tstbit(av->attrs, j) == 1) {
                ECP_BN254_add(&tmp_g1, &(cipher->Ci.mat[j].vec[i]));
            }
        }
        sprintf(buf, "%zu|%s|%s", i, gid, av->s);
        cfe_dippe_hash_G2(&tmp_g2, buf, strlen(buf));
        PAIR_BN254_ate(&tmp_gt, &tmp_g2, &tmp_g1);
        PAIR_BN254_fexp(&tmp_gt);
        FP12_BN254_mul(&Ci_H, &tmp_gt);
    }

    // C0_K * Ci_H
    FP12_BN254_copy(&tmp_gt, &C0_K);
    FP12_BN254_mul(&tmp_gt, &Ci_H);

    // recover secret
    FP12_BN254_inv(&tmp_gt, &tmp_gt);
    FP12_BN254_copy(result, &(cipher->C_prime));
    FP12_BN254_mul(result, &tmp_gt);

    return CFE_ERR_NONE;
}

cfe_error cfe_dippe_attribute_vector_init(cfe_dippe_attribute_vector *av, size_t vec_len, size_t pattern[], size_t pat_len) {
    mpz_init(av->attrs);
    mpz_set_ui(av->attrs, 0);
    for (size_t i = 0; i < pat_len; i++) {
        if (pattern[i] < vec_len) {
            mpz_setbit(av->attrs, pattern[i]);
        } else
            return CFE_ERR_MALFORMED_INPUT;
    }
    mpz_setbit(av->attrs, (vec_len - 1));

    // attribute vector as string
    av->s = (char *)cfe_malloc((mpz_sizeinbase(av->attrs, 62) + 1) * sizeof(char));
    mpz_get_str(av->s, 62, av->attrs);
    // attribute vector length
    av->len = vec_len;

    return CFE_ERR_NONE;
}

void cfe_dippe_attribute_vector_free(cfe_dippe_attribute_vector *av) {
    mpz_clear(av->attrs);
    free(av->s);
}

cfe_error cfe_dippe_threshold_policy_vector_init(cfe_dippe_policy_vector *pv, cfe_dippe *dippe, size_t vec_len, size_t pattern[], size_t pat_len, size_t threshold) {
    mpz_t zero;
    mpz_init(zero);
    mpz_set_ui(zero, 0);
    cfe_vec_init(&(pv->pol), vec_len);
    cfe_vec_set_const(&(pv->pol), zero);

    for (size_t i = 0; i < pat_len; i++) {
        if (pattern[i] < vec_len) {
            mpz_set_ui(pv->pol.vec[pattern[i]], 1);
        } else
            return CFE_ERR_MALFORMED_INPUT;
    }
    mpz_set_ui(pv->pol.vec[vec_len - 1], threshold);
    mpz_neg(pv->pol.vec[vec_len - 1], pv->pol.vec[vec_len - 1]);
    mpz_mod(pv->pol.vec[vec_len - 1], pv->pol.vec[vec_len - 1], dippe->p);

    // policy vector length
    pv->len = vec_len;

    // cleanup
    mpz_clear(zero);

    return CFE_ERR_NONE;
}

cfe_error cfe_dippe_conjunction_policy_vector_init(cfe_dippe_policy_vector *pv, cfe_dippe *dippe, size_t vec_len, size_t pattern[], size_t pat_len) {
    mpz_t zero;
    mpz_init(zero);
    mpz_set_ui(zero, 0);
    cfe_vec_init(&(pv->pol), vec_len);
    cfe_vec_set_const(&(pv->pol), zero);

    for (size_t i = 0; i < pat_len; i++) {
        if (pattern[i] < vec_len) {
            cfe_uniform_sample(pv->pol.vec[pattern[i]], dippe->p);
            mpz_sub(pv->pol.vec[vec_len - 1], pv->pol.vec[vec_len - 1], pv->pol.vec[pattern[i]]);
        } else
            return CFE_ERR_MALFORMED_INPUT;
    }
    mpz_mod(pv->pol.vec[vec_len - 1], pv->pol.vec[vec_len - 1], dippe->p);

    // policy vector length
    pv->len = vec_len;

    // cleanup
    mpz_clear(zero);

    return CFE_ERR_NONE;
}

void cfe_dippe_policy_vector_free(cfe_dippe_policy_vector *pv) {
    cfe_vec_free(&(pv->pol));
}
