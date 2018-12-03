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

#include "sample/normal_cumulative.h"
#include "sample/uniform.h"

void cfe_normal_cumulative_init(cfe_normal_cumulative *s, mpf_t sigma, size_t n, bool two_sided) {
    s->two_sided = two_sided;
    cfe_normal_init(&s->nor, sigma, n);

    cfe_normal_cumulative_precompute(s);

    mpz_init(s->sample_size);
    cfe_vec_get(s->sample_size, &s->precomputed, s->precomputed.size - 1);

    if (two_sided) {
        mpz_mul_ui(s->sample_size, s->sample_size, 2);
    }
}

void cfe_normal_cumulative_free(cfe_normal_cumulative *s) {
    cfe_normal_free(&s->nor);
    mpz_clear(s->sample_size);
    cfe_vec_free(&s->precomputed);
}

void cfe_normal_cumulative_sample(mpz_t res, cfe_normal_cumulative *s) {
    mpz_t sample, max_value, twice, tmp;
    mpz_inits(sample, max_value, twice, tmp, NULL);

    cfe_uniform_sample(sample, s->sample_size);
    cfe_vec_get(max_value, &s->precomputed, s->precomputed.size - 1);
    int sign = 1;

    if (s->two_sided && mpz_cmp(sample, max_value) >= 0) {
        sign = -1;
        mpz_sub(sample, sample, max_value);
    }

    size_t value = cfe_locate_int(sample, 0, s->precomputed.size, &s->precomputed, tmp);

    mpz_set_si(res, (int) value * sign);
    mpz_clears(sample, max_value, twice, tmp, NULL);
}

void cfe_normal_cumulative_precompute(cfe_normal_cumulative *s) {
    mpf_t cut_f, sqrt_n;
    mpf_inits(cut_f, sqrt_n, NULL);
    mpf_sqrt_ui(sqrt_n, s->nor.n);
    mpf_mul(cut_f, s->nor.sigma, sqrt_n);
    size_t cut = mpf_get_ui(cut_f) + 1;

    mpz_t res_entry, i_square, add;
    mpz_inits(res_entry, i_square, add, NULL);

    cfe_vec_init(&s->precomputed, cut + 1);
    mpz_set_ui(res_entry, 0);
    cfe_vec_set(&s->precomputed, res_entry, 0);

    mpf_t value, add_f, two_sigma_square;
    mpf_init(two_sigma_square);

    mpf_mul(two_sigma_square, s->nor.sigma, s->nor.sigma);
    mpf_mul_ui(two_sigma_square, two_sigma_square, 2);

    mpf_init2(add_f, s->nor.n);
    mpf_init2(value, s->nor.n);

    // Setting how precise the approximation of exp with taylor polynomial will
    // be. Assuming that res.size = sigma * sqrt(n) the bellow definition is
    // sufficient to give an approximation with precision at least 1/2^n
    for (size_t i = 0; i < cut; i++) {
        mpz_set_ui(i_square, i * i);
        cfe_taylor_exp(value, i_square, two_sigma_square, s->nor.n * 8, s->nor.n);

        // probability of 0 being sampled is halved since
        // it can appear twice, among positive and negative
        // samples.
        if (i == 0 && s->two_sided) {
            mpf_div_ui(value, value, 2);
        }

        // setting up values for sampling
        mpf_mul(add_f, value, s->nor.pow_nf);
        mpf_trunc(add_f, add_f);
        mpz_set_f(add, add_f);
        cfe_vec_get(res_entry, &s->precomputed, i);
        mpz_add(res_entry, res_entry, add);
        cfe_vec_set(&s->precomputed, res_entry, i + 1);
    }

    mpz_clears(res_entry, i_square, add, NULL);
    mpf_clears(add_f, two_sigma_square, cut_f, sqrt_n, value, NULL);
}

size_t cfe_locate_int(mpz_t sample, size_t start, size_t end, cfe_vec *v, mpz_t middle_value) {
    if (end - start == 1) {
        return start;
    }

    size_t middle = (start + end) / 2;
    cfe_vec_get(middle_value, v, middle);

    if (mpz_cmp(sample, middle_value) < 0) {
        return cfe_locate_int(sample, start, middle, v, middle_value);
    } else {
        return cfe_locate_int(sample, middle, end, v, middle_value);
    }
}

void cfe_normal_cumulative_sample_vec(cfe_vec *res, cfe_normal_cumulative *s) {
    for (size_t i = 0; i < res->size; i++) {
        cfe_normal_cumulative_sample(res->vec[i], s);
    }
}

void cfe_normal_cumulative_sample_mat(cfe_mat *res, cfe_normal_cumulative *s) {
    for (size_t i = 0; i < res->rows; i++) {
        cfe_normal_cumulative_sample_vec(&res->mat[i], s);
    }
}
