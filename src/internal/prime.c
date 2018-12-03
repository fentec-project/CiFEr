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

#include <stdint.h>
#include <stdlib.h>
#include <sodium.h>

#include "internal/prime.h"

// Checks if p is a safe prime, e.g. if (p-1)/2 is also a prime.
bool cfe_is_safe_prime(mpz_t p) {
    // mpz_probab_prime returns 1 if argument is probably prime and 2 if
    // it is definitely prime
    if (mpz_probab_prime_p(p, 20)) {
        mpz_t q;
        mpz_init(q);
        mpz_sub_ui(q, p, 1);
        mpz_div_ui(q, q, 2);

        bool is_safe = mpz_probab_prime_p(q, 20) > 0;
        mpz_clear(q);

        return is_safe;
    }

    return false;
}

uint64_t small_primes[15] = {3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53};
uint64_t small_primes_product = 16294579238595022365u;

// Finds a prime number of specified bit length.
// If the safe parameter is true, the prime will be a safe prime, e.g a prime p
// where (p-1)/2 is also a prime.
// The prime is assigned to the argument n.
// Returns 0 on success, and some other value != 0 on failure.
// adapted from https://github.com/xlab-si/emmy/blob/master/crypto/common/primes.go
cfe_error cfe_get_prime(mpz_t res, size_t bits, bool safe) {
    if (bits < 2) {
        return CFE_ERR_PRECONDITION_FAILED;
    }

    // if we are generating a safe prime, decrease the number of bits by 1
    // as we are actually generating a germain prime and then modifying it to be safe
    // the safe prime will have the correct amount of bits
    size_t n_bits = safe ? bits - 1 : bits;
    size_t n_bytes = (bits + 7) / 8;
    uint8_t *bytes = (uint8_t *) malloc(n_bytes * sizeof(uint8_t));

    size_t b = n_bits % 8;
    if (b == 0) {
        b = 8;
    }

    mpz_t p, p_safe, big_mod;
    mpz_inits(p, p_safe, big_mod, NULL);

    while (true) {
        randombytes_buf(bytes, n_bytes);

        bytes[0] &= (uint8_t) ((1 << b) - 1);

        if (b >= 2) {
            bytes[0] |= 3 << (b - 2);
        } else {
            bytes[0] |= 1;
            if (n_bytes > 1) {
                bytes[1] |= 0x80;
            }
        }

        bytes[n_bytes - 1] |= 1;

        mpz_import(p, n_bytes, 1, 1, 0, 0, bytes);

        uint64_t mod = mpz_mod_ui(big_mod, p, small_primes_product);

        for (uint64_t delta = 0; delta < (1 << 20); delta += 2) {
            uint64_t m = mod + delta;
            bool candidate = true;

            for (size_t i = 0; i < 15; i++) {
                uint64_t prime = small_primes[i];

                if (m % prime == 0 && (n_bits > 6 || m != prime)) {
                    candidate = false;
                    break;
                }

                if (safe) {
                    uint64_t m1 = (2 * m + 1) % small_primes_product;

                    if (m1 % prime == 0 && (n_bits > 6 || m1 != prime)) {
                        candidate = false;
                        break;
                    }
                }

            }

            if (candidate) {
                if (delta > 0) {
                    mpz_set_ui(big_mod, delta);
                    mpz_add(p, p, big_mod);
                }

                if (safe) {
                    mpz_mul_ui(p_safe, p, 2);
                    mpz_add_ui(p_safe, p_safe, 1);
                }
                break;
            }
        }

        if (mpz_probab_prime_p(p, 10) && mpz_sizeinbase(p, 2) == n_bits) {
            if (!safe) {
                if (mpz_probab_prime_p(p, 30)) {
                    break;
                }
            } else if (mpz_probab_prime_p(p_safe, 50) && mpz_probab_prime_p(p, 30)) {
                break;
            }
        }
    }

    if (safe) {
        mpz_set(res, p_safe);
    } else {
        mpz_set(res, p);
    }

    free(bytes);
    mpz_clears(p, p_safe, big_mod, NULL);

    if (mpz_sizeinbase(res, 2) != bits) {
        return CFE_ERR_PRIME_GEN_FAILED;
    }

    return CFE_ERR_NONE;
}
