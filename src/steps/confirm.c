/**
 * This code is responsible for displaying a confirmation summary
 * of all user selections before proceeding with installation.
 */

#include "../all.h"

int run_confirmation_step(WINDOW *modal)
{
    Store *store = get_store();

    // Clear and draw step header.
    clear_modal(modal);
    wattron(modal, A_BOLD | COLOR_PAIR(CUSTOM_COLOR_PAIR_MAIN));
    mvwprintw(modal, 2, 3, "Step 4: Confirm Installation");
    wattroff(modal, A_BOLD);

    // Display summary of selected options.
    mvwprintw(modal, 4, 3, "Ready to install LimeOS with the following settings:");
    mvwprintw(modal, 6, 3, "  Locale: %s", store->locale);
    mvwprintw(modal, 7, 3, "  Disk: %s", store->disk);

    // Display partition summary.
    unsigned long long disk_size = get_disk_size(store->disk);
    unsigned long long used = sum_partition_sizes(store->partitions, store->partition_count);
    unsigned long long free_space = (disk_size > used) ? disk_size - used : 0;
    char free_str[32];
    format_disk_size(free_space, free_str, sizeof(free_str));
    if (store->partition_count > 0)
    {
        mvwprintw(
            modal,
            8, 3,
            "  Partitions: %d partitions, %s left",
            store->partition_count,
            free_str
        );
    }
    else
    {
        mvwprintw(modal, 8, 3, "  Partitions: (none)");
    }

    // Check if root partition exists.
    int has_root = 0;
    for (int i = 0; i < store->partition_count; i++)
    {
        if (strcmp(store->partitions[i].mount_point, "/") == 0)
        {
            has_root = 1;
            break;
        }
    }

    // Check for duplicate mount points (skip [none] and [swap]).
    int has_duplicate = 0;
    for (int i = 0; i < store->partition_count && !has_duplicate; i++)
    {
        if (store->partitions[i].mount_point[0] == '[') continue;
        for (int j = i + 1; j < store->partition_count; j++)
        {
            if (store->partitions[j].mount_point[0] == '[') continue;
            if (strcmp(
                store->partitions[i].mount_point,
                store->partitions[j].mount_point
            ) == 0)
            {
                has_duplicate = 1;
                break;
            }
        }
    }

    // Detect boot mode: UEFI if /sys/firmware/efi exists, otherwise BIOS.
    int is_uefi = (access("/sys/firmware/efi", F_OK) == 0);

    // Check for required boot partition based on boot mode.
    int has_boot_partition = 0;
    int boot_partition_too_small = 0;
    for (int i = 0; i < store->partition_count; i++)
    {
        if (is_uefi && store->partitions[i].flag_esp)
        {
            has_boot_partition = 1;

            // Check if ESP is at least 512MB.
            if (store->partitions[i].size_bytes < 512ULL * 1000000)
            {
                boot_partition_too_small = 1;
            }
            break;
        }
        if (!is_uefi && store->partitions[i].flag_bios_grub)
        {
            has_boot_partition = 1;

            // Check if BIOS GRUB partition is at least 128MB.
            if (store->partitions[i].size_bytes < 128ULL * 1000000)
            {
                boot_partition_too_small = 1;
            }
            break;
        }
    }

    // Determine if installation can proceed.
    int can_install = has_root && !has_duplicate && has_boot_partition
        && !boot_partition_too_small;

    // Render the appropriate message based on validation.
    if (has_duplicate)
    {
        // Display error about duplicate mount points.
        render_error(modal, 10, 3,
            "Multiple partitions share the same mount point.\n"
            "Go back and fix the configuration."
        );

        // Display navigation footer without install option.
        const char *footer[] = {"[Esc] Back", NULL};
        render_footer(modal, footer);
    }
    else if (!has_root)
    {
        // Display error about missing root partition.
        render_error(modal, 10, 3,
            "A root (/) partition is required.\n"
            "Go back and add one to continue."
        );

        // Display navigation footer without install option.
        const char *footer[] = {"[Esc] Back", NULL};
        render_footer(modal, footer);
    }
    else if (!has_boot_partition)
    {
        // Display error about missing boot partition.
        if (is_uefi)
        {
            render_error(modal, 10, 3,
                "UEFI boot requires an EFI System Partition.\n"
                "Add: Size=512MB, Mount=/boot, Flags=esp"
            );
        }
        else
        {
            render_error(modal, 10, 3,
                "GPT on BIOS requires a BIOS Boot Partition.\n"
                "Add: Size=8MB, Mount=none, Flags=bios_grub"
            );
        }

        // Display navigation footer without install option.
        const char *footer[] = {"[Esc] Back", NULL};
        render_footer(modal, footer);
    }
    else if (boot_partition_too_small)
    {
        // Display error about insufficient boot partition size.
        if (is_uefi)
        {
            render_error(modal, 10, 3,
                "EFI System Partition must be at least 512MB.\n"
                "Go back and resize it."
            );
        }
        else
        {
            render_error(modal, 10, 3,
                "BIOS GRUB partition must be at least 128MB.\n"
                "512MB is recommended. Go back and resize it."
            );
        }

        // Display navigation footer without install option.
        const char *footer[] = {"[Esc] Back", NULL};
        render_footer(modal, footer);
    }
    else
    {
        if (store->dry_run)
        {
            // Display info about dry run mode.
            render_info(modal, 10, 3,
                "Dry run mode enabled.\n"
                "No changes will be made to disk."
            );
        }
        else
        {
            // Display warning about disk formatting.
            char warning_text[128];
            snprintf(
                warning_text, sizeof(warning_text),
                "All data on %s will be erased!\n"
                "This action cannot be undone.", store->disk
            );
            render_warning(modal, 10, 3, warning_text);
        }

        // Display navigation footer.
        const char *footer[] = {"[Enter] Install", "[Esc] Back", NULL};
        render_footer(modal, footer);
    }

    wrefresh(modal);

    // Wait for user confirmation or back.
    int key;
    while ((key = wgetch(modal)) != 27 && (key != '\n' || !can_install))
    {
        // Ignore other input.
    }

    return key == '\n' && can_install;
}
