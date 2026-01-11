/**
 * This code is responsible for providing dialog interfaces for adding, editing,
 * and removing partitions during the installation process.
 */

#include "../../all.h"

#define SIZE_COUNT 19
#define MOUNT_COUNT 6
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

semistatic const char *mount_options[] = { "/", "/boot", "/home", "/var", "swap", "none" };
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
        return 4;
    }

    // Handle unmounted partition.
    if (strcmp(mount, "[none]") == 0)
    {
        return 5;
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

static int run_partition_form(
    WINDOW *modal, const char *title, const char *free_str,
    unsigned long long free_space,
    int *size_index, int *mount_index, int *type_index, int *flag_index,
    const char *footer_action
)
{
    int focused = FIELD_SIZE;

    // Run main form loop.
    while (1)
    {
        // Check if selected size exceeds available space.
        unsigned long long selected_size = size_presets[*size_index];
        int exceeds = (selected_size > free_space);

        // Set up form fields.
        const char *size_desc = exceeds
            ? "Selected size exceeds available space.\n"
              "It will be clamped to the remaining free space."
            : "The size you want this partition to be.\n"
              "Sizes exceeding free space will be clamped.";

        FormField fields[FIELD_COUNT] = {
            { "Size",       size_labels,   SIZE_COUNT,  *size_index,  0,
              size_desc, exceeds },
            { "Mount",      mount_options, MOUNT_COUNT, *mount_index, 0,
              "Where this partition will be accessible.\n"
              "Filesystem (ext4, swap) is automatically chosen.", 0 },
            { "Type",       type_options,  TYPE_COUNT,  *type_index,  0,
              "Partition type. Primary is standard for most uses.\n"
              "Use logical partitions inside extended partitions.", 0 },
            { "Flags",      flag_options,  FLAG_COUNT,  *flag_index,  0,
              "Special flags for bootloader configuration.\n"
              "'esp' for UEFI, 'bios_grub' for BIOS+GPT.", 0 }
        };

        // Clear modal and render dialog title.
        clear_modal(modal);
        wattron(modal, A_BOLD);
        mvwprintw(modal, 2, 3, "%s", title);
        wattroff(modal, A_BOLD);

        // Display free space indicator.
        print_dim(modal, 2, 3 + strlen(title) + 1, "(%s free)", free_str);

        // Render the form.
        render_form(modal, 4, 3, 11, fields, FIELD_COUNT, focused);

        // Render footer and refresh display.
        char action_str[32];
        snprintf(action_str, sizeof(action_str), "[Enter] %s", footer_action);
        const char *footer[] = {
            "[Arrows] Navigate", action_str, "[Esc] Cancel", NULL
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
            return 1;
        }
        else if (result == FORM_CANCEL)
        {
            return 0;
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

int add_partition_dialog(
    WINDOW *modal, Store *store, unsigned long long disk_size
)
{
    // Check if maximum partition count has been reached.
    if (store->partition_count >= STORE_MAX_PARTITIONS)
    {
        // Display error message to inform user of partition limit.
        clear_modal(modal);
        wattron(modal, A_BOLD);
        mvwprintw(modal, 2, 3, "Add Partition");
        wattroff(modal, A_BOLD);
        render_error(modal, 5, 3, "Maximum partition limit reached.\n"
            "Remove a partition before adding a new one.");
        const char *footer[] = { "[Enter] OK", NULL };
        render_footer(modal, footer);
        wrefresh(modal);

        // Wait for user to acknowledge the error.
        while (getch() != '\n');
        return 0;
    }

    // Calculate available free space on disk.
    unsigned long long free_space = disk_size - sum_partition_sizes(
        store->partitions, store->partition_count
    );

    // Check if free space is below minimum partition size.
    if (free_space < MIN_PARTITION_SIZE)
    {
        // Display error message to inform user of insufficient space.
        clear_modal(modal);
        wattron(modal, A_BOLD);
        mvwprintw(modal, 2, 3, "Add Partition");
        wattroff(modal, A_BOLD);
        render_error(modal, 5, 3, "Insufficient free space on disk.\n"
            "Remove or resize a partition to continue.");
        const char *footer[] = { "[Enter] OK", NULL };
        render_footer(modal, footer);
        wrefresh(modal);

        // Wait for user to acknowledge the error.
        while (getch() != '\n');
        return 0;
    }

    char free_str[32];
    format_disk_size(free_space, free_str, sizeof(free_str));

    // Initialize form field indices with defaults.
    int size_index = DEFAULT_SIZE_INDEX;
    int mount_index = 0;     // Default to /.
    int type_index = 0;      // Default to primary.
    int flag_index = 0;      // Default to none.

    // Run the partition form.
    if (!run_partition_form(
        modal, "Add Partition", free_str, free_space, &size_index,
        &mount_index, &type_index, &flag_index, "Add"
    ))
    {
        return 0;
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
    if (mount_index == 4)
    {
        snprintf(
            new_partition.mount_point,
            sizeof(new_partition.mount_point),
            "[swap]"
        );
        new_partition.filesystem = FS_SWAP;
    }
    else if (mount_index == 5)
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

    // ESP partitions must be FAT32.
    if (new_partition.flag_esp)
    {
        new_partition.filesystem = FS_FAT32;
    }

    // Add partition to store and return success.
    store->partitions[store->partition_count++] = new_partition;
    return 1;
}

int edit_partition_dialog(
    WINDOW *modal, Store *store, unsigned long long disk_size
)
{
    // Return early if there are no partitions to edit.
    if (store->partition_count == 0)
    {
        return 0;
    }

    // Let user select which partition to edit.
    int selected = select_partition(
        modal, store, disk_size, "Edit Partition - Select"
    );
    if (selected < 0)
    {
        return 0;
    }

    // Get pointer to selected partition.
    Partition *p = &store->partitions[selected];

    // Calculate free space excluding current partition size.
    unsigned long long other_used = sum_partition_sizes(
        store->partitions, store->partition_count
    ) - p->size_bytes;
    unsigned long long free_space = disk_size - other_used;
    char free_str[32];
    format_disk_size(free_space, free_str, sizeof(free_str));

    // Initialize form fields from current partition values.
    int size_index = find_closest_size_index(p->size_bytes);
    int mount_index = find_mount_index(p->mount_point);
    int type_index = (p->type == PART_PRIMARY) ? 0 : 1;
    int flag_index = find_flag_index(p->flag_boot, p->flag_esp, p->flag_bios_grub);

    // Build title with partition number.
    char title[32];
    snprintf(title, sizeof(title), "Edit Partition %d", selected + 1);

    // Run the partition form.
    if (!run_partition_form(
        modal, title, free_str, free_space,
        &size_index, &mount_index, &type_index, &flag_index, "Save"
    ))
    {
        return 0;
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
    if (mount_index == 4)
    {
        snprintf(p->mount_point, sizeof(p->mount_point), "[swap]");
        p->filesystem = FS_SWAP;
    }
    else if (mount_index == 5)
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

    // ESP partitions must be FAT32.
    if (p->flag_esp)
    {
        p->filesystem = FS_FAT32;
    }

    return 1;
}

int remove_partition_dialog(
    WINDOW *modal, Store *store, unsigned long long disk_size
)
{
    // Return early if there are no partitions to remove.
    if (store->partition_count == 0)
    {
        return 0;
    }

    // Let user select which partition to remove.
    int selected = select_partition(
        modal, store, disk_size, "Remove Partition - Select"
    );
    if (selected < 0)
    {
        return 0;
    }

    // Remove partition by shifting remaining partitions down.
    for (int i = selected; i < store->partition_count - 1; i++)
    {
        store->partitions[i] = store->partitions[i + 1];
    }
    store->partition_count--;

    return 1;
}
