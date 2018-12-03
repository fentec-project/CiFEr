/* *
 * Copyright (C) 2018 XLAB d.o.o.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of either:
 *
 *     * the GNU Lesser General Public License as published by the Free
 *     Software Foundation; either version 3 of the License, or (at your
 *     option) any later version.
 *
 * or
 *
 *     * the GNU General Public License as published by the Free Software
 *     Foundation; either version 2 of the License, or (at your option) any
 *     later version.
 *
 * or both in parallel, as here.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

} cfe_error;

#endif
