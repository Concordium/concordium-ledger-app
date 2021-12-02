#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>  // uint*_t
#include <string.h>
#include <cmocka.h>

#include "globals.h"
#include "util.h"

static keyDerivationPath_t *keyPath = &path;

static void test_parseKeyDerivationPath() {
    explicit_bzero(keyPath->rawKeyDerivationPath, 8);
    uint8_t input[37] = {0x08, 0xb4, 0xd5, 0xbc, 0xd3, 0xce, 0x78, 0x89, 0xe5, 0x51, 0x2e, 0x0d, 0x43,
                         0xd2, 0x9e, 0xb2, 0x9a, 0x8e, 0xbf, 0x10, 0xc0, 0x98, 0x47, 0x56, 0xa3, 0x38,
                         0x2b, 0x71, 0x22, 0xeb, 0x79, 0xfe, 0x5a, 0xb8, 0x80, 0x23, 0xe6};
    uint32_t path0 = 0xb4d5bcd3;
    uint32_t path1 = 0xce7889e5;
    uint32_t path2 = 0x512e0d43;
    uint32_t path3 = 0xd29eb29a;
    uint32_t path4 = 0x8ebf10c0;
    uint32_t path5 = 0x984756a3;
    uint32_t path6 = 0x382b7122;
    uint32_t path7 = 0xeb79fe5a;
    uint32_t path8 = 0xb88023e6;
    int result = parseKeyDerivationPath(input);
    assert_int_equal(result, 33);
    assert_int_equal(keyPath->rawKeyDerivationPath[0], path0);
    assert_int_equal(keyPath->rawKeyDerivationPath[1], path1);
    assert_int_equal(keyPath->rawKeyDerivationPath[2], path2);
    assert_int_equal(keyPath->rawKeyDerivationPath[3], path3);
    assert_int_equal(keyPath->rawKeyDerivationPath[4], path4);
    assert_int_equal(keyPath->rawKeyDerivationPath[5], path5);
    assert_int_equal(keyPath->rawKeyDerivationPath[6], path6);
    assert_int_equal(keyPath->rawKeyDerivationPath[7], path7);
    assert_int_not_equal(keyPath->rawKeyDerivationPath[8], path8);
}

static void test_parseKeyDerivationPath_hardened() {
    explicit_bzero(keyPath->keyDerivationPath, 8);
    uint8_t input[37] = {0x08, 0xb4, 0xd5, 0xbc, 0xd3, 0xce, 0x78, 0x89, 0xe5, 0x51, 0x2e, 0x0d, 0x43,
                         0xd2, 0x9e, 0xb2, 0x9a, 0x8e, 0xbf, 0x10, 0xc0, 0x98, 0x47, 0x56, 0xa3, 0x38,
                         0x2b, 0x71, 0x22, 0xeb, 0x79, 0xfe, 0x5a, 0xb8, 0x80, 0x23, 0xe6};
    uint32_t path0 = 0xb4d5bcd3;
    uint32_t path1 = 0xce7889e5;
    uint32_t path2 = 0xd12e0d43;  // 8 | 5 = d
    uint32_t path3 = 0xd29eb29a;
    uint32_t path4 = 0x8ebf10c0;
    uint32_t path5 = 0x984756a3;
    uint32_t path6 = 0xb82b7122;  // 8 | 3 = b
    uint32_t path7 = 0xeb79fe5a;
    uint32_t path8 = 0xb88023e6;
    int result = parseKeyDerivationPath(input);
    assert_int_equal(result, 33);
    assert_int_equal(keyPath->keyDerivationPath[0], path0);
    assert_int_equal(keyPath->keyDerivationPath[1], path1);
    assert_int_equal(keyPath->keyDerivationPath[2], path2);
    assert_int_equal(keyPath->keyDerivationPath[3], path3);
    assert_int_equal(keyPath->keyDerivationPath[4], path4);
    assert_int_equal(keyPath->keyDerivationPath[5], path5);
    assert_int_equal(keyPath->keyDerivationPath[6], path6);
    assert_int_equal(keyPath->keyDerivationPath[7], path7);
    assert_int_not_equal(keyPath->keyDerivationPath[8], path8);
}

static void test_parseKeyDerivationPath_fails_too_long_path() {
    // We want to check that the function throws with the ERROR_INVALID_PATH code.
    // THROW should have been mocked to make the function return the error.
    explicit_bzero(keyPath->keyDerivationPath, 8);
    uint8_t input[37] = {0x09, 0xb4, 0xd5, 0xbc, 0xd3, 0xce, 0x78, 0x89, 0xe5, 0x51, 0x2e, 0x0d, 0x43,
                         0xd2, 0x9e, 0xb2, 0x9a, 0x8e, 0xbf, 0x10, 0xc0, 0x98, 0x47, 0x56, 0xa3, 0x38,
                         0x2b, 0x71, 0x22, 0xeb, 0x79, 0xfe, 0x5a, 0xb8, 0x80, 0x23, 0xe6};
    int result = parseKeyDerivationPath(input);
    uint32_t path0 = 0xb4d5bcd3;
    assert_int_equal(result, 0x6B02);
    assert_int_not_equal(keyPath->keyDerivationPath[0], path0);
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_parseKeyDerivationPath),
        cmocka_unit_test(test_parseKeyDerivationPath_hardened),
        cmocka_unit_test(test_parseKeyDerivationPath_fails_too_long_path),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
