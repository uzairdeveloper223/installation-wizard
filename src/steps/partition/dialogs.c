#include "../../all.h"

#define SIZE_COUNT 20
#define MOUNT_COUNT 5
#define FLAG_COUNT 3
#define TYPE_COUNT 2
#define FIELD_SIZE   0
#define FIELD_MOUNT  1
#define FIELD_TYPE   2
#define FIELD_FLAGS  3
#define FIELD_COUNT  4

static const unsigned long long size_presets[] =
{
    64ULL * 1000000,         // 64M
    128ULL * 1000000,        // 128M
    256ULL * 1000000,        // 256M
    512ULL * 1000000,        // 512M
    1ULL * 1000000000,       // 1G
    2ULL * 1000000000,       // 2G
    4ULL * 1000000000,       // 4G
    8ULL * 1000000000,       // 8G
    16ULL * 1000000000,      // 16G
    32ULL * 1000000000,      // 32G
    64ULL * 1000000000,      // 64G
    128ULL * 1000000000,     // 128G
    256ULL * 1000000000,     // 256G
    512ULL * 1000000000,     // 512G
    1000ULL * 1000000000,    // 1T
    2000ULL * 1000000000,    // 2T
    4000ULL * 1000000000,    // 4T
    5000ULL * 1000000000,    // 5T
    8000ULL * 1000000000,    // 8T
    0                        // "Rest"
};

static const char *size_labels[] =
{
    "64MB", "128MB", "256MB", "512MB", "1GB", "2GB", "4GB", "8GB",
    "16GB", "32GB", "64GB", "128GB", "256GB", "512GB",
    "1TB", "2TB", "4TB", "5TB", "8TB", "Rest"
};

static const char *mount_options[] = { "/", "/boot", "/home", "/var", "swap" };
static const char *flag_options[] = { "none", "boot", "esp" };
static const char *type_options[] = { "primary", "logical" };

static int find_closest_size_idx(unsigned long long size)
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

    // Default to "Rest" if no closer match found.
    return SIZE_COUNT - 1;
}

static int find_mount_idx(const char *mount)
{
    // Handle swap partition mount point.
    if (strcmp(mount, "[swap]") == 0)
    {
        return 4;
    }

    // Search for matching mount point in options.
    for (int i = 0; i < MOUNT_COUNT; i++)
    {
        if (strcmp(mount_options[i], mount) == 0)
        {
            return i;
        }
    }

    // Default to root mount point if not found.
    return 0;
}

static int find_flag_idx(int boot, int esp)
{
    // Return index based on which flag is set.
    if (boot) return 1;
    if (esp) return 2;
    return 0;
}

static int run_partition_form(
    WINDOW *modal, const char *title, const char *free_str,
    int *size_idx, int *mount_idx, int *type_idx, int *flag_idx,
    const char *footer_action
)
{
    int focused = FIELD_SIZE;

    // Main form loop.
    while (1)
    {
        // Set up form fields.
        FormField fields[FIELD_COUNT] = {
            { "Size",       size_labels,   SIZE_COUNT,  *size_idx,  0,
              "The size you want this partition to be.\n"
              "Use 'Rest' to allocate all remaining disk space." },
            { "Mount",      mount_options, MOUNT_COUNT, *mount_idx, 0,
              "Where this partition will be accessible.\n"
              "Filesystem (ext4, swap) is automatically chosen." },
            { "Type",       type_options,  TYPE_COUNT,  *type_idx,  0,
              "Partition type. Primary is standard for most uses.\n"
              "Use logical partitions inside extended partitions." },
            { "Flags",      flag_options,  FLAG_COUNT,  *flag_idx,  0,
              "Special flags for bootloader configuration.\n"
              "'esp' for UEFI boot, 'boot' for legacy BIOS." }
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
        int key = wgetch(modal);
        FormResult result = handle_form_key(key, fields, FIELD_COUNT, &focused);

        // Update indices from form fields after key handling.
        *size_idx = fields[FIELD_SIZE].current;
        *mount_idx = fields[FIELD_MOUNT].current;
        *type_idx = fields[FIELD_TYPE].current;
        *flag_idx = fields[FIELD_FLAGS].current;

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

    // Loop indefinitely until user makes a selection or cancels.
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
        int key = wgetch(modal);
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
            // User cancelled selection.
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
        return 0;
    }

    // Calculate available free space on disk.
    unsigned long long free_space = disk_size - sum_partition_sizes(
        store->partitions, store->partition_count
    );
    char free_str[32];
    format_disk_size(free_space, free_str, sizeof(free_str));

    // Initialize form field indices with defaults.
    int size_idx = 11;     // Default to 128G.
    int mount_idx = 0;     // Default to /.
    int type_idx = 0;      // Default to primary.
    int flag_idx = 0;      // Default to none.

    // Run the partition form.
    if (!run_partition_form(
        modal, "Add Partition", free_str, &size_idx,
        &mount_idx, &type_idx, &flag_idx, "Add"
    ))
    {
        return 0;
    }

    Partition new_partition = {0};

    // Set partition size, using remaining space for "Rest".
    if (size_presets[size_idx] == 0)
    {
        new_partition.size_bytes = free_space;
    }
    else
    {
        new_partition.size_bytes = size_presets[size_idx];
    }

    // Clamp size to available free space.
    if (new_partition.size_bytes > free_space)
    {
        new_partition.size_bytes = free_space;
    }

    // Set mount point and filesystem based on selection.
    if (mount_idx == 4)
    {
        snprintf(new_partition.mount_point, sizeof(new_partition.mount_point),
                 "[swap]");
        new_partition.filesystem = FS_SWAP;
    }
    else
    {
        snprintf(new_partition.mount_point, sizeof(new_partition.mount_point),
                 "%s", mount_options[mount_idx]);
        new_partition.filesystem = FS_EXT4;
    }

    // Set partition type and flags.
    new_partition.type = (type_idx == 0) ? PART_PRIMARY : PART_LOGICAL;
    new_partition.flag_boot = (flag_idx == 1);
    new_partition.flag_esp = (flag_idx == 2);

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
    int size_idx = find_closest_size_idx(p->size_bytes);
    int mount_idx = find_mount_idx(p->mount_point);
    int type_idx = (p->type == PART_PRIMARY) ? 0 : 1;
    int flag_idx = find_flag_idx(p->flag_boot, p->flag_esp);

    // Build title with partition number.
    char title[32];
    snprintf(title, sizeof(title), "Edit Partition %d", selected + 1);

    // Run the partition form.
    if (!run_partition_form(
        modal, title, free_str, &size_idx, &mount_idx, &type_idx, &flag_idx, "Save"
    ))
    {
        return 0;
    }

    // Update partition size from form value.
    if (size_presets[size_idx] == 0)
    {
        p->size_bytes = free_space;
    }
    else
    {
        p->size_bytes = size_presets[size_idx];
    }

    // Clamp size to available free space.
    if (p->size_bytes > free_space)
    {
        p->size_bytes = free_space;
    }

    // Update mount point and filesystem.
    if (mount_idx == 4)
    {
        snprintf(p->mount_point, sizeof(p->mount_point), "[swap]");
        p->filesystem = FS_SWAP;
    }
    else
    {
        snprintf(p->mount_point, sizeof(p->mount_point), "%s",
                 mount_options[mount_idx]);
        p->filesystem = FS_EXT4;
    }

    // Update partition type and flags.
    p->type = (type_idx == 0) ? PART_PRIMARY : PART_LOGICAL;
    p->flag_boot = (flag_idx == 1);
    p->flag_esp = (flag_idx == 2);
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
