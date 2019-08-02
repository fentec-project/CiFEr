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


#ifndef CIFER_DMCFE_H
#define CIFER_DMCFE_H

#include <amcl/big_256_56.h>
#include <amcl/fp12_BN254.h>
#include <amcl/ecp_BN254.h>
#include <amcl/ecp2_BN254.h>
#include "cifer/data/mat.h"

#include <cifer/data/vec_curve.h>

/**
 * \file
 * \ingroup fullysec
 * \brief Damgard multi input scheme.
 */

/**
 * cfe_dmcfe_multi_client represents a client in a decentralized multi-client scheme
 * scheme.
 */
typedef struct cfe_dmcfe_client {
    size_t idx;
    BIG_256_56 client_sec_key_big;
    ECP_BN254 client_pub_key;
    cfe_mat share;
    cfe_vec s;
    mpz_t order;
    BIG_256_56 order_big;
} cfe_dmcfe_client;

void cfe_dmcfe_client_init(cfe_dmcfe_client *c, size_t idx);

void cfe_dmcfe_client_free(cfe_dmcfe_client *c);

void cfe_dmcfe_set_share(cfe_dmcfe_client *c, ECP_BN254 *pub_t, size_t num_clients);

void cfe_dmcfe_encrypt(ECP_BN254 *cipher, mpz_t x, char *label, cfe_dmcfe_client *c);

void cfe_dmcfe_generate_key_share(cfe_vec_G2 *key_share, cfe_vec *y, cfe_dmcfe_client *c);

cfe_error cfe_dmcfe_decrypt(mpz_t res, ECP_BN254 *ciphers, cfe_vec_G2 *key_shares,
                            char *label, cfe_vec *y, mpz_t bound, size_t num_clients);

#endif
