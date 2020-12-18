#include <stdio.h>
#include <gmp.h>
#include "cifer/innerprod/fullysec/damgard.h"
#include "cifer/sample/uniform.h"

// This is an example program that you can use to do a quick
// test of CiFEr libraries. Modify the code to try chosen
// functions.
int main() {
    // We will give an example of an inner product scheme

    // first choose meta-parameters for the scheme
    size_t l = 5; // dimension of encryption vector
    mpz_t bound; // bound of the input values set to 2^10
    mpz_init(bound);
    mpz_set_ui(bound, 2);
    mpz_pow_ui(bound, bound, 10);
    // modulus_len defines the security of the scheme, the higher the better
    // for precomputed parameters it needs to be in {1024, 1536, 2048, 2560, 3072, 4096}
    size_t modulus_len = 2048;

    // initiate the scheme
    cfe_damgard s;
    cfe_error err = cfe_damgard_precomp_init(&s, l, modulus_len, bound);
    if (err != 0) {
        return err;
    }

    // initiate and generate a secret and a public key
    cfe_damgard_sec_key msk;
    cfe_vec mpk;
    cfe_damgard_sec_key_init(&msk, &s);
    cfe_damgard_pub_key_init(&mpk, &s);
    cfe_damgard_generate_master_keys(&msk, &mpk, &s);

    // set the vector (of length l) that you want to encrypt
    // we sample a uniformly random vector x
    cfe_vec x;
    cfe_vec_init(&x, l);
    cfe_uniform_sample_vec(&x, bound);

    // encrypt the the vector x
    cfe_vec ciphertext;
    cfe_damgard_ciphertext_init(&ciphertext, &s);
    err = cfe_damgard_encrypt(&ciphertext, &s, &x, &mpk);
    if (err != 0) {
        return err;
    }

    // choose an inner product vector that will be multiplied with
    // the encrypted x; we set it to a vector [1, 1,...,1]
    mpz_t one;
    mpz_init_set_si(one, 1);
    cfe_vec y;
    cfe_vec_init(&y, l);
    cfe_vec_set_const(&y, one);

    // derive a functional key corresponding to y that will allow
    // to compute and decrypt the inner product xy from the encrypted x
    cfe_damgard_fe_key fe_key;
    cfe_damgard_fe_key_init(&fe_key);
    err = cfe_damgard_derive_fe_key(&fe_key, &s, &msk, &y);
    if (err != 0) {
        return err;
    }

    // we simulate a decryptor who is given encrypted x and
    // FE key corresponding to y and can then decrypt xy
    cfe_damgard decryptor;
    cfe_damgard_copy(&decryptor, &s);
    mpz_t xy;
    mpz_init(xy);
    err = cfe_damgard_decrypt(xy, &decryptor, &ciphertext, &fe_key, &y);
    if (err != 0) {
        return err;
    }
    gmp_printf("The inner product of a random encrypted vector x and y = [1, 1,...,1] is %Zd\n", xy);

    mpz_clears(bound, xy, one, NULL);
    cfe_vec_frees(&x, &y, &mpk, &ciphertext, NULL);

    cfe_damgard_sec_key_free(&msk);
    cfe_damgard_fe_key_free(&fe_key);
    cfe_damgard_free(&s);
    cfe_damgard_free(&decryptor);

    return 0;
}
