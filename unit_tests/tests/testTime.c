#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include <stdint.h>  // uint*_t

#include <cmocka.h>

#include "time.h"

static void test_prefixWithZero_does_prefix_1() {
    uint8_t text[4]  = { '1' };
    int didPrefix = prefixWithZero(text, 1);
    assert_int_equal(didPrefix, 1);
    assert_string_equal(text, "0");
}

static void test_prefixWithZero_does_not_prefix_2001() {
    uint8_t text[1]  = { '1' };
    int didPrefix = prefixWithZero(text, 2001);
    assert_int_equal(didPrefix, 0);
    assert_string_equal(text, "1");
}

static void test_seconds_to_display_0() {
    uint8_t display[30];
    tm t;
    long long time = 0;
    secondsToTm(time, &t);
    timeToDisplayText(t, display);
    assert_string_equal(display, "1970-01-01 00:00:00");
}

static void test_seconds_to_display_2021() {
    uint8_t display[30];
    tm t;
    long long time = 1633526457;
    secondsToTm(time, &t);
    timeToDisplayText(t, display);
    assert_string_equal(display, "2021-10-06 13:20:57");
}

static void test_seconds_to_display_2030() {
    uint8_t display[30];
    tm t;
    long long time = 1905426279;
    secondsToTm(time, &t);
    timeToDisplayText(t, display);
    assert_string_equal(display, "2030-05-19 13:04:39");
}

static void test_secondsToTm_overflow() {
    tm t;
    long long time = 1 + INT_MAX * 31622400LL;
    int result = secondsToTm(time, &t);
    assert_int_equal(result, -1);
    time = -1 - INT_MIN * 31622400LL;
    result = secondsToTm(time, &t);
    assert_int_equal(result, -1);
}

static void test_memmove() {
    uint8_t text[3];
    text[2] = '1';
    memmove(text, "No", 3);
    assert_int_equal(text[2], 0);
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_prefixWithZero_does_prefix_1),
        cmocka_unit_test(test_prefixWithZero_does_not_prefix_2001),
        cmocka_unit_test(test_seconds_to_display_0),
        cmocka_unit_test(test_seconds_to_display_2021),
        cmocka_unit_test(test_seconds_to_display_2030),
        cmocka_unit_test(test_secondsToTm_overflow),
        cmocka_unit_test(test_memmove),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
