/**
 * This code is responsible for testing the confirmation step validation
 * functions. The validation functions are exposed via semistatic
 * when TESTING is defined.
 */

#include "../../../all.h"

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

/** Verifies has_root_partition() returns 0 when no partitions exist. */
static void test_has_root_partition_empty(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 0;

    int result = has_root_partition(store);

    assert_int_equal(0, result);
}

/** Verifies has_root_partition() returns 1 when root partition exists. */
static void test_has_root_partition_exists(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = has_root_partition(store);

    assert_int_equal(1, result);
}

/** Verifies has_root_partition() returns 0 when only non-root exists. */
static void test_has_root_partition_no_root(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 2;
    strncpy(store->partitions[0].mount_point, "/home", STORE_MAX_MOUNT_LEN);
    strncpy(store->partitions[1].mount_point, "/boot", STORE_MAX_MOUNT_LEN);

    int result = has_root_partition(store);

    assert_int_equal(0, result);
}

/** Verifies has_root_partition() finds root among multiple partitions. */
static void test_has_root_partition_multiple(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 3;
    strncpy(store->partitions[0].mount_point, "/boot", STORE_MAX_MOUNT_LEN);
    strncpy(store->partitions[1].mount_point, "/", STORE_MAX_MOUNT_LEN);
    strncpy(store->partitions[2].mount_point, "/home", STORE_MAX_MOUNT_LEN);

    int result = has_root_partition(store);

    assert_int_equal(1, result);
}

/** Verifies has_duplicate_mount_points() returns 0 when no partitions. */
static void test_has_duplicate_mount_points_empty(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 0;

    int result = has_duplicate_mount_points(store);

    assert_int_equal(0, result);
}

/** Verifies has_duplicate_mount_points() returns 0 with unique mounts. */
static void test_has_duplicate_mount_points_unique(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 3;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);
    strncpy(store->partitions[1].mount_point, "/home", STORE_MAX_MOUNT_LEN);
    strncpy(store->partitions[2].mount_point, "/boot", STORE_MAX_MOUNT_LEN);

    int result = has_duplicate_mount_points(store);

    assert_int_equal(0, result);
}

/** Verifies has_duplicate_mount_points() returns 1 with duplicates. */
static void test_has_duplicate_mount_points_duplicate(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 2;
    strncpy(store->partitions[0].mount_point, "/home", STORE_MAX_MOUNT_LEN);
    strncpy(store->partitions[1].mount_point, "/home", STORE_MAX_MOUNT_LEN);

    int result = has_duplicate_mount_points(store);

    assert_int_equal(1, result);
}

/** Verifies has_duplicate_mount_points() ignores [swap] partitions. */
static void test_has_duplicate_mount_points_ignores_swap(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 2;
    strncpy(store->partitions[0].mount_point, "[swap]", STORE_MAX_MOUNT_LEN);
    strncpy(store->partitions[1].mount_point, "[swap]", STORE_MAX_MOUNT_LEN);

    int result = has_duplicate_mount_points(store);

    assert_int_equal(0, result);
}

/** Verifies has_duplicate_mount_points() ignores [none] partitions. */
static void test_has_duplicate_mount_points_ignores_none(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 2;
    strncpy(store->partitions[0].mount_point, "[none]", STORE_MAX_MOUNT_LEN);
    strncpy(store->partitions[1].mount_point, "[none]", STORE_MAX_MOUNT_LEN);

    int result = has_duplicate_mount_points(store);

    assert_int_equal(0, result);
}

/** Verifies has_duplicate_mount_points() detects duplicate among mixed. */
static void test_has_duplicate_mount_points_mixed(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);
    strncpy(store->partitions[1].mount_point, "[swap]", STORE_MAX_MOUNT_LEN);
    strncpy(store->partitions[2].mount_point, "/home", STORE_MAX_MOUNT_LEN);
    strncpy(store->partitions[3].mount_point, "/home", STORE_MAX_MOUNT_LEN);

    int result = has_duplicate_mount_points(store);

    assert_int_equal(1, result);
}

/** Verifies has_required_boot_partition() returns 0 when no ESP in UEFI mode. */
static void test_has_required_boot_partition_no_esp_uefi(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);
    store->partitions[0].flag_esp = 0;

    int result = has_required_boot_partition(store, 1);

    assert_int_equal(0, result);
}

/** Verifies has_required_boot_partition() returns 1 when ESP exists in UEFI. */
static void test_has_required_boot_partition_has_esp_uefi(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 2;
    strncpy(store->partitions[0].mount_point, "/boot/efi", STORE_MAX_MOUNT_LEN);
    store->partitions[0].flag_esp = 1;
    strncpy(store->partitions[1].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = has_required_boot_partition(store, 1);

    assert_int_equal(1, result);
}

/** Verifies has_required_boot_partition() returns 0 when no BIOS boot in BIOS. */
static void test_has_required_boot_partition_no_bios_grub(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);
    store->partitions[0].flag_bios_grub = 0;

    int result = has_required_boot_partition(store, 0);

    assert_int_equal(0, result);
}

/** Verifies has_required_boot_partition() returns 1 when BIOS boot exists. */
static void test_has_required_boot_partition_has_bios_grub(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 2;
    strncpy(store->partitions[0].mount_point, "[none]", STORE_MAX_MOUNT_LEN);
    store->partitions[0].flag_bios_grub = 1;
    strncpy(store->partitions[1].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = has_required_boot_partition(store, 0);

    assert_int_equal(1, result);
}

/** Verifies ESP flag is ignored in BIOS mode. */
static void test_has_required_boot_partition_esp_ignored_bios(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_esp = 1;
    store->partitions[0].flag_bios_grub = 0;

    int result = has_required_boot_partition(store, 0);

    assert_int_equal(0, result);
}

/** Verifies BIOS boot flag is ignored in UEFI mode. */
static void test_has_required_boot_partition_bios_ignored_uefi(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_esp = 0;
    store->partitions[0].flag_bios_grub = 1;

    int result = has_required_boot_partition(store, 1);

    assert_int_equal(0, result);
}

/** Verifies is_boot_partition_too_small() returns 0 when ESP is large enough. */
static void test_is_boot_partition_too_small_esp_ok(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_esp = 1;
    store->partitions[0].size_bytes = 512ULL * 1000000; /* 512MB */

    int result = is_boot_partition_too_small(store, 1);

    assert_int_equal(0, result);
}

/** Verifies is_boot_partition_too_small() returns 1 when ESP is too small. */
static void test_is_boot_partition_too_small_esp_too_small(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_esp = 1;
    store->partitions[0].size_bytes = 256ULL * 1000000; /* 256MB */

    int result = is_boot_partition_too_small(store, 1);

    assert_int_equal(1, result);
}

/** Verifies is_boot_partition_too_small() returns 0 when BIOS boot is large. */
static void test_is_boot_partition_too_small_bios_ok(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_bios_grub = 1;
    store->partitions[0].size_bytes = 8ULL * 1000000; /* 8MB */

    int result = is_boot_partition_too_small(store, 0);

    assert_int_equal(0, result);
}

/** Verifies is_boot_partition_too_small() returns 1 when BIOS boot too small. */
static void test_is_boot_partition_too_small_bios_too_small(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_bios_grub = 1;
    store->partitions[0].size_bytes = 1ULL * 1000000; /* 1MB */

    int result = is_boot_partition_too_small(store, 0);

    assert_int_equal(1, result);
}

/** Verifies is_boot_partition_too_small() returns 0 when no boot partition. */
static void test_is_boot_partition_too_small_no_boot(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);
    store->partitions[0].flag_esp = 0;
    store->partitions[0].flag_bios_grub = 0;

    int result = is_boot_partition_too_small(store, 1);

    assert_int_equal(0, result);
}

/** Verifies ESP at exact minimum size passes. */
static void test_is_boot_partition_too_small_esp_exact_min(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_esp = 1;
    store->partitions[0].size_bytes = 512ULL * 1000000; /* Exactly 512MB */

    int result = is_boot_partition_too_small(store, 1);

    assert_int_equal(0, result);
}

/** Verifies BIOS boot at exact minimum size passes. */
static void test_is_boot_partition_too_small_bios_exact_min(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_bios_grub = 1;
    store->partitions[0].size_bytes = 2ULL * 1000000; /* Exactly 2MB */

    int result = is_boot_partition_too_small(store, 0);

    assert_int_equal(0, result);
}

/** Verifies ESP just below minimum fails. */
static void test_is_boot_partition_too_small_esp_just_under(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_esp = 1;
    store->partitions[0].size_bytes = 511ULL * 1000000; /* 511MB */

    int result = is_boot_partition_too_small(store, 1);

    assert_int_equal(1, result);
}

/** Verifies BIOS boot just below minimum fails. */
static void test_is_boot_partition_too_small_bios_just_under(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_bios_grub = 1;
    store->partitions[0].size_bytes = 1999999; /* Just under 2MB */

    int result = is_boot_partition_too_small(store, 0);

    assert_int_equal(1, result);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        // has_root_partition tests
        cmocka_unit_test_setup_teardown(test_has_root_partition_empty, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_root_partition_exists, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_root_partition_no_root, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_root_partition_multiple, setup, teardown),

        // has_duplicate_mount_points tests
        cmocka_unit_test_setup_teardown(test_has_duplicate_mount_points_empty, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_duplicate_mount_points_unique, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_duplicate_mount_points_duplicate, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_duplicate_mount_points_ignores_swap, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_duplicate_mount_points_ignores_none, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_duplicate_mount_points_mixed, setup, teardown),

        // has_required_boot_partition tests
        cmocka_unit_test_setup_teardown(test_has_required_boot_partition_no_esp_uefi, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_required_boot_partition_has_esp_uefi, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_required_boot_partition_no_bios_grub, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_required_boot_partition_has_bios_grub, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_required_boot_partition_esp_ignored_bios, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_required_boot_partition_bios_ignored_uefi, setup, teardown),

        // is_boot_partition_too_small tests
        cmocka_unit_test_setup_teardown(test_is_boot_partition_too_small_esp_ok, setup, teardown),
        cmocka_unit_test_setup_teardown(test_is_boot_partition_too_small_esp_too_small, setup, teardown),
        cmocka_unit_test_setup_teardown(test_is_boot_partition_too_small_bios_ok, setup, teardown),
        cmocka_unit_test_setup_teardown(test_is_boot_partition_too_small_bios_too_small, setup, teardown),
        cmocka_unit_test_setup_teardown(test_is_boot_partition_too_small_no_boot, setup, teardown),
        cmocka_unit_test_setup_teardown(test_is_boot_partition_too_small_esp_exact_min, setup, teardown),
        cmocka_unit_test_setup_teardown(test_is_boot_partition_too_small_bios_exact_min, setup, teardown),
        cmocka_unit_test_setup_teardown(test_is_boot_partition_too_small_esp_just_under, setup, teardown),
        cmocka_unit_test_setup_teardown(test_is_boot_partition_too_small_bios_just_under, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
