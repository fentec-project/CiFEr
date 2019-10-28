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

#include <math.h>

#include "cifer/innerprod/fullysec/lwe_fs.h"
#include "cifer/internal/prime.h"
#include "cifer/sample/normal_double.h"
#include "cifer/sample/uniform.h"

// Initializes scheme struct with the desired confifuration
// and configures public parameters for the scheme.
cfe_error cfe_lwe_fs_init(cfe_lwe_fs *s, size_t l, size_t n, mpz_t bound_x, mpz_t bound_y) {
    cfe_error err = CFE_ERR_NONE;

    s->l = l;
    s->n = n;
    s->A.mat = NULL;
    mpz_init_set(s->bound_x, bound_x);
    mpz_init_set(s->bound_y, bound_y);

    mpz_t bound_for_q_z;
    mpz_inits(s->K, s->q, bound_for_q_z, NULL);
    mpz_mul(s->K, bound_x, bound_y);
    mpz_mul_ui(s->K, s->K, l * 2);

    mpf_t max, sqrt_max, tmp, k_f, k_squared_f, sigma, sigma_prime, bound2, one, bound_for_q;
    mpf_inits(s->siqma_q, s->sigma1, s->sigma2, max, sqrt_max, tmp, k_f, k_squared_f, sigma, sigma_prime, bound2, one,
              bound_for_q, NULL);

    size_t n_bits_q = 1;
    double n_f = (double) n;

    mpf_set_z(k_f, s->K);
    mpf_set_ui(one, 1);
    mpf_mul(k_squared_f, k_f, k_f);

    for (size_t i = 1; true; i++) {
        // assuming that the final q will have at most i bits we calculate a bound
        double bound_m_f = (double) n * i;
        double log2_m = log2(bound_m_f);
        mpf_set_d(tmp, bound_m_f);

        if (mpf_cmp(k_squared_f, tmp) > 0) {
            mpf_set(max, tmp);
        } else {
            mpf_set(max, k_squared_f);
        }

        mpf_set_d(tmp, sqrt(n_f * log2_m));
        mpf_sqrt(sqrt_max, max);

        mpf_mul(s->sigma1, tmp, sqrt_max);
        mpf_ceil(s->sigma1, s->sigma1);

        double n_pow_3 = pow(n_f, 3);
        double pow_sqrt_log_m_5 = pow(sqrt(log2_m), 5);
        double mul_val = sqrt(n_f) * n_pow_3 * pow_sqrt_log_m_5 * sqrt(bound_m_f);
        mpf_set_d(tmp, mul_val);

        mpf_mul(s->sigma2, tmp, max);
        mpf_ceil(s->sigma2, s->sigma2);

        mpf_mul(tmp, s->sigma1, s->sigma1);
        mpf_set(bound2, tmp);
        mpf_mul(tmp, s->sigma2, s->sigma2);
        mpf_add(bound2, bound2, tmp);

        mpf_div(sigma, one, k_squared_f);
        mpf_div(sigma, sigma, bound2);
        mpf_set_d(tmp, log2(n_f));
        mpf_div(sigma, sigma, tmp);

        double n_f_pow_6 = pow(n_f, 6);
        double n_bits_q_pow_2 = pow((double) n_bits_q, 2);
        double sqrt_log_n_f_pow_5 = pow(sqrt(log2(n_f)), 5);
        mpf_set_d(tmp, n_f_pow_6 * n_bits_q_pow_2 * sqrt_log_n_f_pow_5);
        mpf_div(sigma_prime, sigma, k_f);
        mpf_div(sigma_prime, sigma_prime, tmp);

        mpf_set_d(tmp, sqrt(log2(n_f)));
        mpf_div(bound_for_q, tmp, sigma_prime);
        mpz_set_f(bound_for_q_z, bound_for_q);
        n_bits_q = mpz_sizeinbase(bound_for_q_z, 2) + 1;

        if (n_bits_q < i) {
            break;
        }

        i = n_bits_q;
    }

    if (cfe_get_prime(s->q, n_bits_q, false)) {
        cfe_lwe_fs_free(s);
        err = CFE_ERR_PRIME_GEN_FAILED;
        goto cleanup;
    }

    s->m = (size_t) (1.01 * n_f * (double) n_bits_q);

    mpf_set_z(tmp, s->q);
    mpf_mul(s->siqma_q, sigma, tmp);
    mpf_ceil(s->siqma_q, s->siqma_q);

    cfe_mat_init(&s->A, s->m, s->n);
    cfe_uniform_sample_mat(&s->A, s->q);

    cleanup:
    mpf_clears(max, sqrt_max, tmp, k_f, k_squared_f, sigma, sigma_prime, bound2, one, bound_for_q, NULL);
    mpz_clear(bound_for_q_z);

    return err;
}

void cfe_lwe_fs_sec_key_init(cfe_mat *SK, cfe_lwe_fs *s) {
    cfe_mat_init(SK, s->l, s->m);
}

cfe_error cfe_lwe_fs_generate_sec_key(cfe_mat *SK, cfe_lwe_fs *s) {
    mpf_t one;
    mpf_init_set_ui(one, 1);

    cfe_normal_double sampler1, sampler2;

    if (cfe_normal_double_init(&sampler1, s->sigma1, s->n, one)) {
        mpf_clear(one);
        return CFE_ERR_SEC_KEY_GEN_FAILED;
    }

    if (cfe_normal_double_init(&sampler2, s->sigma2, s->n, one)) {
        mpf_clear(one);
        return CFE_ERR_SEC_KEY_GEN_FAILED;
    }

    // sample the matrix
    mpz_t val;
    mpz_init(val);
    size_t half_rows = s->m / 2;

    for (size_t i = 0; i < s->l; ++i) {
        for (size_t j = 0; j < s->m; ++j) {
            if (j < half_rows) {
                cfe_normal_double_sample(val, &sampler1);
            } else {
                cfe_normal_double_sample(val, &sampler2);
                if (j - half_rows == i) {
                    mpz_add_ui(val, val, 1);
                }
            }
            cfe_mat_set(SK, val, i, j);
        }
    }

    mpz_clear(val);
    mpf_clear(one);
    cfe_normal_double_free(&sampler1);
    cfe_normal_double_free(&sampler2);
    return CFE_ERR_NONE;
}

void cfe_lwe_fs_pub_key_init(cfe_mat *PK, cfe_lwe_fs *s) {
    cfe_mat_init(PK, s->l, s->n);
}

cfe_error cfe_lwe_fs_generate_pub_key(cfe_mat *PK, cfe_lwe_fs *s, cfe_mat *SK) {
    if (SK->rows != s->l || SK->cols != s->m) {
        return CFE_ERR_MALFORMED_SEC_KEY;
    }

    cfe_mat_mul(PK, SK, &s->A);
    cfe_mat_mod(PK, PK, s->q);
    return CFE_ERR_NONE;
}

void cfe_lwe_fs_fe_key_init(cfe_vec *z_y, cfe_lwe_fs *s) {
    cfe_vec_init(z_y, s->m);
}

// generates a key (vector) z_y that only decrypts the inner product with y
cfe_error cfe_lwe_fs_derive_fe_key(cfe_vec *z_y, cfe_lwe_fs *s, cfe_vec *y, cfe_mat *SK) {
    if (!cfe_vec_check_bound(y, s->bound_y)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }
    if (SK->rows != s->l || SK->cols != s->m) {
        return CFE_ERR_MALFORMED_SEC_KEY;
    }

    cfe_vec_mul_matrix(z_y, y, SK);
    cfe_vec_mod(z_y, z_y, s->q);
    return CFE_ERR_NONE;
}

void cfe_lwe_fs_ciphertext_init(cfe_vec *ct, cfe_lwe_fs *s) {
    cfe_vec_init(ct, s->m + s->l);
}

// Encrypts vector x using a public key.
cfe_error cfe_lwe_fs_encrypt(cfe_vec *ct, cfe_lwe_fs *s, cfe_vec *x, cfe_mat *PK) {
    if (!cfe_vec_check_bound(x, s->bound_x)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }
    if (x->size != s->l) {
        return CFE_ERR_MALFORMED_INPUT;
    }
    if (PK->rows != s->l || PK->cols != s->n) {
        return CFE_ERR_MALFORMED_PUB_KEY;
    }

    mpf_t one;
    mpf_init_set_ui(one, 1);
    cfe_normal_double sampler;
    if (cfe_normal_double_init(&sampler, s->siqma_q, s->n, one)) {
        mpf_clear(one);
        return CFE_ERR_PARAM_GEN_FAILED;
    }

    // create a random vector r
    cfe_vec r, t, e0, e1, c0, c1;
    cfe_vec_init(&r, s->n);
    cfe_uniform_sample_vec(&r, s->q);

    cfe_vec_init(&e0, s->m);
    cfe_normal_double_sample_vec(&e0, &sampler);

    cfe_vec_init(&e1, s->l);
    cfe_normal_double_sample_vec(&e1, &sampler);

    // calculate first part of the cipher
    cfe_vec_init(&c0, s->m);
    cfe_mat_mul_vec(&c0, &s->A, &r);
    cfe_vec_add(&c0, &c0, &e0);
    cfe_vec_mod(&c0, &c0, s->q);

    // calculate second part of the cipher
    mpz_t q_div_k;
    mpz_init(q_div_k);
    mpz_fdiv_q(q_div_k, s->q, s->K);

    cfe_vec_init(&t, s->l);
    cfe_vec_mul_scalar(&t, x, q_div_k);

    cfe_vec_init(&c1, s->l);
    cfe_mat_mul_vec(&c1, PK, &r);
    cfe_vec_add(&c1, &c1, &e1);
    cfe_vec_add(&c1, &c1, &t);
    cfe_vec_mod(&c1, &c1, s->q);

    cfe_vec_join(ct, &c0, &c1);

    mpz_clear(q_div_k);
    mpf_clear(one);
    cfe_vec_frees(&r, &t, &e0, &e1, &c0, &c1, NULL);
    cfe_normal_double_free(&sampler);
    return CFE_ERR_NONE;
}

// Decrypts a the inner product of the message times a vector out of
// the encryption, using a key generated for this. Saves it to res.
cfe_error cfe_lwe_fs_decrypt(mpz_t res, cfe_lwe_fs *s, cfe_vec *ct, cfe_vec *z_y, cfe_vec *y) {
    if (!cfe_vec_check_bound(y, s->bound_y)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }
    if (z_y->size != s->m) {
        return CFE_ERR_MALFORMED_FE_KEY;
    }
    if (y->size != s->l) {
        return CFE_ERR_MALFORMED_INPUT;
    }
    if (ct->size != s->m + s->l) {
        return CFE_ERR_MALFORMED_CIPHER;
    }

    cfe_vec c0, c1;
    cfe_vec_init(&c0, s->m);
    cfe_vec_init(&c1, s->l);
    cfe_vec_extract(&c0, ct, 0, s->m);
    cfe_vec_extract(&c1, ct, s->m, s->l);

    mpz_t y_dot_c1, z_y_dot_c0, mu1, k_times_2, q_div_k_times_2, q_div_k, half_q;
    mpz_inits(y_dot_c1, z_y_dot_c0, mu1, k_times_2, q_div_k_times_2, q_div_k, half_q, NULL);

    cfe_vec_dot(y_dot_c1, y, &c1);
    cfe_vec_dot(z_y_dot_c0, z_y, &c0);

    mpz_sub(mu1, y_dot_c1, z_y_dot_c0);
    mpz_mod(mu1, mu1, s->q);
    mpz_fdiv_q_ui(half_q, s->q, 2);

    if (mpz_cmp(mu1, half_q) > 0) {
        mpz_sub(mu1, mu1, s->q);
    }

    mpz_mul_ui(k_times_2, s->K, 2);
    mpz_div(q_div_k, s->q, s->K);
    mpz_div(q_div_k_times_2, s->q, k_times_2);

    mpz_add(res, mu1, q_div_k_times_2);
    mpz_div(res, res, q_div_k);

    mpz_clears(y_dot_c1, z_y_dot_c0, mu1, k_times_2, q_div_k_times_2, q_div_k, half_q, NULL);
    cfe_vec_frees(&c0, &c1, NULL);
    return CFE_ERR_NONE;
}

// Frees the memory allocated for configuration of the scheme.
void cfe_lwe_fs_free(cfe_lwe_fs *s) {
    mpz_clears(s->bound_x, s->bound_y, s->K, s->q, NULL);
    mpf_clears(s->siqma_q, s->sigma1, s->sigma2, NULL);

    if (s->A.mat != NULL) {
        cfe_mat_free(&s->A);
    }
}
