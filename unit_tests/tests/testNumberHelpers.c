#include <cmocka.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stddef.h>  // size_t
#include <stdint.h>
#include <stdint.h>  // uint*_t
#include <string.h>

#include "numberHelpers.c"
#include "responseCodes.h"

static void test_lengthOfNumbers() {
    assert_int_equal(lengthOfNumber(0), 1);
    assert_int_equal(lengthOfNumber(1), 1);
    assert_int_equal(lengthOfNumber(9), 1);
    assert_int_equal(lengthOfNumber(10), 2);
    assert_int_equal(lengthOfNumber(11), 2);
    assert_int_equal(lengthOfNumber(57), 2);
    assert_int_equal(lengthOfNumber(99), 2);
    assert_int_equal(lengthOfNumber(100), 3);
    assert_int_equal(lengthOfNumber(101), 3);
    assert_int_equal(lengthOfNumber(369), 3);
    assert_int_equal(lengthOfNumber(999), 3);
    assert_int_equal(lengthOfNumber(1000), 4);
    assert_int_equal(lengthOfNumber(1001), 4);
    assert_int_equal(lengthOfNumber(32768L), 5);
    assert_int_equal(lengthOfNumber(18446744073709551615ULL), 20);
}

static void test_numberToText() {
    uint8_t text[4];
    numberToText(text, sizeof(text), 2001);
    assert_string_equal(text, "2001");
    numberToText(text, sizeof(text), 4041);
    assert_string_equal(text, "4041");
}

static void test_numberToText_max() {
    uint8_t text[21];
    int size = numberToText(text, sizeof(text) - 1, 18446744073709551615ULL);
    assert_int_equal(size, 20);
    text[20] = '\0';
    assert_string_equal(text, "18446744073709551615");
}

static void test_numberToText_zero() {
    uint8_t text[1];
    numberToText(text, sizeof(text), 0);
    assert_string_equal(text, "0");
}

static void test_numberToText_shorter_number() {
    uint8_t text[4];

    numberToText(text, sizeof(text), 46);
    assert_string_equal(text, "46");

    numberToText(text, sizeof(text), 100);
    assert_string_equal(text, "100");

    numberToText(text, sizeof(text), 23);
    assert_string_equal(text, "230");

    numberToText(text, sizeof(text), 1);
    assert_string_equal(text, "130");
}

static void test_numberToText_longer_number() {
    uint8_t text[4];
    // This should fail, because the text is not long enough
    int result = numberToText(text, sizeof(text), 12041);
    assert_int_equal(result, ERROR_BUFFER_OVERFLOW);
}

static void test_bin2dec() {
    uint8_t text[5];
    bin2dec(text, sizeof(text), 0);
    assert_string_equal(text, "0");
    bin2dec(text, sizeof(text), 2001);
    assert_string_equal(text, "2001");
    bin2dec(text, sizeof(text), 4041);
    assert_string_equal(text, "4041");
}

static void test_bin2dec_max() {
    uint8_t text[21];
    bin2dec(text, sizeof(text), 18446744073709551615ULL);
    assert_string_equal(text, "18446744073709551615");
}

static void test_bin2dec_shorter_number() {
    uint8_t text[4];
    bin2dec(text, sizeof(text), 46);
    assert_string_equal(text, "46");

    bin2dec(text, sizeof(text), 100);
    assert_string_equal(text, "100");

    bin2dec(text, sizeof(text), 23);
    assert_string_equal(text, "23");

    bin2dec(text, sizeof(text), 1);
    assert_string_equal(text, "1");

    bin2dec(text, sizeof(text), 0);
    assert_string_equal(text, "0");
}

static void test_bin2dec_longer_number() {
    uint8_t text[4];
    // This should fail, because the text is not long enough to contain 1204 and a termination character
    int result = bin2dec(text, sizeof(text), 1204);
    assert_int_equal(result, ERROR_BUFFER_OVERFLOW);

    result = bin2dec(text, sizeof(text), 120400);
    assert_int_equal(result, ERROR_BUFFER_OVERFLOW);
}

static void test_decimalAmountToGtuDisplay() {
    uint8_t text[6];
    decimalAmountToGtuDisplay(text, sizeof(text), 2100112);
    assert_string_equal(text, "100112");
    decimalAmountToGtuDisplay(text, sizeof(text), 4041);
    assert_string_equal(text, "004041");
    decimalAmountToGtuDisplay(text, sizeof(text), 1);
    assert_string_equal(text, "000001");
    decimalAmountToGtuDisplay(text, sizeof(text), 0);
    assert_string_equal(text, "000000");
}

static void test_amountToGtuDisplay_zero() {
    uint8_t text[20];
    amountToGtuDisplay(text, sizeof(text), 0);
    assert_string_equal(text, "0");
}

static void test_amountToGtuDisplay_no_microGtu() {
    uint8_t text[20];
    amountToGtuDisplay(text, sizeof(text), 105000000);
    assert_string_equal(text, "105");
    amountToGtuDisplay(text, sizeof(text), 27000000);
    assert_string_equal(text, "27");
    amountToGtuDisplay(text, sizeof(text), 1000000);
    assert_string_equal(text, "1");
}

static void test_amountToGtuDisplay_only_microGtu() {
    uint8_t text[20];
    amountToGtuDisplay(text, sizeof(text), 5400);
    assert_string_equal(text, "0.0054");
    amountToGtuDisplay(text, sizeof(text), 1);
    assert_string_equal(text, "0.000001");
    amountToGtuDisplay(text, sizeof(text), 4041);
    assert_string_equal(text, "0.004041");
    amountToGtuDisplay(text, sizeof(text), 112428);
    assert_string_equal(text, "0.112428");
}

static void test_amountToGtuDisplay_mixed() {
    uint8_t text[20];
    amountToGtuDisplay(text, sizeof(text), 2100112);
    assert_string_equal(text, "2.100112");
    amountToGtuDisplay(text, sizeof(text), 5001000);
    assert_string_equal(text, "5.001");
}

static void test_amountToGtuDisplay_with_thousand_separator() {
    uint8_t text[21];
    amountToGtuDisplay(text, sizeof(text), 1200000000);
    assert_string_equal(text, "1,200");
    amountToGtuDisplay(text, sizeof(text), 720005200122);
    assert_string_equal(text, "720,005.200122");
    amountToGtuDisplay(text, sizeof(text), 1111111111111111);
    assert_string_equal(text, "1,111,111,111.111111");
}

static void test_amountToGtuDisplay_max() {
    uint8_t text[26];
    int result = amountToGtuDisplay(text, sizeof(text), 18446744073709551615ULL);
    assert_int_not_equal(result, -1);
    assert_string_equal(text, "18,446,744,073,709.551615");
}

static void test_toPaginatedHex() {
    char text[70];
    uint8_t bytes[] = {171, 34, 31, 72, 83, 171, 34, 29, 72, 83, 34, 29, 31, 72, 34, 29, 31, 72, 34, 29, 31, 72};
    toPaginatedHex(bytes, 22, text, sizeof(text));
    assert_string_equal(text, "ab221f4853ab221d 4853221d1f48221d 1f48221d1f48");
}

static void test_toPaginatedHex_stops_after_given_length() {
    char text[12];
    text[11] = 100;
    uint8_t bytes[] = {171, 34, 31, 72, 83, 170, 34, 1, 1, 1};
    toPaginatedHex(bytes, 5, text, sizeof(text));
    assert_string_equal(text, "ab221f4853");
    assert_int_equal(text[10], '\0');
    assert_int_equal(text[11], 100);
}

static void test_toPaginatedHex_does_not_have_trailing_space() {
    char text[18];
    uint8_t bytes[] = {171, 34, 31, 72, 83, 171, 34, 29};
    toPaginatedHex(bytes, 8, text, sizeof(text));
    assert_string_equal(text, "ab221f4853ab221d");
    assert_int_equal(text[16], '\0');
}

static void test_toPaginatedHex_inserts_white_space() {
    char text[20];
    uint8_t bytes[] = {171, 34, 31, 72, 83, 171, 34, 29, 171};
    toPaginatedHex(bytes, 9, text, sizeof(text));
    assert_string_equal(text, "ab221f4853ab221d ab");
    assert_int_equal(text[19], '\0');
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_lengthOfNumbers),
        cmocka_unit_test(test_numberToText),
        cmocka_unit_test(test_numberToText_zero),
        cmocka_unit_test(test_numberToText_max),
        cmocka_unit_test(test_numberToText_shorter_number),
        cmocka_unit_test(test_numberToText_longer_number),
        cmocka_unit_test(test_bin2dec),
        cmocka_unit_test(test_bin2dec_max),
        cmocka_unit_test(test_bin2dec_shorter_number),
        cmocka_unit_test(test_bin2dec_longer_number),
        cmocka_unit_test(test_decimalAmountToGtuDisplay),
        cmocka_unit_test(test_amountToGtuDisplay_only_microGtu),
        cmocka_unit_test(test_amountToGtuDisplay_no_microGtu),
        cmocka_unit_test(test_amountToGtuDisplay_with_thousand_separator),
        cmocka_unit_test(test_amountToGtuDisplay_mixed),
        cmocka_unit_test(test_amountToGtuDisplay_zero),
        cmocka_unit_test(test_amountToGtuDisplay_max),
        cmocka_unit_test(test_toPaginatedHex),
        cmocka_unit_test(test_toPaginatedHex_stops_after_given_length),
        cmocka_unit_test(test_toPaginatedHex_does_not_have_trailing_space),
        cmocka_unit_test(test_toPaginatedHex_inserts_white_space),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
