/**
 * This code is responsible for testing the command execution utility,
 * including dry-run mode logging and actual command execution.
 */

#include "../all.h"

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
 * Verifies run_cmd() returns zero in dry-run mode.
 */
static void test_run_cmd_dry_run_returns_zero(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    int result = run_cmd("echo test");

    // Dry-run mode should always return success.
    assert_int_equal(0, result);
}

/**
 * Verifies run_cmd() creates a log file in dry-run mode.
 */
static void test_run_cmd_dry_run_creates_log_file(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    run_cmd("echo test");
    close_dry_run_log();

    // Verify log file was created.
    FILE *file = fopen(DRY_RUN_LOG_PATH, "r");
    assert_non_null(file);
    fclose(file);
}

/**
 * Verifies run_cmd() logs the exact command string in dry-run mode.
 */
static void test_run_cmd_dry_run_logs_command(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    run_cmd("parted /dev/sda mklabel gpt");
    close_dry_run_log();

    // Read log file and verify command was written.
    FILE *file = fopen(DRY_RUN_LOG_PATH, "r");
    assert_non_null(file);

    char buffer[256];
    char *line = fgets(buffer, sizeof(buffer), file);
    fclose(file);

    assert_non_null(line);
    assert_string_equal("parted /dev/sda mklabel gpt\n", buffer);
}

/**
 * Verifies run_cmd() logs multiple commands on separate lines.
 */
static void test_run_cmd_dry_run_logs_multiple_commands(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    // Execute multiple commands.
    run_cmd("command1");
    run_cmd("command2");
    run_cmd("command3");
    close_dry_run_log();

    // Count lines in log file.
    FILE *file = fopen(DRY_RUN_LOG_PATH, "r");
    assert_non_null(file);

    char buffer[256];
    int line_count = 0;
    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        line_count++;
    }
    fclose(file);

    assert_int_equal(3, line_count);
}

/**
 * Verifies run_cmd() executes and returns success for successful commands.
 */
static void test_run_cmd_not_dry_run_executes_command(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 0;

    // The 'true' command always succeeds.
    int result = run_cmd("true");

    assert_int_equal(0, result);
}

/**
 * Verifies run_cmd() returns non-zero for failed commands.
 */
static void test_run_cmd_not_dry_run_returns_failure(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 0;

    // The 'false' command always fails.
    int result = run_cmd("false");

    assert_int_not_equal(0, result);
}

/**
 * Verifies run_cmd() does not create a log file in normal mode.
 */
static void test_run_cmd_not_dry_run_no_log_file(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 0;

    run_cmd("true");

    // Verify no log file was created in normal mode.
    assert_int_not_equal(0, access(DRY_RUN_LOG_PATH, F_OK));
}

/**
 * Verifies close_dry_run_log() is safe to call multiple times and
 * the system remains functional afterward.
 */
static void test_close_dry_run_log_safe_when_not_open(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    // Calling close multiple times should not cause issues.
    close_dry_run_log();
    close_dry_run_log();

    // Verify system still works: can create new log after multiple closes.
    run_cmd("test command after close");
    close_dry_run_log();

    FILE *file = fopen(DRY_RUN_LOG_PATH, "r");
    assert_non_null(file);

    char buffer[256];
    char *line = fgets(buffer, sizeof(buffer), file);
    fclose(file);

    assert_non_null(line);
    assert_string_equal("test command after close\n", buffer);
}

/**
 * Verifies close_dry_run_log() flushes content to disk.
 */
static void test_close_dry_run_log_flushes_content(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    run_cmd("test command");
    close_dry_run_log();

    // Verify content was flushed to disk.
    FILE *file = fopen(DRY_RUN_LOG_PATH, "r");
    assert_non_null(file);

    char buffer[256];
    char *line = fgets(buffer, sizeof(buffer), file);
    fclose(file);

    assert_non_null(line);
    assert_true(strlen(buffer) > 0);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_run_cmd_dry_run_returns_zero, setup, teardown),
        cmocka_unit_test_setup_teardown(test_run_cmd_dry_run_creates_log_file, setup, teardown),
        cmocka_unit_test_setup_teardown(test_run_cmd_dry_run_logs_command, setup, teardown),
        cmocka_unit_test_setup_teardown(test_run_cmd_dry_run_logs_multiple_commands, setup, teardown),
        cmocka_unit_test_setup_teardown(test_run_cmd_not_dry_run_executes_command, setup, teardown),
        cmocka_unit_test_setup_teardown(test_run_cmd_not_dry_run_returns_failure, setup, teardown),
        cmocka_unit_test_setup_teardown(test_run_cmd_not_dry_run_no_log_file, setup, teardown),
        cmocka_unit_test_setup_teardown(test_close_dry_run_log_safe_when_not_open, setup, teardown),
        cmocka_unit_test_setup_teardown(test_close_dry_run_log_flushes_content, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
