/**
 * This file contains unit tests for the partitioning functionality
 * of the installer (e.g., create_partitions behavior).
 */

#include "../../all.h"

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

/**
 * Sets up the test environment before each test.
 */
static int setup(void **state)
{
    (void)state;
    reset_store();
    close_dry_run_log();
    unlink(DRY_RUN_LOG_PATH);
    return 0;
}

/**
 * Cleans up the test environment after each test.
 */
static int teardown(void **state)
{
    (void)state;
    close_dry_run_log();
    unlink(DRY_RUN_LOG_PATH);
    return 0;
}

/**
 * Verifies create_partitions() creates GPT label on the disk.
 */
static void test_create_partitions_creates_gpt_label(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 0;

    int result = create_partitions();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    assert_true(count >= 1);
    assert_string_equal("parted -s /dev/sda mklabel gpt 2>/dev/null", lines[0]);
}

/**
 * Verifies create_partitions() creates single partition with correct boundaries.
 */
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
    assert_string_equal("parted -s /dev/sda mklabel gpt 2>/dev/null", lines[0]);
    // Create partition: starts at 1MiB, ends at 1025MiB (1 + 1024).
    assert_string_equal("parted -s /dev/sda mkpart primary 1MiB 1025MiB 2>/dev/null", lines[1]);
    // Format as ext4.
    assert_string_equal("mkfs.ext4 -F /dev/sda1 2>/dev/null", lines[2]);
    // Mount root.
    assert_string_equal("mount /dev/sda1 /mnt 2>/dev/null", lines[3]);
}

/**
 * Verifies create_partitions() creates multiple partitions with correct boundaries.
 */
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
    assert_string_equal("parted -s /dev/sda mkpart primary 1MiB 513MiB 2>/dev/null", lines[1]);
    // Partition 2: 513MiB to 2561MiB.
    assert_string_equal("parted -s /dev/sda mkpart primary 513MiB 2561MiB 2>/dev/null", lines[2]);
}

/**
 * Verifies create_partitions() sets boot flag when requested.
 */
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
    int found = 0;
    for (int i = 0; i < count; i++)
    {
        if (strcmp(lines[i], "parted -s /dev/sda set 1 boot on 2>/dev/null") == 0)
        {
            found = 1;
            break;
        }
    }
    assert_true(found);
}

/**
 * Verifies create_partitions() sets ESP flag when requested.
 */
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
    int found = 0;
    for (int i = 0; i < count; i++)
    {
        if (strcmp(lines[i], "parted -s /dev/sda set 1 esp on 2>/dev/null") == 0)
        {
            found = 1;
            break;
        }
    }
    assert_true(found);
}

/**
 * Verifies create_partitions() formats swap partitions with mkswap.
 */
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
    int found_mkswap = 0;
    int found_swapon = 0;
    for (int i = 0; i < count; i++)
    {
        if (strcmp(lines[i], "mkswap /dev/sda2 2>/dev/null") == 0)
        {
            found_mkswap = 1;
        }
        if (strcmp(lines[i], "swapon /dev/sda2 2>/dev/null") == 0)
        {
            found_swapon = 1;
        }
    }
    assert_true(found_mkswap);
    assert_true(found_swapon);
}

/**
 * Verifies create_partitions() handles NVMe device naming.
 */
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

    // Find mkfs command with correct NVMe partition naming.
    int found = 0;
    for (int i = 0; i < count; i++)
    {
        if (strcmp(lines[i], "mkfs.ext4 -F /dev/nvme0n1p1 2>/dev/null") == 0)
        {
            found = 1;
            break;
        }
    }
    assert_true(found);
}

/**
 * Verifies create_partitions() mounts non-root partitions correctly.
 */
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
    int found = 0;
    for (int i = 0; i < count; i++)
    {
        if (strcmp(lines[i], "mkdir -p /mnt/home && mount /dev/sda2 /mnt/home 2>/dev/null") == 0)
        {
            found = 1;
            break;
        }
    }
    assert_true(found);
}

/**
 * Verifies create_partitions() returns success with no partitions.
 */
static void test_create_partitions_empty(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 0;

    int result = create_partitions();
    close_dry_run_log();

    assert_int_equal(0, result);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_create_partitions_creates_gpt_label, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_single_partition, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_multiple_partitions, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_sets_boot_flag, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_sets_esp_flag, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_formats_swap, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_nvme_naming, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_mounts_nonroot, setup, teardown),
        cmocka_unit_test_setup_teardown(test_create_partitions_empty, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
