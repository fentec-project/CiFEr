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

#include <string.h>
#include <sodium.h>
#include <amcl/pbc_support.h>

#include "cifer/innerprod/fullysec/damgard_dec_multi.h"
#include "cifer/sample/uniform.h"
#include "cifer/internal/common.h"

void cfe_damgard_dec_multi_client_init(cfe_damgard_dec_multi_client *c,
                                       cfe_damgard_multi *damgard_multi, size_t idx) {
    mpz_inits(c->client_sec_key, c->client_pub_key, NULL);

    cfe_damgard_multi_copy(&c->scheme, damgard_multi);
    c->idx = idx;

    cfe_uniform_sample(c->client_sec_key, c->scheme.scheme.q);
    mpz_powm(c->client_pub_key, c->scheme.scheme.g, c->client_sec_key,
             c->scheme.scheme.p);

    cfe_mat_init(&(c->share), c->scheme.num_clients, c->scheme.scheme.l);
}

void cfe_damgard_dec_multi_client_free(cfe_damgard_dec_multi_client *c) {
    mpz_clears(c->client_sec_key, c->client_pub_key, NULL);
    cfe_mat_free(&(c->share));
    cfe_damgard_multi_free(&c->scheme);
}

void cfe_damgard_dec_multi_client_set_share(cfe_damgard_dec_multi_client *c,
                                            mpz_t *pub_keys) {
    cfe_mat add;
    cfe_mat_init(&add, c->scheme.num_clients, c->scheme.scheme.l);
    mpz_t shared_num;
    mpz_init(shared_num);
    char h[randombytes_SEEDBYTES];
    octet tmp_hash = {0, sizeof(h), h};
    for (size_t k = 0; k < c->scheme.num_clients; k++) {
        if (k == c->idx) {
            continue;
        }

        mpz_powm(shared_num, pub_keys[k], c->client_sec_key,
                 c->scheme.scheme.p);

        int num_size = (int) mpz_sizeinbase(shared_num, 32);
        char *num_str = (char *) cfe_malloc((num_size + 1) * sizeof(char));
        mpz_get_str(num_str, 32, shared_num);
        octet tmp_oct = {num_size, num_size, num_str};
        mhashit(SHA256, -1, &tmp_oct, &tmp_hash);
        free(num_str);

        cfe_uniform_sample_mat_det(&add, c->scheme.scheme.q,
                                   (unsigned char *) tmp_hash.val);

        if (k > c->idx) {
            cfe_mat_neg(&add, &add);
        }
        cfe_mat_add(&(c->share), &(c->share), &add);
        cfe_mat_mod(&(c->share), &(c->share), c->scheme.scheme.q);
    }

    cfe_mat_free(&add);
    mpz_clear(shared_num);
}

void cfe_damgard_dec_multi_sec_key_init(cfe_damgard_dec_multi_sec_key *sec_key,
                                        cfe_damgard_dec_multi_client *c) {
    cfe_damgard_sec_key_init(&(sec_key->dam_sec_key), &(c->scheme.scheme));
    cfe_damgard_pub_key_init(&(sec_key->dam_pub_key), &(c->scheme.scheme));
    cfe_vec_init(&(sec_key->otp_key), c->scheme.scheme.l);
}

void cfe_damgard_dec_multi_sec_key_free(cfe_damgard_dec_multi_sec_key *sec_key) {
    cfe_damgard_sec_key_free(&(sec_key->dam_sec_key));
    cfe_vec_frees(&(sec_key->dam_pub_key), &(sec_key->otp_key), NULL);
}

void cfe_damgard_dec_multi_generate_keys(cfe_damgard_dec_multi_sec_key *sec_key,
                                         cfe_damgard_dec_multi_client *c) {
    cfe_damgard_generate_master_keys(&(sec_key->dam_sec_key), &(sec_key->dam_pub_key),
                                     &(c->scheme.scheme));
    cfe_uniform_sample_vec(&(sec_key->otp_key), c->scheme.scheme.q);
}

void cfe_damgard_dec_multi_ciphertext_init(cfe_vec *ciphertext,
                                           cfe_damgard_dec_multi_client *c) {
    cfe_damgard_ciphertext_init(ciphertext, &(c->scheme.scheme));
}

cfe_error cfe_damgard_dec_multi_encrypt(cfe_vec *cipher, cfe_vec *x,
                                        cfe_damgard_dec_multi_sec_key *sec_key,
                                        cfe_damgard_dec_multi_client *c) {
    if (!cfe_vec_check_bound(x, c->scheme.bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    cfe_vec x_add_otp;
    cfe_vec_init(&x_add_otp, x->size);
    cfe_vec_add(&x_add_otp, x, &(sec_key->otp_key));
    cfe_vec_mod(&x_add_otp, &x_add_otp, c->scheme.scheme.q);

    cfe_error err = cfe_damgard_encrypt(cipher, &(c->scheme.scheme), &x_add_otp,
                                        &(sec_key->dam_pub_key));
    cfe_vec_free(&x_add_otp);

    return err;
}

void cfe_damgard_dec_multi_fe_key_share_init(cfe_damgard_dec_multi_fe_key_part *fe_key_share) {
    cfe_damgard_fe_key_init(&(fe_key_share->key_part));
    mpz_init(fe_key_share->otp_key_part);
}

void cfe_damgard_dec_multi_fe_key_free(cfe_damgard_dec_multi_fe_key_part *fe_key_part) {
    cfe_damgard_fe_key_free(&(fe_key_part->key_part));
    mpz_clear(fe_key_part->otp_key_part);
}

cfe_error cfe_damgard_dec_multi_client_derive_fe_key_part(cfe_damgard_dec_multi_fe_key_part *fe_key_part,
                                                          cfe_mat *y, cfe_damgard_dec_multi_sec_key *sec_key,
                                                          cfe_damgard_dec_multi_client *c) {
    if (!cfe_mat_check_bound(y, c->scheme.bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    cfe_vec y_part;
    cfe_vec_init(&y_part, y->cols);
    cfe_mat_get_row(&y_part, y, c->idx);

    mpz_t z_1, z_2;
    mpz_inits(z_1, z_2, NULL);

    cfe_vec_dot(z_1, &(sec_key->otp_key), &y_part);
    cfe_mat_dot(z_2, &(c->share), y);

    mpz_add(fe_key_part->otp_key_part, z_1, z_2);
    mpz_mod(fe_key_part->otp_key_part, fe_key_part->otp_key_part,
            c->scheme.scheme.q);

    cfe_error err = cfe_damgard_derive_fe_key(&(fe_key_part->key_part),
                                              &(c->scheme.scheme), &(sec_key->dam_sec_key), &y_part);

    mpz_clears(z_1, z_2, NULL);
    cfe_vec_free(&y_part);

    return err;
}

void cfe_damgard_dec_multi_dec_init(cfe_damgard_dec_multi_dec *d,
                                    cfe_damgard_multi *damgard_multi) {
    cfe_damgard_multi_copy(&d->scheme, damgard_multi);
}

void cfe_damgard_dec_multi_dec_free(cfe_damgard_dec_multi_dec *d) {
    cfe_damgard_multi_free(&d->scheme);
}

cfe_error cfe_damgard_dec_multi_decrypt(mpz_t res, cfe_vec *ciphers,
                                        cfe_damgard_dec_multi_fe_key_part *fe_key_parts,
                                        cfe_mat *y, cfe_damgard_dec_multi_dec *d) {
    if (!cfe_mat_check_bound(y, d->scheme.bound)) {
        return CFE_ERR_BOUND_CHECK_FAILED;
    }

    cfe_damgard_multi_fe_key key;
    key.num_clients = d->scheme.num_clients;
    mpz_init(key.z);
    key.keys = (cfe_damgard_fe_key *) cfe_malloc(sizeof(cfe_damgard_fe_key) * d->scheme.num_clients);

    mpz_set_ui(key.z, 0);

    for (size_t i = 0; i < d->scheme.num_clients; i++) {
        mpz_add(key.z, key.z, fe_key_parts[i].otp_key_part);
        key.keys[i] = fe_key_parts[i].key_part;

    }
    mpz_mod(key.z, key.z, d->scheme.scheme.q);

    cfe_error err = cfe_damgard_multi_decrypt(res, &(d->scheme), ciphers, &key, y);

    mpz_clear(key.z);
    free(key.keys);

    return err;
}
