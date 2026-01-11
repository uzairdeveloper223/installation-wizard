/**
 * This code is responsible for testing the cleanup module that handles
 * unmounting filesystems after installation.
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

/** Helper to read all lines from the dry-run log into a buffer. */
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

/** Helper to check if a command exists in the log. */
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

/** Helper to find the index of a command in the log. */
static int log_find_index(char lines[][512], int count, const char *substring)
{
    for (int i = 0; i < count; i++)
    {
        if (strstr(lines[i], substring) != NULL)
        {
            return i;
        }
    }
    return -1;
}

/** Verifies cleanup_mounts() unmounts /mnt/sys. */
static void test_cleanup_mounts_unmounts_sys(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    store->partition_count = 0;

    cleanup_mounts();
    close_dry_run_log();

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    assert_true(log_contains(lines, count, "umount /mnt/sys"));
}

/** Verifies cleanup_mounts() unmounts /mnt/proc. */
static void test_cleanup_mounts_unmounts_proc(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    store->partition_count = 0;

    cleanup_mounts();
    close_dry_run_log();

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    assert_true(log_contains(lines, count, "umount /mnt/proc"));
}

/** Verifies cleanup_mounts() unmounts /mnt/dev. */
static void test_cleanup_mounts_unmounts_dev(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    store->partition_count = 0;

    cleanup_mounts();
    close_dry_run_log();

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    assert_true(log_contains(lines, count, "umount /mnt/dev"));
}

/** Verifies cleanup_mounts() attempts to unmount EFI partition. */
static void test_cleanup_mounts_unmounts_efi(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    store->partition_count = 0;

    cleanup_mounts();
    close_dry_run_log();

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    assert_true(log_contains(lines, count, "umount /mnt/boot/efi"));
}

/** Verifies cleanup_mounts() unmounts root partition last. */
static void test_cleanup_mounts_unmounts_root_last(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    store->partition_count = 0;

    cleanup_mounts();
    close_dry_run_log();

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    int idx_root = log_find_index(lines, count, "umount /mnt >");
    int idx_sys = log_find_index(lines, count, "umount /mnt/sys");
    int idx_proc = log_find_index(lines, count, "umount /mnt/proc");
    int idx_dev = log_find_index(lines, count, "umount /mnt/dev");

    // Root unmount should come after chroot bind mounts.
    assert_true(idx_root > idx_sys);
    assert_true(idx_root > idx_proc);
    assert_true(idx_root > idx_dev);
}

/** Verifies cleanup_mounts() disables swap partitions. */
static void test_cleanup_mounts_disables_swap(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 2;

    // Root partition.
    store->partitions[0].size_bytes = 10ULL * 1000000000;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    // Swap partition.
    store->partitions[1].size_bytes = 2ULL * 1000000000;
    store->partitions[1].filesystem = FS_SWAP;
    strncpy(store->partitions[1].mount_point, "[swap]", STORE_MAX_MOUNT_LEN);

    cleanup_mounts();
    close_dry_run_log();

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    assert_true(log_contains(lines, count, "swapoff"));
}

/** Verifies cleanup_mounts() unmounts non-root mount points. */
static void test_cleanup_mounts_unmounts_other_partitions(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 2;

    // Root partition.
    store->partitions[0].size_bytes = 10ULL * 1000000000;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    // Home partition.
    store->partitions[1].size_bytes = 20ULL * 1000000000;
    store->partitions[1].filesystem = FS_EXT4;
    strncpy(store->partitions[1].mount_point, "/home", STORE_MAX_MOUNT_LEN);

    cleanup_mounts();
    close_dry_run_log();

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    assert_true(log_contains(lines, count, "umount '/mnt/home'"));
}

/** Verifies cleanup_mounts() processes partitions in reverse order. */
static void test_cleanup_mounts_reverse_order(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 3;

    // Root partition.
    store->partitions[0].size_bytes = 10ULL * 1000000000;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    // Home partition.
    store->partitions[1].size_bytes = 20ULL * 1000000000;
    store->partitions[1].filesystem = FS_EXT4;
    strncpy(store->partitions[1].mount_point, "/home", STORE_MAX_MOUNT_LEN);

    // Var partition.
    store->partitions[2].size_bytes = 5ULL * 1000000000;
    store->partitions[2].filesystem = FS_EXT4;
    strncpy(store->partitions[2].mount_point, "/var", STORE_MAX_MOUNT_LEN);

    cleanup_mounts();
    close_dry_run_log();

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Var should be unmounted before home (reverse order).
    int idx_var = log_find_index(lines, count, "/mnt/var");
    int idx_home = log_find_index(lines, count, "/mnt/home");

    assert_true(idx_var >= 0);
    assert_true(idx_home >= 0);
    assert_true(idx_var < idx_home);
}

/** Verifies cleanup_mounts() skips unmounted partitions. */
static void test_cleanup_mounts_skips_unmounted(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 2;

    // Root partition.
    store->partitions[0].size_bytes = 10ULL * 1000000000;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    // Unmounted partition.
    store->partitions[1].size_bytes = 5ULL * 1000000000;
    store->partitions[1].filesystem = FS_NONE;
    strncpy(store->partitions[1].mount_point, "[none]", STORE_MAX_MOUNT_LEN);

    cleanup_mounts();
    close_dry_run_log();

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Should not try to unmount [none] partitions.
    assert_false(log_contains(lines, count, "[none]"));
}

/** Verifies cleanup_mounts() returns 0 in dry-run mode. */
static void test_cleanup_mounts_returns_zero_dry_run(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    store->partition_count = 0;

    int result = cleanup_mounts();
    close_dry_run_log();

    assert_int_equal(0, result);
}

/** Verifies cleanup_mounts() handles NVMe disk naming. */
static void test_cleanup_mounts_nvme_disk(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/nvme0n1", STORE_MAX_DISK_LEN);
    store->partition_count = 2;

    // Root partition.
    store->partitions[0].size_bytes = 10ULL * 1000000000;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    // Swap partition.
    store->partitions[1].size_bytes = 2ULL * 1000000000;
    store->partitions[1].filesystem = FS_SWAP;
    strncpy(store->partitions[1].mount_point, "[swap]", STORE_MAX_MOUNT_LEN);

    cleanup_mounts();
    close_dry_run_log();

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // NVMe partitions use p suffix (nvme0n1p2).
    assert_true(log_contains(lines, count, "nvme0n1p2"));
}

/** Verifies cleanup_mounts() unmounts chroot mounts before partitions. */
static void test_cleanup_mounts_chroot_before_partitions(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 1;

    store->partitions[0].size_bytes = 10ULL * 1000000000;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    cleanup_mounts();
    close_dry_run_log();

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    int idx_sys = log_find_index(lines, count, "umount /mnt/sys");
    int idx_root = log_find_index(lines, count, "umount /mnt >");

    // Chroot mounts should be unmounted before root.
    assert_true(idx_sys < idx_root);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_cleanup_mounts_unmounts_sys, setup, teardown),
        cmocka_unit_test_setup_teardown(test_cleanup_mounts_unmounts_proc, setup, teardown),
        cmocka_unit_test_setup_teardown(test_cleanup_mounts_unmounts_dev, setup, teardown),
        cmocka_unit_test_setup_teardown(test_cleanup_mounts_unmounts_efi, setup, teardown),
        cmocka_unit_test_setup_teardown(test_cleanup_mounts_unmounts_root_last, setup, teardown),
        cmocka_unit_test_setup_teardown(test_cleanup_mounts_disables_swap, setup, teardown),
        cmocka_unit_test_setup_teardown(test_cleanup_mounts_unmounts_other_partitions, setup, teardown),
        cmocka_unit_test_setup_teardown(test_cleanup_mounts_reverse_order, setup, teardown),
        cmocka_unit_test_setup_teardown(test_cleanup_mounts_skips_unmounted, setup, teardown),
        cmocka_unit_test_setup_teardown(test_cleanup_mounts_returns_zero_dry_run, setup, teardown),
        cmocka_unit_test_setup_teardown(test_cleanup_mounts_nvme_disk, setup, teardown),
        cmocka_unit_test_setup_teardown(test_cleanup_mounts_chroot_before_partitions, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
