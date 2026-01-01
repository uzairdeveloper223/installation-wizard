#include "../../all.h"

const char *fs_to_string(PartitionFS fs)
{
    switch (fs)
    {
        case FS_EXT4: return "ext4";
        case FS_SWAP: return "swap";
        default: return "?";
    }
}

const char *type_to_string(PartitionType type)
{
    switch (type)
    {
        case PART_PRIMARY: return "primary";
        case PART_LOGICAL: return "logical";
        default: return "?";
    }
}

void render_partition_table(
    WINDOW *modal, Store *store, unsigned long long disk_size,
    int selected_partition, int in_partition_select_mode,
    int scroll_offset
)
{
    char size_str[32];
    char disk_size_str[32];
    char free_str[32];

    // Format disk size and free space strings.
    format_disk_size(disk_size, disk_size_str, sizeof(disk_size_str));
    unsigned long long used = sum_partition_sizes(
        store->partitions, store->partition_count
    );
    unsigned long long free_space = (disk_size > used) ? disk_size - used : 0;
    format_disk_size(free_space, free_str, sizeof(free_str));

    // Display header with disk info and free space.
    mvwprintw(
        modal,
        4, 3,
        "%s (%s, %s free)",
        store->disk, disk_size_str, free_str
    );

    // Calculate the table width, reducing by 1 if scrollbar is needed.
    int table_width = MODAL_WIDTH - 6;
    if (store->partition_count > MAX_VISIBLE_PARTITIONS)
    {
        table_width--;
    }

    // Render column headers with darker background.
    wattron(modal, COLOR_PAIR(UI_COLOR_HEADER));
    char header[64];
    snprintf(
        header, sizeof(header),
        " #  %-10s %-8s %-5s %-8s %-6s",
        "Size", "Mount", "FS", "Type", "Flags"
    );
    mvwprintw(modal, 6, 3, "%-*s", table_width, header);
    wattroff(modal, COLOR_PAIR(UI_COLOR_HEADER));

    // Render partition rows.
    for (int i = 0; i < MAX_VISIBLE_PARTITIONS; i++)
    {
        int part_index = scroll_offset + i;

        // Apply alternating row background color.
        int row_color = (part_index % 2 == 0) ? UI_COLOR_ROW_ODD : UI_COLOR_ROW_EVEN;
        wattron(modal, COLOR_PAIR(row_color));

        if (part_index < store->partition_count)
        {
            // Format partition data for display.
            Partition *p = &store->partitions[part_index];
            format_disk_size(p->size_bytes, size_str, sizeof(size_str));

            // Build flags string from partition flags.
            char flags[16] = "";
            if (p->flag_boot) strcat(flags, "boot ");
            if (p->flag_esp) strcat(flags, "esp");

            // Use "[swap]" label for swap partitions.
            const char *mount = (p->filesystem == FS_SWAP)
                ? "[swap]" : p->mount_point;

            // Highlight selected partition in selection mode.
            if (in_partition_select_mode && part_index == selected_partition)
            {
                wattron(modal, A_REVERSE);
            }

            // Render the partition row.
            char row[64];
            snprintf(
                row, sizeof(row),
                " %-2d %-10s %-8s %-5s %-8s %-6s",
                part_index + 1, size_str, mount,
                fs_to_string(p->filesystem),
                type_to_string(p->type), flags
            );
            mvwprintw(modal, 7 + i, 3, "%-*s", table_width, row);

            // Remove highlight after rendering.
            if (in_partition_select_mode && part_index == selected_partition)
            {
                wattroff(modal, A_REVERSE);
            }
        }
        else
        {
            // Render empty row for unused table slots.
            mvwprintw(modal, 7 + i, 3, "%-*s", table_width, "");
        }

        wattroff(modal, COLOR_PAIR(row_color));
    }

    // Draw scrollbar if there are more partitions than visible rows.
    render_scrollbar(
        modal, MODAL_TABLE_START_Y, MODAL_WIDTH - 3,
        MAX_VISIBLE_PARTITIONS, scroll_offset,
        MAX_VISIBLE_PARTITIONS, store->partition_count
    );
}
