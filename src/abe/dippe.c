
#include <math.h>
#include <string.h>
#include <amcl/big_256_56.h>
#include <amcl/pair_BN254.h>

#include "cifer/internal/common.h"
#include "cifer/internal/big.h"
#include "cifer/sample/uniform.h"
#include "cifer/abe/dippe.h"


/**
 * Helper function that returns the number of digits for an int
 *
 * @param i The integer to determine the digit count from
 * @return The digit count
 */
int cfe_dippe_int_len(int i) {
    return i ? (((int)log10((double)i))+1) : 1;
}

/**
 * Hashes the given buffer to an element of G2
 *
 * @param out A pointer to a ECP2_BN254 struct; The resulting group element
 * @param in_buf Buffer that is about to be hashed
 * @param in_buf_size Size of the buffer
 */
void cfe_dippe_hash_G2(ECP2_BN254 *out, char* in_buf, int in_buf_size) {
    // rounds
    int r = (in_buf_size / MODBYTES_256_56) + ((in_buf_size % MODBYTES_256_56) != 0);
    // hash buffer
    char buf[MODBYTES_256_56];

    octet tmp;
    tmp.val = (char*)&buf;
    tmp.len = MODBYTES_256_56;
    tmp.max = MODBYTES_256_56;

    ECP2_BN254_inf(out);
    ECP2_BN254 tmp_g2;

    for (int i=0; i<r; i++) {
        for (int j=0; j<MODBYTES_256_56; j++) {
            if (((i*MODBYTES_256_56)+j) < in_buf_size) buf[j] = in_buf[(i*MODBYTES_256_56)+j]; else buf[j] = '\0';
            if ((((i*MODBYTES_256_56)+j+1) % MODBYTES_256_56) == 0) {
                ECP2_BN254_mapit(&tmp_g2, &tmp);
                ECP2_BN254_add(out, &tmp_g2);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Clears the scheme and frees allocated memory
 *
 * @param dippe A pointer to a cfe_dippe struct
 */
void cfe_dippe_free(cfe_dippe *dippe) {
    mpz_clear(dippe->p);
    free(dippe->g1_A);
    free(dippe->g1_UA);
}

/**
 * Clears the public key and frees allocated memory
 *
 * @param pk A pointer to a cfe_dippe_pub_key struct
 */
void cfe_dippe_pub_key_free(cfe_dippe_pub_key *pk) {
	free(pk->gt_alpha_A);
	free(pk->g1_W_A);
}

/**
 * Clears the secret key and frees allocated memory
 *
 * @param sk A pointer to a cfe_dippe_sec_key struct
 */
void cfe_dippe_sec_key_free(cfe_dippe_sec_key *sk) {
	mpz_clear(sk->sigma);
	cfe_vec_free(&(sk->alpha));
	cfe_vec_free(&(sk->W));
}

/**
 * Clears the ciphertext and frees allocated memory
 *
 * @param cipher A pointer to a cfe_dippe_cipher struct
 */
void cfe_dippe_cipher_free(cfe_dippe_cipher *cipher) {
    free(cipher->C0);
    free(cipher->Ci);
}

/**
 * Clears the user secret key and frees allocated memory
 *
 * @param usk A pointer to a cfe_dippe_user_secret_key struct
 */
void cfe_dippe_user_secret_key_free(cfe_dippe_user_secret_key *usk) {
    free(usk->Ki);
}

/**
 * Initializes the DIPPE scheme and generates the global parameters
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param assump_size The size of the underlying assumption
 */
void cfe_dippe_setup(cfe_dippe *dippe, unsigned int assump_size) {

    // temp vars
    BIG_256_56 tmp_big;
    mpz_t tmp_mpz;
    mpz_init(tmp_mpz);

    dippe->assump_size = assump_size;
    mpz_init(dippe->p);
    mpz_from_BIG_256_56(dippe->p, (int64_t*)CURVE_Order_BN254);

    // A (k+1 x k)
    cfe_vec A;
    cfe_vec_init(&A, ((assump_size+1) * assump_size));
    cfe_uniform_sample_vec(&A, dippe->p);

    // g1^A (k+1 x k)
    dippe->g1_A = (ECP_BN254*)cfe_malloc(sizeof(ECP_BN254) * (assump_size+1) * assump_size);
    for (unsigned int i=0; i<(assump_size+1); i++) {
        for (unsigned int j=0; j<assump_size; j++) {
            ECP_BN254_generator(&(dippe->g1_A[(assump_size*i)+j]));
            BIG_256_56_from_mpz(tmp_big, A.vec[(assump_size*i)+j]);
            ECP_BN254_mul(&(dippe->g1_A[(assump_size*i)+j]), tmp_big);
        }
    }

    // U (k+1 x k+1)
    cfe_vec U;
    cfe_vec_init(&U, ((assump_size+1) * (assump_size+1)));
    cfe_uniform_sample_vec(&U, dippe->p);

    // U(T)A  (k+1 x k)
    cfe_vec UA;
    cfe_vec_init(&UA, ((assump_size+1) * assump_size));
    for (unsigned int i=0; i<(assump_size+1); i++) {
        for (unsigned int j=0; j<assump_size; j++) {
            mpz_set_ui(UA.vec[(assump_size*i)+j], 0);
            for (unsigned int k=0; k<(assump_size+1); k++) {
                mpz_mul(tmp_mpz, U.vec[i+(k*(assump_size+1))], A.vec[(assump_size*k)+j]);
                mpz_add(UA.vec[(assump_size*i)+j], UA.vec[(assump_size*i)+j], tmp_mpz);
                mpz_mod(UA.vec[(assump_size*i)+j], UA.vec[(assump_size*i)+j], dippe->p);
            }
        }
    }

    // g1^U(T)A (k+1 x k)
    dippe->g1_UA = (ECP_BN254*)cfe_malloc(sizeof(ECP_BN254) * (assump_size+1) * assump_size);
    for (unsigned int i=0; i<(assump_size+1); i++) {
        for (unsigned int j=0; j<assump_size; j++) {
            ECP_BN254_generator(&(dippe->g1_UA[(assump_size*i)+j]));
            BIG_256_56_from_mpz(tmp_big, UA.vec[(assump_size*i)+j]);
            ECP_BN254_mul(&(dippe->g1_UA[(assump_size*i)+j]), tmp_big);
        }
    }

    // cleanup
    cfe_vec_free(&A);
    cfe_vec_free(&U);
    cfe_vec_free(&UA);
    mpz_clear(tmp_mpz);
}

/**
 * Initializes an authority nad generates its public and private key
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param pk A pointer to a cfe_dippe_pub_key struct; Represents the public key that is about to be populated
 * @param sk A pointer to a cfe_dippe_sec_key struct; Represents the secret key that is about to be populated
 */
void cfe_dippe_authsetup(cfe_dippe *dippe, cfe_dippe_pub_key *pk, cfe_dippe_sec_key *sk) {

    // temp vars
    ECP_BN254  tmp_g1;
    FP12_BN254 tmp_gt;
    BIG_256_56 tmp_big;
    ECP2_BN254 tmp_g2;

    ECP2_BN254_generator(&tmp_g2);

    ///////////////////////////////////////////////
    // Secret key section
    ///////////////////////////////////////////////

    // sigma
    mpz_init(sk->sigma);
    cfe_uniform_sample(sk->sigma, dippe->p);

    // alpha (k+1 x 1)
    cfe_vec_init(&(sk->alpha), (dippe->assump_size+1));
    cfe_uniform_sample_vec(&(sk->alpha), dippe->p);

    // W (k+1 x k+1)
    cfe_vec_init(&(sk->W), (dippe->assump_size+1) * (dippe->assump_size+1));
    cfe_uniform_sample_vec(&(sk->W), dippe->p);

    ///////////////////////////////////////////////
    // Public key section
    ///////////////////////////////////////////////

    // g2^sigma
    ECP2_BN254_generator(&(pk->g2_sigma));
    BIG_256_56_from_mpz(tmp_big, sk->sigma);
    ECP2_BN254_mul(&(pk->g2_sigma), tmp_big);

    // W(T)A (k+1 x k)
    pk->g1_W_A = (ECP_BN254*)cfe_malloc(sizeof(ECP_BN254) * (dippe->assump_size+1) * dippe->assump_size);
    for (unsigned int i=0; i<(dippe->assump_size+1); i++) {
        for (unsigned int j=0; j<(dippe->assump_size); j++) {
            ECP_BN254_inf(&(pk->g1_W_A[(dippe->assump_size*i)+j]));
            for (unsigned int k=0; k<(dippe->assump_size+1); k++) {
                ECP_BN254_copy(&tmp_g1, &(dippe->g1_A[(dippe->assump_size*k)+j]));
                BIG_256_56_from_mpz(tmp_big, sk->W.vec[i+(k*(dippe->assump_size+1))]);
                ECP_BN254_mul(&tmp_g1, tmp_big);
                ECP_BN254_add(&(pk->g1_W_A[(dippe->assump_size*i)+j]), &tmp_g1);
            }
        }
    }

    // gt^A (k+1 x k)
    FP12_BN254 gt_A[((dippe->assump_size+1) * dippe->assump_size)];
    for (unsigned int i=0; i<((dippe->assump_size+1) * dippe->assump_size); i++) {
        PAIR_BN254_ate(&(gt_A[i]), &tmp_g2, &(dippe->g1_A[i]));
        PAIR_BN254_fexp(&(gt_A[i]));
    }

    // gt^(alpha(T)A) (k x 1)
    pk->gt_alpha_A = (FP12_BN254*)cfe_malloc(sizeof(FP12_BN254) * dippe->assump_size);
    for (unsigned int i=0; i<dippe->assump_size; i++) {
        FP12_BN254_one(&(pk->gt_alpha_A[i]));
        for (unsigned int k=0; k<(dippe->assump_size+1); k++) {
            BIG_256_56_from_mpz(tmp_big, sk->alpha.vec[k]);
            FP12_BN254_pow(&tmp_gt, &(gt_A[(dippe->assump_size*k)+i]), tmp_big);
            FP12_BN254_mul(&(pk->gt_alpha_A[i]), &tmp_gt);
        }
    }
}

/**
 * Encrypts a given message by the provided policy
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param cipher A pointer to a cfe_dippe_cipher struct; Represents the ciphertext that is about to be populated
 * @param pks An array containing references to cfe_dippe_pub_key structs;
 * @param pks_len Length of the public key array
 * @param pol A reference to a cfe_vec struct; Represents the policy under which the message will be encrypted
 * @param msg A reference to a FP12_BN254 struct; Represents the message that is about to be encrypted
 * @return Error code
 */
cfe_error cfe_dippe_encrypt(cfe_dippe *dippe, cfe_dippe_cipher *cipher, cfe_dippe_pub_key *pks[], size_t pks_len, cfe_vec *pol, FP12_BN254 *msg) {
    // check whether there is the same number of keys as policy vector components
    if (pks_len != pol->size) {
        return CFE_ERR_INSUFFICIENT_KEYS;
    }

    // temp vars
    BIG_256_56 tmp_big;
    ECP_BN254  tmp_g1;
    FP12_BN254 tmp_gt;

    // s (k x 1)
    cfe_vec s;
    cfe_vec_init(&s, dippe->assump_size);
    cfe_uniform_sample_vec(&s, dippe->p);

    // C0 (k+1 x 1)
    cipher->C0 = (ECP_BN254*)malloc(sizeof(ECP_BN254) * (dippe->assump_size+1));
    for (unsigned int i=0; i<(dippe->assump_size+1); i++) {
        ECP_BN254_inf(&(cipher->C0[i]));
        for (unsigned int k=0; k<dippe->assump_size; k++) {
            ECP_BN254_copy(&tmp_g1, &(dippe->g1_A[(dippe->assump_size*i)+k]));
            BIG_256_56_from_mpz(tmp_big, s.vec[k]);
            ECP_BN254_mul(&tmp_g1, tmp_big);
            ECP_BN254_add(&(cipher->C0[i]), &tmp_g1);
        }
    }

    // g1^(W_A_s)
    ECP_BN254 g1_W_A_s[dippe->assump_size+1];
    // g1^(x_UA_s)
    ECP_BN254 g1_x_UA_s[dippe->assump_size+1];

    // Ci (pol->size x k+1)
    cipher->Ci = (ECP_BN254*)malloc(sizeof(ECP_BN254) * pol->size * (dippe->assump_size+1));
    for (unsigned int m=0; m<(pol->size); m++) {

        // g1^(W_A_s)
        for (unsigned int i=0; i<(dippe->assump_size+1); i++) {
            ECP_BN254_inf(&(g1_W_A_s[i]));
            for (unsigned int k=0; k<dippe->assump_size; k++) {
                ECP_BN254_copy(&tmp_g1, &(pks[m]->g1_W_A[(dippe->assump_size*i)+k]));
                BIG_256_56_from_mpz(tmp_big, s.vec[k]);
                ECP_BN254_mul(&tmp_g1, tmp_big);
                ECP_BN254_add(&(g1_W_A_s[i]), &tmp_g1);
            }
        }

        // g1^(x_UA_s)
        for (unsigned int i=0; i<(dippe->assump_size+1); i++) {
            ECP_BN254_inf(&(g1_x_UA_s[i]));
            for (unsigned int k=0; k<dippe->assump_size; k++) {
                ECP_BN254_copy(&tmp_g1, &(dippe->g1_UA[(dippe->assump_size*i)+k]));
                BIG_256_56_from_mpz(tmp_big, s.vec[k]);
                ECP_BN254_mul(&tmp_g1, tmp_big);
                ECP_BN254_add(&(g1_x_UA_s[i]), &tmp_g1);
            }
            BIG_256_56_from_mpz(tmp_big, pol->vec[m]);
            ECP_BN254_mul(&(g1_x_UA_s[i]), tmp_big);
        }

        // add both matrices
        for (unsigned int i=0; i<(dippe->assump_size+1); i++) {
            ECP_BN254_copy(&(cipher->Ci[(m*(dippe->assump_size+1))+i]), &(g1_x_UA_s[i]));
            ECP_BN254_add(&(cipher->Ci[(m*(dippe->assump_size+1))+i]), &(g1_W_A_s[i]));
        }
    }

    // C_prime
    FP12_BN254_one(&(cipher->C_prime));
    for (unsigned int m=0; m<pol->size; m++) {
        for (unsigned int k=0; k<dippe->assump_size; k++) {
            BIG_256_56_from_mpz(tmp_big, s.vec[k]);
            FP12_BN254_pow(&tmp_gt, &(pks[m]->gt_alpha_A[k]), tmp_big);
            FP12_BN254_mul(&(cipher->C_prime), &tmp_gt);
        }
    }

    // blind message
    FP12_BN254_mul(&(cipher->C_prime), msg);

    // cleanup
    cfe_vec_free(&s);

    return CFE_ERR_NONE;
}

/**
 * Used with an authority's secret key to create a new user secret key for a given attribute vector
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param usk A pointer to a cfe_dippe_user_secret_key struct; Represents the user secret key that is about to be populated
 * @param usk_id Index of the given attribute vector for which a user secret key will be created
 * @param pks An array containing references to cfe_dippe_pub_key structs
 * @param pks_len Length of the public key array
 * @param attrs A reference to a mpz_t struct; Represents the attribute vector
 * @param gid String that represents a unique user; Required for collusion prevention
 * @return Error code
 */
cfe_error cfe_dippe_keygen(cfe_dippe *dippe, cfe_dippe_user_secret_key *usk, unsigned int usk_id, cfe_dippe_pub_key *pks[], unsigned int pks_len, cfe_dippe_sec_key *sk, mpz_t *attrs, char gid[]) {
    // check whether there is the same number of keys as attribute vector components
    if ((mpz_tstbit((*attrs), (pks_len-1)) == 0) || (pks_len <= usk_id)) {
        return CFE_ERR_INSUFFICIENT_KEYS;
    }

    // temp vars
    BIG_256_56 tmp_big;
    ECP2_BN254 tmp_g2;
    ECP2_BN254 hashed;

    // attrs as string
    char *vec_buf = (char*)cfe_malloc((mpz_sizeinbase((*attrs), 62) + 1) * sizeof(char));
    mpz_get_str(vec_buf, 62, (*attrs));

    // char buffer that is about to be hashed into a group element
    int buf_len = (4*MODBYTES_256_56) + strlen(gid) + strlen(vec_buf) + 3;
    char buf[buf_len];
    octet oct;
    oct.val = (char*)&buf;

    // mü
    ECP2_BN254 mue;
    ECP2_BN254_inf(&mue);
    for (unsigned int i=0; i<pks_len; i++) {
        ECP2_BN254_copy(&tmp_g2, &(pks[i]->g2_sigma));
        BIG_256_56_from_mpz(tmp_big, sk->sigma);
        ECP2_BN254_mul(&tmp_g2, tmp_big);

        // hash element
        ECP2_BN254_toOctet(&oct, &tmp_g2);
        sprintf(buf+(4*MODBYTES_256_56), "|%s|%s", gid, vec_buf);
        cfe_dippe_hash_G2(&hashed, buf, buf_len);

        if (i<usk_id) {
            ECP2_BN254_add(&mue, &hashed);
        } else if (i>usk_id) {
            ECP2_BN254_sub(&mue, &hashed);
        }
    }

    char buf2[cfe_dippe_int_len(dippe->assump_size+1) + strlen(gid) + strlen(vec_buf) + 3];
    // g2^h (k+1 x 1)
    ECP2_BN254 g_h[dippe->assump_size+1];
    for (unsigned int i=0; i<(dippe->assump_size+1); i++) {
        sprintf(buf2, "%i|%s|%s", i, gid, vec_buf);
        cfe_dippe_hash_G2(&(g_h[i]), buf2, strlen(buf2));
    }

    // Ki (k+1 x 1)
    usk->Ki = (ECP2_BN254*)malloc(sizeof(ECP2_BN254) * (dippe->assump_size+1));
    for (unsigned int i=0; i<(dippe->assump_size+1); i++) {
        ECP2_BN254_inf(&(usk->Ki[i]));
        // add vi
        if (mpz_tstbit((*attrs), usk_id) == 1) {
            for (unsigned int k=0; k<dippe->assump_size+1; k++) {
                 ECP2_BN254_copy(&tmp_g2, &(g_h[k]));
                 BIG_256_56_from_mpz(tmp_big, sk->W.vec[((dippe->assump_size+1)*i)+k]);
                 ECP2_BN254_mul(&tmp_g2, tmp_big);
                 ECP2_BN254_add(&(usk->Ki[i]), &tmp_g2);
            }
            // negate
            ECP2_BN254_neg(&(usk->Ki[i]));
        }

        // add alpha in the exponent
        BIG_256_56_from_mpz(tmp_big, sk->alpha.vec[i]);
        ECP2_BN254_generator(&tmp_g2);
        ECP2_BN254_mul(&tmp_g2, tmp_big);
        ECP2_BN254_add(&(usk->Ki[i]), &tmp_g2);

        // add mue
        ECP2_BN254_add(&(usk->Ki[i]), &mue);
    }

    // cleanup
    free(vec_buf);

    return CFE_ERR_NONE;
}

/**
 * Restores the underlying message of a given ciphertext
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param result A pointer to a FP12_BN254 struct; Represents the resulting message
 * @param usks An array containing cfe_dippe_user_secret_key structs; Represents the set of user secretes key used for decryption
 * @param usks_len Length of the user secret key array
 * @param cipher A pointer to a cfe_dippe_cipher struct; Represents the ciphertext that is about to be decrypted
 * @param attrs A pointer to a mpz_t struct; Represents the attribute vector
 * @param gid String that represents a unique user; Required for collusion prevention
 */
cfe_error cfe_dippe_decrypt(cfe_dippe *dippe, FP12_BN254 *result, cfe_dippe_user_secret_key *usks, unsigned int usks_len, cfe_dippe_cipher *cipher, mpz_t *attrs, char gid[]) {
    // check whether there is the same number of keys as attribute vector components
    if (mpz_tstbit((*attrs), (usks_len-1)) == 0) {
        return CFE_ERR_INSUFFICIENT_KEYS;
    }

    // temp vars
    ECP_BN254  tmp2_g1;
    ECP2_BN254 tmp_g2;
    FP12_BN254 tmp_gt;

    // C0_K
    FP12_BN254 C0_K;
    FP12_BN254_one(&C0_K);
    for (unsigned int i=0; i<(dippe->assump_size+1); i++) {
        ECP2_BN254_inf(&tmp_g2);
        for (unsigned int j=0; j<usks_len; j++) {
            ECP2_BN254_add(&tmp_g2, &((usks+j)->Ki[i]));
        }
        PAIR_BN254_ate(&tmp_gt, &tmp_g2, &(cipher->C0[i]));
        PAIR_BN254_fexp(&tmp_gt);
        FP12_BN254_mul(&C0_K, &tmp_gt);
    }

    // attrs as string
    char *vec_buf = (char*)cfe_malloc((mpz_sizeinbase((*attrs), 62) + 1) * sizeof(char));
    mpz_get_str(vec_buf, 62, (*attrs));

    // Ci_H
    FP12_BN254 Ci_H;
    FP12_BN254_one(&Ci_H);
    char buf[cfe_dippe_int_len(dippe->assump_size+1) + strlen(gid) + strlen(vec_buf) + 3];
    for (unsigned int i=0; i<(dippe->assump_size+1); i++) {
        ECP_BN254_inf(&tmp2_g1);
        for (unsigned int j=0; j<usks_len; j++) {
            if (mpz_tstbit((*attrs), j) == 1) {
                ECP_BN254_add(&tmp2_g1, &(cipher->Ci[((dippe->assump_size+1)*j)+i]));
            }
        }
        sprintf(buf, "%i|%s|%s", i, gid, vec_buf);
        cfe_dippe_hash_G2(&tmp_g2, buf, strlen(buf));
        PAIR_BN254_ate(&tmp_gt, &tmp_g2, &tmp2_g1);
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

    // cleanup
    free(vec_buf);

    return CFE_ERR_NONE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Helper function that build a conjunction policy vector out of a given pattern
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param pol A pointer to a cfe_vec struct; Represents the resulting policy vector
 * @param pattern String that is used as template for the policy vector
 * @return Error code
 */
cfe_error cfe_dippe_build_conjunction_policy_vector(cfe_dippe *dippe, cfe_vec *pol, const char pattern[]) {
    cfe_vec_init(pol, (strlen(pattern)+1));
    mpz_set_ui(pol->vec[strlen(pattern)], 0);
    for (unsigned int i=0; i<strlen(pattern); i++) {
        if (pattern[i] == '1') {
            cfe_uniform_sample(pol->vec[i], dippe->p);
            mpz_sub(pol->vec[strlen(pattern)], pol->vec[strlen(pattern)], pol->vec[i]);
        } else if (pattern[i] == '0'){
            mpz_set_ui(pol->vec[i], 0);
        }
        else return CFE_ERR_MALFORMED_INPUT;
    }
    mpz_mod(pol->vec[strlen(pattern)], pol->vec[strlen(pattern)], dippe->p);
    return CFE_ERR_NONE;
}

/**
 * Helper function that build a conjunction attribute vector out of a given pattern
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param attrs A pointer to a mpz_t struct; Represents the resulting attribute vector
 * @param pattern String that is used as template for the attribute vector
 * @return Error code
 */
cfe_error cfe_dippe_build_conjunction_attribute_vector(cfe_dippe *dippe, mpz_t *attrs, const char pattern[]) {
    mpz_init((*attrs));
    mpz_set_ui((*attrs), 0);
    for (unsigned int i=0; i<strlen(pattern); i++) {
        if (pattern[i] == '1') {
           mpz_setbit((*attrs), i);
        }
        else if (pattern[i] != '0'){
            return CFE_ERR_MALFORMED_INPUT;
        }
    }
    mpz_setbit((*attrs), strlen(pattern));
    return CFE_ERR_NONE;
}

/**
 * Helper function that build an exact threshold policy vector out of a given pattern
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param pol A pointer to a cfe_vec struct; Represents the resulting policy vector
 * @param pattern String that is used as template for the policy vector
 * @param threshold Threshold value
 * @return Error code
 */
cfe_error cfe_dippe_build_exact_threshold_policy_vector(cfe_dippe *dippe, cfe_vec *pol, const char pattern[], unsigned long threshold) {
    cfe_vec_init(pol, (strlen(pattern)+1));
    mpz_set_ui(pol->vec[strlen(pattern)], threshold);
    for (unsigned int i=0; i<strlen(pattern); i++) {
        if (pattern[i] == '1') {
            mpz_set_ui(pol->vec[i], 1);
        } else if (pattern[i] == '0'){
            mpz_set_ui(pol->vec[i], 0);
        }
        else return CFE_ERR_MALFORMED_INPUT;
    }
    mpz_neg(pol->vec[strlen(pattern)], pol->vec[strlen(pattern)]);
    mpz_mod(pol->vec[strlen(pattern)], pol->vec[strlen(pattern)], dippe->p);
    return CFE_ERR_NONE;
}

/**
 * Helper function that build an exact threshold attribute vector out of a given pattern
 *
 * @param dippe A pointer to a cfe_dippe struct
 * @param attrs A pointer to a cfe_dippe_vec struct; Represents the resulting attribute vector
 * @param pattern String that is used as template for the attribute vector
 * @return Error code
 */
cfe_error cfe_dippe_build_exact_threshold_attribute_vector(cfe_dippe *dippe, mpz_t *attrs, const char pattern[]) {
    mpz_init((*attrs));
    mpz_set_ui((*attrs), 0);
    for (unsigned int i=0; i<strlen(pattern); i++) {
        if (pattern[i] == '1') {
            mpz_setbit((*attrs), i);
        } else if (pattern[i] != '0'){
            return CFE_ERR_MALFORMED_INPUT;
        }
    }
    mpz_setbit((*attrs), strlen(pattern));
    return CFE_ERR_NONE;
}
