/** This code is responsible for testing the locale configuration module. */

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

/** Verifies configure_locale() generates correct sed command for valid locale. */
static void test_configure_locale_valid_locale(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->locale, "en_US.UTF-8", STORE_MAX_LOCALE_LEN);

    int result = configure_locale();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[16][512];
    int count = read_dry_run_log(lines, 16);

    assert_true(count >= 3);

    // Should uncomment locale in locale.gen.
    assert_true(log_contains(lines, count, "sed -i '/^# en_US.UTF-8/s/^# //' /mnt/etc/locale.gen"));

    // Should run locale-gen in chroot.
    assert_true(log_contains(lines, count, "chroot /mnt /usr/sbin/locale-gen"));

    // Should set LANG in /etc/default/locale.
    assert_true(log_contains(lines, count, "echo 'LANG=en_US.UTF-8' > /mnt/etc/default/locale"));
}

/** Verifies configure_locale() works with locale containing @ modifier. */
static void test_configure_locale_with_modifier(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->locale, "sr_RS@latin", STORE_MAX_LOCALE_LEN);

    int result = configure_locale();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[16][512];
    int count = read_dry_run_log(lines, 16);

    // Should handle @ modifier correctly.
    assert_true(log_contains(lines, count, "sr_RS@latin"));
}

/** Verifies configure_locale() rejects empty locale. */
static void test_configure_locale_empty_locale(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    store->locale[0] = '\0';

    int result = configure_locale();
    close_dry_run_log();

    // Should return -1 for invalid locale.
    assert_int_equal(-1, result);

    // No commands should have been logged.
    char lines[16][512];
    int count = read_dry_run_log(lines, 16);
    assert_int_equal(0, count);
}

/** Verifies configure_locale() rejects locale without underscore. */
static void test_configure_locale_no_underscore(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->locale, "POSIX", STORE_MAX_LOCALE_LEN);

    int result = configure_locale();
    close_dry_run_log();

    // Should return -1 for invalid locale (no underscore).
    assert_int_equal(-1, result);
}

/** Verifies configure_locale() rejects overly long locale strings. */
static void test_configure_locale_too_long(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    // Create a locale string longer than 32 characters.
    strncpy(store->locale, "en_US.UTF-8.this_is_way_too_long_locale", STORE_MAX_LOCALE_LEN);

    int result = configure_locale();
    close_dry_run_log();

    // Should return -1 for overly long locale.
    assert_int_equal(-1, result);
}

/** Verifies configure_locale() rejects shell injection attempts. */
static void test_configure_locale_shell_injection(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    // Attempt shell injection with semicolon.
    strncpy(store->locale, "en_US; rm -rf /", STORE_MAX_LOCALE_LEN);

    int result = configure_locale();
    close_dry_run_log();

    // Should return -1 due to invalid characters.
    assert_int_equal(-1, result);
}

/** Verifies configure_locale() rejects locale with backticks. */
static void test_configure_locale_backtick_injection(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    // Attempt command substitution with backticks.
    strncpy(store->locale, "en_US`whoami`", STORE_MAX_LOCALE_LEN);

    int result = configure_locale();
    close_dry_run_log();

    // Should return -1 due to invalid characters.
    assert_int_equal(-1, result);
}

/** Verifies configure_locale() rejects locale with dollar sign. */
static void test_configure_locale_dollar_injection(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    // Attempt variable expansion.
    strncpy(store->locale, "en_US$(whoami)", STORE_MAX_LOCALE_LEN);

    int result = configure_locale();
    close_dry_run_log();

    // Should return -1 due to invalid characters.
    assert_int_equal(-1, result);
}

/** Verifies configure_locale() allows valid special characters. */
static void test_configure_locale_valid_special_chars(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;

    // Valid locale with hyphen, dot, and at-sign.
    strncpy(store->locale, "ca_ES.UTF-8@valencia", STORE_MAX_LOCALE_LEN);

    int result = configure_locale();
    close_dry_run_log();

    assert_int_equal(0, result);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_configure_locale_valid_locale, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_locale_with_modifier, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_locale_empty_locale, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_locale_no_underscore, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_locale_too_long, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_locale_shell_injection, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_locale_backtick_injection, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_locale_dollar_injection, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_locale_valid_special_chars, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
