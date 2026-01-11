/** This code is responsible for testing the rootfs extraction module. */

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

/** Verifies extract_rootfs() generates correct tar command in dry-run mode. */
static void test_extract_rootfs_generates_tar_command(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    int result = extract_rootfs();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[16][512];
    int count = read_dry_run_log(lines, 16);

    assert_true(count >= 1);

    // Should run tar extraction to /mnt.
    assert_true(log_contains(lines, count, "tar -xzf /usr/share/limeos/rootfs.tar.gz -C /mnt"));
}

/** Verifies extract_rootfs() redirects output to install log. */
static void test_extract_rootfs_redirects_to_log(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    int result = extract_rootfs();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[16][512];
    int count = read_dry_run_log(lines, 16);

    // Should include log redirection.
    assert_true(log_contains(lines, count, ">>" INSTALL_LOG_PATH " 2>&1"));
}

/** Verifies extract_rootfs() skips file existence check in dry-run mode. */
static void test_extract_rootfs_skips_existence_check_in_dry_run(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    // In dry-run mode, the rootfs.tar.gz doesn't need to exist.
    // The function should succeed and log the command.
    int result = extract_rootfs();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[16][512];
    int count = read_dry_run_log(lines, 16);

    // Command should still be logged.
    assert_true(count >= 1);
}

/** Verifies extract_rootfs() uses correct tarball path. */
static void test_extract_rootfs_uses_correct_path(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    int result = extract_rootfs();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[16][512];
    int count = read_dry_run_log(lines, 16);

    // Verify the exact tarball path is used.
    assert_true(log_contains(lines, count, "/usr/share/limeos/rootfs.tar.gz"));
}

/** Verifies extract_rootfs() extracts to /mnt target. */
static void test_extract_rootfs_extracts_to_mnt(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    int result = extract_rootfs();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[16][512];
    int count = read_dry_run_log(lines, 16);

    // Verify extraction target is /mnt.
    assert_true(log_contains(lines, count, "-C /mnt"));
}

/** Verifies extract_rootfs() uses gzip decompression flag. */
static void test_extract_rootfs_uses_gzip(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    int result = extract_rootfs();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[16][512];
    int count = read_dry_run_log(lines, 16);

    // Verify -z flag for gzip decompression is used.
    assert_true(log_contains(lines, count, "tar -xzf"));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_extract_rootfs_generates_tar_command, setup, teardown),
        cmocka_unit_test_setup_teardown(test_extract_rootfs_redirects_to_log, setup, teardown),
        cmocka_unit_test_setup_teardown(test_extract_rootfs_skips_existence_check_in_dry_run, setup, teardown),
        cmocka_unit_test_setup_teardown(test_extract_rootfs_uses_correct_path, setup, teardown),
        cmocka_unit_test_setup_teardown(test_extract_rootfs_extracts_to_mnt, setup, teardown),
        cmocka_unit_test_setup_teardown(test_extract_rootfs_uses_gzip, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
