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

#include <stdlib.h>
#include "sample/normal.h"

void cfe_normal_init(cfe_normal *s, mpf_t sigma, size_t n) {
    mpf_init_set(s->sigma, sigma);
    s->n = n;
    s->pre_exp.vec = NULL;

    mpz_init(s->pow_n);
    mpz_ui_pow_ui(s->pow_n, 2, n);

    mpf_init2(s->pow_nf, n);
    mpf_set_z(s->pow_nf, s->pow_n);
}

void cfe_normal_free(cfe_normal *s) {
    mpf_clears(s->sigma, s->pow_nf, NULL);
    mpz_clear(s->pow_n);

    // it's possible that pre_exp was not allocated, so a check is needed
    if (s->pre_exp.vec != NULL) {
        cfe_vec_float_free(&s->pre_exp);
    }
}

void cfe_normal_precomp_exp(cfe_normal *s) {
    mpf_t max_float, sqrt_n;
    mpf_inits(max_float, sqrt_n, NULL);
    mpf_sqrt_ui(sqrt_n, s->n);
    mpf_mul(max_float, s->sigma, sqrt_n);

    mpf_t two_sigma_square;
    mpf_init2(two_sigma_square, s->n);
    mpf_mul(two_sigma_square, s->sigma, s->sigma);
    mpf_mul_ui(two_sigma_square, two_sigma_square, 2);

    mpz_t max_float_z;
    mpz_init(max_float_z);
    mpz_set_f(max_float_z, max_float);
    size_t max_bits = mpz_sizeinbase(max_float_z, 2) * 2;

    cfe_vec_float_init(&s->pre_exp, max_bits + 1, s->n);

    mpz_t x;
    mpz_init_set_ui(x, 1);

    // calculate values
    for (size_t i = 0; i < max_bits + 1; i++) {
        cfe_taylor_exp(s->pre_exp.vec[i], x, two_sigma_square, s->n * 8, s->n);
        mpz_mul_ui(x, x, 2);
    }

    // cleanup
    mpz_clears(x, max_float_z, NULL);
    mpf_clears(two_sigma_square, max_float, sqrt_n, NULL);
}


// Function decides if y > exp(-x/(2*sigma^2)) with minimal calculations of
// exp(-x/(2*sigma^2)) based on the precomputed values.
// sigma is implicit in the precomputed values.
bool cfe_normal_is_exp_greater(cfe_normal *s, mpf_t y, mpz_t x) {
    // bounds on the upper and lower value of the exp
    mpf_t upper, lower, tmp;
    mpf_init2(upper, s->n);
    mpf_init2(lower, s->n);
    mpf_init2(tmp, s->n);
    mpf_set_ui(upper, 1);
    size_t max_bits = mpz_sizeinbase(x, 2);

    cfe_vec_float_get(lower, &s->pre_exp, max_bits);
    cfe_vec_float_get(tmp, &s->pre_exp, 0);
    mpf_div(lower, lower, tmp);

    if (mpf_cmp(lower, y) == 1) {
        mpf_clears(upper, lower, tmp, NULL);
        return false;
    }

    bool ret = false;
    // iteratively updating the bounds; if y is out of bound the function
    // returns the value
    for (size_t i = 0; i < max_bits; i++) {
        int bit = mpz_tstbit(x, max_bits - 1 - i);
        cfe_vec_float_get(tmp, &s->pre_exp, max_bits - 1 - i);

        if (bit == 1) {
            mpf_mul(upper, upper, tmp);
            if (mpf_cmp(y, upper) == 1) {
                ret = true;
                break;
            }
        } else {
            mpf_div(lower, lower, tmp);
            if (mpf_cmp(lower, y) == 1) {
                ret = false;
                break;
            }
        }
    }

    mpf_clears(upper, lower, tmp, NULL);
    return ret;
}

// an approximation of a exp(-x/sigma) with taylor
// polynomial of degree k, precise up to n bits, assuming
// res has been initialized with precision n
void cfe_taylor_exp(mpf_t res, mpz_t x, mpf_t alpha, size_t k, size_t n) {
    // prepare the values for calculating the taylor polynomial of exp(x/sigma)
    mpf_set_ui(res, 1);
    mpf_set_prec(res, n);

    mpf_t value, power_of_value, tmp, factorial, eps, one;

    mpz_t one_over_eps;
    mpz_init(one_over_eps);
    mpz_ui_pow_ui(one_over_eps, 2, n);

    mpf_init_set_ui(one, 1);
    mpf_init2(eps, n);
    mpf_set_z(eps, one_over_eps);
    mpf_div(eps, one, eps);

    mpf_init2(value, n);
    mpf_set_z(value, x);
    mpf_div(value, value, alpha);

    mpf_init2(power_of_value, n);
    mpf_set(power_of_value, value);

    mpf_init2(tmp, n);
    mpf_init2(factorial, n);
    mpf_set_ui(factorial, 1);

    // calculate the polynomial up to degree k or stop early
    // if the desired precision is already achieved
    for (size_t i = 1; i <= k; i++) {
        mpf_div(tmp, power_of_value, factorial);
        mpf_add(res, res, tmp);
        if (mpf_cmp(tmp, eps) == -1) {
            break;
        }

        mpf_mul(power_of_value, power_of_value, value);
        mpf_mul_ui(factorial, factorial, i + 1);
    }
    // invert the result
    mpf_div(res, one, res);
    mpf_clears(value, power_of_value, tmp, factorial, eps, one, NULL);
    mpz_clear(one_over_eps);
}

void cfe_mean(mpf_t res, cfe_vec *vec) {
    mpz_t sum, value;
    mpz_inits(sum, value, NULL);
    for (size_t i = 0; i < vec->size; i++) {
        cfe_vec_get(value, vec, i);
        mpz_add(sum, sum, value);
    }

    mpf_set_z(res, sum);
    mpf_div_ui(res, res, vec->size);
    mpz_clears(sum, value, NULL);
}

void cfe_variance(mpf_t res, cfe_vec *vec) {
    mpz_t sum_sqr, value;
    mpz_inits(sum_sqr, value, NULL);
    for (size_t i = 0; i < vec->size; i++) {
        cfe_vec_get(value, vec, i);
        mpz_mul(value, value, value);
        mpz_add(sum_sqr, sum_sqr, value);
    }

    mpf_set_z(res, sum_sqr);
    mpf_div_ui(res, res, vec->size);
    mpz_clears(sum_sqr, value, NULL);
}
