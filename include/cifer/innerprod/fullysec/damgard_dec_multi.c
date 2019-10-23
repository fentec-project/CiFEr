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

#ifndef CIFER_DAMGARD_DEC_MULTI_H
#define CIFER_DAMGARD_DEC_MULTI_H

#include "cifer/data/vec.h"
#include "cifer/internal/errors.h"
#include "cifer/innerprod/fullysec/damgard_multi.h"



/**
 * \file
 * \ingroup fullysec
 * \brief Damgard style decentralized multi input scheme.
 */

/**
 * cfe_damgard_multi represents a multi input variant of the underlying Damgard
 * scheme.
 */
typedef struct cfe_damgard_dec_multi_client {
    size_t idx;
    cfe_damgard_multi scheme;
    mpz_t client_pub_key;
    mpz_t client_sec_key;
    cfe_mat share;
} cfe_damgard_dec_multi_client;

/**
 * cfe_damgard_multi represents a multi input variant of the underlying Damgard
 * scheme.
 */
typedef struct cfe_damgard_dec_multi_dec {
    cfe_damgard_multi scheme;
} cfe_damgard_dec_multi_dec;

/**
 * cfe_damgard_multi represents a multi input variant of the underlying Damgard
 * scheme.
 */
typedef struct cfe_damgard_dec_multi_sec_key {
    cfe_damgard_sec_key dam_sec_key;
    cfe_vec dam_pub_key;
    cfe_vec otp_key;
} cfe_damgard_dec_multi_sec_key;

/**
 * cfe_damgard_multi represents a multi input variant of the underlying Damgard
 * scheme.
 */
typedef struct cfe_damgard_dec_multi_derived_key_part {
    cfe_damgard_fe_key key_part;
    mpz_t otp_key_part;
} cfe_damgard_dec_multi_derived_key_part;

void cfe_damgard_dec_multi_client_init(cfe_damgard_dec_multi_client *c, cfe_damgard_multi *damgard_multi, size_t idx);

void cfe_damgard_dec_multi_client_set_share(cfe_damgard_dec_multi_client *c, mpz_t *pub_keys);

void cfe_damgard_dec_multi_sec_key_init(cfe_damgard_dec_multi_sec_key *sec_key, cfe_damgard_dec_multi_client *c);

void cfe_damgard_dec_multi_generate_keys(cfe_damgard_dec_multi_sec_key *sec_key, cfe_damgard_dec_multi_client *c);

void cfe_damgard_dec_multi_ciphertext_init(cfe_vec *ciphertext, cfe_damgard_dec_multi_client *c);

void cfe_damgard_dec_multi_encrypt(cfe_vec *cipher, cfe_vec *x, cfe_damgard_dec_multi_sec_key *sec_key, cfe_damgard_dec_multi_client *c);

void cfe_damgard_dec_multi_derived_key_init(cfe_damgard_dec_multi_derived_key_part *derived_key_share);

void cfe_damgard_dec_multi_derive_key_share(cfe_damgard_dec_multi_derived_key_part *derived_key_share,
                                cfe_mat *y, cfe_damgard_dec_multi_sec_key *sec_key, cfe_damgard_dec_multi_client *c);

void cfe_damgard_dec_multi_dec_init(cfe_damgard_dec_multi_dec *d, cfe_damgard_multi *damgard_multi);

cfe_error cfe_damgard_dec_multi_decrypt(mpz_t res, cfe_vec *cipher,
                                   cfe_damgard_dec_multi_derived_key_part *part_key, cfe_mat *y, cfe_damgard_dec_multi_dec *d);

#endif
