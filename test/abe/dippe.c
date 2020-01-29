
#include <amcl/pair_BN254.h>
#include "cifer/abe/dippe.h"
#include "cifer/data/vec.h"
#include "cifer/test.h"

MunitResult test_dippe_end_to_end_conjunction(const MunitParameter *params, void *data) {
    cfe_error err;

    // Init DIPPE scheme
    cfe_dippe dippe;
    cfe_dippe_init(&dippe, 2);

    // Setup two Authorities
    cfe_dippe_pub_key pk[2];
    cfe_dippe_sec_key sk[2];
    for (int i = 0; i < 2; i++) {
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

    // Length of attribute and policy vectors
    size_t vlen = 6;

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
    size_t pattern[] = {0, 1, 4};
    cfe_vec pv;
    err = cfe_dippe_conjunction_policy_vector_init(&pv, &dippe, (vlen - 1), pattern,
                                                   (sizeof(pattern) / sizeof(size_t)));
    munit_assert(err == CFE_ERR_NONE);

    // Test patterns
    size_t ap0[] = {0, 1, 3, 4};     // "11011" - valid
    size_t ap1[] = {0, 1, 2, 3, 4};  // "11111" - valid
    size_t ap2[] = {1, 4};           // "01001" - invalid
    size_t ap3[] = {0, 1, 3};        // "11010" - invalid

    size_t *aps[] = {ap0, ap1, ap2, ap3};
    size_t aps_len[] = {
            sizeof(ap0) / sizeof(size_t),
            sizeof(ap1) / sizeof(size_t),
            sizeof(ap2) / sizeof(size_t),
            sizeof(ap3) / sizeof(size_t)};

    // Test results
    int decryption_results[] = {1, 1, 0, 0};

    cfe_dippe_cipher cipher;
    cfe_dippe_cipher_init(&cipher, &dippe, pv.size);

    // Encrypt message under given policy vector
    err = cfe_dippe_encrypt(&cipher, &dippe, pks, (sizeof(pks) / sizeof(cfe_dippe_pub_key *)), &pv, &msg);
    munit_assert(err == CFE_ERR_NONE);

    // Decrypt message
    FP12_BN254 result;

    // Collection of user secret keys
    cfe_dippe_user_sec_key usks[vlen];
    for (size_t i = 0; i < vlen; i++) {
        cfe_dippe_user_sec_key_init(&(usks[i]), &dippe);
    }

    cfe_vec av;
    char gid[9];
    for (size_t i = 0; i < (sizeof(aps) / sizeof(size_t *)); i++) {
        sprintf(gid, "TESTGID%zu", i);
        size_t gid_len = 8; // lendth of GID string

        // Attribute vector
        err = cfe_dippe_attribute_vector_init(&av, (vlen - 1), aps[i], aps_len[i]);
        munit_assert(err == CFE_ERR_NONE);

        // User secret keys
        for (size_t j = 0; j < vlen; j++) {
            err = cfe_dippe_keygen(&(usks[j]), &dippe, j, pks, (sizeof(pks) / sizeof(cfe_dippe_pub_key *)),
                                   &sk[(j & 1)], &av, gid, gid_len);
            munit_assert(err == CFE_ERR_NONE);
        }

        // Decrypt message
        err = cfe_dippe_decrypt(&result, &dippe, (cfe_dippe_user_sec_key *) &usks,
                                (sizeof(usks) / sizeof(cfe_dippe_user_sec_key)), &cipher, &av, gid, gid_len);
        munit_assert(err == CFE_ERR_NONE);

        // Check decryption
        munit_assert(FP12_BN254_equals(&msg, &result) == decryption_results[i]);

        // Cleanup
        cfe_vec_free(&av);
    }

    // Cleanup
    for (int i = 0; i < 2; i++) {
        cfe_dippe_sec_key_free(&(sk[i]));
        cfe_dippe_pub_key_free(&(pk[i]));
    }
    for (size_t i = 0; i < vlen; i++) {
        cfe_dippe_user_sec_key_free(&(usks[i]));
    }
    cfe_dippe_cipher_free(&cipher);
    cfe_dippe_free(&dippe);
    cfe_vec_free(&pv);

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

    // Length of attribute and policy vectors
    size_t vlen = 5;

    // Policy vector (one additional component is added)
    // t is the threshold value
    //    ------            -----
    //    Policy            Attrs
    //    ------            -----
    //     1                 1
    //     1                 1
    //     0         .       0      =  (1+1+0+0-t) = 0
    //     1                 0
    // -> -t                 1
    cfe_vec pv;
    size_t pattern[] = {0, 1, 3};
    err = cfe_dippe_exact_threshold_policy_vector_init(&pv, &dippe, (vlen - 1), pattern,
                                                       (sizeof(pattern) / sizeof(size_t)), 2);
    munit_assert(err == CFE_ERR_NONE);

    // Test patterns
    size_t ap0[] = {0, 1};        // "1100" - valid
    size_t ap1[] = {0, 3};        // "1001" - valid
    size_t ap2[] = {1, 3};        // "0101" - valid
    size_t ap3[] = {1, 2, 3};     // "0111" - valid
    size_t ap4[] = {0, 1, 3};     // "1101" - invalid
    size_t ap5[] = {0, 1, 2, 3};  // "1111" - invalid

    size_t *aps[] = {ap0, ap1, ap2, ap3, ap4, ap5};
    size_t aps_len[] = {
            sizeof(ap0) / sizeof(size_t),
            sizeof(ap1) / sizeof(size_t),
            sizeof(ap2) / sizeof(size_t),
            sizeof(ap3) / sizeof(size_t),
            sizeof(ap4) / sizeof(size_t),
            sizeof(ap5) / sizeof(size_t)};

    // Test results
    int decryption_results[] = {1, 1, 1, 1, 0, 0};

    cfe_dippe_cipher cipher;
    cfe_dippe_cipher_init(&cipher, &dippe, pv.size);

    // Encrypt message under given policy vector
    err = cfe_dippe_encrypt(&cipher, &dippe, pks, (sizeof(pks) / sizeof(cfe_dippe_pub_key *)), &pv, &msg);
    munit_assert(err == CFE_ERR_NONE);

    // Decrypt message
    FP12_BN254 result;

    // Collection of user secret keys
    cfe_dippe_user_sec_key usks[vlen];
    for (size_t i = 0; i < vlen; i++) {
        cfe_dippe_user_sec_key_init(&(usks[i]), &dippe);
    }

    cfe_vec av;
    char gid[9];

    for (size_t i = 0; i < (sizeof(aps) / sizeof(size_t *)); i++) {
        sprintf(gid, "TESTGID%zu", i);
        size_t gid_len = 8; // lendth of GID string

        // Attribute vector
        err = cfe_dippe_attribute_vector_init(&av, (vlen - 1), aps[i], aps_len[i]);
        munit_assert(err == CFE_ERR_NONE);

        // User secret keys from Auth0
        for (size_t j = 0; j < vlen; j++) {
            err = cfe_dippe_keygen(&(usks[j]), &dippe, j, pks, (sizeof(pks) / sizeof(cfe_dippe_pub_key *)), &sk0, &av,
                                   gid, 8);
            munit_assert(err == CFE_ERR_NONE);
        }

        // Decrypt message
        err = cfe_dippe_decrypt(&result, &dippe, (cfe_dippe_user_sec_key *) &usks,
                                (sizeof(usks) / sizeof(cfe_dippe_user_sec_key)), &cipher, &av, gid, gid_len);
        munit_assert(err == CFE_ERR_NONE);

        // Check decryption
        munit_assert(FP12_BN254_equals(&msg, &result) == decryption_results[i]);

        // Cleanup
        cfe_vec_free(&av);
    }

    // Cleanup
    for (size_t i = 0; i < vlen; i++) {
        cfe_dippe_user_sec_key_free(&(usks[i]));
    }
    cfe_dippe_sec_key_free(&sk0);
    cfe_dippe_pub_key_free(&pk0);
    cfe_dippe_cipher_free(&cipher);
    cfe_dippe_free(&dippe);
    cfe_vec_free(&pv);

    return MUNIT_OK;
}

MunitTest dippe_tests[] = {
        {(char *) "/end-to-end-conjunction", test_dippe_end_to_end_conjunction, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {(char *) "/end-to-end-threshold",   test_dippe_end_to_end_threshold,   NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
        {NULL, NULL,                                                            NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}};

MunitSuite dippe_suite = {
        (char *) "/abe/dippe", dippe_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE};
