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

#ifndef CIFER_ERRORS_H
#define CIFER_ERRORS_H

/**
 * \file    
 * \ingroup internal
 * \brief Error definitions

 */


typedef enum cfe_error {
    CFE_ERR_NONE = 0,
    CFE_ERR_INIT,

    CFE_ERR_MALFORMED_PUB_KEY,
    CFE_ERR_MALFORMED_SEC_KEY,
    CFE_ERR_MALFORMED_FE_KEY,
    CFE_ERR_MALFORMED_CIPHER,
    CFE_ERR_MALFORMED_INPUT,
    CFE_ERR_BOUND_CHECK_FAILED,

    CFE_ERR_DLOG_NOT_FOUND,
    CFE_ERR_DLOG_CALC_FAILED,
    CFE_ERR_PRIME_GEN_FAILED,
    CFE_ERR_PARAM_GEN_FAILED,
    CFE_ERR_PUB_KEY_GEN_FAILED,
    CFE_ERR_SEC_KEY_GEN_FAILED,

    CFE_ERR_PRECONDITION_FAILED,

    CFE_ERR_INSUFFICIENT_KEYS,
    CFE_ERR_CORRUPTED_BOOL_EXPRESSION,
    CFE_ERR_NO_SOLUTION_EXISTS,
    CFE_ERR_NO_INVERSE,
} cfe_error;

#endif
