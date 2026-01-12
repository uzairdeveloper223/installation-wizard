/**
 * This code is responsible for testing the user configuration module,
 * including user creation, password setting, admin privileges, and hostname.
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

/** Helper to set up minimal valid user configuration. */
static void setup_user_config(void)
{
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->hostname, "testhost", STORE_MAX_HOSTNAME_LEN);
    store->user_count = 1;
    strncpy(store->users[0].username, "testuser", STORE_MAX_USERNAME_LEN);
    strncpy(store->users[0].password, "testpass", STORE_MAX_PASSWORD_LEN);
    store->users[0].is_admin = 1;
}

/** Verifies configure_users() returns 0 with a single valid user. */
static void test_configure_users_single_user(void **state)
{
    (void)state;
    setup_user_config();

    int result = configure_users();
    close_dry_run_log();

    assert_int_equal(0, result);
}

/** Verifies configure_users() creates all users when multiple exist. */
static void test_configure_users_multiple_users(void **state)
{
    (void)state;
    setup_user_config();

    Store *store = get_store();
    store->user_count = 3;
    strncpy(store->users[1].username, "alice", STORE_MAX_USERNAME_LEN);
    strncpy(store->users[1].password, "alicepass", STORE_MAX_PASSWORD_LEN);
    store->users[1].is_admin = 0;
    strncpy(store->users[2].username, "bob", STORE_MAX_USERNAME_LEN);
    strncpy(store->users[2].password, "bobpass", STORE_MAX_PASSWORD_LEN);
    store->users[2].is_admin = 1;

    int result = configure_users();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Verify all three users were created.
    assert_true(log_contains(lines, count, "useradd"));
    assert_true(log_contains(lines, count, "'testuser'"));
    assert_true(log_contains(lines, count, "'alice'"));
    assert_true(log_contains(lines, count, "'bob'"));
}

/** Verifies configure_users() adds admin users to sudo group. */
static void test_configure_users_admin_user(void **state)
{
    (void)state;
    setup_user_config();

    Store *store = get_store();
    store->users[0].is_admin = 1;

    int result = configure_users();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Verify usermod -aG sudo was called for admin user.
    assert_true(log_contains(lines, count, "usermod -aG sudo"));
}

/** Verifies configure_users() skips sudo group for non-admin users. */
static void test_configure_users_non_admin_user(void **state)
{
    (void)state;
    setup_user_config();

    Store *store = get_store();
    store->users[0].is_admin = 0;

    int result = configure_users();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Verify usermod was NOT called for non-admin user.
    assert_false(log_contains(lines, count, "usermod"));
}

/** Verifies configure_users() writes hostname to /mnt/etc/hostname. */
static void test_configure_users_sets_hostname(void **state)
{
    (void)state;
    setup_user_config();

    Store *store = get_store();
    strncpy(store->hostname, "myhostname", STORE_MAX_HOSTNAME_LEN);

    int result = configure_users();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Verify hostname command was generated.
    assert_true(log_contains(lines, count, "/mnt/etc/hostname"));
    assert_true(log_contains(lines, count, "'myhostname'"));
}

/** Verifies configure_users() uses correct useradd command format. */
static void test_configure_users_useradd_command_format(void **state)
{
    (void)state;
    setup_user_config();

    int result = configure_users();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Verify useradd command with -m -s /bin/bash flags.
    assert_true(log_contains(lines, count, "chroot /mnt useradd -m -s /bin/bash"));
}

/** Verifies configure_users() uses correct chpasswd command format. */
static void test_configure_users_chpasswd_command_format(void **state)
{
    (void)state;
    setup_user_config();

    int result = configure_users();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Verify chpasswd command format.
    assert_true(log_contains(lines, count, "chroot /mnt sh -c"));
    assert_true(log_contains(lines, count, "chpasswd"));
}

/** Verifies configure_users() uses correct usermod command format. */
static void test_configure_users_usermod_command_format(void **state)
{
    (void)state;
    setup_user_config();

    Store *store = get_store();
    store->users[0].is_admin = 1;

    int result = configure_users();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Verify usermod command format.
    assert_true(log_contains(lines, count, "chroot /mnt usermod -aG sudo"));
}

/** Verifies configure_users() uses correct hostname command format. */
static void test_configure_users_hostname_command_format(void **state)
{
    (void)state;
    setup_user_config();

    int result = configure_users();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Verify hostname echo command format.
    assert_true(log_contains(lines, count, "echo"));
    assert_true(log_contains(lines, count, "> /mnt/etc/hostname"));
}

/** Verifies configure_users() rejects empty user list. */
static void test_configure_users_rejects_empty_user_list(void **state)
{
    (void)state;
    Store *store = get_store();
    store->dry_run = 1;
    strncpy(store->hostname, "testhost", STORE_MAX_HOSTNAME_LEN);
    store->user_count = 0;

    int result = configure_users();
    close_dry_run_log();

    // Should return -2 for empty user list.
    assert_int_equal(-2, result);
}

/** Verifies configure_users() shell-escapes usernames. */
static void test_configure_users_escapes_username(void **state)
{
    (void)state;
    setup_user_config();

    Store *store = get_store();
    strncpy(store->users[0].username, "test'user", STORE_MAX_USERNAME_LEN);

    int result = configure_users();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Verify username is escaped (single quotes escaped).
    assert_true(log_contains(lines, count, "'test'\\''user'"));
}

/** Verifies configure_users() shell-escapes passwords. */
static void test_configure_users_escapes_password(void **state)
{
    (void)state;
    setup_user_config();

    Store *store = get_store();
    strncpy(store->users[0].password, "pass'word", STORE_MAX_PASSWORD_LEN);

    int result = configure_users();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Verify password is escaped.
    assert_true(log_contains(lines, count, "'pass'\\''word'"));
}

/** Verifies configure_users() shell-escapes hostname. */
static void test_configure_users_escapes_hostname(void **state)
{
    (void)state;
    setup_user_config();

    Store *store = get_store();
    strncpy(store->hostname, "host'name", STORE_MAX_HOSTNAME_LEN);

    int result = configure_users();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Verify hostname is escaped.
    assert_true(log_contains(lines, count, "'host'\\''name'"));
}

/** Verifies configure_users() handles special characters in passwords safely. */
static void test_configure_users_handles_special_chars_in_password(void **state)
{
    (void)state;
    setup_user_config();

    Store *store = get_store();
    strncpy(store->users[0].password, "p@ss$word!#%", STORE_MAX_PASSWORD_LEN);

    int result = configure_users();
    close_dry_run_log();

    assert_int_equal(0, result);

    char lines[32][512];
    int count = read_dry_run_log(lines, 32);

    // Special chars should be safe inside single quotes.
    assert_true(log_contains(lines, count, "'p@ss$word!#%'"));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_configure_users_single_user, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_users_multiple_users, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_users_admin_user, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_users_non_admin_user, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_users_sets_hostname, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_users_useradd_command_format, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_users_chpasswd_command_format, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_users_usermod_command_format, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_users_hostname_command_format, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_users_rejects_empty_user_list, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_users_escapes_username, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_users_escapes_password, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_users_escapes_hostname, setup, teardown),
        cmocka_unit_test_setup_teardown(test_configure_users_handles_special_chars_in_password, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
