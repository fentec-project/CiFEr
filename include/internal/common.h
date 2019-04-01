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

#ifndef CIFER_COMMON_H
#define CIFER_COMMON_H

#include <stddef.h>

#include "internal/errors.h"

/**
 * \file
 * \ingroup internal
 * \brief Common library functionality
 */

/**
 * This function initializes the library. It ensures that the system's random
 * number generator has been properly seeded.
 * This function must be called before any other functions from this library.
 *
 * @return Error code
 */
cfe_error cfe_init(void);

/**
 * Malloc wrapper which aborts on failure (out-of-memory error).
 *
 * @param size Size of memory block
 * @return Pointer to allocated memory
 */
void *cfe_malloc(size_t size);

#endif
