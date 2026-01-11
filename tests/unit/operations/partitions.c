/**
 * This code is responsible for testing the partitioning functionality,
 * including partition creation, formatting, and mounting behavior.
 */

#include "../../all.h"

/** Sets up the test environment before each test. */
static int setup(void **state)
{
    (void)state;
    reset_store();
    close_dry_run_log();
    unlink(DRY_RUN_LOG_PATH);
    return 0;
}

/** Cleans up the test environment after each test. */
static int teardown(void **state)
{
    (void)state;
    close_dry_run_log();
    unlink(DRY_RUN_LOG_PATH);
    return 0;
}

/**
 * Helper to read all lines from the dry-run log into a buffer.
 * Returns the number of lines read.
 */
static int read_dry_run_log(char lines[][512], int max_lines)
{
    FILE *file = fopen(DRY_RUN_LOG_PATH, "r");
    if (!file)
    {
        return 0;
    }

    int count = 0;
    while (count < max_lines && fgets(lines[count], 512, file) != NULL)
    {
        // Remove trailing newline.
        size_t len = strlen(lines[count]);
        if (len > 0 && lines[count][len - 1] == '\n')
        {
            lines[count][len - 1] = '\0';
        }
        count++;
    }
    fclose(file);
    return count;
}

/** Helper to check if a command exists in the log (substring match). */
static int log_contains(char lines[][512], int count, const char *substring)
{
    for (int i = 0; i < count; i++)
    {
        if (strstr(lines[i], substring) != NULL)
        {
            return 1;
        }
    }
    return 0;
}

/** Verifies create_partitions() creates GPT label on the disk. */
static void test_create_partitions_creates_gpt_label(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);

    // Need at least a root partition to succeed.
    store->partition_count = 1;
    store->partitions[0].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[0].type = PART_PRIMARY;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = create_partitions();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    assert_true(count >= 1);
    assert_string_equal("parted -s '/dev/sda' mklabel gpt >>" INSTALL_LOG_PATH " 2>&1", lines[0]);
}

/** Verifies create_partitions() creates single partition with correct boundaries. */
static void test_create_partitions_single_partition(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);

    // Add a 1GB root partition.
    store->partition_count = 1;
    store->partitions[0].size_bytes = 1024ULL * 1024 * 1024; // 1GB
    store->partitions[0].type = PART_PRIMARY;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = create_partitions();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    assert_true(count >= 4);
    // GPT label.
    assert_string_equal("parted -s '/dev/sda' mklabel gpt >>" INSTALL_LOG_PATH " 2>&1", lines[0]);
    // Create partition: starts at 1MiB, ends at 1025MiB (1 + 1024).
    assert_string_equal("parted -s '/dev/sda' mkpart primary 1MiB 1025MiB >>" INSTALL_LOG_PATH " 2>&1", lines[1]);
    // Format as ext4.
    assert_string_equal("mkfs.ext4 -F '/dev/sda1' >>" INSTALL_LOG_PATH " 2>&1", lines[2]);
    // Mount root.
    assert_string_equal("mount '/dev/sda1' /mnt >>" INSTALL_LOG_PATH " 2>&1", lines[3]);
}

/** Verifies create_partitions() creates multiple partitions with correct boundaries. */
static void test_create_partitions_multiple_partitions(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);

    // Add 512MB boot + 2GB root.
    store->partition_count = 2;

    store->partitions[0].size_bytes = 512ULL * 1024 * 1024; // 512MB
    store->partitions[0].type = PART_PRIMARY;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/boot", STORE_MAX_MOUNT_LEN);

    store->partitions[1].size_bytes = 2048ULL * 1024 * 1024; // 2GB
    store->partitions[1].type = PART_PRIMARY;
    store->partitions[1].filesystem = FS_EXT4;
    strncpy(store->partitions[1].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = create_partitions();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    assert_true(count >= 6);
    // Partition 1: 1MiB to 513MiB.
    assert_string_equal("parted -s '/dev/sda' mkpart primary 1MiB 513MiB >>" INSTALL_LOG_PATH " 2>&1", lines[1]);
    // Partition 2: 513MiB to 2561MiB.
    assert_string_equal("parted -s '/dev/sda' mkpart primary 513MiB 2561MiB >>" INSTALL_LOG_PATH " 2>&1", lines[2]);
}

/** Verifies create_partitions() sets boot flag when requested. */
static void test_create_partitions_sets_boot_flag(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);

    store->partition_count = 1;
    store->partitions[0].size_bytes = 512ULL * 1024 * 1024;
    store->partitions[0].type = PART_PRIMARY;
    store->partitions[0].filesystem = FS_EXT4;
    store->partitions[0].flag_boot = 1;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = create_partitions();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Find boot flag command.
    assert_true(log_contains(lines, count, "parted -s '/dev/sda' set 1 boot on"));
}

/** Verifies create_partitions() sets ESP flag when requested. */
static void test_create_partitions_sets_esp_flag(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);

    store->partition_count = 1;
    store->partitions[0].size_bytes = 512ULL * 1024 * 1024;
    store->partitions[0].type = PART_PRIMARY;
    store->partitions[0].filesystem = FS_EXT4;
    store->partitions[0].flag_esp = 1;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = create_partitions();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Find ESP flag command.
    assert_true(log_contains(lines, count, "parted -s '/dev/sda' set 1 esp on"));
}

/** Verifies create_partitions() sets BIOS boot flag when requested. */
static void test_create_partitions_sets_bios_grub_flag(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);

    store->partition_count = 2;

    // BIOS boot partition.
    store->partitions[0].size_bytes = 1ULL * 1024 * 1024; // 1MB
    store->partitions[0].type = PART_PRIMARY;
    store->partitions[0].filesystem = FS_NONE;
    store->partitions[0].flag_bios_grub = 1;
    store->partitions[0].mount_point[0] = '\0';

    // Root partition.
    store->partitions[1].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[1].type = PART_PRIMARY;
    store->partitions[1].filesystem = FS_EXT4;
    strncpy(store->partitions[1].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = create_partitions();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Find bios_grub flag command.
    assert_true(log_contains(lines, count, "parted -s '/dev/sda' set 1 bios_grub on"));
}

/** Verifies create_partitions() formats swap partitions with mkswap. */
static void test_create_partitions_formats_swap(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);

    store->partition_count = 2;

    // Root partition.
    store->partitions[0].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[0].type = PART_PRIMARY;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    // Swap partition.
    store->partitions[1].size_bytes = 512ULL * 1024 * 1024;
    store->partitions[1].type = PART_PRIMARY;
    store->partitions[1].filesystem = FS_SWAP;
    store->partitions[1].mount_point[0] = '\0';

    int result = create_partitions();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Find mkswap and swapon commands.
    assert_true(log_contains(lines, count, "mkswap '/dev/sda2'"));
    assert_true(log_contains(lines, count, "swapon '/dev/sda2'"));
}

/** Verifies create_partitions() formats FAT32 partitions correctly. */
static void test_create_partitions_formats_fat32(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);

    store->partition_count = 2;

    // EFI partition (FAT32).
    store->partitions[0].size_bytes = 512ULL * 1024 * 1024;
    store->partitions[0].type = PART_PRIMARY;
    store->partitions[0].filesystem = FS_FAT32;
    store->partitions[0].flag_esp = 1;
    strncpy(store->partitions[0].mount_point, "/boot/efi", STORE_MAX_MOUNT_LEN);

    // Root partition.
    store->partitions[1].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[1].type = PART_PRIMARY;
    store->partitions[1].filesystem = FS_EXT4;
    strncpy(store->partitions[1].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = create_partitions();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Find mkfs.vfat command.
    assert_true(log_contains(lines, count, "mkfs.vfat -F 32 '/dev/sda1'"));
}

/** Verifies create_partitions() handles NVMe device naming. */
static void test_create_partitions_nvme_naming(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/nvme0n1", STORE_MAX_DISK_LEN);

    store->partition_count = 1;
    store->partitions[0].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[0].type = PART_PRIMARY;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = create_partitions();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Find mkfs command with correct NVMe partition naming (p1 suffix).
    assert_true(log_contains(lines, count, "mkfs.ext4 -F '/dev/nvme0n1p1'"));
}

/** Verifies create_partitions() mounts non-root partitions correctly. */
static void test_create_partitions_mounts_nonroot(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);

    store->partition_count = 2;

    // Root partition.
    store->partitions[0].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[0].type = PART_PRIMARY;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    // Home partition.
    store->partitions[1].size_bytes = 2048ULL * 1024 * 1024;
    store->partitions[1].type = PART_PRIMARY;
    store->partitions[1].filesystem = FS_EXT4;
    strncpy(store->partitions[1].mount_point, "/home", STORE_MAX_MOUNT_LEN);

    int result = create_partitions();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Find mkdir and mount command for /home.
    assert_true(log_contains(lines, count, "mkdir -p '/mnt/home'"));
    assert_true(log_contains(lines, count, "mount '/dev/sda2' '/mnt/home'"));
}

/** Verifies create_partitions() fails when no root partition is defined. */
static void test_create_partitions_fails_without_root(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);

    // Only a boot partition, no root.
    store->partition_count = 1;
    store->partitions[0].size_bytes = 512ULL * 1024 * 1024;
    store->partitions[0].type = PART_PRIMARY;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/boot", STORE_MAX_MOUNT_LEN);

    int result = create_partitions();
    close_dry_run_log();

    // Should return -7 (no root partition found).
    assert_int_equal(-7, result);
}

/** Verifies create_partitions() fails with zero partitions (no root). */
static void test_create_partitions_empty_fails(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 0;

    int result = create_partitions();
    close_dry_run_log();

    // Should return -7 (no root partition found).
    assert_int_equal(-7, result);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_create_partitions_creates_gpt_label, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_single_partition, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_multiple_partitions, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_sets_boot_flag, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_sets_esp_flag, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_sets_bios_grub_flag, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_formats_swap, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_formats_fat32, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_nvme_naming, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_mounts_nonroot, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_fails_without_root, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_empty_fails, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
