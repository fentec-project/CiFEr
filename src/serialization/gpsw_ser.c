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


#include <stdio.h>
#include <stdlib.h>
#include <cifer/abe/gpsw.h>
#include "cifer/internal/common.h"
#include <cifer/serialization/gpsw_ser.h>
#include <cifer/serialization/gpsw.pb-c.h>

void cfe_gpsw_pub_key_pack(cfe_gpsw_pub_key *a, CfeGpswPubKeySer *msg, OctetSer *octets_val) {
    cfe_FP12_BN254_pack(&(a->y), msg->y);

    cfe_vec_octet octets;
    cfe_vec_octet_init(&octets, a->t.size);
    for (size_t i =0; i<a->t.size; i++) {
        char *h1 = (char *) cfe_malloc((4 * MODBYTES_256_56) * sizeof(char));
        octet oct = {0, (4 * MODBYTES_256_56) * sizeof(char), h1};
        ECP2_BN254_toOctet(&oct, &(a->t.vec[i]));
        octets.vec[i] = oct;
    }
    cfe_vec_octet_pack(&octets, msg->t, octets_val);

    for (size_t i =0; i<a->t.size; i++) {
        free(octets.vec[i].val);
    }
    free(octets.vec);
}

void cfe_gpsw_pub_key_ser(cfe_gpsw_pub_key *a, cfe_ser *buf) {
    CfeGpswPubKeySer msg = CFE_GPSW_PUB_KEY_SER__INIT;
    VecOctetSer t = VEC_OCTET_SER__INIT;
    msg.t = &t;
    OctetSer y = OCTET_SER__INIT;
    msg.y = &y;
    OctetSer *octets_val = cfe_malloc(sizeof(OctetSer) * a->t.size);

    cfe_gpsw_pub_key_pack(a, &msg, octets_val);

    buf->len = cfe_gpsw_pub_key_ser__get_packed_size(&msg);
    buf->ser = cfe_malloc(buf->len);

    cfe_gpsw_pub_key_ser__pack(&msg, buf->ser);

    for (size_t i =0; i<a->t.size; i++) {
        free(msg.t->vec[i]->val);
    }
    free(msg.t->vec);
    free(octets_val);
}

void cfe_gpsw_pub_key_unpack(cfe_gpsw_pub_key *a, CfeGpswPubKeySer *msg) {
    cfe_FP12_BN254_unpack(&(a->y), msg->y);

    cfe_vec_octet octets;
    cfe_vec_octet_unpack(&octets, msg->t);
    cfe_vec_G2_init(&(a->t), octets.size);
    for (size_t i =0; i< octets.size; i++) {
        ECP2_BN254_fromOctet(&(a->t.vec[i]), &(octets.vec[i]));
        free(octets.vec[i].val);
    }
    cfe_vec_octet_free(&octets);
}

cfe_error cfe_gpsw_pub_key_read(cfe_gpsw_pub_key *a, cfe_ser *buf) {
    CfeGpswPubKeySer *msg;
    msg = cfe_gpsw_pub_key_ser__unpack(NULL, buf->len, buf->ser);
    if (msg == NULL) {
        return 1;
    }

    cfe_gpsw_pub_key_unpack(a, msg);
    // Free the unpacked message
    cfe_gpsw_pub_key_ser__free_unpacked(msg, NULL);
    return 0;
}

void cfe_gpsw_key_pack(cfe_gpsw_key *a, CfeGpswKeySer *msg, MpzSer *val, OctetSer *octets_val) {
    cfe_msp_pack(&(a->msp), msg->msp, val);

    cfe_vec_octet octets;
    cfe_vec_octet_init(&octets, a->d.size);
    for (size_t i =0; i<a->d.size; i++) {
        char *h1 = (char *) cfe_malloc(MODBYTES_256_56 * sizeof(char));
        octet oct = {0, MODBYTES_256_56 * sizeof(char), h1};
        ECP_BN254_toOctet(&oct, &(a->d.vec[i]), true);
        octets.vec[i] = oct;
    }
    cfe_vec_octet_pack(&octets, msg->d, octets_val);

    for (size_t i =0; i<a->d.size; i++) {
        free(octets.vec[i].val);
    }
    free(octets.vec);
}

void cfe_gpsw_key_ser(cfe_gpsw_key *a, cfe_ser *buf) {
    CfeGpswKeySer msg = CFE_GPSW_KEY_SER__INIT;

    MspSer msp = MSP_SER__INIT;
    MatSer mat = MAT_SER__INIT;
    msp.mat = &mat;

    MpzSer *val = cfe_malloc(sizeof(MpzSer) * a->msp.mat.cols * a->msp.mat.rows);
    cfe_msp_pack(&(a->msp), &msp, val);
    msg.msp = &msp;

    VecOctetSer d = VEC_OCTET_SER__INIT;
    msg.d = &d;
    OctetSer *octets_val = cfe_malloc(sizeof(OctetSer) * a->d.size);

    cfe_gpsw_key_pack(a, &msg, val, octets_val);

    buf->len = cfe_gpsw_key_ser__get_packed_size(&msg);
    buf->ser = cfe_malloc(buf->len);

    cfe_gpsw_key_ser__pack(&msg, buf->ser);

    for (size_t i =0; i<a->msp.mat.rows; i++) {
        for (size_t j =0; j < a->msp.mat.cols; j++) {
            free(msg.msp->mat->val[i * a->msp.mat.cols + j]->val);
        }
    }
    free(msg.msp->mat->val);
    free(msg.msp->row_to_attrib);
    for (size_t i =0; i<a->d.size; i++) {
        free(msg.d->vec[i]->val);
    }
    free(msg.d->vec);
    free(val);
    free(octets_val);
}

void cfe_gpsw_key_unpack(cfe_gpsw_key *a, CfeGpswKeySer *msg) {
    cfe_msp_unpack(&(a->msp), msg->msp);

    cfe_vec_octet octets;
    cfe_vec_octet_unpack(&octets, msg->d);
    cfe_vec_G1_init(&(a->d), octets.size);
    for (size_t i =0; i< octets.size; i++) {
        ECP_BN254_fromOctet(&(a->d.vec[i]), &(octets.vec[i]));
        free(octets.vec[i].val);
    }
    cfe_vec_octet_free(&octets);
}

cfe_error cfe_gpsw_key_read(cfe_gpsw_key *a, cfe_ser *buf) {
    CfeGpswKeySer *msg;
    msg = cfe_gpsw_key_ser__unpack(NULL, buf->len, buf->ser);
    if (msg == NULL) {
        return 1;
    }

    cfe_gpsw_key_unpack(a, msg);

    // Free the unpacked message
    cfe_gpsw_key_ser__free_unpacked(msg, NULL);

    return 0;
}

void cfe_gpsw_cipher_pack(cfe_gpsw_cipher *a, CfeGpswCipherSer *msg, OctetSer *octets_val) {
    cfe_FP12_BN254_pack(&(a->e0), msg->e0);

    msg->n_gamma = a->e.size;
    msg->gamma = cfe_malloc(sizeof(int64_t) * a->e.size);
    for (size_t j =0; j < a->e.size; j++) {
        msg->gamma[j] = (int64_t)a->gamma[j];
    }

    cfe_vec_octet octets;
    cfe_vec_octet_init(&octets, a->e.size);
    for (size_t i =0; i<a->e.size; i++) {
        char *h1 = (char *) cfe_malloc((4 * MODBYTES_256_56) * sizeof(char));
        octet oct = {0, (4 * MODBYTES_256_56) * sizeof(char), h1};
        ECP2_BN254_toOctet(&oct, &(a->e.vec[i]));
        octets.vec[i] = oct;
    }
    cfe_vec_octet_pack(&octets, msg->e, octets_val);

    for (size_t i =0; i<a->e.size; i++) {
        free(octets.vec[i].val);
    }
    free(octets.vec);
}

void cfe_gpsw_cipher_ser(cfe_gpsw_cipher *a, cfe_ser *buf) {
    CfeGpswCipherSer msg = CFE_GPSW_CIPHER_SER__INIT;
    VecOctetSer e = VEC_OCTET_SER__INIT;
    msg.e = &e;
    OctetSer e0 = OCTET_SER__INIT;
    msg.e0 = &e0;
    OctetSer *octets_val = cfe_malloc(sizeof(OctetSer) * a->e.size);

    cfe_gpsw_cipher_pack(a, &msg, octets_val);

    buf->len = cfe_gpsw_cipher_ser__get_packed_size(&msg);
    buf->ser = cfe_malloc(buf->len);

    cfe_gpsw_cipher_ser__pack(&msg, buf->ser);

    for (size_t i =0; i<a->e.size; i++) {
        free(msg.e->vec[i]->val);
    }
    free(msg.e->vec);
    free(octets_val);
}

void cfe_gpsw_cipher_unpack(cfe_gpsw_cipher *a, CfeGpswCipherSer *msg) {
    cfe_FP12_BN254_unpack(&(a->e0), msg->e0);

    cfe_vec_octet octets;
    cfe_vec_octet_unpack(&octets, msg->e);
    cfe_vec_G2_init(&(a->e), octets.size);
    for (size_t i =0; i< octets.size; i++) {
        ECP2_BN254_fromOctet(&(a->e.vec[i]), &(octets.vec[i]));
        free(octets.vec[i].val);
    }
    cfe_vec_octet_free(&octets);

    a->gamma = cfe_malloc(sizeof(int) * a->e.size);
    for (size_t j =0; j < msg->n_gamma; j++) {
        a->gamma[j] = (int)msg->gamma[j];
    }
}

cfe_error cfe_gpsw_cipher_read(cfe_gpsw_cipher *a, cfe_ser *buf) {
    CfeGpswCipherSer *msg;
    msg = cfe_gpsw_cipher_ser__unpack(NULL, buf->len, buf->ser);
    if (msg == NULL) {
        return 1;
    }

    cfe_gpsw_cipher_unpack(a, msg);
    // Free the unpacked message
    cfe_gpsw_cipher_ser__free_unpacked(msg, NULL);

    return 0;
}
