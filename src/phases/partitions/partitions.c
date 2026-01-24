/**
 * This code is responsible for creating, formatting, and mounting disk
 * partitions based on the user's configuration stored in the global store.
 */

#include "../../all.h"

static int create_gpt_table(const char *disk)
{
    // Escape disk path for shell command.
    char escaped_disk[256];
    if (shell_escape(disk, escaped_disk, sizeof(escaped_disk)) != 0)
    {
        return -1;
    }

    // Build and run parted command to create GPT label.
    char command[512];
    snprintf(command, sizeof(command), "parted -s %s mklabel gpt >>" CONFIG_INSTALL_LOG_PATH " 2>&1", escaped_disk);

    return run_command(command) == 0 ? 0 : -2;
}

static int create_partition_entries(const char *disk, Store *store)
{
    // Escape disk path for shell commands.
    char escaped_disk[256];
    if (shell_escape(disk, escaped_disk, sizeof(escaped_disk)) != 0)
    {
        return -1;
    }

    // Track partition start position in megabytes.
    unsigned long long start_mb = 1;

    // Iterate through each partition in the store.
    for (int i = 0; i < store->partition_count; i++)
    {
        Partition *partition = &store->partitions[i];
        unsigned long long size_mb = partition->size_bytes / (1024 * 1024);
        unsigned long long end_mb = start_mb + size_mb;

        write_install_log("Creating partition %d: %lluMB (%lluMB - %lluMB), mount=%s",
            i + 1, size_mb, start_mb, end_mb, partition->mount_point);

        // Create partition with calculated boundaries.
        char command[512];
        snprintf(
            command, sizeof(command),
            "parted -s %s mkpart %s %lluMiB %lluMiB >>" CONFIG_INSTALL_LOG_PATH " 2>&1",
            escaped_disk,
            (partition->type == PART_PRIMARY) ? "primary" : "logical",
            start_mb, end_mb
        );
        if (run_command(command) != 0)
        {
            return -2;
        }

        // Set boot flag if needed.
        if (partition->flag_boot)
        {
            snprintf(
                command, sizeof(command),
                "parted -s %s set %d boot on >>" CONFIG_INSTALL_LOG_PATH " 2>&1",
                escaped_disk, i + 1
            );
            if (run_command(command) != 0)
            {
                return -3;
            }
        }

        // Set ESP flag if needed.
        if (partition->flag_esp)
        {
            snprintf(
                command, sizeof(command),
                "parted -s %s set %d esp on >>" CONFIG_INSTALL_LOG_PATH " 2>&1",
                escaped_disk, i + 1
            );
            if (run_command(command) != 0)
            {
                return -4;
            }
        }

        // Set BIOS boot flag if needed (for GPT + BIOS boot).
        if (partition->flag_bios_grub)
        {
            snprintf(
                command, sizeof(command),
                "parted -s %s set %d bios_grub on >>" CONFIG_INSTALL_LOG_PATH " 2>&1",
                escaped_disk, i + 1
            );
            if (run_command(command) != 0)
            {
                return -5;
            }
        }

        // Update start for next partition.
        start_mb = end_mb;
    }

    return 0;
}

static int format_partitions(const char *disk, Store *store)
{
    // Iterate through each partition in the store.
    for (int i = 0; i < store->partition_count; i++)
    {
        Partition *partition = &store->partitions[i];

        // Get partition device path.
        char partition_device[128];
        get_partition_device(disk, i + 1, partition_device, sizeof(partition_device));

        // Escape device path for shell command.
        char escaped_device[256];
        if (shell_escape(partition_device, escaped_device, sizeof(escaped_device)) != 0)
        {
            return -1;
        }

        // Determine formatting command based on filesystem type.
        char command[512];
        const char *fs_name = NULL;
        if (partition->filesystem == FS_EXT4)
        {
            fs_name = "ext4";
            snprintf(command, sizeof(command), "mkfs.ext4 -F %s >>" CONFIG_INSTALL_LOG_PATH " 2>&1", escaped_device);
        }
        else if (partition->filesystem == FS_SWAP)
        {
            fs_name = "swap";
            snprintf(command, sizeof(command), "mkswap %s >>" CONFIG_INSTALL_LOG_PATH " 2>&1", escaped_device);
        }
        else if (partition->filesystem == FS_FAT32)
        {
            fs_name = "fat32";
            snprintf(command, sizeof(command), "mkfs.vfat -F 32 %s >>" CONFIG_INSTALL_LOG_PATH " 2>&1", escaped_device);
        }
        else
        {
            // No filesystem specified, skip formatting.
            continue;
        }

        write_install_log("Formatting %s as %s", partition_device, fs_name);

        // Execute formatting command.
        if (run_command(command) != 0)
        {
            return -2;
        }
    }

    return 0;
}

static int find_root_partition_index(Store *store)
{
    // Search for partition with "/" mount point.
    for (int i = 0; i < store->partition_count; i++)
    {
        if (strcmp(store->partitions[i].mount_point, "/") == 0)
        {
            return i;
        }
    }
    return -1;
}

static int mount_root_partition(const char *disk, int root_index)
{
    // Get root partition device path.
    char root_device[128];
    get_partition_device(disk, root_index + 1, root_device, sizeof(root_device));

    // Escape device path for shell command.
    char escaped_device[256];
    if (shell_escape(root_device, escaped_device, sizeof(escaped_device)) != 0)
    {
        return -1;
    }

    // Build and execute mount command.
    char command[512];
    snprintf(command, sizeof(command), "mount %s /mnt >>" CONFIG_INSTALL_LOG_PATH " 2>&1", escaped_device);

    return run_command(command) == 0 ? 0 : -2;
}

static int mount_remaining_partitions(const char *disk, Store *store)
{
    // Iterate through each partition in the store.
    for (int i = 0; i < store->partition_count; i++)
    {
        Partition *partition = &store->partitions[i];

        // Enable swap or mount at mount point as needed.
        if (partition->filesystem == FS_SWAP)
        {
            // Get partition device path.
            char partition_device[128];
            get_partition_device(disk, i + 1, partition_device, sizeof(partition_device));

            // Escape device path for shell command.
            char escaped_device[256];
            if (shell_escape(partition_device, escaped_device, sizeof(escaped_device)) != 0)
            {
                write_install_log("Warning: failed to escape device path %s", partition_device);
                continue;
            }

            // Enable swap.
            write_install_log("Enabling swap on %s", partition_device);
            char command[512];
            snprintf(command, sizeof(command), "swapon %s >>" CONFIG_INSTALL_LOG_PATH " 2>&1", escaped_device);
            if (run_command(command) != 0)
            {
                write_install_log("Warning: failed to enable swap on %s", partition_device);
            }
        }
        else if (strcmp(partition->mount_point, "/") != 0 && partition->mount_point[0] == '/')
        {
            // Get partition device path.
            char partition_device[128];
            get_partition_device(disk, i + 1, partition_device, sizeof(partition_device));

            // Construct full mount path.
            char mount_path[256];
            snprintf(mount_path, sizeof(mount_path), "/mnt%s", partition->mount_point);
            
            // Escape paths for shell command.
            char escaped_mount[512];
            char escaped_device[256];
            if (
                shell_escape(partition_device, escaped_device, sizeof(escaped_device)) != 0 ||
                shell_escape(mount_path, escaped_mount, sizeof(escaped_mount)) != 0)
            {
                write_install_log("Warning: failed to escape paths for mount");
                continue;
            }

            // Create mount point and mount partition.
            write_install_log("Mounting %s at %s", partition_device, mount_path);
            char command[512];
            snprintf(
                command, sizeof(command),
                "mkdir -p %s && mount %s %s >>" CONFIG_INSTALL_LOG_PATH " 2>&1",
                escaped_mount, escaped_device, escaped_mount
            );
            if (run_command(command) != 0)
            {
                write_install_log("Warning: failed to mount %s at %s", partition_device, mount_path);
            }
        }
    }

    return 0;
}

int create_partitions(void)
{
    Store *store = get_store();
    const char *disk = store->disk;

    write_install_log("Target disk: %s", disk);
    write_install_log("Partition count: %d", store->partition_count);

    // Create GPT partition table.
    write_install_log("Creating GPT partition table on %s", disk);
    if (create_gpt_table(disk) != 0)
    {
        write_install_log("Failed to create GPT table");
        return -1;
    }

    // Create each partition in sequence.
    if (create_partition_entries(disk, store) != 0)
    {
        return -2;
    }

    // Format each partition with appropriate filesystem.
    if (format_partitions(disk, store) != 0)
    {
        return -3;
    }

    // Find and validate root partition.
    int root_index = find_root_partition_index(store);
    if (root_index < 0)
    {
        write_install_log("No root partition (/) found");
        return -4;
    }
    write_install_log("Root partition found at index %d", root_index + 1);

    // Mount the root partition.
    write_install_log("Mounting root partition to /mnt");
    if (mount_root_partition(disk, root_index) != 0)
    {
        write_install_log("Failed to mount root partition");
        return -5;
    }

    // Mount remaining partitions and enable swap.
    if (mount_remaining_partitions(disk, store) != 0)
    {
        return -6;
    }

    return 0;
}
