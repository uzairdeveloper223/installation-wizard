/**
 * This code is responsible for testing the rootfs extraction module.
 */

#include "../../all.h"

/**
 * Sets up the test environment before each test.
 */
static int setup(void **state)
{
    (void)state;
    reset_store();
    return 0;
}

/**
 * Cleans up the test environment after each test.
 */
static int teardown(void **state)
{
    (void)state;
    return 0;
}

/**
 * Placeholder test for extract_rootfs().
 */
static void test_extract_rootfs_placeholder(void **state)
{
    // TODO: Implement actual tests for extract_rootfs().
    (void)state;
    assert_true(1);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_extract_rootfs_placeholder, setup, teardown),
        // TODO: Add more tests here.
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
