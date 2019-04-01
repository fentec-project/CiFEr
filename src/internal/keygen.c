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
