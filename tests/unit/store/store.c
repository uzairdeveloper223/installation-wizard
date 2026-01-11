/**
 * This code is responsible for testing the store module, which manages the
 * global installer state including user selections and installation progress.
 */

#include "../../all.h"

/** Sets up the test environment before each test. */
static int setup(void **state)
{
    (void)state;
    reset_store();
    return 0;
}

/** Cleans up the test environment after each test. */
static int teardown(void **state)
{
    (void)state;
    return 0;
}

/** Verifies that get_store() returns a valid non-null pointer. */
static void test_get_store_returns_non_null(void **state)
{
    (void)state;
    Store *store = get_store();
    assert_non_null(store);
}

/**
 * Verifies that get_store() implements the singleton pattern by returning
 * the same pointer on multiple calls.
 */
static void test_get_store_returns_same_instance(void **state)
{
    (void)state;
    Store *store1 = get_store();
    Store *store2 = get_store();
    assert_ptr_equal(store1, store2);
}

/** Verifies that reset_store() resets current_step to zero. */
static void test_reset_store_clears_current_step(void **state)
{
    (void)state;
    Store *store = get_store();
    store->current_step = 5;
    reset_store();
    assert_int_equal(0, store->current_step);
}

/** Verifies that reset_store() resets dry_run flag to zero. */
static void test_reset_store_clears_dry_run(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    reset_store();
    assert_int_equal(0, store->dry_run);
}

/** Verifies that reset_store() clears the locale string. */
static void test_reset_store_clears_locale(void **state)
{
    (void)state;
    Store *store = get_store();
    strncpy(store->locale, "en_US.UTF-8", STORE_MAX_LOCALE_LEN);
    reset_store();
    assert_string_equal("", store->locale);
}

/** Verifies that reset_store() clears the disk path string. */
static void test_reset_store_clears_disk(void **state)
{
    (void)state;
    Store *store = get_store();
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    reset_store();
    assert_string_equal("", store->disk);
}

/** Verifies that reset_store() resets partition_count to zero. */
static void test_reset_store_clears_partition_count(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 3;
    reset_store();
    assert_int_equal(0, store->partition_count);
}

/** Verifies that reset_store() zeroes all partition structure fields. */
static void test_reset_store_clears_partitions(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partitions[0].size_bytes = 1000000;
    store->partitions[0].flag_boot = 1;
    strncpy(store->partitions[0].mount_point, "/boot", STORE_MAX_MOUNT_LEN);

    reset_store();

    assert_int_equal(0, (int)store->partitions[0].size_bytes);
    assert_int_equal(0, store->partitions[0].flag_boot);
    assert_string_equal("", store->partitions[0].mount_point);
}

/** Verifies the locale field can hold a maximum-length string. */
static void test_store_locale_max_length(void **state)
{
    (void)state;
    Store *store = get_store();

    // Fill locale to maximum capacity (63 chars + null terminator).
    char max_locale[STORE_MAX_LOCALE_LEN];
    memset(max_locale, 'a', STORE_MAX_LOCALE_LEN - 1);
    max_locale[STORE_MAX_LOCALE_LEN - 1] = '\0';

    strncpy(store->locale, max_locale, STORE_MAX_LOCALE_LEN);

    assert_int_equal(STORE_MAX_LOCALE_LEN - 1, (int)strlen(store->locale));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_get_store_returns_non_null, setup, teardown),
        cmocka_unit_test_setup_teardown(test_get_store_returns_same_instance, setup, teardown),
        cmocka_unit_test_setup_teardown(test_reset_store_clears_current_step, setup, teardown),
        cmocka_unit_test_setup_teardown(test_reset_store_clears_dry_run, setup, teardown),
        cmocka_unit_test_setup_teardown(test_reset_store_clears_locale, setup, teardown),
        cmocka_unit_test_setup_teardown(test_reset_store_clears_disk, setup, teardown),
        cmocka_unit_test_setup_teardown(test_reset_store_clears_partition_count, setup, teardown),
        cmocka_unit_test_setup_teardown(test_reset_store_clears_partitions, setup, teardown),
        cmocka_unit_test_setup_teardown(test_store_locale_max_length, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
