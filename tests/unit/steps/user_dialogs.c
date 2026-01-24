/**
 * This code is responsible for testing the user dialog helper functions.
 * The helper functions are exposed via semistatic when TESTING is defined.
 */

#include "../../all.h"

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

/** Verifies has_duplicate_username() returns 0 when no users. */
static void test_has_duplicate_username_empty(void **state)
{
    (void)state;
    Store *store = get_store();
    store->user_count = 0;

    int result = has_duplicate_username(store, "testuser", -1);

    assert_int_equal(0, result);
}

/** Verifies has_duplicate_username() returns 0 with unique names. */
static void test_has_duplicate_username_unique(void **state)
{
    (void)state;
    Store *store = get_store();
    store->user_count = 2;
    strncpy(store->users[0].username, "alice", MAX_USERNAME_LEN);
    strncpy(store->users[1].username, "bob", MAX_USERNAME_LEN);

    int result = has_duplicate_username(store, "charlie", -1);

    assert_int_equal(0, result);
}

/** Verifies has_duplicate_username() returns 1 with duplicate. */
static void test_has_duplicate_username_duplicate(void **state)
{
    (void)state;
    Store *store = get_store();
    store->user_count = 1;
    strncpy(store->users[0].username, "alice", MAX_USERNAME_LEN);

    int result = has_duplicate_username(store, "alice", -1);

    assert_int_equal(1, result);
}

/** Verifies has_duplicate_username() excludes user being edited. */
static void test_has_duplicate_username_excludes_self(void **state)
{
    (void)state;
    Store *store = get_store();
    store->user_count = 1;
    strncpy(store->users[0].username, "alice", MAX_USERNAME_LEN);

    // Editing user 0 with same name should not count as duplicate.
    int result = has_duplicate_username(store, "alice", 0);

    assert_int_equal(0, result);
}

/** Verifies has_duplicate_username() detects duplicate when editing. */
static void test_has_duplicate_username_duplicate_when_editing(void **state)
{
    (void)state;
    Store *store = get_store();
    store->user_count = 2;
    strncpy(store->users[0].username, "alice", MAX_USERNAME_LEN);
    strncpy(store->users[1].username, "bob", MAX_USERNAME_LEN);

    // Editing user 1 to "alice" should detect duplicate.
    int result = has_duplicate_username(store, "alice", 1);

    assert_int_equal(1, result);
}

/** Verifies has_duplicate_username() is case sensitive. */
static void test_has_duplicate_username_case_sensitive(void **state)
{
    (void)state;
    Store *store = get_store();
    store->user_count = 1;
    strncpy(store->users[0].username, "alice", MAX_USERNAME_LEN);

    // "Alice" should not match "alice" (case sensitive).
    int result = has_duplicate_username(store, "Alice", -1);

    assert_int_equal(0, result);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        // has_duplicate_username tests
        cmocka_unit_test_setup_teardown(test_has_duplicate_username_empty, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_duplicate_username_unique, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_duplicate_username_duplicate, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_duplicate_username_excludes_self, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_duplicate_username_duplicate_when_editing, setup, teardown),
        cmocka_unit_test_setup_teardown(test_has_duplicate_username_case_sensitive, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
