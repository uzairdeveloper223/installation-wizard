#pragma once
#include "../all.h"

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

/** Disk label types (partition table). */
typedef enum {
    DISK_LABEL_GPT,
    DISK_LABEL_MBR
} DiskLabel;

/** Firmware types. */
typedef enum {
    FIRMWARE_UNKNOWN = -1,
    FIRMWARE_UEFI,
    FIRMWARE_BIOS
} FirmwareType;

/** Represents a single partition configuration. */
typedef struct Partition {
    unsigned long long size_bytes;
    char mount_point[MAX_MOUNT_LEN];
    PartitionFS filesystem;
    PartitionType type;
    int flag_boot;
    int flag_esp;
    int flag_bios_grub;
} Partition;

/** A type representing a user account configuration. */
typedef struct User
{
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    int is_admin;
} User;

/** A type representing a selectable option. */
typedef struct {
    char value[128];
    char label[256];
} StoreOption;

/** Global store containing user selections and installation settings. */
typedef struct {
    int dry_run;
    DiskLabel disk_label;
    char locale[MAX_LOCALE_LEN];
    char hostname[MAX_HOSTNAME_LEN];
    User users[MAX_USERS];
    int user_count;
    char disk[MAX_DISK_LEN];
    unsigned long long disk_size;
    Partition partitions[MAX_PARTITIONS];
    int partition_count;

    // Detected system information (populated once on first access).
    StoreOption locales[MAX_OPTIONS];
    int locale_count;         // -1 = not yet populated
    StoreOption disks[MAX_OPTIONS];
    int disk_count;           // -1 = not yet populated
    FirmwareType firmware;    // FIRMWARE_UNKNOWN = not yet detected
} Store;

/**
 * Retrieves the global store singleton.
 *
 * @return Pointer to the global Store instance.
 */
Store *get_store(void);

/** Resets the global store to default values. */
void reset_store(void);
