
#include <amcl/big_256_56.h>
#include <amcl/pair_BN254.h>
#include <amcl/ecp_BN254.h>
#include <amcl/ecp2_BN254.h>

#include "cifer/test.h"
#include "cifer/data/vec.h"
#include "cifer/abe/dippe.h"

MunitResult test_dippe_end_conjunction(const MunitParameter *params, void *data) {
    cfe_error err;

    // Init DIPPE scheme
    cfe_dippe dippe;
    cfe_dippe_setup(&dippe, 2);

    // Setup two Authorities
    cfe_dippe_pub_key pk[2];
    cfe_dippe_sec_key sk[2];
    cfe_dippe_authsetup(&dippe, &(pk[0]), &(sk[0]));
    cfe_dippe_authsetup(&dippe, &(pk[1]), &(sk[1]));

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
    err = cfe_dippe_build_conjunction_policy_vector(&dippe, &pol, "11001");
    munit_assert(err == CFE_ERR_NONE);

    mpz_t     attrs;
    cfe_dippe_cipher cipher;

    // Decrypt message
    FP12_BN254 result;

    // Collection of user secret keys
    cfe_dippe_user_secret_key usks[6];

    for (int i=0; i<4; i++) {
        // Attribute vector (one additional component is added)
        err = cfe_dippe_build_conjunction_attribute_vector(&dippe, &attrs, attr_patterns[i]);
        munit_assert(err == CFE_ERR_NONE);

        // Encrypt message under given policy vector
        err = cfe_dippe_encrypt(&dippe, &cipher, pks, (sizeof(pks)/sizeof(cfe_dippe_pub_key*)), &pol, &msg);
        munit_assert(err == CFE_ERR_NONE);

        // User secret keys
        for (int j=0; j<6; j++) {
            err = cfe_dippe_keygen(&dippe, &(usks[j]), j, pks, (sizeof(pks)/sizeof(cfe_dippe_pub_key*)), &sk[(j & 1)], &attrs, gid);
            munit_assert(err == CFE_ERR_NONE);
        }

        // Decrypt message
        err = cfe_dippe_decrypt(&dippe, &result, (cfe_dippe_user_secret_key*)&usks, (sizeof(usks)/sizeof(cfe_dippe_user_secret_key)), &cipher, &attrs, gid);
        munit_assert(err == CFE_ERR_NONE);

        // Check decryption
        munit_assert(FP12_BN254_equals(&msg, &result) == decryption_results[i]);

        // Cleanup
        mpz_clear(attrs);
        cfe_dippe_cipher_clear(&cipher);
        for (int j=0; j<6; j++) {
            cfe_dippe_user_secret_key_clear(&(usks[j]));
        }
    }

    // Cleanup
    cfe_vec_free(&pol);
    cfe_dippe_sec_key_clear(&(sk[0]));
    cfe_dippe_sec_key_clear(&(sk[1]));
    cfe_dippe_pub_key_clear(&(pk[0]));
    cfe_dippe_pub_key_clear(&(pk[1]));
    cfe_dippe_clear(&dippe);

    return MUNIT_OK;
}

MunitResult test_dippe_end_to_end_threshold(const MunitParameter *params, void *data) {
    cfe_error err;

    // Init DIPPE scheme
    cfe_dippe dippe;
    cfe_dippe_setup(&dippe, 2);

    // Setup one Authority
    cfe_dippe_pub_key pk0;
    cfe_dippe_sec_key sk0;
    cfe_dippe_authsetup(&dippe, &pk0, &sk0);

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
    err = cfe_dippe_build_exact_threshold_policy_vector(&dippe, &pol, "1101", 2);
    munit_assert(err == CFE_ERR_NONE);

    mpz_t     attrs;
    cfe_dippe_cipher cipher;

    // Decrypt message
    FP12_BN254 result;

    // Collection of user secret keys
    cfe_dippe_user_secret_key usks[5];

    for (int i=0; i<6; i++) {
        // Attribute vector (one additional component is added)
        err = cfe_dippe_build_exact_threshold_attribute_vector(&dippe, &attrs, attr_patterns[i]);
        munit_assert(err == CFE_ERR_NONE);

        // Encrypt message under given policy vector
        err = cfe_dippe_encrypt(&dippe, &cipher, pks, (sizeof(pks)/sizeof(cfe_dippe_pub_key*)), &pol, &msg);
        munit_assert(err == CFE_ERR_NONE);

        // User secret keys from Auth0
        for (int j=0; j<5; j++) {
            err = cfe_dippe_keygen(&dippe, &(usks[j]), j, pks, (sizeof(pks)/sizeof(cfe_dippe_pub_key*)), &sk0, &attrs, gid);
            munit_assert(err == CFE_ERR_NONE);
        }

        // Decrypt message
        err = cfe_dippe_decrypt(&dippe, &result, (cfe_dippe_user_secret_key*)&usks, (sizeof(usks)/sizeof(cfe_dippe_user_secret_key)), &cipher, &attrs, gid);
        munit_assert(err == CFE_ERR_NONE);

        // Check decryption
        munit_assert(FP12_BN254_equals(&msg, &result) == decryption_results[i]);

        // Cleanup
        mpz_clear(attrs);
        cfe_dippe_cipher_clear(&cipher);
        for (int j=0; j<5; j++) {
            cfe_dippe_user_secret_key_clear(&(usks[j]));
        }
    }

    // Cleanup
    cfe_vec_free(&pol);
    cfe_dippe_sec_key_clear(&sk0);
    cfe_dippe_pub_key_clear(&pk0);
    cfe_dippe_clear(&dippe);

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
