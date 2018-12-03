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
 * \defgroup simple Simple schemes
 * Simple schemes for functional encryption of inner products.
 * All implementations in this package are based on the reference paper by
 * Abdalla et. al (see https://eprint.iacr.org/2015/017.pdf). The reference
 * scheme offers selective security under chosen-plaintext attacks (s-IND-CPA
 * security).
 *
 * The reference scheme is public key, which means that no master secret key is
 * required for the encryption.
 *
 * For instantiation from the decisional Diffie-Hellman assumption (DDH), see
 * struct ddh (and its multi-input variant ddh_multi, which is a secret key
 * scheme, because a part of the secret key is required for the encryption).
 *
 * For instantiation from learning with errors (LWE), see structs lwe and
 * ring_lwe.
 *
 * \ingroup innerprod
 */
