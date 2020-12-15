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

#ifndef CIFER_DATA_SER_H
#define CIFER_DATA_SER_H

#include <cifer/data/mat.h>
#include <cifer/data/vec_curve.h>
#include <cifer/abe/policy.h>
#include <cifer/abe/gpsw.h>
#include "data.pb-c.h"

typedef struct cfe_ser {
    uint8_t *ser;
    size_t len;
} cfe_ser;

void cfe_ser_free(cfe_ser *s);

void cfe_mpz_ser_init(MpzSer *o);

void cfe_mpz_pack(mpz_t a, MpzSer *msg);

void cfe_mpz_ser(mpz_t aa, cfe_ser *buf);

void cfe_mpz_unpack(mpz_t a, MpzSer *msg);

cfe_error cfe_mpz_read(mpz_t a, cfe_ser *buf);

void cfe_mat_pack(cfe_mat *a, MatSer *msg, MpzSer *val);

void cfe_mat_ser(cfe_mat *a, cfe_ser *buf);

void cfe_mat_unpack(cfe_mat *a, MatSer *msg);

cfe_error cfe_mat_read(cfe_mat *a, cfe_ser *buf);

void cfe_octet_ser_init(OctetSer *o);

void cfe_ECP_BN254_pack(ECP_BN254 *a, OctetSer *msg);

void cfe_ECP_BN254_ser(ECP_BN254 *a, cfe_ser *buf);

void cfe_ECP_BN254_unpack(ECP_BN254 *a, OctetSer *msg);

cfe_error cfe_ECP_BN254_read(ECP_BN254 *a, cfe_ser *buf);

void cfe_ECP2_BN254_pack(ECP2_BN254 *a, OctetSer *msg);

void cfe_ECP2_BN254_ser(ECP2_BN254 *a, cfe_ser *buf);

void cfe_ECP2_BN254_unpack(ECP2_BN254 *a, OctetSer *msg);

cfe_error cfe_ECP2_BN254_read(ECP2_BN254 *a, cfe_ser *buf);

void cfe_FP12_BN254_pack(FP12_BN254 *a, OctetSer *msg);

void cfe_FP12_BN254_ser(FP12_BN254 *a, cfe_ser *buf);

void cfe_FP12_BN254_unpack(FP12_BN254 *a, OctetSer *msg);

cfe_error cfe_FP12_BN254_read(FP12_BN254 *a, cfe_ser *buf);

void cfe_msp_pack(cfe_msp *a, MspSer *msg, MpzSer *val);

void cfe_msp_ser(cfe_msp *a, cfe_ser *buf);

void cfe_msp_unpack(cfe_msp *a, MspSer *msg);

cfe_error cfe_msp_read(cfe_msp *a, cfe_ser *buf);

void cfe_vec_octet_pack(cfe_vec_octet *a, VecOctetSer *msg, OctetSer *val);

void cfe_vec_octet_ser(cfe_vec_octet *a, cfe_ser *buf);

void cfe_vec_octet_unpack(cfe_vec_octet *a, VecOctetSer *msg);

cfe_error cfe_vec_octet_read(cfe_vec_octet *a, cfe_ser *buf);

void cfe_vec_ECP2_BN254_ser(cfe_vec_G2 *a, cfe_ser *buf);

cfe_error cfe_vec_ECP2_BN254_read(cfe_vec_G2 *a, cfe_ser *buf);

void cfe_gpsw_pub_key_ser(cfe_gpsw_pub_key *a, cfe_ser *buf);

cfe_error cfe_gpsw_pub_key_read(cfe_gpsw_pub_key *a, cfe_ser *buf);

#endif
