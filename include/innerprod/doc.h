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

/**
 * \defgroup innerprod Inner product schemes
 * Functional encryption schemes for inner products.
 * Based on security assumptions, the schemes are organized into submodules
 * simple (s-IND-CPA security), and fullysec (e.g. "fully secure", offering
 * adaptive IND-CPA security).
 *
 * Note that in both modules you will find single input as well as multi input
 * schemes. Construction of all multi input schemes is based on the work of
 * Abdalla et. al (see paper: https://eprint.iacr.org/2017/972.pdf)
 *
 * Most of the functions write their results to pointers which are passed as
 * parameters. The memory used by these results needs to be freed by the caller
 * with *xyz_free* functions (each struct defined in this library has a respective
 * *struct_name_free* function). These free the memory used by the struct's
 * members, but not the memory used by the struct itself. This is due to the
 * initialization of the structs' members in functions such as *generate_keys*
 * and *derive_key* - this makes the API simpler and easier to use as the user
 * of the library does not to know all struct fields.
 *
 * The results of the functions are the
 * parameters listed _before_ the pointer to the struct in the parameter list.
 * Consult the appropriate documentation for each scheme for more thorough
 * descriptions of functions and their results.
 *
 */