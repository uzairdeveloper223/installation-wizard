/**
 * This code is responsible for providing dialog interfaces for adding, editing,
 * and removing partitions during the installation process.
 */

#include "../../all.h"

#define SIZE_COUNT 19
#define MOUNT_COUNT 7
#define FLAG_COUNT 4
#define TYPE_COUNT 2
#define FIELD_SIZE   0
#define FIELD_MOUNT  1
#define FIELD_TYPE   2
#define FIELD_FLAGS  3
#define FIELD_COUNT  4
#define DEFAULT_SIZE_INDEX 12
#define MIN_PARTITION_SIZE (1ULL * 1000000)

semistatic const unsigned long long size_presets[] =
{
    1ULL * 1000000,          // 1MB
    2ULL * 1000000,          // 2MB
    4ULL * 1000000,          // 4MB
    8ULL * 1000000,          // 8MB
    16ULL * 1000000,         // 16MB
    32ULL * 1000000,         // 32MB
    64ULL * 1000000,         // 64MB
    128ULL * 1000000,        // 128MB
    512ULL * 1000000,        // 512MB
    1ULL * 1000000000,       // 1GB
    2ULL * 1000000000,       // 2GB
    4ULL * 1000000000,       // 4GB
    8ULL * 1000000000,       // 8GB
    16ULL * 1000000000,      // 16GB
    32ULL * 1000000000,      // 32GB
    64ULL * 1000000000,      // 64GB
    128ULL * 1000000000,     // 128GB
    512ULL * 1000000000,     // 512GB
    1000ULL * 1000000000,    // 1TB
};

semistatic const char *size_labels[] =
{
    "1MB", "2MB", "4MB", "8MB", "16MB", "32MB", "64MB", "128MB", "512MB",
    "1GB", "2GB", "4GB", "8GB", "16GB", "32GB", "64GB", "128GB", "512GB", "1TB"
};

semistatic const char *mount_options[] = { "/", "/boot", "/boot/efi", "/home", "/var", "swap", "none" };
static const char *flag_options[] = { "none", "boot", "esp", "bios_grub" };
static const char *type_options[] = { "primary", "logical" };

semistatic int find_closest_size_index(unsigned long long size)
{
    // Iterate through presets to find the closest match.
    for (int i = 0; i < SIZE_COUNT - 1; i++)
    {
        // Return if preset is greater than or equal to size.
        if (size_presets[i] >= size)
        {
            return i;
        }

        // Check if size falls between current and next preset.
        if (i + 1 < SIZE_COUNT - 1 && size_presets[i + 1] > size)
        {
            // Return the preset that is closer to the given size.
            if (size - size_presets[i] < size_presets[i + 1] - size)
            {
                return i;
            }
            return i + 1;
        }
    }

    // Return largest size index if no closer match found.
    return SIZE_COUNT - 1;
}

semistatic int find_mount_index(const char *mount)
{
    // Handle swap partition mount point.
    if (strcmp(mount, "[swap]") == 0)
    {
        return 5;
    }

    // Handle unmounted partition.
    if (strcmp(mount, "[none]") == 0)
    {
        return 6;
    }

    // Search for matching mount point in options.
    for (int i = 0; i < MOUNT_COUNT; i++)
    {
        if (strcmp(mount_options[i], mount) == 0)
        {
            return i;
        }
    }

    // Return root mount point index if not found.
    return 0;
}

semistatic int find_flag_index(int boot, int esp, int bios_grub)
{
    // Return index based on which flag is set.
    if (boot) return 1;
    if (esp) return 2;
    if (bios_grub) return 3;
    return 0;
}

semistatic int has_duplicate_mount_point(Store *store, int mount_index, int edit_index)
{
    // Skip check for swap and none (duplicates allowed).
    if (mount_index == 5 || mount_index == 6)
    {
        return 0;
    }

    // Get the mount point string for the selected option.
    const char *mount = mount_options[mount_index];

    // Check all partitions for duplicates.
    for (int i = 0; i < store->partition_count; i++)
    {
        // Skip the partition being edited.
        if (i == edit_index)
        {
            continue;
        }

        if (strcmp(store->partitions[i].mount_point, mount) == 0)
        {
            return 1;
        }
    }

    return 0;
}

static int run_partition_form(
    WINDOW *modal, const char *title, const char *free_string,
    unsigned long long free_space,
    int *size_index, int *mount_index, int *type_index, int *flag_index,
    const char *footer_action, Store *store, int edit_index
)
{
    int focused = FIELD_SIZE;

    // Run main form loop.
    while (1)
    {
        // Check if selected size exceeds available space.
        unsigned long long selected_size = size_presets[*size_index];
        int exceeds = (selected_size > free_space);

        // Check for duplicate mount point.
        int duplicate = has_duplicate_mount_point(store, *mount_index, edit_index);

        // Set up form fields.
        const char *size_desc = exceeds
            ? "Selected size exceeds available space.\n"
              "It will be clamped to the remaining free space."
            : "The size you want this partition to be.\n"
              "Sizes exceeding free space will be clamped.";

        const char *mount_desc = duplicate
            ? "Another partition already uses this mount point.\n"
              "Choose a different mount point."
            : "Where this partition will be accessible.\n"
              "Filesystem (ext4, swap) is automatically chosen.";

        FormField fields[FIELD_COUNT] = {
            { "Size",       size_labels,   SIZE_COUNT,  *size_index,  0,
              size_desc, exceeds, 0 },
            { "Mount",      mount_options, MOUNT_COUNT, *mount_index, 0,
              mount_desc, 0, duplicate },
            { "Type",       type_options,  TYPE_COUNT,  *type_index,  0,
              "Partition type. Primary is standard for most uses.\n"
              "Use logical partitions inside extended partitions.", 0, 0 },
            { "Flags",      flag_options,  FLAG_COUNT,  *flag_index,  0,
              "Special flags for bootloader configuration.\n"
              "'esp' for UEFI, 'bios_grub' for BIOS+GPT.", 0, 0 }
        };

        // Clear modal and render dialog title.
        clear_modal(modal);
        wattron(modal, A_BOLD);
        mvwprintw(modal, 2, 3, "%s", title);
        wattroff(modal, A_BOLD);

        // Display free space indicator.
        print_dim(modal, 2, 3 + strlen(title) + 1, "(%s free)", free_string);

        // Render the form.
        render_form(modal, 4, 3, 11, fields, FIELD_COUNT, focused);

        // Render footer and refresh display.
        char action_string[32];
        snprintf(action_string, sizeof(action_string), "[Enter] %s", footer_action);
        const char *footer[] = {
            "[Arrows] Navigate", action_string, "[Esc] Cancel", NULL
        };
        render_footer(modal, footer);
        wrefresh(modal);

        // Handle user input.
        int key = getch();
        FormResult result = handle_form_key(key, fields, FIELD_COUNT, &focused);

        // Update indices from form fields after key handling.
        *size_index = fields[FIELD_SIZE].current;
        *mount_index = fields[FIELD_MOUNT].current;
        *type_index = fields[FIELD_TYPE].current;
        *flag_index = fields[FIELD_FLAGS].current;

        if (result == FORM_SUBMIT)
        {
            // Block submission if there's a duplicate mount point.
            if (duplicate)
            {
                show_notice(modal, NOTICE_ERROR, "Duplicate Mount Point",
                    "Another partition already uses this mount point.\n"
                    "Choose a different mount point.");
                continue;
            }
            return 0;
        }
        else if (result == FORM_CANCEL)
        {
            return -1;
        }
    }
}

static int select_partition(
    WINDOW *modal, Store *store, unsigned long long disk_size, const char *title
)
{
    int selected = 0;
    int scroll_offset = 0;

    // Run loop until user makes a selection or cancels.
    while (1)
    {
        // Clear modal and render selection header.
        clear_modal(modal);
        wattron(modal, A_BOLD);
        mvwprintw(modal, 2, 3, "%s", title);
        wattroff(modal, A_BOLD);

        // Render partition table with selection highlighting.
        render_partition_table(
            modal, store, disk_size, selected, 1, scroll_offset
        );

        // Render footer and refresh display.
        const char *footer[] =
        {
            "[Up][Down] Navigate", "[Enter] Select", "[Esc] Cancel", NULL
        };
        render_footer(modal, footer);
        wrefresh(modal);

        // Handle user input for partition selection.
        int key = getch();
        if (key == KEY_UP && selected > 0)
        {
            // Move selection up.
            selected--;
            if (selected < scroll_offset)
            {
                scroll_offset = selected;
            }
        }
        else if (key == KEY_DOWN && selected < store->partition_count - 1)
        {
            // Move selection down.
            selected++;
            if (selected >= scroll_offset + MAX_VISIBLE_PARTITIONS)
            {
                scroll_offset = selected - MAX_VISIBLE_PARTITIONS + 1;
            }
        }
        else if (key == '\n')
        {
            // Return selected partition index.
            return selected;
        }
        else if (key == 27)
        {
            // Return -1 to indicate user cancelled selection.
            return -1;
        }
    }
}

semistatic unsigned long long calculate_ideal_swap_size(unsigned long long ram_bytes)
{
    unsigned long long swap_size;

    // Define size thresholds in bytes.
    const unsigned long long GB_8 = 8ULL * 1000000000;
    const unsigned long long GB_16 = 16ULL * 1000000000;

    // Determine swap size based on RAM amount.
    if (ram_bytes < GB_8)
    {
        // For systems with less than 8GB RAM, swap equals RAM.
        swap_size = ram_bytes;
    }
    else if (ram_bytes <= GB_16)
    {
        // For 8-16GB RAM, use 4GB swap.
        swap_size = 4ULL * 1000000000;
    }
    else
    {
        // For more than 16GB RAM, use 4GB swap.
        swap_size = 4ULL * 1000000000;
    }

    // Round to nearest available size preset.
    int index = find_closest_size_index(swap_size);
    return size_presets[index];
}

int add_partition_dialog(
    WINDOW *modal, Store *store, unsigned long long disk_size
)
{
    // Check if maximum partition count has been reached.
    if (store->partition_count >= MAX_PARTITIONS)
    {
        show_notice(modal, NOTICE_ERROR, "Add Partition",
            "Maximum partition limit reached.\n"
            "Remove a partition before adding a new one.");
        return -1;
    }

    // Calculate available free space on disk.
    unsigned long long free_space = disk_size - sum_partition_sizes(
        store->partitions, store->partition_count
    );

    // Check if free space is below minimum partition size.
    if (free_space < MIN_PARTITION_SIZE)
    {
        show_notice(modal, NOTICE_ERROR, "Add Partition",
            "Insufficient free space on disk.\n"
            "Remove or resize a partition to continue.");
        return -2;
    }

    // Format free space for display.
    char free_string[32];
    format_disk_size(free_space, free_string, sizeof(free_string));

    // Initialize form field indices with defaults.
    int size_index = DEFAULT_SIZE_INDEX;
    int mount_index = 0;
    int type_index = 0;
    int flag_index = 0;

    // Run the partition form.
    if (run_partition_form(
        modal, "Add Partition", free_string, free_space, &size_index,
        &mount_index, &type_index, &flag_index, "Add", store, -1
    ) != 0)
    {
        return -3;
    }

    // Create new partition, clamping size to free space and minimum.
    Partition new_partition = {0};
    new_partition.size_bytes = size_presets[size_index];
    if (new_partition.size_bytes > free_space)
    {
        new_partition.size_bytes = free_space;
    }
    if (new_partition.size_bytes < MIN_PARTITION_SIZE)
    {
        new_partition.size_bytes = MIN_PARTITION_SIZE;
    }

    // Set mount point and filesystem based on selection.
    if (mount_index == 5)
    {
        snprintf(
            new_partition.mount_point,
            sizeof(new_partition.mount_point),
            "[swap]"
        );
        new_partition.filesystem = FS_SWAP;
    }
    else if (mount_index == 6)
    {
        snprintf(
            new_partition.mount_point,
            sizeof(new_partition.mount_point),
            "[none]"
        );
        new_partition.filesystem = FS_NONE;
    }
    else
    {
        snprintf(
            new_partition.mount_point, sizeof(new_partition.mount_point),
            "%s", mount_options[mount_index]
        );
        new_partition.filesystem = FS_EXT4;
    }

    // Set partition type and flags.
    new_partition.type = (type_index == 0) ? PART_PRIMARY : PART_LOGICAL;
    new_partition.flag_boot = (flag_index == 1);
    new_partition.flag_esp = (flag_index == 2);
    new_partition.flag_bios_grub = (flag_index == 3);

    // Override filesystem to FAT32 for ESP partitions.
    if (new_partition.flag_esp)
    {
        new_partition.filesystem = FS_FAT32;
    }

    // Override filesystem and mount point for BIOS boot partitions.
    if (new_partition.flag_bios_grub)
    {
        new_partition.filesystem = FS_NONE;
        strncpy(new_partition.mount_point, "[none]", sizeof(new_partition.mount_point));
    }

    // Add partition to store.
    store->partitions[store->partition_count++] = new_partition;
    return 0;
}

int edit_partition_dialog(
    WINDOW *modal, Store *store, unsigned long long disk_size
)
{
    // Return early if there are no partitions to edit.
    if (store->partition_count == 0)
    {
        return -1;
    }

    // Let user select which partition to edit.
    int selected = select_partition(
        modal, store, disk_size, "Edit Partition - Select"
    );
    if (selected < 0)
    {
        return -2;
    }

    // Get pointer to selected partition.
    Partition *p = &store->partitions[selected];

    // Calculate free space excluding current partition size.
    unsigned long long other_used = sum_partition_sizes(
        store->partitions, store->partition_count
    ) - p->size_bytes;
    unsigned long long free_space = disk_size - other_used;
    char free_string[32];
    format_disk_size(free_space, free_string, sizeof(free_string));

    // Initialize form fields from current partition values.
    int size_index = find_closest_size_index(p->size_bytes);
    int mount_index = find_mount_index(p->mount_point);
    int type_index = (p->type == PART_PRIMARY) ? 0 : 1;
    int flag_index = find_flag_index(p->flag_boot, p->flag_esp, p->flag_bios_grub);

    // Build title with partition number.
    char title[32];
    snprintf(title, sizeof(title), "Edit Partition %d", selected + 1);

    // Run the partition form.
    if (run_partition_form(
        modal, title, free_string, free_space,
        &size_index, &mount_index, &type_index, &flag_index, "Save",
        store, selected
    ) != 0)
    {
        return -3;
    }

    // Update partition size, clamping to free space and minimum.
    p->size_bytes = size_presets[size_index];
    if (p->size_bytes > free_space)
    {
        p->size_bytes = free_space;
    }
    if (p->size_bytes < MIN_PARTITION_SIZE)
    {
        p->size_bytes = MIN_PARTITION_SIZE;
    }

    // Update mount point and filesystem.
    if (mount_index == 5)
    {
        snprintf(p->mount_point, sizeof(p->mount_point), "[swap]");
        p->filesystem = FS_SWAP;
    }
    else if (mount_index == 6)
    {
        snprintf(p->mount_point, sizeof(p->mount_point), "[none]");
        p->filesystem = FS_NONE;
    }
    else
    {
        snprintf(
            p->mount_point, sizeof(p->mount_point),
            "%s",
            mount_options[mount_index]
        );
        p->filesystem = FS_EXT4;
    }

    // Update partition type and flags.
    p->type = (type_index == 0) ? PART_PRIMARY : PART_LOGICAL;
    p->flag_boot = (flag_index == 1);
    p->flag_esp = (flag_index == 2);
    p->flag_bios_grub = (flag_index == 3);

    // Override filesystem to FAT32 for ESP partitions.
    if (p->flag_esp)
    {
        p->filesystem = FS_FAT32;
    }

    // Override filesystem and mount point for BIOS boot partitions.
    if (p->flag_bios_grub)
    {
        p->filesystem = FS_NONE;
        strncpy(p->mount_point, "[none]", sizeof(p->mount_point));
    }

    return 0;
}

int remove_partition_dialog(
    WINDOW *modal, Store *store, unsigned long long disk_size
)
{
    // Return early if there are no partitions to remove.
    if (store->partition_count == 0)
    {
        return -1;
    }

    // Let user select which partition to remove.
    int selected = select_partition(
        modal, store, disk_size, "Remove Partition - Select"
    );
    if (selected < 0)
    {
        return -2;
    }

    // Remove partition by shifting remaining partitions down.
    for (int i = selected; i < store->partition_count - 1; i++)
    {
        store->partitions[i] = store->partitions[i + 1];
    }
    store->partition_count--;

    return 0;
}

int autofill_partitions(Store *store, unsigned long long disk_size)
{
    unsigned long long used_space = 0;

    // Detect system configuration.
    FirmwareType firmware = detect_firmware_type();
    DiskLabel disk_label = get_disk_label();
    unsigned long long ram_bytes = get_system_ram();

    // Use default 4GB if RAM detection fails.
    if (ram_bytes == 0)
    {
        ram_bytes = 4ULL * 1000000000;
    }

    // Clear existing partitions.
    store->partition_count = 0;

    // Create boot partition based on system type.
    // Note that BIOS + MBR doesn't need a special boot partition.
    if (firmware == FIRMWARE_UEFI)
    {
        // UEFI systems need ESP: 512MB, /boot/efi, FAT32, esp flag.
        Partition esp = {0};
        esp.size_bytes = 512ULL * 1000000;
        strncpy(esp.mount_point, "/boot/efi", sizeof(esp.mount_point));
        esp.filesystem = FS_FAT32;
        esp.type = PART_PRIMARY;
        esp.flag_esp = 1;

        // Add ESP partition to store.
        store->partitions[store->partition_count++] = esp;
        used_space += esp.size_bytes;
    }
    else if (disk_label == DISK_LABEL_GPT)
    {
        // BIOS + GPT needs bios_grub: 2MB, no mount, no FS, bios_grub flag.
        Partition bios_grub = {0};
        bios_grub.size_bytes = 2ULL * 1000000;
        strncpy(bios_grub.mount_point, "[none]", sizeof(bios_grub.mount_point));
        bios_grub.filesystem = FS_NONE;
        bios_grub.type = PART_PRIMARY;
        bios_grub.flag_bios_grub = 1;

        // Add bios_grub partition to store.
        store->partitions[store->partition_count++] = bios_grub;
        used_space += bios_grub.size_bytes;
    }

    // Calculate ideal swap size based on RAM.
    unsigned long long swap_size = calculate_ideal_swap_size(ram_bytes);

    // Ensure swap doesn't exceed remaining space (leave at least 1GB for root).
    unsigned long long remaining_for_swap = disk_size - used_space - (1ULL * 1000000000);
    if (swap_size > remaining_for_swap)
    {
        // Find largest preset that fits.
        swap_size = 0;
        for (int i = SIZE_COUNT - 1; i >= 0; i--)
        {
            if (size_presets[i] <= remaining_for_swap)
            {
                swap_size = size_presets[i];
                break;
            }
        }
    }

    // Add swap partition if size is valid.
    if (swap_size >= MIN_PARTITION_SIZE)
    {
        Partition swap = {0};
        swap.size_bytes = swap_size;
        strncpy(swap.mount_point, "[swap]", sizeof(swap.mount_point));
        swap.filesystem = FS_SWAP;
        swap.type = PART_PRIMARY;

        store->partitions[store->partition_count++] = swap;
        used_space += swap.size_bytes;
    }

    // Calculate remaining space for root partition.
    unsigned long long root_size = disk_size - used_space;

    // Find the largest size preset that fits.
    int root_index = SIZE_COUNT - 1;
    for (int i = SIZE_COUNT - 1; i >= 0; i--)
    {
        if (size_presets[i] <= root_size)
        {
            root_index = i;
            break;
        }
    }

    // Determine optimal root partition size:
    // If remaining space is within 5GB of a preset, use the preset for a cleaner number.
    // If the difference is larger than 5GB, fill the remaining space entirely.
    const unsigned long long THRESHOLD = 5ULL * 1000000000; // 5GB threshold
    unsigned long long preset_size = size_presets[root_index];
    unsigned long long difference = root_size - preset_size;
    unsigned long long final_root_size;

    if (difference <= THRESHOLD)
    {
        // Small difference: use the clean preset size.
        final_root_size = preset_size;
    }
    else
    {
        // Large difference: fill remaining space to avoid wasting disk space.
        final_root_size = root_size;
    }

    // Configure root partition with the calculated size.
    Partition root = {0};
    root.size_bytes = final_root_size;
    strncpy(root.mount_point, "/", sizeof(root.mount_point));
    root.filesystem = FS_EXT4;
    root.type = PART_PRIMARY;

    // Add root partition to store.
    store->partitions[store->partition_count++] = root;

    return 0;
}
