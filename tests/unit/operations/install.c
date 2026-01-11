/** This code is responsible for testing the install orchestration module. */

#include "../../all.h"

// Track progress callback invocations.
static int callback_invocations[32];
static int callback_count;
static InstallEvent last_event;
static InstallStep last_step;
static int last_error_code;

/** Test progress callback that records all invocations. */
static void test_progress_cb(
    InstallEvent event,
    InstallStep step,
    int error_code,
    void *context
)
{
    (void)context;
    if (callback_count < 32)
    {
        callback_invocations[callback_count++] = (int)event;
    }
    last_event = event;
    last_step = step;
    last_error_code = error_code;
}

/** Sets up the test environment before each test. */
static int setup(void **state)
{
    (void)state;
    reset_store();
    close_dry_run_log();
    unlink(DRY_RUN_LOG_PATH);
    unlink(INSTALL_LOG_PATH);
    callback_count = 0;
    memset(callback_invocations, 0, sizeof(callback_invocations));
    last_event = 0;
    last_step = 0;
    last_error_code = 0;
    return 0;
}

/** Cleans up the test environment after each test. */
static int teardown(void **state)
{
    (void)state;
    close_dry_run_log();
    unlink(DRY_RUN_LOG_PATH);
    unlink(INSTALL_LOG_PATH);
    return 0;
}

/** Helper to set up a minimal valid store configuration. */
static void setup_minimal_config(void)
{
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->disk, "/dev/sda", STORE_MAX_DISK_LEN);
    strncpy(store->locale, "en_US.UTF-8", STORE_MAX_LOCALE_LEN);
    store->partition_count = 1;
    store->partitions[0].size_bytes = 10ULL * 1000000000;
    store->partitions[0].filesystem = FS_EXT4;
    strncpy(store->partitions[0].mount_point, "/", STORE_MAX_MOUNT_LEN);
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

/** Verifies run_install() returns 0 on successful installation. */
static void test_run_install_returns_zero_on_success(void **state)
{
    (void)state;
    setup_minimal_config();

    int result = run_install(test_progress_cb, NULL);
    close_dry_run_log();

    assert_int_equal(0, result);
}

/** Verifies run_install() invokes progress callback with INSTALL_START. */
static void test_run_install_sends_start_event(void **state)
{
    (void)state;
    setup_minimal_config();

    run_install(test_progress_cb, NULL);
    close_dry_run_log();

    assert_true(callback_count > 0);
    assert_int_equal(INSTALL_START, callback_invocations[0]);
}

/** Verifies run_install() invokes progress callback for each step. */
static void test_run_install_sends_step_begin_events(void **state)
{
    (void)state;
    setup_minimal_config();

    run_install(test_progress_cb, NULL);
    close_dry_run_log();

    // Should have at least: START, 4x(BEGIN+OK), COMPLETE, AWAIT_REBOOT.
    assert_true(callback_count >= 11);

    // At minimum, verify multiple STEP_BEGIN events occurred.
    int begin_count = 0;
    for (int i = 0; i < callback_count; i++)
    {
        if (callback_invocations[i] == INSTALL_STEP_BEGIN)
        {
            begin_count++;
        }
    }
    assert_int_equal(4, begin_count);
}

/** Verifies run_install() sends INSTALL_COMPLETE on success. */
static void test_run_install_sends_complete_event(void **state)
{
    (void)state;
    setup_minimal_config();

    run_install(test_progress_cb, NULL);
    close_dry_run_log();

    int found_complete = 0;
    for (int i = 0; i < callback_count; i++)
    {
        if (callback_invocations[i] == INSTALL_COMPLETE)
        {
            found_complete = 1;
            break;
        }
    }
    assert_true(found_complete);
}

/** Verifies run_install() sends INSTALL_AWAIT_REBOOT after completion. */
static void test_run_install_sends_await_reboot_event(void **state)
{
    (void)state;
    setup_minimal_config();

    run_install(test_progress_cb, NULL);
    close_dry_run_log();

    int found_await_reboot = 0;
    for (int i = 0; i < callback_count; i++)
    {
        if (callback_invocations[i] == INSTALL_AWAIT_REBOOT)
        {
            found_await_reboot = 1;
            break;
        }
    }
    assert_true(found_await_reboot);
}

/** Verifies run_install() executes reboot command on success. */
static void test_run_install_executes_reboot(void **state)
{
    (void)state;
    setup_minimal_config();

    run_install(test_progress_cb, NULL);
    close_dry_run_log();

    char lines[128][512];
    int count = read_dry_run_log(lines, 128);

    assert_true(log_contains(lines, count, "reboot"));
}

/** Verifies run_install() works without progress callback. */
static void test_run_install_works_without_callback(void **state)
{
    (void)state;
    setup_minimal_config();

    int result = run_install(NULL, NULL);
    close_dry_run_log();

    assert_int_equal(0, result);
}

/** Verifies run_install() initializes the install log. */
static void test_run_install_initializes_log(void **state)
{
    (void)state;
    setup_minimal_config();

    run_install(test_progress_cb, NULL);
    close_dry_run_log();

    // Install log should have been created.
    assert_int_equal(0, access(INSTALL_LOG_PATH, F_OK));
}

/** Verifies run_install() writes step headers to log. */
static void test_run_install_writes_log_headers(void **state)
{
    (void)state;
    setup_minimal_config();

    run_install(test_progress_cb, NULL);
    close_dry_run_log();

    // Read install log.
    FILE *f = fopen(INSTALL_LOG_PATH, "r");
    assert_non_null(f);

    char buffer[4096];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[len] = '\0';
    fclose(f);

    // Should contain step headers.
    assert_non_null(strstr(buffer, "Partitioning"));
    assert_non_null(strstr(buffer, "Extracting system files"));
    assert_non_null(strstr(buffer, "Installing bootloader"));
    assert_non_null(strstr(buffer, "Configuring locale"));
}

/** Verifies run_install() calls cleanup_mounts on success. */
static void test_run_install_calls_cleanup_on_success(void **state)
{
    (void)state;
    setup_minimal_config();

    run_install(test_progress_cb, NULL);
    close_dry_run_log();

    char lines[128][512];
    int count = read_dry_run_log(lines, 128);

    // Should contain unmount commands from cleanup.
    assert_true(log_contains(lines, count, "umount"));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_run_install_returns_zero_on_success, setup, teardown),
        cmocka_unit_test_setup_teardown(test_run_install_sends_start_event, setup, teardown),
        cmocka_unit_test_setup_teardown(test_run_install_sends_step_begin_events, setup, teardown),
        cmocka_unit_test_setup_teardown(test_run_install_sends_complete_event, setup, teardown),
        cmocka_unit_test_setup_teardown(test_run_install_sends_await_reboot_event, setup, teardown),
        cmocka_unit_test_setup_teardown(test_run_install_executes_reboot, setup, teardown),
        cmocka_unit_test_setup_teardown(test_run_install_works_without_callback, setup, teardown),
        cmocka_unit_test_setup_teardown(test_run_install_initializes_log, setup, teardown),
        cmocka_unit_test_setup_teardown(test_run_install_writes_log_headers, setup, teardown),
        cmocka_unit_test_setup_teardown(test_run_install_calls_cleanup_on_success, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
