/**
 * This code is responsible for executing the installation process, including
 * disk formatting, rootfs extraction, and bootloader installation.
 */

#include "../all.h"

static int run_command(const char *cmd)
{
    // Execute command and capture result.
    int result = system(cmd);
    if (result == -1)
    {
        return 1;
    }

    // Return exit status if process exited normally.
    if (WIFEXITED(result))
    {
        return WEXITSTATUS(result);
    }

    return 1;
}

static int create_partitions(const char *disk, Store *store)
{
    char cmd[512];

    // Create GPT partition table.
    snprintf(cmd, sizeof(cmd), "parted -s %s mklabel gpt 2>/dev/null", disk);
    if (run_command(cmd) != 0)
    {
        return 1;
    }

    // Create each partition in sequence.
    unsigned long long start_mb = 1;
    for (int i = 0; i < store->partition_count; i++)
    {
        Partition *p = &store->partitions[i];
        unsigned long long size_mb = p->size_bytes / (1024 * 1024);
        unsigned long long end_mb = start_mb + size_mb;

        // Create partition with calculated boundaries.
        snprintf(
            cmd, sizeof(cmd),
            "parted -s %s mkpart %s %lluMiB %lluMiB 2>/dev/null",
            disk,
            (p->type == PART_PRIMARY) ? "primary" : "logical",
            start_mb, end_mb
        );
        if (run_command(cmd) != 0)
        {
            return 1;
        }

        // Set boot flag if needed.
        if (p->flag_boot)
        {
            snprintf(
                cmd, sizeof(cmd),
                "parted -s %s set %d boot on 2>/dev/null",
                disk, i + 1
            );
            run_command(cmd);
        }

        // Set ESP flag if needed.
        if (p->flag_esp)
        {
            snprintf(cmd, sizeof(cmd),
                     "parted -s %s set %d esp on 2>/dev/null",
                     disk, i + 1);
            run_command(cmd);
        }

        start_mb = end_mb;
    }

    // Format each partition with appropriate filesystem.
    for (int i = 0; i < store->partition_count; i++)
    {
        // Get partition device.
        Partition *p = &store->partitions[i];
        char partition_device[128];
        get_partition_device(disk, i + 1, partition_device, sizeof(partition_device));

        // Build format command based on filesystem type.
        if (p->filesystem == FS_EXT4)
        {
            snprintf(cmd, sizeof(cmd), "mkfs.ext4 -F %s 2>/dev/null", partition_device);
        }
        else if (p->filesystem == FS_SWAP)
        {
            snprintf(cmd, sizeof(cmd), "mkswap %s 2>/dev/null", partition_device);
        }

        // Execute format command.
        if (run_command(cmd) != 0)
        {
            return 1;
        }
    }

    // Mount root partition first.
    for (int i = 0; i < store->partition_count; i++)
    {
        Partition *p = &store->partitions[i];
        if (strcmp(p->mount_point, "/") == 0)
        {
            char partition_device[128];
            get_partition_device(disk, i + 1, partition_device, sizeof(partition_device));
            snprintf(cmd, sizeof(cmd), "mount %s /mnt 2>/dev/null", partition_device);
            if (run_command(cmd) != 0)
            {
                return 1;
            }
            break;
        }
    }

    // Mount remaining partitions.
    for (int i = 0; i < store->partition_count; i++)
    {
        Partition *p = &store->partitions[i];
        if (p->filesystem == FS_SWAP)
        {
            // Enable swap partition.
            char partition_device[128];
            get_partition_device(disk, i + 1, partition_device, sizeof(partition_device));
            snprintf(cmd, sizeof(cmd), "swapon %s 2>/dev/null", partition_device);
            run_command(cmd);
        }
        else if (strcmp(p->mount_point, "/") != 0 && p->mount_point[0] == '/')
        {
            // Mount non-root partition.
            char partition_device[128];
            char mount_path[256];
            get_partition_device(disk, i + 1, partition_device, sizeof(partition_device));
            snprintf(mount_path, sizeof(mount_path), "/mnt%s", p->mount_point);
            snprintf(
                cmd, sizeof(cmd),
                "mkdir -p %s && mount %s %s 2>/dev/null",
                mount_path, partition_device, mount_path
            );
            run_command(cmd);
        }
    }

    return 0;
}

static int extract_rootfs(const char *disk)
{
    // Suppress unused parameter warning.
    (void)disk;

    // Ensure the placeholder rootfs archive exists.
    if (access("assets/placeholder-rootfs.gz", F_OK) != 0)
    {
        return 1;
    }

    // Extract the rootfs archive to /mnt.
    int cmd = system("tar -xzf assets/placeholder-rootfs.gz -C /mnt &> /dev/null");
    if (cmd == -1)
    {
        return 1;
    }
    if (WIFEXITED(cmd))
    {
        int exit_code = WEXITSTATUS(cmd);
        if (exit_code != 0)
        {
            return 1;
        }
    }

    // Set up necessary mounts for chroot environment.
    cmd = system("sh src/install/mounts.sh &> /dev/null");
    if (cmd == -1)
    {
        return 1;
    }
    if (WIFEXITED(cmd))
    {
        int exit_code = WEXITSTATUS(cmd);
        if (exit_code != 0)
        {
            return 1;
        }
    }

    return 0;
}

static int setup_bootloader(const char *disk)
{
    // TODO: Implement actual bootloader installation logic.
    (void)disk;
    napms(500);
    return 0;
}

static int configure_locale(const char *locale)
{
    // TODO: Implement locale configuration logic.
    (void)locale;
    napms(200);
    return 0;
}

int run_install(WINDOW *modal)
{
    // Get global store for installation settings.
    Store *store = get_store();

    // Display installation progress header.
    clear_modal(modal);
    mvwprintw(modal, 2, 3, "Installing LimeOS...");
    wrefresh(modal);

    // Step 1: Create partitions and format disk.
    mvwprintw(modal, 4, 3, "Partitioning %s...", store->disk);
    wrefresh(modal);
    if (create_partitions(store->disk, store) != 0)
    {
        mvwprintw(modal, 4, 3, "Partitioning %s... FAILED", store->disk);
        wrefresh(modal);
        return INSTALL_ERROR_DISK;
    }
    mvwprintw(modal, 4, 3, "Partitioning %s... OK", store->disk);
    wrefresh(modal);

    // Step 2: Extract rootfs archive to target.
    mvwprintw(modal, 5, 3, "Extracting system files...");
    wrefresh(modal);
    if (extract_rootfs(store->disk) != 0)
    {
        mvwprintw(modal, 5, 3, "Extracting system files... FAILED");
        wrefresh(modal);
        return INSTALL_ERROR_EXTRACT;
    }
    mvwprintw(modal, 5, 3, "Extracting system files... OK");
    wrefresh(modal);

    // Step 3: Install and configure bootloader.
    mvwprintw(modal, 6, 3, "Installing bootloader...");
    wrefresh(modal);
    if (setup_bootloader(store->disk) != 0)
    {
        mvwprintw(modal, 6, 3, "Installing bootloader... FAILED");
        wrefresh(modal);
        return INSTALL_ERROR_BOOTLOADER;
    }
    mvwprintw(modal, 6, 3, "Installing bootloader... OK");
    wrefresh(modal);

    // Step 4: Configure system locale.
    mvwprintw(modal, 7, 3, "Configuring locale...");
    wrefresh(modal);
    configure_locale(store->locale);
    mvwprintw(modal, 7, 3, "Configuring locale... OK");
    wrefresh(modal);

    // Display completion message.
    mvwprintw(modal, 9, 3, "Installation complete!");
    mvwprintw(modal, 10, 3, "Press Enter to exit...");
    wrefresh(modal);

    return INSTALL_SUCCESS;
}
