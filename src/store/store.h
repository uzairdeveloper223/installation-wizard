#pragma once
#include "../all.h"

/** Maximum length for locale string storage. */
#define STORE_MAX_LOCALE_LEN 64

/** Maximum length for disk path storage. */
#define STORE_MAX_DISK_LEN 64

/** Maximum number of partitions. */
#define STORE_MAX_PARTITIONS 16

/** Maximum length for mount point path. */
#define STORE_MAX_MOUNT_LEN 64

/** Maximum length for username. */
#define STORE_MAX_USERNAME_LEN 32

/** Maximum length for hostname. */
#define STORE_MAX_HOSTNAME_LEN 64

/** Maximum length for password. */
#define STORE_MAX_PASSWORD_LEN 128

/** Maximum number of users. */
#define STORE_MAX_USERS 8

/** Filesystem types for partitions. */
typedef enum {
    FS_EXT4,
    FS_SWAP,
    FS_FAT32,
    FS_NONE
} PartitionFS;

/** Partition types. */
typedef enum {
    PART_PRIMARY,
    PART_LOGICAL
} PartitionType;

/** Represents a single partition configuration. */
typedef struct Partition {
    unsigned long long size_bytes;
    char mount_point[STORE_MAX_MOUNT_LEN];
    PartitionFS filesystem;
    PartitionType type;
    int flag_boot;
    int flag_esp;
    int flag_bios_grub;
} Partition;

/** A type representing a user account configuration. */
typedef struct User
{
    char username[STORE_MAX_USERNAME_LEN];
    char password[STORE_MAX_PASSWORD_LEN];
    int is_admin;
} User;

/** Global store containing user selections and installation settings. */
typedef struct {
    int current_step;
    int dry_run;
    int force_uefi; // 0 = auto-detect, 1 = force UEFI, -1 = force BIOS
    char locale[STORE_MAX_LOCALE_LEN];
    char hostname[STORE_MAX_HOSTNAME_LEN];
    User users[STORE_MAX_USERS];
    int user_count;
    char disk[STORE_MAX_DISK_LEN];
    Partition partitions[STORE_MAX_PARTITIONS];
    int partition_count;
} Store;

/**
 * Retrieves the global store singleton.
 *
 * @return Pointer to the global Store instance.
 */
Store *get_store(void);

/** Resets the global store to default values. */
void reset_store(void);
