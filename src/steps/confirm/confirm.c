/**
 * This code is responsible for displaying a confirmation summary
 * of all user selections before proceeding with installation.
 */

#include "../../all.h"

semistatic int has_root_partition(Store *store)
{
    for (int i = 0; i < store->partition_count; i++)
    {
        if (strcmp(store->partitions[i].mount_point, "/") == 0)
        {
            return 1;
        }
    }
    return 0;
}

semistatic int has_duplicate_mount_points(Store *store)
{
    for (int i = 0; i < store->partition_count; i++)
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
                return 1;
            }
        }
    }
    return 0;
}

semistatic int has_required_boot_partition(Store *store, int is_uefi)
{
    for (int i = 0; i < store->partition_count; i++)
    {
        if (is_uefi && store->partitions[i].flag_esp)
        {
            return 1;
        }
        if (!is_uefi && store->partitions[i].flag_bios_grub)
        {
            return 1;
        }
    }
    return 0;
}

semistatic int is_boot_partition_too_small(Store *store, int is_uefi)
{
    for (int i = 0; i < store->partition_count; i++)
    {
        if (is_uefi && store->partitions[i].flag_esp)
        {
            // ESP must be at least 512MB.
            return store->partitions[i].size_bytes < 512ULL * 1000000;
        }
        if (!is_uefi && store->partitions[i].flag_bios_grub)
        {
            // BIOS GRUB partition must be at least 2MB.
            return store->partitions[i].size_bytes < 2ULL * 1000000;
        }
    }
    return 0;
}

static void render_config_summary(WINDOW *modal, Store *store)
{
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
            modal, 8, 3,
            "  Partitions: %d partitions, %s left",
            store->partition_count, free_str
        );
    }
    else
    {
        mvwprintw(modal, 8, 3, "  Partitions: (none)");
    }
}

static void render_duplicate_error(WINDOW *modal)
{
    render_error(modal, 10, 3,
        "Multiple partitions share the same mount point.\n"
        "Go back and fix the configuration."
    );
    const char *footer[] = {"[Esc] Back", NULL};
    render_footer(modal, footer);
}

static void render_no_root_error(WINDOW *modal)
{
    render_error(modal, 10, 3,
        "A root (/) partition is required.\n"
        "Go back and add one to continue."
    );
    const char *footer[] = {"[Esc] Back", NULL};
    render_footer(modal, footer);
}

static void render_no_boot_error(WINDOW *modal, int is_uefi)
{
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
    const char *footer[] = {"[Esc] Back", NULL};
    render_footer(modal, footer);
}

static void render_boot_too_small_error(WINDOW *modal, int is_uefi)
{
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
            "BIOS GRUB partition must be at least 2MB.\n"
            "Go back and resize it."
        );
    }
    const char *footer[] = {"[Esc] Back", NULL};
    render_footer(modal, footer);
}

static void render_ready_message(WINDOW *modal, Store *store)
{
    if (store->dry_run)
    {
        render_info(modal, 10, 3,
            "Dry run mode enabled.\n"
            "No changes will be made to disk."
        );
    }
    else
    {
        char warning_text[128];
        snprintf(
            warning_text, sizeof(warning_text),
            "All data on %s will be erased!\n"
            "This action cannot be undone.", store->disk
        );
        render_warning(modal, 10, 3, warning_text);
    }
    const char *footer[] = {"[Enter] Install", "[Esc] Back", NULL};
    render_footer(modal, footer);
}

int run_confirmation_step(WINDOW *modal)
{
    Store *store = get_store();

    // Clear and draw step header.
    clear_modal(modal);
    wattron(modal, A_BOLD | COLOR_PAIR(CUSTOM_COLOR_PAIR_MAIN));
    mvwprintw(modal, 2, 3, "Step 4: Confirm Installation");
    wattroff(modal, A_BOLD);

    // Render configuration summary.
    render_config_summary(modal, store);

    // Perform validations.
    int has_root = has_root_partition(store);
    int has_duplicate = has_duplicate_mount_points(store);
    int is_uefi = (access("/sys/firmware/efi", F_OK) == 0);
    int has_boot = has_required_boot_partition(store, is_uefi);
    int boot_too_small = is_boot_partition_too_small(store, is_uefi);
    int can_install = has_root && !has_duplicate && has_boot && !boot_too_small;

    // Render the appropriate message based on validation.
    if (has_duplicate)
    {
        render_duplicate_error(modal);
    }
    else if (!has_root)
    {
        render_no_root_error(modal);
    }
    else if (!has_boot)
    {
        render_no_boot_error(modal, is_uefi);
    }
    else if (boot_too_small)
    {
        render_boot_too_small_error(modal, is_uefi);
    }
    else
    {
        render_ready_message(modal, store);
    }

    wrefresh(modal);

    // Wait for user confirmation or back.
    int key;
    while ((key = getch()) != 27 && (key != '\n' || !can_install))
    {
        // Ignore other input.
    }

    return key == '\n' && can_install;
}
