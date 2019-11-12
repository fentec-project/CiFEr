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


#ifndef CIFER_FHIPE_H
#define CIFER_FHIPE_H

#include <gmp.h>
#include <amcl/ecp_BN254.h>
#include <cifer/data/mat.h>
#include <amcl/ecp2_BN254.h>
#include <cifer/data/vec_curve.h>


/**
 * \file
 * \ingroup fullysec
 * \brief Decentralized multi-client inner-product scheme based on the paper
 * "Decentralized Multi-Client Functional Encryption for Inner Product" by
 * Chotard, Dufour Sans, Gay, Phan, and Pointcheval.
 */

/**
 * cfe_dmcfe_client represents a client in a decentralized multi-client scheme.
 */
typedef struct cfe_fhipe {
    size_t l;
    mpz_t bound_x;
    mpz_t bound_y;
    BIG_256_56 order_big;
    mpz_t order;
} cfe_fhipe;

/**
 * Configures a new client for the dmcfe scheme.
 *
 * @param c A pointer to an uninitialized struct representing the scheme
 * @param idx Identification value of the client; it is assumed that if there are
 * n clients, their identifications are from [0,n)
 */
void cfe_fhipe_init(cfe_fhipe *c, size_t l, mpz_t bound_x, mpz_t bound_y);

/**
 * cfe_dmcfe_client represents a client in a decentralized multi-client scheme.
 */
typedef struct cfe_fhipe_sec_key {
    ECP_BN254 g1;
    ECP2_BN254 g2;
    cfe_mat B;
    cfe_mat B_star;
} cfe_fhipe_sec_key;

void cfe_fhipe_master_key_init(cfe_fhipe_sec_key *sec_key, cfe_fhipe *c);


void cfe_fhipe_master_key_free(cfe_fhipe_sec_key *sec_key);

cfe_error cfe_fhipe_generate_master_key(cfe_fhipe_sec_key *sec_key, cfe_fhipe *c);

/**
 * cfe_dmcfe_client represents a client in a decentralized multi-client scheme.
 */
typedef struct cfe_fhipe_FE_key {
    ECP_BN254 k1;
    cfe_vec_G1 k2;
} cfe_fhipe_FE_key;

void cfe_fhipe_FE_key_init(cfe_fhipe_FE_key *fe_key, cfe_fhipe *c);

void cfe_fhipe_derive_FE_key(cfe_fhipe_FE_key *fe_key, cfe_vec *y, cfe_fhipe_sec_key *sec_key, cfe_fhipe *c);


#endif
