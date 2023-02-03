#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdint.h>  // uint*_t
#include <string.h>
#include <cmocka.h>

#include "base58check.h"

static void test_base58_encode() {
    unsigned char input[32] = {0x19, 0x38, 0x8c, 0x7a, 0x4e, 0x6c, 0x67, 0xec, 0xf8, 0x1b, 0xe8,
                               0x80, 0x47, 0x8d, 0x6b, 0x30, 0x04, 0x55, 0xdc, 0x85, 0x4a, 0x5d,
                               0xc9, 0xc3, 0x55, 0xfe, 0x5a, 0x85, 0xfd, 0xf2, 0xdc, 0xa1};
    unsigned char out[57];
    size_t outLen = sizeof(out);
    int result = base58check_encode(input, sizeof(input), out, &outLen);
    assert_int_equal(result, 0);
    assert_memory_equal(out, "38rQoCqvUk fVQ1fTwVPL BgjLkkZ8x7 9HozGYsWsZ mtCyipiMnp", 50);
}

static void test_base58_encode_fails_on_too_short_output_length() {
    unsigned char input[32] = {0x19, 0x38, 0x8c, 0x7a, 0x4e, 0x6c, 0x67, 0xec, 0xf8, 0x1b, 0xe8,
                               0x80, 0x47, 0x8d, 0x6b, 0x30, 0x04, 0x55, 0xdc, 0x85, 0x4a, 0x5d,
                               0xc9, 0xc3, 0x55, 0xfe, 0x5a, 0x85, 0xfd, 0xf2, 0xdc, 0xa1};
    unsigned char out[57];
    size_t outLen = 20;
    int result = base58check_encode(input, sizeof(input), out, &outLen);
    assert_int_equal(result, -1);
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_base58_encode),
        cmocka_unit_test(test_base58_encode_fails_on_too_short_output_length)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}
