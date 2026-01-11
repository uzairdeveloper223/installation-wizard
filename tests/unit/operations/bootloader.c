/** This code is responsible for testing the bootloader setup module. */

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

/** Helper to find the index of a command in the log. Returns -1 if not found. */
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

/** Verifies setup_bootloader() binds mounts /dev, /proc, /sys. */
static void test_setup_bootloader_bind_mounts(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 1;
    store->partitions[0].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = setup_bootloader();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[64][512];
    int count = read_dry_run_log(lines, 64);

    // Should bind mount /dev.
    assert_true(log_contains(lines, count, "mount --bind /dev /mnt/dev"));

    // Should mount proc.
    assert_true(log_contains(lines, count, "mount -t proc proc /mnt/proc"));

    // Should mount sysfs.
    assert_true(log_contains(lines, count, "mount -t sysfs sys /mnt/sys"));
}

/** Verifies setup_bootloader() verifies chroot works before proceeding. */
static void test_setup_bootloader_verifies_chroot(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 1;
    store->partitions[0].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = setup_bootloader();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[64][512];
    int count = read_dry_run_log(lines, 64);

    // Should create chroot verification marker.
    assert_true(log_contains(lines, count, "echo 'limeos' > '/mnt/tmp/.chroot_verify'"));

    // Should verify marker is visible from chroot.
    assert_true(log_contains(lines, count, "chroot /mnt cat /tmp/.chroot_verify"));

    // Should clean up marker.
    assert_true(log_contains(lines, count, "rm -f '/mnt/tmp/.chroot_verify'"));
}

/** Verifies setup_bootloader() copies BIOS packages in BIOS mode. */
static void test_setup_bootloader_bios_copies_packages(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    store->force_uefi = -1; // Force BIOS mode
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 1;
    store->partitions[0].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = setup_bootloader();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[64][512];
    int count = read_dry_run_log(lines, 64);

    // BIOS mode should copy from BIOS packages directory.
    assert_true(log_contains(lines, count, "/usr/share/limeos/packages/bios"));
    assert_false(log_contains(lines, count, "/usr/share/limeos/packages/efi"));
}

/** Verifies setup_bootloader() copies EFI packages in UEFI mode. */
static void test_setup_bootloader_uefi_copies_packages(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    store->force_uefi = 1; // Force UEFI mode
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 2;

    // EFI system partition.
    store->partitions[0].size_bytes = 512ULL * 1024 * 1024;
    store->partitions[0].filesystem = FS_FAT32;
    store->partitions[0].flag_esp = 1;
    strncpy(store->partitions[0].mount_point, "/boot/efi", STORE_MAX_MOUNT_LEN);

    // Root partition.
    store->partitions[1].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[1].filesystem = FS_EXT4;
    strncpy(store->partitions[1].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = setup_bootloader();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[64][512];
    int count = read_dry_run_log(lines, 64);

    // UEFI mode should copy from EFI packages directory.
    assert_true(log_contains(lines, count, "/usr/share/limeos/packages/efi"));
    assert_false(log_contains(lines, count, "/usr/share/limeos/packages/bios"));
}

/** Verifies setup_bootloader() installs packages via dpkg in chroot. */
static void test_setup_bootloader_installs_packages(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 1;
    store->partitions[0].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = setup_bootloader();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[64][512];
    int count = read_dry_run_log(lines, 64);

    // Should install packages via dpkg in chroot.
    assert_true(log_contains(lines, count, "chroot /mnt sh -c 'dpkg -i /tmp/*.deb'"));

    // Should clean up deb files after install.
    assert_true(log_contains(lines, count, "rm -f /mnt/tmp/*.deb"));
}

/** Verifies setup_bootloader() runs grub-install with disk path in BIOS mode. */
static void test_setup_bootloader_bios_grub_install(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    store->force_uefi = -1; // Force BIOS mode
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 1;
    store->partitions[0].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = setup_bootloader();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[64][512];
    int count = read_dry_run_log(lines, 64);

    // BIOS mode should run grub-install with disk path.
    assert_true(log_contains(lines, count, "grub-install '/dev/sda'"));
    // Should NOT contain UEFI flags.
    assert_false(log_contains(lines, count, "--target=x86_64-efi"));
}

/** Verifies setup_bootloader() runs grub-install with UEFI target in UEFI mode. */
static void test_setup_bootloader_uefi_grub_install(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    store->force_uefi = 1; // Force UEFI mode
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 2;

    // EFI system partition.
    store->partitions[0].size_bytes = 512ULL * 1024 * 1024;
    store->partitions[0].filesystem = FS_FAT32;
    store->partitions[0].flag_esp = 1;
    strncpy(store->partitions[0].mount_point, "/boot/efi", STORE_MAX_MOUNT_LEN);

    // Root partition.
    store->partitions[1].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[1].filesystem = FS_EXT4;
    strncpy(store->partitions[1].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = setup_bootloader();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[64][512];
    int count = read_dry_run_log(lines, 64);

    // UEFI mode should run grub-install with x86_64-efi target.
    assert_true(log_contains(lines, count, "--target=x86_64-efi"));
    assert_true(log_contains(lines, count, "--efi-directory=/boot/efi"));
}

/** Verifies setup_bootloader() runs update-grub. */
static void test_setup_bootloader_runs_update_grub(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 1;
    store->partitions[0].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = setup_bootloader();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[64][512];
    int count = read_dry_run_log(lines, 64);

    // Should run update-grub to generate config.
    assert_true(log_contains(lines, count, "chroot /mnt /usr/sbin/update-grub"));
}

/** Verifies setup_bootloader() executes commands in correct order. */
static void test_setup_bootloader_command_order(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 1;
    store->partitions[0].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = setup_bootloader();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[64][512];
    int count = read_dry_run_log(lines, 64);

    // Find indices of key commands.
    int idx_dev = log_find_index(lines, count, "mount --bind /dev");
    int idx_proc = log_find_index(lines, count, "mount -t proc");
    int idx_sys = log_find_index(lines, count, "mount -t sysfs");
    int idx_verify = log_find_index(lines, count, "chroot /mnt cat /tmp/.chroot_verify");
    int idx_dpkg = log_find_index(lines, count, "dpkg -i");
    int idx_grub = log_find_index(lines, count, "grub-install");
    int idx_update = log_find_index(lines, count, "update-grub");

    // All commands should be found.
    assert_true(idx_dev >= 0);
    assert_true(idx_proc >= 0);
    assert_true(idx_sys >= 0);
    assert_true(idx_verify >= 0);
    assert_true(idx_dpkg >= 0);
    assert_true(idx_grub >= 0);
    assert_true(idx_update >= 0);

    // Verify order: mounts -> verify -> dpkg -> grub-install -> update-grub
    assert_true(idx_dev < idx_proc);
    assert_true(idx_proc < idx_sys);
    assert_true(idx_sys < idx_verify);
    assert_true(idx_verify < idx_dpkg);
    assert_true(idx_dpkg < idx_grub);
    assert_true(idx_grub < idx_update);
}

/** Verifies setup_bootloader() uses quoted disk path in BIOS mode. */
static void test_setup_bootloader_quotes_disk_path(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    store->force_uefi = -1; // Force BIOS mode to test disk path quoting
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    store->partition_count = 1;
    store->partitions[0].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = setup_bootloader();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[64][512];
    int count = read_dry_run_log(lines, 64);

    // BIOS mode should quote disk path for shell safety.
    assert_true(log_contains(lines, count, "grub-install '/dev/sda'"));
}

/** Verifies setup_bootloader() handles NVMe disk names in BIOS mode. */
static void test_setup_bootloader_nvme_disk(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    store->force_uefi = -1; // Force BIOS mode to test NVMe disk path
    strncpy(store->disk, "/dev/nvme0n1", STORE_MAX_DISK_LEN);
    store->partition_count = 1;
    store->partitions[0].size_bytes = 1024ULL * 1024 * 1024;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);

    int result = setup_bootloader();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[64][512];
    int count = read_dry_run_log(lines, 64);

    // BIOS mode should use NVMe disk path in grub-install.
    assert_true(log_contains(lines, count, "grub-install '/dev/nvme0n1'"));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_setup_bootloader_bind_mounts, setup, teardown),
        cmocka_unit_test_setup_teardown(test_setup_bootloader_verifies_chroot, setup, teardown),
        cmocka_unit_test_setup_teardown(test_setup_bootloader_bios_copies_packages, setup, teardown),
        cmocka_unit_test_setup_teardown(test_setup_bootloader_uefi_copies_packages, setup, teardown),
        cmocka_unit_test_setup_teardown(test_setup_bootloader_installs_packages, setup, teardown),
        cmocka_unit_test_setup_teardown(test_setup_bootloader_bios_grub_install, setup, teardown),
        cmocka_unit_test_setup_teardown(test_setup_bootloader_uefi_grub_install, setup, teardown),
        cmocka_unit_test_setup_teardown(test_setup_bootloader_runs_update_grub, setup, teardown),
        cmocka_unit_test_setup_teardown(test_setup_bootloader_command_order, setup, teardown),
        cmocka_unit_test_setup_teardown(test_setup_bootloader_quotes_disk_path, setup, teardown),
        cmocka_unit_test_setup_teardown(test_setup_bootloader_nvme_disk, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
