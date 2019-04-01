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

#ifndef CIFER_BIG_H
#define CIFER_BIG_H

/**
 * \file
 * \ingroup internal
 * \brief Functionalities for transformating mpz_t and
 * BIG_256_56 integers.
 */

#include <gmp.h>
#include <amcl/big_256_56.h>

/**
 * This function transforms an integer of type mpz_t to
 * type BIG_256_56.
 *
 * @param dst Integer will be saved here
 * @param src Integer will be copied from
 */
void BIG_256_56_from_mpz(BIG_256_56 dst, mpz_t src);

/**
 * This function transforms an integer of type BIG_256_56
 * to type mpz_t.
 *
 * @param dst Integer will be saved here
 * @param src Integer will be copied from
 */void mpz_from_BIG_256_56(mpz_t dst, BIG_256_56 src);

#endif
