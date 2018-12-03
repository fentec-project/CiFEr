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

#include "internal/keygen.h"
#include "internal/prime.h"
#include "sample/uniform.h"

// Initializes elgamal struct.
cfe_error cfe_elgamal_init(cfe_elgamal *key, size_t modulus_len) {
    mpz_t x, two, check, g_inv;
    mpz_inits(x, two, check, g_inv, key->p, key->g, key->q, NULL);

    // set p to a safe prime
    cfe_error err = cfe_get_prime(key->p, modulus_len, true);
    if (err) {
        mpz_clears(key->p, key->g, key->q, NULL);
        goto cleanup;
    }

    mpz_sub_ui(key->q, key->p, 1);
    mpz_div_ui(key->q, key->q, 2);

    mpz_set_ui(two, 2);

    while (true) {
        cfe_uniform_sample_range_i_mpz(key->g, 3, key->p);

        // make g an element of the subgroup of quadratic residues
        mpz_powm(key->g, key->g, two, key->p);

        // additional checks to avoid some known attacks
        mpz_sub_ui(check, key->p, 1);
        mpz_mod(check, check, key->g);
        if (mpz_cmp_ui(check, 0) == 0) {
            continue;
        }

        mpz_invert(g_inv, key->g, key->p);
        mpz_sub_ui(check, key->p, 1);
        mpz_mod(check, check, g_inv);
        if (mpz_cmp_ui(check, 0) == 0) {
            continue;
        }

        break;
    }

    // x is randomly generated secret key
    cfe_uniform_sample_range(x, two, key->q);

    cleanup:
    mpz_clears(x, two, check, g_inv, NULL);
    return err;
}

// Frees the memory allocated for the members of cfe_elgamal struct.
void cfe_elgamal_free(cfe_elgamal *key) {
    mpz_clears(key->g, key->p, key->q, NULL);
}
