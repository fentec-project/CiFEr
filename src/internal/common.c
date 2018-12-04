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

#include <sodium.h>

#include "internal/common.h"

cfe_error cfe_init(void) {
    if (sodium_init() == -1) {
        return CFE_ERR_INIT;
    }
    return CFE_ERR_NONE;
}

void *cfe_malloc(size_t size) {
    void *ptr = malloc(size);

    if (size != 0 && ptr == NULL) {
        perror("Failed to allocate memory");
        abort();
    }

    return ptr;
}
