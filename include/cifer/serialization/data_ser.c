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
#include <cifer/serialization/data_ser.h>
#include <cifer/serialization/data.pb-c.h>
#include <cifer/internal/common.h>
#include <cifer/abe/policy.h>

void cfe_mpz_ser_init(MpzSer *o) {
    o->base.descriptor = &mpz_ser__descriptor;
    o->base.n_unknown_fields = 0;
    o->base.unknown_fields = NULL;
    o->val = NULL;
    o->n_val = 0;
}

void cfe_mpz_pack(mpz_t a, MpzSer *msg) {
    size_t numb = 8 * sizeof(int64_t);
    size_t count = (mpz_sizeinbase(a, 2) + numb-1) / numb;
    int64_t *out = cfe_malloc(count * sizeof(int64_t));
    mpz_export(out, &count, 1, sizeof(int64_t), 0, 0, a);

    msg->n_val = count;
    msg->val = out;
}

void cfe_mpz_ser(mpz_t a, cfe_ser *buf) {
    MpzSer msg;
    cfe_mpz_ser_init(&msg);
    cfe_mpz_pack(a, &msg);
    buf->len = mpz_ser__get_packed_size(&msg);
    buf->ser = cfe_malloc(buf->len);

    mpz_ser__pack(&msg, buf->ser);
}

void cfe_mpz_unpack(mpz_t a, MpzSer *msg) {
    mpz_import(a, msg->n_val, 1, sizeof(int64_t), 0, 0, msg->val);
}

cfe_error cfe_mpz_read(mpz_t a, cfe_ser *buf) {
    MpzSer *msg;
    msg = mpz_ser__unpack(NULL, buf->len, buf->ser);
    if (msg == NULL)
    {
        return 1;
    }

    cfe_mpz_unpack(a, msg);
    // Free the unpacked message
    mpz_ser__free_unpacked(msg, NULL);

    return 0;
}

void cfe_mat_pack(cfe_mat *a, MatSer *msg) {
    msg->n_val = a->cols * a->rows;
    msg->val = cfe_malloc(sizeof(MpzSer *) * a->cols * a->rows);
    MpzSer *val = cfe_malloc(sizeof(MpzSer) * a->cols * a->rows);
    for (size_t i =0; i<a->rows; i++) {
        for (size_t j =0; j < a->cols; j++) {
            cfe_mpz_ser_init(&val[i * a->cols + j]);
            cfe_mpz_pack(a->mat[i].vec[j], &val[i * a->cols + j]);
            msg->val[i * a->cols + j] = &val[i * a->cols + j];
        }
    }

    msg->cols = a->cols;
    msg->rows = a->rows;
}

void cfe_mat_ser(cfe_mat *a, cfe_ser *buf) {
    MatSer msg = MAT_SER__INIT;
    cfe_mat_pack(a, &msg);
    buf->len = mat_ser__get_packed_size(&msg);
    buf->ser = cfe_malloc(buf->len);

    mat_ser__pack(&msg, buf->ser);
}

void cfe_mat_unpack(cfe_mat *a, MatSer *msg) {
    cfe_mat_init(a, (size_t)msg->rows, (size_t)msg->cols);
    for (size_t i =0; i<a->rows; i++) {
        for (size_t j =0; j < a->cols; j++) {
            cfe_mpz_unpack(a->mat[i].vec[j], msg->val[i * a->cols + j]);
        }
    }
}

cfe_error cfe_mat_read(cfe_mat *a, cfe_ser *buf) {
    MatSer *msg;
    msg = mat_ser__unpack(NULL, buf->len, buf->ser);
    if (msg == NULL)
    {
        return 1;
    }

    cfe_mat_unpack(a, msg);
    // Free the unpacked message
    mat_ser__free_unpacked(msg, NULL);

    return 0;
}


void cfe_octet_ser_init(OctetSer *o) {
    o->base.descriptor = &octet_ser__descriptor;
    o->base.n_unknown_fields = 0;
    o->base.unknown_fields = NULL;
    o->len = 0;
    o->val = NULL;
    o->max = 0;
    o->n_val = 0;
}

void cfe_octet_pack(octet *oct, OctetSer *msg) {
    msg->n_val = (size_t)oct->len;
    msg->val = cfe_malloc(sizeof (int32_t) * (size_t)oct->len);
    for (int i = 0; i < oct->len; i++)
        msg->val[i] = (int32_t) oct->val[i];
    msg->len = oct->len;
    msg->max = oct->max;
}

void cfe_octet_buf(OctetSer *msg, cfe_ser *buf) {
    size_t msg_len;
    msg_len = octet_ser__get_packed_size(msg);
    buf->ser = cfe_malloc(msg_len);

    octet_ser__pack(msg, buf->ser);
    buf->len = msg_len;
}

void cfe_octet_unpack(octet *oct, OctetSer *msg) {
    oct->val = (char *)cfe_malloc(sizeof(char) * (size_t)msg->len);
    for (int i = 0; i < msg->len; i++) {
        oct->val[i] = (char) msg->val[i];
    }
    oct->len = (int) msg->len;
    oct->max = (int) msg->max;
}

void cfe_ECP_BN254_pack(ECP_BN254 *a, OctetSer *msg) {
    char h1[MODBYTES_256_56 + 1];
    octet oct = {0, sizeof(h1), h1};
    ECP_BN254_toOctet(&oct, a, true);

    cfe_octet_pack(&oct, msg);
}

void cfe_ECP_BN254_ser(ECP_BN254 *a, cfe_ser *buf) {
    OctetSer msg = OCTET_SER__INIT;
    cfe_ECP_BN254_pack(a, &msg);
    cfe_octet_buf(&msg, buf);
}

void cfe_ECP_BN254_unpack(ECP_BN254 *a, OctetSer *msg) {
    octet oct;
    cfe_octet_unpack(&oct, msg);
    ECP_BN254_fromOctet(a, &oct);
}

cfe_error cfe_ECP_BN254_read(ECP_BN254 *a, cfe_ser *buf) {
    OctetSer *msg;

    msg = octet_ser__unpack(NULL, buf->len, buf->ser);
    if (msg == NULL)
    {
        return 1;
    }

    cfe_ECP_BN254_unpack(a, msg);
    // Free the unpacked message
    octet_ser__free_unpacked(msg, NULL);
    return 0;
}

void cfe_ECP2_BN254_pack(ECP2_BN254 *a, OctetSer *msg) {
    char *h1 = (char *) cfe_malloc((4 * MODBYTES_256_56) * sizeof(char));
    octet oct = {0, (4 * MODBYTES_256_56) * sizeof(char), h1};
    ECP2_BN254_toOctet(&oct, a);

    cfe_octet_pack(&oct, msg);
}

void cfe_ECP2_BN254_ser(ECP2_BN254 *a, cfe_ser *buf) {
    OctetSer msg = OCTET_SER__INIT;
    cfe_ECP2_BN254_pack(a, &msg);
    cfe_octet_buf(&msg, buf);
}



void cfe_ECP2_BN254_unpack(ECP2_BN254 *a, OctetSer *msg) {
    octet oct;
    cfe_octet_unpack(&oct, msg);
    ECP2_BN254_fromOctet(a, &oct);
}

cfe_error cfe_ECP2_BN254_read(ECP2_BN254 *a, cfe_ser *buf) {
    OctetSer *msg;

    msg = octet_ser__unpack(NULL, buf->len, buf->ser);
    if (msg == NULL)
    {
        return 1;
    }

    cfe_ECP2_BN254_unpack(a, msg);
    // Free the unpacked message
    octet_ser__free_unpacked(msg, NULL);
    return 0;
}

void cfe_FP12_BN254_pack(FP12_BN254 *a, OctetSer *msg) {
    octet oct;
    oct.val = cfe_malloc(12 * MODBYTES_256_56 * sizeof(char));
    FP12_BN254_toOctet(&oct, a);

    cfe_octet_pack(&oct, msg);
}

void cfe_FP12_BN254_ser(FP12_BN254 *a, cfe_ser *buf) {
    OctetSer msg = OCTET_SER__INIT;
    cfe_FP12_BN254_pack(a, &msg);
    cfe_octet_buf(&msg, buf);
}


void cfe_FP12_BN254_unpack(FP12_BN254 *a, OctetSer *msg) {
    octet oct;
    cfe_octet_unpack(&oct, msg);
    FP12_BN254_fromOctet(a, &oct);
}

cfe_error cfe_FP12_BN254_read(FP12_BN254 *a, cfe_ser *buf) {
    OctetSer *msg;

    msg = octet_ser__unpack(NULL, buf->len, buf->ser);
    if (msg == NULL)
    {
        return 1;
    }

    cfe_FP12_BN254_unpack(a, msg);
    // Free the unpacked message
    octet_ser__free_unpacked(msg, NULL);
    return 0;
}

void cfe_msp_pack(cfe_msp *a, MspSer *msg) {
    MatSer mat = MAT_SER__INIT;
    cfe_mat_pack(&(a->mat), &mat);
    msg->mat = &mat;
    cfe_mat_print(&(a->mat));
    cfe_mat b;
    cfe_mat_unpack(&b, msg->mat);
    cfe_mat_print(&b);

    printf("here\n");
//    msg->n_row_to_attrib = 0;
//    msg->n_row_to_attrib = a->mat.rows;
//    msg->row_to_attrib = cfe_malloc(sizeof(int64_t) * a->mat.rows);
//    printf("here\n");
//
//    for (size_t j =0; j < a->mat.rows; j++) {
//        msg->row_to_attrib[j] = (int64_t)a->row_to_attrib[j];
//    }
//    printf("here\n");

}

void cfe_msp_ser(cfe_msp *a, cfe_ser *buf) {
    MspSer msg = MSP_SER__INIT;
    cfe_msp_pack(a, &msg);
    printf("here3\n");

    buf->len = msp_ser__get_packed_size(&msg);
//    printf("here3\n");
//
//    buf->ser = cfe_malloc(buf->len);
//
//    msp_ser__pack(&msg, buf->ser);
}

void cfe_msp_unpack(cfe_msp *a, MspSer *msg) {
    cfe_mat_unpack(&a->mat, msg->mat);

    a->row_to_attrib = cfe_malloc(sizeof(int) * a->mat.rows);
//    for (size_t j =0; j < msg->n_row_to_attrib; j++) {
//        a->row_to_attrib[j] = (int)msg->row_to_attrib[j];
//    }

}

cfe_error cfe_msp_read(cfe_msp *a, cfe_ser *buf) {
    MspSer *msg;
    msg = msp_ser__unpack(NULL, buf->len, buf->ser);
    if (msg == NULL) {
        return 1;
    }

    cfe_msp_unpack(a, msg);
    // Free the unpacked message
    msp_ser__free_unpacked(msg, NULL);

    return 0;
}
