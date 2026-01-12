/**
 * This code is responsible for testing the hostname utility module,
 * including chassis detection and hostname generation.
 */

#include "../../all.h"

/** Sets up the test environment before each test. */
static int setup(void **state)
{
    (void)state;
    return 0;
}

/** Cleans up the test environment after each test. */
static int teardown(void **state)
{
    (void)state;
    return 0;
}

/** Verifies generate_hostname() produces correct username-suffix format. */
static void test_generate_hostname_format(void **state)
{
    (void)state;
    char hostname[64];

    generate_hostname("alice", hostname, sizeof(hostname));

    // Should start with username followed by hyphen.
    assert_non_null(strstr(hostname, "alice-"));

    // Should end with valid suffix.
    int valid_suffix = (strstr(hostname, "-laptop") != NULL) ||
                       (strstr(hostname, "-pc") != NULL);
    assert_true(valid_suffix);
}

/** Verifies generate_hostname() works with short usernames. */
static void test_generate_hostname_with_short_username(void **state)
{
    (void)state;
    char hostname[64];

    generate_hostname("a", hostname, sizeof(hostname));

    // Should produce valid hostname even with single-char username.
    assert_true(strlen(hostname) >= 3);
    assert_true(hostname[0] == 'a');
    assert_true(hostname[1] == '-');
}

/** Verifies generate_hostname() respects buffer size limit. */
static void test_generate_hostname_buffer_sizing(void **state)
{
    (void)state;
    char hostname[10];

    generate_hostname("verylongusername", hostname, sizeof(hostname));

    // Output should be truncated to fit buffer.
    assert_true(strlen(hostname) < sizeof(hostname));

    // Should be null-terminated.
    assert_int_equal('\0', hostname[strlen(hostname)]);
}

/** Verifies generate_hostname() handles typical usernames. */
static void test_generate_hostname_typical_username(void **state)
{
    (void)state;
    char hostname[64];

    generate_hostname("john", hostname, sizeof(hostname));

    // Should produce "john-laptop" or "john-pc".
    int is_john_laptop = (strcmp(hostname, "john-laptop") == 0);
    int is_john_pc = (strcmp(hostname, "john-pc") == 0);
    assert_true(is_john_laptop || is_john_pc);
}

/** Verifies get_default_hostname_suffix() returns non-NULL string. */
static void test_get_default_hostname_suffix_returns_string(void **state)
{
    (void)state;

    const char *suffix = get_default_hostname_suffix();

    assert_non_null(suffix);
    assert_true(strlen(suffix) > 0);
}

/** Verifies get_default_hostname_suffix() returns valid value. */
static void test_get_default_hostname_suffix_valid_values(void **state)
{
    (void)state;

    const char *suffix = get_default_hostname_suffix();

    // Should return either "laptop" or "pc".
    int is_laptop = (strcmp(suffix, "laptop") == 0);
    int is_pc = (strcmp(suffix, "pc") == 0);
    assert_true(is_laptop || is_pc);
}

/** Verifies detect_chassis_type() returns valid enum value. */
static void test_detect_chassis_type_returns_valid_enum(void **state)
{
    (void)state;

    ChassisType type = detect_chassis_type();

    // Should return one of the valid enum values.
    int is_valid = (type == CHASSIS_DESKTOP) ||
                   (type == CHASSIS_LAPTOP) ||
                   (type == CHASSIS_UNKNOWN);
    assert_true(is_valid);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_generate_hostname_format, setup, teardown),
        cmocka_unit_test_setup_teardown(test_generate_hostname_with_short_username, setup, teardown),
        cmocka_unit_test_setup_teardown(test_generate_hostname_buffer_sizing, setup, teardown),
        cmocka_unit_test_setup_teardown(test_generate_hostname_typical_username, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_default_hostname_suffix_returns_string, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_default_hostname_suffix_valid_values, setup, teardown),
        cmocka_unit_test_setup_teardown(test_detect_chassis_type_returns_valid_enum, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
