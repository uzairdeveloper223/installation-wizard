/**
 * This code is responsible for testing the dependency checking utilities,
 * including library availability and command availability checks.
 */

#include "../../all.h"

/** Verifies is_library_available() returns 1 for an existing library. */
static void test_is_library_available_returns_true_for_existing(void **state)
{
    (void)state;

    // libc should always be available on any Linux system.
    int result = is_library_available("libc.so.6");

    assert_int_equal(1, result);
}

/** Verifies is_library_available() returns 0 for a non-existent library. */
static void test_is_library_available_returns_false_for_missing(void **state)
{
    (void)state;

    int result = is_library_available("libnonexistent12345.so.99");

    assert_int_equal(0, result);
}

/** Verifies is_command_available() returns 1 for an existing command. */
static void test_is_command_available_returns_true_for_existing(void **state)
{
    (void)state;

    // 'ls' should always be available on any Linux system.
    int result = is_command_available("ls");

    assert_int_equal(1, result);
}

/** Verifies is_command_available() returns 0 for a non-existent command. */
static void test_is_command_available_returns_false_for_missing(void **state)
{
    (void)state;

    int result = is_command_available("nonexistentcommand12345");

    assert_int_equal(0, result);
}

/** Verifies is_command_available() returns 1 for 'sh' (POSIX shell). */
static void test_is_command_available_finds_sh(void **state)
{
    (void)state;

    // 'sh' is required by POSIX and should always exist.
    int result = is_command_available("sh");

    assert_int_equal(1, result);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_is_library_available_returns_true_for_existing),
        cmocka_unit_test(test_is_library_available_returns_false_for_missing),
        cmocka_unit_test(test_is_command_available_returns_true_for_existing),
        cmocka_unit_test(test_is_command_available_returns_false_for_missing),
        cmocka_unit_test(test_is_command_available_finds_sh),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
