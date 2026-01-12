/**
 * This code is responsible for testing the confirmation step validation
 * functions. The validation functions are exposed via semistatic
 * when TESTING is defined.
 */

#include "../../../all.h"

/** Minimum sizes for boot-related partitions (matching confirm.c). */
#define ESP_MIN_SIZE_BYTES      (100ULL * 1000000)
#define BIOS_GRUB_MIN_SIZE_BYTES (1ULL * 1000000)
#define BOOT_PART_MIN_SIZE_BYTES (300ULL * 1000000)

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

/** Verifies validate_uefi_boot() returns error when no ESP exists. */
static void test_validate_uefi_boot_no_esp(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);
    store->partitions[0].flag_esp = 0;

    BootValidationError result = validate_uefi_boot(store);

    assert_int_equal(BOOT_ERR_UEFI_NO_ESP, result);
}

/** Verifies validate_uefi_boot() returns OK with valid ESP. */
static void test_validate_uefi_boot_valid_esp(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 2;
    store->partitions[0].flag_esp = 1;
    store->partitions[0].filesystem = FS_FAT32;
    store->partitions[0].size_bytes = ESP_MIN_SIZE_BYTES;
    strncpy(store->partitions[0].mount_point, "/boot/efi", STORE_MAX_MOUNT_LEN);
    strncpy(store->partitions[1].mount_point, "/", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_uefi_boot(store);

    assert_int_equal(BOOT_OK, result);
}

/** Verifies validate_uefi_boot() returns error if ESP not FAT32. */
static void test_validate_uefi_boot_esp_not_fat32(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_esp = 1;
    store->partitions[0].filesystem = FS_EXT4;
    store->partitions[0].size_bytes = ESP_MIN_SIZE_BYTES;
    strncpy(store->partitions[0].mount_point, "/boot/efi", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_uefi_boot(store);

    assert_int_equal(BOOT_ERR_UEFI_ESP_NOT_FAT32, result);
}

/** Verifies validate_uefi_boot() returns error if ESP wrong mount. */
static void test_validate_uefi_boot_esp_wrong_mount(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_esp = 1;
    store->partitions[0].filesystem = FS_FAT32;
    store->partitions[0].size_bytes = ESP_MIN_SIZE_BYTES;
    strncpy(store->partitions[0].mount_point, "/boot", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_uefi_boot(store);

    assert_int_equal(BOOT_ERR_UEFI_ESP_WRONG_MOUNT, result);
}

/** Verifies validate_uefi_boot() returns error if ESP too small. */
static void test_validate_uefi_boot_esp_too_small(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_esp = 1;
    store->partitions[0].filesystem = FS_FAT32;
    store->partitions[0].size_bytes = ESP_MIN_SIZE_BYTES - 1;
    strncpy(store->partitions[0].mount_point, "/boot/efi", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_uefi_boot(store);

    assert_int_equal(BOOT_ERR_UEFI_ESP_TOO_SMALL, result);
}

/** Verifies validate_uefi_boot() returns error if bios_grub present. */
static void test_validate_uefi_boot_has_bios_grub(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 2;
    store->partitions[0].flag_bios_grub = 1;
    store->partitions[1].flag_esp = 1;
    store->partitions[1].filesystem = FS_FAT32;
    store->partitions[1].size_bytes = ESP_MIN_SIZE_BYTES;
    strncpy(store->partitions[1].mount_point, "/boot/efi", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_uefi_boot(store);

    assert_int_equal(BOOT_ERR_UEFI_HAS_BIOS_GRUB, result);
}

/** Verifies validate_bios_gpt_boot() returns error when no bios_grub. */
static void test_validate_bios_gpt_boot_no_bios_grub(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_bios_gpt_boot(store);

    assert_int_equal(BOOT_ERR_BIOS_GPT_NO_BIOS_GRUB, result);
}

/** Verifies validate_bios_gpt_boot() returns OK with valid bios_grub. */
static void test_validate_bios_gpt_boot_valid(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 2;
    store->partitions[0].flag_bios_grub = 1;
    store->partitions[0].filesystem = FS_NONE;
    store->partitions[0].size_bytes = BIOS_GRUB_MIN_SIZE_BYTES;
    strncpy(store->partitions[0].mount_point, "[none]", STORE_MAX_MOUNT_LEN);
    strncpy(store->partitions[1].mount_point, "/", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_bios_gpt_boot(store);

    assert_int_equal(BOOT_OK, result);
}

/** Verifies validate_bios_gpt_boot() returns error if bios_grub has filesystem. */
static void test_validate_bios_gpt_boot_has_fs(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_bios_grub = 1;
    store->partitions[0].filesystem = FS_EXT4;
    store->partitions[0].size_bytes = BIOS_GRUB_MIN_SIZE_BYTES;
    strncpy(store->partitions[0].mount_point, "[none]", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_bios_gpt_boot(store);

    assert_int_equal(BOOT_ERR_BIOS_GPT_BIOS_GRUB_HAS_FS, result);
}

/** Verifies validate_bios_gpt_boot() returns error if bios_grub has mount. */
static void test_validate_bios_gpt_boot_has_mount(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_bios_grub = 1;
    store->partitions[0].filesystem = FS_NONE;
    store->partitions[0].size_bytes = BIOS_GRUB_MIN_SIZE_BYTES;
    strncpy(store->partitions[0].mount_point, "/boot", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_bios_gpt_boot(store);

    assert_int_equal(BOOT_ERR_BIOS_GPT_BIOS_GRUB_HAS_MOUNT, result);
}

/** Verifies validate_bios_gpt_boot() returns error if bios_grub too small. */
static void test_validate_bios_gpt_boot_too_small(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    store->partitions[0].flag_bios_grub = 1;
    store->partitions[0].filesystem = FS_NONE;
    store->partitions[0].size_bytes = BIOS_GRUB_MIN_SIZE_BYTES - 1;
    strncpy(store->partitions[0].mount_point, "[none]", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_bios_gpt_boot(store);

    assert_int_equal(BOOT_ERR_BIOS_GPT_BIOS_GRUB_TOO_SMALL, result);
}

/** Verifies validate_bios_gpt_boot() returns error if ESP present. */
static void test_validate_bios_gpt_boot_has_esp(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 2;
    store->partitions[0].flag_esp = 1;
    store->partitions[1].flag_bios_grub = 1;
    store->partitions[1].filesystem = FS_NONE;
    store->partitions[1].size_bytes = BIOS_GRUB_MIN_SIZE_BYTES;
    strncpy(store->partitions[1].mount_point, "[none]", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_bios_gpt_boot(store);

    assert_int_equal(BOOT_ERR_BIOS_GPT_HAS_ESP, result);
}

/** Verifies validate_bios_mbr_boot() always returns OK. */
static void test_validate_bios_mbr_boot_ok(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_bios_mbr_boot(store);

    assert_int_equal(BOOT_OK, result);
}

/** Verifies validate_optional_boot() returns OK when no /boot. */
static void test_validate_optional_boot_no_boot(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_optional_boot(store);

    assert_int_equal(BOOT_OK, result);
}

/** Verifies validate_optional_boot() returns OK with valid /boot. */
static void test_validate_optional_boot_valid(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 2;
    strncpy(store->partitions[0].mount_point, "/boot", STORE_MAX_MOUNT_LEN);
    store->partitions[0].filesystem = FS_EXT4;
    store->partitions[0].size_bytes = BOOT_PART_MIN_SIZE_BYTES;
    store->partitions[0].flag_bios_grub = 0;
    strncpy(store->partitions[1].mount_point, "/", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_optional_boot(store);

    assert_int_equal(BOOT_OK, result);
}

/** Verifies validate_optional_boot() returns error if /boot too small. */
static void test_validate_optional_boot_too_small(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    strncpy(store->partitions[0].mount_point, "/boot", STORE_MAX_MOUNT_LEN);
    store->partitions[0].filesystem = FS_EXT4;
    store->partitions[0].size_bytes = BOOT_PART_MIN_SIZE_BYTES - 1;

    BootValidationError result = validate_optional_boot(store);

    assert_int_equal(BOOT_ERR_BOOT_TOO_SMALL, result);
}

/** Verifies validate_optional_boot() returns error if /boot has no fs. */
static void test_validate_optional_boot_no_fs(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    strncpy(store->partitions[0].mount_point, "/boot", STORE_MAX_MOUNT_LEN);
    store->partitions[0].filesystem = FS_NONE;
    store->partitions[0].size_bytes = BOOT_PART_MIN_SIZE_BYTES;

    BootValidationError result = validate_optional_boot(store);

    assert_int_equal(BOOT_ERR_BOOT_NO_FS, result);
}

/** Verifies validate_optional_boot() returns error if /boot is bios_grub. */
static void test_validate_optional_boot_is_bios_grub(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    strncpy(store->partitions[0].mount_point, "/boot", STORE_MAX_MOUNT_LEN);
    store->partitions[0].filesystem = FS_EXT4;
    store->partitions[0].size_bytes = BOOT_PART_MIN_SIZE_BYTES;
    store->partitions[0].flag_bios_grub = 1;

    BootValidationError result = validate_optional_boot(store);

    assert_int_equal(BOOT_ERR_BOOT_IS_BIOS_GRUB, result);
}

/** Verifies validate_boot_config() uses UEFI rules for UEFI firmware. */
static void test_validate_boot_config_uefi(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 2;
    store->partitions[0].flag_esp = 1;
    store->partitions[0].filesystem = FS_FAT32;
    store->partitions[0].size_bytes = ESP_MIN_SIZE_BYTES;
    strncpy(store->partitions[0].mount_point, "/boot/efi", STORE_MAX_MOUNT_LEN);
    strncpy(store->partitions[1].mount_point, "/", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_boot_config(
        store, FIRMWARE_UEFI, DISK_LABEL_GPT
    );

    assert_int_equal(BOOT_OK, result);
}

/** Verifies validate_boot_config() uses BIOS+GPT rules for BIOS with GPT. */
static void test_validate_boot_config_bios_gpt(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 2;
    store->partitions[0].flag_bios_grub = 1;
    store->partitions[0].filesystem = FS_NONE;
    store->partitions[0].size_bytes = BIOS_GRUB_MIN_SIZE_BYTES;
    strncpy(store->partitions[0].mount_point, "[none]", STORE_MAX_MOUNT_LEN);
    strncpy(store->partitions[1].mount_point, "/", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_boot_config(
        store, FIRMWARE_BIOS, DISK_LABEL_GPT
    );

    assert_int_equal(BOOT_OK, result);
}

/** Verifies validate_boot_config() uses BIOS+MBR rules for BIOS with MBR. */
static void test_validate_boot_config_bios_mbr(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 1;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_boot_config(
        store, FIRMWARE_BIOS, DISK_LABEL_MBR
    );

    assert_int_equal(BOOT_OK, result);
}

/** Verifies validate_boot_config() checks optional /boot after firmware. */
static void test_validate_boot_config_with_boot(void **state)
{
    (void)state;
    Store *store = get_store();
    store->partition_count = 3;

    // Valid ESP.
    store->partitions[0].flag_esp = 1;
    store->partitions[0].filesystem = FS_FAT32;
    store->partitions[0].size_bytes = ESP_MIN_SIZE_BYTES;
    strncpy(store->partitions[0].mount_point, "/boot/efi", STORE_MAX_MOUNT_LEN);

    // Invalid /boot (too small).
    store->partitions[1].filesystem = FS_EXT4;
    store->partitions[1].size_bytes = BOOT_PART_MIN_SIZE_BYTES - 1;
    strncpy(store->partitions[1].mount_point, "/boot", STORE_MAX_MOUNT_LEN);

    strncpy(store->partitions[2].mount_point, "/", STORE_MAX_MOUNT_LEN);

    BootValidationError result = validate_boot_config(
        store, FIRMWARE_UEFI, DISK_LABEL_GPT
    );

    assert_int_equal(BOOT_ERR_BOOT_TOO_SMALL, result);
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

        // validate_uefi_boot tests
        cmocka_unit_test_setup_teardown(test_validate_uefi_boot_no_esp, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_uefi_boot_valid_esp, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_uefi_boot_esp_not_fat32, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_uefi_boot_esp_wrong_mount, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_uefi_boot_esp_too_small, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_uefi_boot_has_bios_grub, setup, teardown),

        // validate_bios_gpt_boot tests
        cmocka_unit_test_setup_teardown(test_validate_bios_gpt_boot_no_bios_grub, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_bios_gpt_boot_valid, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_bios_gpt_boot_has_fs, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_bios_gpt_boot_has_mount, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_bios_gpt_boot_too_small, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_bios_gpt_boot_has_esp, setup, teardown),

        // validate_bios_mbr_boot tests
        cmocka_unit_test_setup_teardown(test_validate_bios_mbr_boot_ok, setup, teardown),

        // validate_optional_boot tests
        cmocka_unit_test_setup_teardown(test_validate_optional_boot_no_boot, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_optional_boot_valid, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_optional_boot_too_small, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_optional_boot_no_fs, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_optional_boot_is_bios_grub, setup, teardown),

        // validate_boot_config integration tests
        cmocka_unit_test_setup_teardown(test_validate_boot_config_uefi, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_boot_config_bios_gpt, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_boot_config_bios_mbr, setup, teardown),
        cmocka_unit_test_setup_teardown(test_validate_boot_config_with_boot, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
