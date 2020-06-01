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

#ifndef CIFER_FAME_SER_H
#define CIFER_FAME_SER_H

#include <cifer/abe/fame.h>
#include "data_ser.h"

//size_t cfe_fame_sec_key_ser(cfe_fame_sec_key *sec_key, void *buf);
//
//int cfe_fame_sec_key_read(cfe_fame_sec_key *sec_key, void *buf, size_t len);
//
//size_t cfe_fame_bla_ser(cfe_fame_sec_key *sec_key, uint8_t *buf);
//
//int cfe_fame_bla_read(cfe_fame_sec_key *sec_key, uint8_t *buf, size_t len);

void cfe_fame_pub_key_ser(cfe_fame_pub_key *pub_key, cfe_ser *buf);

cfe_error cfe_fame_pub_key_read(cfe_fame_pub_key *pub_key, cfe_ser *buf);

void cfe_fame_attrib_keys_ser(cfe_fame_attrib_keys *keys, cfe_ser *buf);

cfe_error cfe_fame_attrib_keys_read(cfe_fame_attrib_keys *keys, cfe_ser *buf);

#endif
