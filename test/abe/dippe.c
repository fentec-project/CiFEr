
#include <amcl/pair_BN254.h>

#include "cifer/test.h"
#include "cifer/data/vec.h"
#include "cifer/abe/dippe.h"

MunitResult test_dippe_end_to_end_conjunction(const MunitParameter *params, void *data) {
    cfe_error err;

    // Init DIPPE scheme
    cfe_dippe dippe;
    cfe_dippe_init(&dippe, 2);

    // Setup two Authorities
    cfe_dippe_pub_key pk[2];
    cfe_dippe_sec_key sk[2];
    for (int i=0; i<2; i++) {
        cfe_dippe_pub_key_init(&(pk[i]), &dippe);
        cfe_dippe_sec_key_init(&(sk[i]), &dippe);
        cfe_dippe_generate_master_keys(&(pk[i]), &(sk[i]), &dippe);
    }

    // Sample message
    ECP_BN254 g1;
    ECP_BN254_generator(&g1);
    ECP2_BN254 g2;
    ECP2_BN254_generator(&g2);
    FP12_BN254 msg;
    PAIR_BN254_ate(&msg, &g2, &g1);
    PAIR_BN254_fexp(&msg);

    // Collection of 6 public keys
    // Each of them correspondents to an index of the Attribute-/ Policy-vector
    cfe_dippe_pub_key *pks[] = {&(pk[0]), &(pk[1]), &(pk[0]), &(pk[1]), &(pk[0]), &(pk[1])};

    // unique GID
    char gid[] = "TESTGID";

    // Test patterns
    char attr_patterns[4][6] = {
        "11011", "11111",   // valid patterns
        "01001", "11010"    // invalid patterns
    };
    // Test results
    int decryption_results[] = {1, 1, 0, 0};

    // Policy vector (one additional component is added)
    //   ------            -----
    //   Policy            Attrs
    //   ------            -----
    //    x0               1
    //    x1               1
    //    0         .      0      =  (x0+x1+x4-x0-x1-x4) = 0
    //    0                0
    //    x4               1
    // -> x5=(-x0-x1-x4)   1
    cfe_vec pol;
    err = cfe_dippe_build_conjunction_policy_vector(&pol, &dippe, "11001");
    munit_assert(err == CFE_ERR_NONE);

    mpz_t attrs;
    cfe_dippe_cipher cipher;
    cfe_dippe_cipher_init(&cipher, &dippe, pol.size);

    // Encrypt message under given policy vector
    err = cfe_dippe_encrypt(&cipher, &dippe, pks, (sizeof(pks)/sizeof(cfe_dippe_pub_key*)), &pol, &msg);
    munit_assert(err == CFE_ERR_NONE);

    // Decrypt message
    FP12_BN254 result;

    // Collection of user secret keys
    cfe_dippe_user_sec_key usks[6];
    for (int i=0; i<6; i++) {
        cfe_dippe_user_sec_key_init(&(usks[i]), &dippe);
    }

    for (int i=0; i<4; i++) {
        // Attribute vector (one additional component is added)
        err = cfe_dippe_build_attribute_vector(attrs, &dippe, attr_patterns[i]);
        munit_assert(err == CFE_ERR_NONE);

        // User secret keys
        for (int j=0; j<6; j++) {
            err = cfe_dippe_keygen(&(usks[j]), &dippe, j, pks, (sizeof(pks)/sizeof(cfe_dippe_pub_key*)), &sk[(j & 1)], attrs, gid);
            munit_assert(err == CFE_ERR_NONE);
        }

        // Decrypt message
        err = cfe_dippe_decrypt(&result, &dippe, (cfe_dippe_user_sec_key*)&usks, (sizeof(usks)/sizeof(cfe_dippe_user_sec_key)), &cipher, attrs, gid);
        munit_assert(err == CFE_ERR_NONE);

        // Check decryption
        munit_assert(FP12_BN254_equals(&msg, &result) == decryption_results[i]);

        // Cleanup
        mpz_clear(attrs);
    }

    // Cleanup
    for (int i=0; i<2; i++) {
        cfe_dippe_sec_key_free(&(sk[i]));
        cfe_dippe_pub_key_free(&(pk[i]));
    }
    for (int i=0; i<6; i++) {
        cfe_dippe_user_sec_key_free(&(usks[i]));
    }
    cfe_dippe_cipher_free(&cipher);
    cfe_dippe_free(&dippe);
    cfe_vec_free(&pol);

    return MUNIT_OK;
}

MunitResult test_dippe_end_to_end_threshold(const MunitParameter *params, void *data) {
    cfe_error err;

    // Init DIPPE scheme
    cfe_dippe dippe;
    cfe_dippe_init(&dippe, 2);

    // Setup one Authority
    cfe_dippe_pub_key pk0;
    cfe_dippe_pub_key_init(&pk0, &dippe);
    cfe_dippe_sec_key sk0;
    cfe_dippe_sec_key_init(&sk0, &dippe);
    cfe_dippe_generate_master_keys(&pk0, &sk0, &dippe);

    // Sample message
    ECP_BN254 g1;
    ECP_BN254_generator(&g1);
    ECP2_BN254 g2;
    ECP2_BN254_generator(&g2);
    FP12_BN254 msg;
    PAIR_BN254_ate(&msg, &g2, &g1);
    PAIR_BN254_fexp(&msg);

    // Collection of public keys
    // Each of them correspondents to an index of the Attribute-/ Policy-vector
    cfe_dippe_pub_key *pks[] = {&pk0, &pk0, &pk0, &pk0, &pk0};

    // unique GID
    char gid[] = "TESTGID";

    // Test patterns
    char attr_patterns[6][5] = {
        "1100", "1001", "0101", "0111", // valid patterns
        "1101", "1111"                  // invalid patterns
    };
    // Test results
    int decryption_results[] = {1, 1, 1, 1, 0, 0};

    // Policy vector (one additional component is added)
    //   ------            -----
    //   Policy            Attrs
    //   ------            -----
    //    1                 1
    //    1                 1
    //    0         .       0      =  (1+1+0+0-t) = 0
    //    1                 0
    //   -t                 1
    cfe_vec pol;
    err = cfe_dippe_build_exact_threshold_policy_vector(&pol, &dippe, "1101", 2);
    munit_assert(err == CFE_ERR_NONE);

    mpz_t attrs;
    cfe_dippe_cipher cipher;
    cfe_dippe_cipher_init(&cipher, &dippe, pol.size);

    // Encrypt message under given policy vector
    err = cfe_dippe_encrypt(&cipher, &dippe, pks, (sizeof(pks)/sizeof(cfe_dippe_pub_key*)), &pol, &msg);
    munit_assert(err == CFE_ERR_NONE);

    // Decrypt message
    FP12_BN254 result;

    // Collection of user secret keys
    cfe_dippe_user_sec_key usks[5];
    for (int i=0; i<5; i++) {
        cfe_dippe_user_sec_key_init(&(usks[i]), &dippe);
    }

    for (int i=0; i<6; i++) {
        // Attribute vector (one additional component is added)
        err = cfe_dippe_build_attribute_vector(attrs, &dippe, attr_patterns[i]);
        munit_assert(err == CFE_ERR_NONE);

        // User secret keys from Auth0
        for (int j=0; j<5; j++) {
            err = cfe_dippe_keygen(&(usks[j]), &dippe, j, pks, (sizeof(pks)/sizeof(cfe_dippe_pub_key*)), &sk0, attrs, gid);
            munit_assert(err == CFE_ERR_NONE);
        }

        // Decrypt message
        err = cfe_dippe_decrypt(&result, &dippe, (cfe_dippe_user_sec_key*)&usks, (sizeof(usks)/sizeof(cfe_dippe_user_sec_key)), &cipher, attrs, gid);
        munit_assert(err == CFE_ERR_NONE);

        // Check decryption
        munit_assert(FP12_BN254_equals(&msg, &result) == decryption_results[i]);

        // Cleanup
        mpz_clear(attrs);
    }

    // Cleanup
    for (int i=0; i<5; i++) {
        cfe_dippe_user_sec_key_free(&(usks[i]));
    }
    cfe_dippe_sec_key_free(&sk0);
    cfe_dippe_pub_key_free(&pk0);
    cfe_dippe_cipher_free(&cipher);
    cfe_dippe_free(&dippe);
    cfe_vec_free(&pol);

    return MUNIT_OK;
}

MunitTest dippe_tests[] = {
    {(char *) "/end-to-end-conjunction",    test_dippe_end_to_end_conjunction,  NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {(char *) "/end-to-end-threshold",      test_dippe_end_to_end_threshold,    NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL,                                                                NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}
};

MunitSuite dippe_suite = {
    (char *) "/abe/dippe", dippe_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};
