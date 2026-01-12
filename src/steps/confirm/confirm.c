/**
 * This code is responsible for displaying a confirmation summary
 * of all user selections before proceeding with installation.
 */

#include "../../all.h"

/** Minimum sizes for boot-related partitions. */
#define ESP_MIN_SIZE_BYTES      (100ULL * 1000000)
#define BIOS_GRUB_MIN_SIZE_BYTES (1ULL * 1000000)
#define BOOT_PART_MIN_SIZE_BYTES (300ULL * 1000000)

/** Error codes for boot partition validation. */
typedef enum {
    BOOT_OK = 0,
    BOOT_ERR_UEFI_NO_ESP,
    BOOT_ERR_UEFI_ESP_NOT_FAT32,
    BOOT_ERR_UEFI_ESP_WRONG_MOUNT,
    BOOT_ERR_UEFI_ESP_TOO_SMALL,
    BOOT_ERR_UEFI_HAS_BIOS_GRUB,
    BOOT_ERR_BIOS_GPT_NO_BIOS_GRUB,
    BOOT_ERR_BIOS_GPT_BIOS_GRUB_HAS_FS,
    BOOT_ERR_BIOS_GPT_BIOS_GRUB_HAS_MOUNT,
    BOOT_ERR_BIOS_GPT_BIOS_GRUB_TOO_SMALL,
    BOOT_ERR_BIOS_GPT_HAS_ESP,
    BOOT_ERR_BOOT_TOO_SMALL,
    BOOT_ERR_BOOT_NO_FS,
    BOOT_ERR_BOOT_IS_BIOS_GRUB
} BootValidationError;

semistatic int has_root_partition(Store *store)
{
    // Search for a partition with root mount point.
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
    // Compare each partition's mount point against all subsequent partitions.
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

/** Finds a partition with the ESP flag set. */
static Partition *find_esp_partition(Store *store)
{
    // Search for a partition with the ESP flag enabled.
    for (int i = 0; i < store->partition_count; i++)
    {
        if (store->partitions[i].flag_esp)
        {
            return &store->partitions[i];
        }
    }

    return NULL;
}

/** Finds a partition with the bios_grub flag set. */
static Partition *find_bios_grub_partition(Store *store)
{
    // Search for a partition with the bios_grub flag enabled.
    for (int i = 0; i < store->partition_count; i++)
    {
        if (store->partitions[i].flag_bios_grub)
        {
            return &store->partitions[i];
        }
    }

    return NULL;
}

/** Finds a partition mounted at /boot. */
static Partition *find_boot_partition(Store *store)
{
    // Search for a partition with /boot mount point.
    for (int i = 0; i < store->partition_count; i++)
    {
        if (strcmp(store->partitions[i].mount_point, "/boot") == 0)
        {
            return &store->partitions[i];
        }
    }

    return NULL;
}

/**
 * Validates boot partition configuration for UEFI systems.
 * Requires ESP with FAT32, mounted at /boot/efi, size >= 100MB.
 * Forbids bios_grub partitions.
 */
semistatic BootValidationError validate_uefi_boot(Store *store)
{
    // Forbid bios_grub on UEFI systems.
    if (find_bios_grub_partition(store) != NULL)
    {
        return BOOT_ERR_UEFI_HAS_BIOS_GRUB;
    }

    // Require ESP partition.
    Partition *esp = find_esp_partition(store);
    if (esp == NULL)
    {
        return BOOT_ERR_UEFI_NO_ESP;
    }

    // Validate ESP filesystem is FAT32.
    if (esp->filesystem != FS_FAT32)
    {
        return BOOT_ERR_UEFI_ESP_NOT_FAT32;
    }

    // Validate ESP mount point.
    if (strcmp(esp->mount_point, "/boot/efi") != 0)
    {
        return BOOT_ERR_UEFI_ESP_WRONG_MOUNT;
    }

    // Validate ESP size.
    if (esp->size_bytes < ESP_MIN_SIZE_BYTES)
    {
        return BOOT_ERR_UEFI_ESP_TOO_SMALL;
    }

    return BOOT_OK;
}

/**
 * Validates boot partition configuration for BIOS + GPT systems.
 * Requires bios_grub partition with no filesystem, no mount, size >= 1MB.
 * Forbids ESP partitions.
 */
semistatic BootValidationError validate_bios_gpt_boot(Store *store)
{
    // Forbid ESP on BIOS + GPT systems.
    if (find_esp_partition(store) != NULL)
    {
        return BOOT_ERR_BIOS_GPT_HAS_ESP;
    }

    // Require bios_grub partition.
    Partition *bios_grub = find_bios_grub_partition(store);
    if (bios_grub == NULL)
    {
        return BOOT_ERR_BIOS_GPT_NO_BIOS_GRUB;
    }

    // Validate bios_grub has no filesystem.
    if (bios_grub->filesystem != FS_NONE)
    {
        return BOOT_ERR_BIOS_GPT_BIOS_GRUB_HAS_FS;
    }

    // Validate bios_grub has no mount point.
    if (bios_grub->mount_point[0] != '[' && bios_grub->mount_point[0] != '\0')
    {
        return BOOT_ERR_BIOS_GPT_BIOS_GRUB_HAS_MOUNT;
    }

    // Validate bios_grub size.
    if (bios_grub->size_bytes < BIOS_GRUB_MIN_SIZE_BYTES)
    {
        return BOOT_ERR_BIOS_GPT_BIOS_GRUB_TOO_SMALL;
    }

    return BOOT_OK;
}

semistatic BootValidationError validate_bios_mbr_boot(Store *store)
{
    (void)store;
    return BOOT_OK;
}

semistatic BootValidationError validate_optional_boot(Store *store)
{
    // Return early if no /boot partition is present.
    Partition *boot = find_boot_partition(store);
    if (boot == NULL)
    {
        return BOOT_OK;
    }

    // Validate /boot has a filesystem.
    if (boot->filesystem == FS_NONE)
    {
        return BOOT_ERR_BOOT_NO_FS;
    }

    // Validate /boot is not a bios_grub partition.
    if (boot->flag_bios_grub)
    {
        return BOOT_ERR_BOOT_IS_BIOS_GRUB;
    }

    // Validate /boot size.
    if (boot->size_bytes < BOOT_PART_MIN_SIZE_BYTES)
    {
        return BOOT_ERR_BOOT_TOO_SMALL;
    }

    return BOOT_OK;
}

semistatic BootValidationError validate_boot_config(
    Store *store, FirmwareType firmware, DiskLabel disk_label
)
{
    BootValidationError err;

    // Validate firmware-specific boot partition.
    if (firmware == FIRMWARE_UEFI)
    {
        err = validate_uefi_boot(store);
    }
    else if (disk_label == DISK_LABEL_GPT)
    {
        err = validate_bios_gpt_boot(store);
    }
    else
    {
        err = validate_bios_mbr_boot(store);
    }

    if (err != BOOT_OK)
    {
        return err;
    }

    // Validate optional /boot partition.
    return validate_optional_boot(store);
}

static void render_config_summary(WINDOW *modal, Store *store)
{
    // Display summary of selected options.
    mvwprintw(modal, 4, 3, "Ready to install LimeOS with the following settings:");
    mvwprintw(modal, 5, 3, "  Locale: %s", store->locale);
    mvwprintw(
        modal, 6, 3, "  User: %s (%s, %d total)",
        store->users[0].username, store->hostname, store->user_count
    );
    mvwprintw(modal, 7, 3, "  Disk: %s", store->disk);

    // Display partition summary.
    unsigned long long disk_size = get_disk_size(store->disk);
    unsigned long long used = sum_partition_sizes(store->partitions, store->partition_count);
    unsigned long long free_space = (disk_size > used) ? disk_size - used : 0;
    char free_string[32];
    format_disk_size(free_space, free_string, sizeof(free_string));

    if (store->partition_count > 0)
    {
        mvwprintw(
            modal, 8, 3,
            "  Partitions: %d partitions, %s left",
            store->partition_count, free_string
        );
    }
    else
    {
        mvwprintw(modal, 8, 3, "  Partitions: (none)");
    }
}

static void render_duplicate_error(WINDOW *modal)
{
    // Display error message for duplicate mount points.
    render_error(modal, 10, 3,
        "Multiple partitions share the same mount point.\n"
        "Go back and fix the configuration."
    );

    // Display footer with navigation options.
    const char *footer[] = {"[Esc] Back", NULL};
    render_footer(modal, footer);
}

static void render_no_root_error(WINDOW *modal)
{
    // Display error message for missing root partition.
    render_error(modal, 10, 3,
        "A root (/) partition is required.\n"
        "Go back and add one to continue."
    );

    // Display footer with navigation options.
    const char *footer[] = {"[Esc] Back", NULL};
    render_footer(modal, footer);
}

static void render_boot_validation_error(WINDOW *modal, BootValidationError err)
{
    // Select error message based on validation error code.
    const char *msg = NULL;
    switch (err)
    {
        case BOOT_ERR_UEFI_NO_ESP:
            msg = "UEFI boot requires an EFI System Partition.\n"
                  "Add: FAT32, Mount=/boot/efi, Flags=esp";
            break;
        case BOOT_ERR_UEFI_ESP_NOT_FAT32:
            msg = "EFI System Partition must be FAT32.\n"
                  "Go back and change the filesystem.";
            break;
        case BOOT_ERR_UEFI_ESP_WRONG_MOUNT:
            msg = "EFI System Partition must mount at /boot/efi.\n"
                  "Go back and set the mount point.";
            break;
        case BOOT_ERR_UEFI_ESP_TOO_SMALL:
            msg = "EFI System Partition must be at least 100MB.\n"
                  "Go back and resize it.";
            break;
        case BOOT_ERR_UEFI_HAS_BIOS_GRUB:
            msg = "UEFI systems cannot have a BIOS boot partition.\n"
                  "Remove the bios_grub partition.";
            break;
        case BOOT_ERR_BIOS_GPT_NO_BIOS_GRUB:
            msg = "GPT on BIOS requires a BIOS boot partition.\n"
                  "Add: 1-2MB, No filesystem, Flags=bios_grub";
            break;
        case BOOT_ERR_BIOS_GPT_BIOS_GRUB_HAS_FS:
            msg = "BIOS boot partition must have no filesystem.\n"
                  "Go back and set filesystem to 'none'.";
            break;
        case BOOT_ERR_BIOS_GPT_BIOS_GRUB_HAS_MOUNT:
            msg = "BIOS boot partition must have no mount point.\n"
                  "Go back and set mount to '[none]'.";
            break;
        case BOOT_ERR_BIOS_GPT_BIOS_GRUB_TOO_SMALL:
            msg = "BIOS boot partition must be at least 1MB.\n"
                  "Go back and resize it.";
            break;
        case BOOT_ERR_BIOS_GPT_HAS_ESP:
            msg = "BIOS systems cannot have an ESP partition.\n"
                  "Remove the ESP or switch flags to bios_grub.";
            break;
        case BOOT_ERR_BOOT_TOO_SMALL:
            msg = "/boot partition must be at least 300MB.\n"
                  "Go back and resize it.";
            break;
        case BOOT_ERR_BOOT_NO_FS:
            msg = "/boot partition must have a filesystem.\n"
                  "Go back and set a filesystem.";
            break;
        case BOOT_ERR_BOOT_IS_BIOS_GRUB:
            msg = "/boot cannot be a BIOS boot partition.\n"
                  "Go back and remove bios_grub flag.";
            break;
        default:
            msg = "Unknown boot configuration error.";
            break;
    }

    // Display the error message.
    render_error(modal, 10, 3, msg);

    // Display footer with navigation options.
    const char *footer[] = {"[Esc] Back", NULL};
    render_footer(modal, footer);
}

static void render_ready_message(WINDOW *modal, Store *store)
{
    // Display appropriate message based on dry-run mode.
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

    // Display footer with confirmation options.
    const char *footer[] = {"[Enter] Install", "[Esc] Back", NULL};
    render_footer(modal, footer);
}

int run_confirmation_step(WINDOW *modal)
{
    Store *store = get_store();

    // Clear and draw step header.
    clear_modal(modal);
    wattron(modal, A_BOLD | COLOR_PAIR(CUSTOM_COLOR_PAIR_MAIN));
    mvwprintw(modal, 2, 3, "Step 5: Confirm Installation");
    wattroff(modal, A_BOLD);

    // Render configuration summary.
    render_config_summary(modal, store);

    // Detect firmware and disk label.
    FirmwareType firmware = detect_firmware_type();
    DiskLabel disk_label = get_disk_label();

    // Perform validations.
    int has_root = has_root_partition(store);
    int has_duplicate = has_duplicate_mount_points(store);
    BootValidationError boot_error = validate_boot_config(store, firmware, disk_label);
    int can_install = has_root && !has_duplicate && (boot_error == BOOT_OK);

    // Render the appropriate message based on validation.
    if (has_duplicate)
    {
        render_duplicate_error(modal);
    }
    else if (!has_root)
    {
        render_no_root_error(modal);
    }
    else if (boot_error != BOOT_OK)
    {
        render_boot_validation_error(modal, boot_error);
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
