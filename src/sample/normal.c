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

#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <sodium.h>
#include <memory.h>

#include "cifer/sample/normal.h"

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

static const double EXP_COFF[] = {1.43291003789439094275872613876154915146798884961754e-7,
                                  1.2303944375555413249736938854916878938183799618855e-6,
                                  1.5359914219462011698283041005730353845137869939208e-5,
                                  1.5396043210538638053991311593904356413986533880234e-4,
                                  1.3333877552501097445841748978523355617653578519821e-3,
                                  9.6181209331756452318717975913386908359825611114502e-3,
                                  5.5504109841318247098307381293125217780470848083496e-2,
                                  0.24022650687652774559310842050763312727212905883789,
                                  0.69314718056193380668617010087473317980766296386719,
                                  1};
static const size_t EXP_LEN = 10;
static const uint64_t EXP_MANTISSA_PRECISION = 52;
static const uint64_t EXP_MANTISSA_MASK = (((uint64_t) 1 << 52) - 1);
static const uint64_t MAX_EXP = 1023;
static const uint64_t BIT_LEN_FOR_SAMPLE = 19;
static const uint64_t CMP_MASK = (uint64_t) 1 << 61;

bool cfe_bernoulli(mpz_t t, mpf_t k_square_inv) {
    mpf_t a_big;
    mpf_init(a_big);
    mpf_set_z(a_big, t);
    mpf_mul(a_big, a_big, k_square_inv);
    double a = mpf_get_d(a_big);
    a = -a;
    mpf_clear(a_big);

    double neg_floor_a = -floor(a);
    double z = a + neg_floor_a;

    double pow_of_z = EXP_COFF[0];
    for (size_t i = 1; i < EXP_LEN; i++) {
        pow_of_z = pow_of_z * z + EXP_COFF[i];
    }
    uint64_t pow_of_a_mantissa, pow_of_a_exponent;
    memcpy(&pow_of_a_mantissa, &pow_of_z, 8);
    pow_of_a_mantissa = pow_of_a_mantissa & EXP_MANTISSA_MASK;
    memcpy(&pow_of_a_exponent, &pow_of_z, 8);
    pow_of_a_exponent = (pow_of_a_exponent >> EXP_MANTISSA_PRECISION) - (uint64_t) neg_floor_a;

    uint8_t r[16];
    randombytes_buf(r, 16);

    uint64_t r1, r2;
    memcpy(&r1, r, 8);
    memcpy(&r2, r + 8, 8);

    r1 = r1 >> (64 - (EXP_MANTISSA_PRECISION + 1));
    r2 = r2 >> (64 - BIT_LEN_FOR_SAMPLE);

    uint64_t check1 = pow_of_a_mantissa | (1LL << EXP_MANTISSA_PRECISION);
    uint64_t check2 = (uint64_t) 1 << (BIT_LEN_FOR_SAMPLE + pow_of_a_exponent + 1 - MAX_EXP);

    if ((CMP_MASK & (r1 - check1) & (r2 - check2)) ||  pow_of_z == 1) {
        return true;
    }

    return false;
}
