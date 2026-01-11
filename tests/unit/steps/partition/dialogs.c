/**
 * This code is responsible for testing the partition dialog helper functions.
 * The helper functions are exposed via semistatic when TESTING is defined.
 */

#include "../../../all.h"

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

/** Verifies find_closest_size_index() returns 0 for minimum size. */
static void test_find_closest_size_index_minimum(void **state)
{
    (void)state;

    int result = find_closest_size_index(1ULL * 1000000); /* 1MB */

    assert_int_equal(0, result);
}

/** Verifies find_closest_size_index() returns correct index for exact match. */
static void test_find_closest_size_index_exact_match(void **state)
{
    (void)state;

    // 512MB is at index 8
    int result = find_closest_size_index(512ULL * 1000000);

    assert_int_equal(8, result);
}

/** Verifies find_closest_size_index() returns last index for very large size. */
static void test_find_closest_size_index_maximum(void **state)
{
    (void)state;

    // 2TB should return max index (SIZE_COUNT - 1 = 18)
    int result = find_closest_size_index(2000ULL * 1000000000);

    assert_int_equal(18, result);
}

/** Verifies find_closest_size_index() rounds to closest preset (lower). */
static void test_find_closest_size_index_rounds_down(void **state)
{
    (void)state;

    // 300MB is between 128MB (index 7) and 512MB (index 8).
    // 300 - 128 = 172, 512 - 300 = 212, so rounds to 128MB = index 7
    int result = find_closest_size_index(300ULL * 1000000);

    assert_int_equal(7, result);
}

/** Verifies find_closest_size_index() rounds to closest preset (upper). */
static void test_find_closest_size_index_rounds_up(void **state)
{
    (void)state;

    // 400MB is between 128MB and 512MB.
    // 400 - 128 = 272, 512 - 400 = 112, so rounds to 512MB = index 8
    int result = find_closest_size_index(400ULL * 1000000);

    assert_int_equal(8, result);
}

/** Verifies find_closest_size_index() handles zero size. */
static void test_find_closest_size_index_zero(void **state)
{
    (void)state;

    int result = find_closest_size_index(0);

    // Should return 0 (1MB preset).
    assert_int_equal(0, result);
}

/** Verifies find_closest_size_index() handles 1GB. */
static void test_find_closest_size_index_1gb(void **state)
{
    (void)state;

    // 1GB is at index 9
    int result = find_closest_size_index(1ULL * 1000000000);

    assert_int_equal(9, result);
}

/** Verifies find_closest_size_index() handles 8GB. */
static void test_find_closest_size_index_8gb(void **state)
{
    (void)state;

    // 8GB is at index 12
    int result = find_closest_size_index(8ULL * 1000000000);

    assert_int_equal(12, result);
}

/** Verifies find_closest_size_index() handles size between 1GB and 2GB. */
static void test_find_closest_size_index_1_5gb(void **state)
{
    (void)state;

    // 1.5GB is between 1GB (index 9) and 2GB (index 10).
    // 1.5 - 1 = 0.5, 2 - 1.5 = 0.5, exact middle, should round to higher.
    int result = find_closest_size_index(1500ULL * 1000000);

    assert_int_equal(10, result);
}

/** Verifies find_closest_size_index() handles 2MB exact. */
static void test_find_closest_size_index_2mb(void **state)
{
    (void)state;

    int result = find_closest_size_index(2ULL * 1000000);

    assert_int_equal(1, result);
}

/** Verifies find_mount_index() returns 0 for root mount point. */
static void test_find_mount_index_root(void **state)
{
    (void)state;

    int result = find_mount_index("/");

    assert_int_equal(0, result);
}

/** Verifies find_mount_index() returns 1 for /boot mount point. */
static void test_find_mount_index_boot(void **state)
{
    (void)state;

    int result = find_mount_index("/boot");

    assert_int_equal(1, result);
}

/** Verifies find_mount_index() returns 2 for /home mount point. */
static void test_find_mount_index_home(void **state)
{
    (void)state;

    int result = find_mount_index("/home");

    assert_int_equal(2, result);
}

/** Verifies find_mount_index() returns 3 for /var mount point. */
static void test_find_mount_index_var(void **state)
{
    (void)state;

    int result = find_mount_index("/var");

    assert_int_equal(3, result);
}

/** Verifies find_mount_index() returns 4 for [swap] mount point. */
static void test_find_mount_index_swap_bracket(void **state)
{
    (void)state;

    int result = find_mount_index("[swap]");

    assert_int_equal(4, result);
}

/** Verifies find_mount_index() returns 5 for [none] mount point. */
static void test_find_mount_index_none_bracket(void **state)
{
    (void)state;

    int result = find_mount_index("[none]");

    assert_int_equal(5, result);
}

/** Verifies find_mount_index() returns 0 for unknown mount point. */
static void test_find_mount_index_unknown(void **state)
{
    (void)state;

    int result = find_mount_index("/unknown");

    // Falls back to root (0).
    assert_int_equal(0, result);
}

/** Verifies find_mount_index() handles "swap" without brackets. */
static void test_find_mount_index_swap_option(void **state)
{
    (void)state;

    int result = find_mount_index("swap");

    // This matches mount_options[4] = "swap".
    assert_int_equal(4, result);
}

/** Verifies find_mount_index() handles "none" without brackets. */
static void test_find_mount_index_none_option(void **state)
{
    (void)state;

    int result = find_mount_index("none");

    // This matches mount_options[5] = "none".
    assert_int_equal(5, result);
}

/** Verifies find_mount_index() handles empty string. */
static void test_find_mount_index_empty(void **state)
{
    (void)state;

    int result = find_mount_index("");

    // Falls back to root (0).
    assert_int_equal(0, result);
}

/** Verifies find_flag_index() returns 0 for no flags. */
static void test_find_flag_index_none(void **state)
{
    (void)state;

    int result = find_flag_index(0, 0, 0);

    assert_int_equal(0, result);
}

/** Verifies find_flag_index() returns 1 for boot flag. */
static void test_find_flag_index_boot(void **state)
{
    (void)state;

    int result = find_flag_index(1, 0, 0);

    assert_int_equal(1, result);
}

/** Verifies find_flag_index() returns 2 for ESP flag. */
static void test_find_flag_index_esp(void **state)
{
    (void)state;

    int result = find_flag_index(0, 1, 0);

    assert_int_equal(2, result);
}

/** Verifies find_flag_index() returns 3 for BIOS GRUB flag. */
static void test_find_flag_index_bios_grub(void **state)
{
    (void)state;

    int result = find_flag_index(0, 0, 1);

    assert_int_equal(3, result);
}

/** Verifies find_flag_index() prioritizes boot over esp. */
static void test_find_flag_index_boot_priority(void **state)
{
    (void)state;

    // When multiple flags are set, boot has priority.
    int result = find_flag_index(1, 1, 0);

    assert_int_equal(1, result);
}

/** Verifies find_flag_index() prioritizes esp over bios_grub. */
static void test_find_flag_index_esp_priority(void **state)
{
    (void)state;

    // When esp and bios_grub are set, esp has priority.
    int result = find_flag_index(0, 1, 1);

    assert_int_equal(2, result);
}

/** Verifies find_flag_index() prioritizes boot over all. */
static void test_find_flag_index_boot_over_all(void **state)
{
    (void)state;

    // When all flags are set, boot has priority.
    int result = find_flag_index(1, 1, 1);

    assert_int_equal(1, result);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        // find_closest_size_index tests
        cmocka_unit_test_setup_teardown(test_find_closest_size_index_minimum, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_closest_size_index_exact_match, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_closest_size_index_maximum, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_closest_size_index_rounds_down, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_closest_size_index_rounds_up, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_closest_size_index_zero, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_closest_size_index_1gb, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_closest_size_index_8gb, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_closest_size_index_1_5gb, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_closest_size_index_2mb, setup, teardown),

        // find_mount_index tests
        cmocka_unit_test_setup_teardown(test_find_mount_index_root, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_mount_index_boot, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_mount_index_home, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_mount_index_var, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_mount_index_swap_bracket, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_mount_index_none_bracket, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_mount_index_unknown, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_mount_index_swap_option, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_mount_index_none_option, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_mount_index_empty, setup, teardown),

        // find_flag_index tests
        cmocka_unit_test_setup_teardown(test_find_flag_index_none, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_flag_index_boot, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_flag_index_esp, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_flag_index_bios_grub, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_flag_index_boot_priority, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_flag_index_esp_priority, setup, teardown),
        cmocka_unit_test_setup_teardown(test_find_flag_index_boot_over_all, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
