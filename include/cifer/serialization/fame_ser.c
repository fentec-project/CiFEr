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
#include <cifer/abe/fame.h>
#include <cifer/internal/common.h>
#include "fame.pb-c.h"
#include "data_ser.h"

void cfe_fame_pub_key_ser(cfe_fame_pub_key *pub_key, cfe_ser *buf) {
    CfeFamePubKeySer msg = CFE_FAME_PUB_KEY_SER__INIT;
    OctetSer part_g2_1  = OCTET_SER__INIT;
    OctetSer part_g2_2  = OCTET_SER__INIT;
    OctetSer part_gT_1  = OCTET_SER__INIT;
    OctetSer part_gT_2  = OCTET_SER__INIT;

    cfe_ECP2_BN254_pack(&(pub_key->part_G2[0]), &part_g2_1);
    cfe_ECP2_BN254_pack(&(pub_key->part_G2[1]), &part_g2_2);
    cfe_FP12_BN254_pack(&(pub_key->part_GT[0]), &part_gT_1);
    cfe_FP12_BN254_pack(&(pub_key->part_GT[1]), &part_gT_2);
    msg.part_g2_1 = &part_g2_1;
    msg.part_g2_2 = &part_g2_2;
    msg.part_gt_1 = &part_gT_1;
    msg.part_gt_2 = &part_gT_2;

    size_t msg_len;
    msg_len = cfe_fame_pub_key_ser__get_packed_size(&msg);
    buf->ser = cfe_malloc(msg_len);

    cfe_fame_pub_key_ser__pack(&msg, buf->ser);
    buf->len = msg_len;
}


cfe_error cfe_fame_pub_key_read(cfe_fame_pub_key *pub_key, cfe_ser *buf) {
    CfeFamePubKeySer *msg;

    msg = cfe_fame_pub_key_ser__unpack(NULL, buf->len, buf->ser);
    if (msg == NULL)
    {
        return 1;
    }

    cfe_ECP2_BN254_unpack(&(pub_key->part_G2[0]), msg->part_g2_1);
    cfe_ECP2_BN254_unpack(&(pub_key->part_G2[1]), msg->part_g2_2);
    cfe_FP12_BN254_unpack(&(pub_key->part_GT[0]), msg->part_gt_1);
    cfe_FP12_BN254_unpack(&(pub_key->part_GT[1]), msg->part_gt_2);
    // Free the unpacked message
    cfe_fame_pub_key_ser__free_unpacked(msg, NULL);
    return 0;
}



void cfe_fame_attrib_keys_ser(cfe_fame_attrib_keys *keys, cfe_ser *buf) {
    CfeFameAttribKeysSer msg = CFE_FAME_ATTRIB_KEYS_SER__INIT;
    msg.n_k0 = 3;
    msg.k0 = (OctetSer **) cfe_malloc(sizeof(OctetSer *) * 3);
    OctetSer k0[3];
    for (int i =0; i<3; i++) {
        cfe_octet_ser_init(&k0[i]);
        cfe_ECP2_BN254_pack(&(keys->k0[i]), &k0[i]);
        msg.k0[i] = &k0[i];
    }

    msg.n_k = 3 * keys->num_attrib;
    msg.k = cfe_malloc(sizeof(OctetSer *) * 3 * keys->num_attrib);
    OctetSer *k = cfe_malloc(sizeof(OctetSer) * 3 * keys->num_attrib);
    for (int i =0; i<3; i++) {
        for (size_t j =0; j < keys->num_attrib; j++) {
            cfe_octet_ser_init(&k[3*j + i]);
            cfe_ECP_BN254_pack(&(keys->k[j][i]), &k[3*j + i]);
            msg.k[3*j + i] = &k[3*j + i];
        }
    }

    msg.n_k_prime = 3;
    msg.k_prime = cfe_malloc(sizeof(OctetSer *) * 3);
    OctetSer k_prime[3];
    for (int i =0; i<3; i++) {
        cfe_octet_ser_init(&k_prime[i]);
        cfe_ECP_BN254_pack(&(keys->k_prime[i]), &k_prime[i]);
            msg.k_prime[i] = &k_prime[i];
    }

    msg.n_row_to_attrib = keys->num_attrib;
    msg.row_to_attrib = cfe_malloc(sizeof(int64_t) * keys->num_attrib);
    for (size_t j =0; j < keys->num_attrib; j++) {
        msg.row_to_attrib[j] = keys->row_to_attrib[j];
    }

    size_t msg_len;
    msg_len = cfe_fame_attrib_keys_ser__get_packed_size(&msg);
    buf->ser = cfe_malloc(msg_len);

    cfe_fame_attrib_keys_ser__pack(&msg, buf->ser);
    buf->len = msg_len;
}

cfe_error cfe_fame_attrib_keys_read(cfe_fame_attrib_keys *keys, cfe_ser *buf) {
    CfeFameAttribKeysSer *msg;

    msg = cfe_fame_attrib_keys_ser__unpack(NULL, buf->len, buf->ser);
    if (msg == NULL)
    {
        return 1;
    }

    for (int i =0; i<3; i++) {
        cfe_ECP2_BN254_unpack(&(keys->k0[i]), msg->k0[i]);
    }

    for (int i =0; i<3; i++) {
        for (size_t j =0; j < msg->n_row_to_attrib; j++) {
            cfe_ECP_BN254_unpack(&(keys->k[j][i]), msg->k[3*j + i]);
        }
    }

    for (int i =0; i<3; i++) {
        cfe_ECP_BN254_pack(&(keys->k_prime[i]), msg->k_prime[i]);
    }

    for (size_t j =0; j < msg->n_row_to_attrib; j++) {
        keys->row_to_attrib[j] = (int)msg->row_to_attrib[j];
    }

    keys->num_attrib = msg->n_row_to_attrib;
    // Free the unpacked message
    cfe_fame_attrib_keys_ser__free_unpacked(msg, NULL);
    return 0;
}
