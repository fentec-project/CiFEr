#include <stdio.h>
#include <stdlib.h>
#include <cifer/data/mat.h>


size_t cfe_mpz_ser(mpz_t a, char *buf) {
    mpz_get_str(buf, 32, a)
}

size_t cfe_mpz_read(mpz_t a, char *buf) {
    mpz_set_str(a, buf, 32)
}