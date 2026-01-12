/**
 * This code is responsible for managing the global store singleton that
 * holds user selections and installation settings throughout the wizard.
 */

#include "../all.h"

static Store store = {
    .current_step = 0,
    .dry_run = 0,
    .force_uefi = 0,
    .force_disk_label = 0,
    .disk_label = DISK_LABEL_GPT,
    .locale = "",
    .hostname = "",
    .users = {{0}},
    .user_count = 0,
    .disk = "",
    .partitions = {{0}},
    .partition_count = 0
};

Store *get_store(void)
{
    return &store;
}

void reset_store(void)
{
    // Reset step and mode state.
    store.current_step = 0;
    store.dry_run = 0;
    store.force_uefi = 0;
    store.force_disk_label = 0;
    store.disk_label = DISK_LABEL_GPT;

    // Clear user selection strings.
    store.locale[0] = '\0';
    store.disk[0] = '\0';

    // Initialize default hostname based on chassis type.
    snprintf(
        store.hostname, sizeof(store.hostname),
        "user-%s", get_default_hostname_suffix()
    );

    // Initialize default user.
    memset(store.users, 0, sizeof(store.users));
    snprintf(store.users[0].username, sizeof(store.users[0].username), "user");
    snprintf(store.users[0].password, sizeof(store.users[0].password), "password");
    store.users[0].is_admin = 1;
    store.user_count = 1;

    // Clear partition configuration.
    memset(store.partitions, 0, sizeof(store.partitions));
    store.partition_count = 0;
}
