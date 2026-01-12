/**
 * This code is responsible for detecting available block devices
 * and presenting them for user selection.
 */

#include "../all.h"

int populate_disk_options(StepOption *out_options, int max_count)
{
    // Open `/sys/block` to read block devices.
    DIR *dir = opendir("/sys/block");
    if (dir == NULL)
    {
        // Use fallback if `/sys/block` is unavailable.
        snprintf(out_options[0].value, sizeof(out_options[0].value), "/dev/sda");
        snprintf(out_options[0].label, sizeof(out_options[0].label), "/dev/sda (Unknown size)");
        return 1;
    }

    // Iterate over block devices in `/sys/block`.
    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && count < max_count)
    {
        const char *name = entry->d_name;

        // Skip virtual and special devices.
        if (strncmp(name, "loop", 4) == 0 ||
            strncmp(name, "ram", 3) == 0 ||
            strncmp(name, "dm-", 3) == 0 ||
            strncmp(name, "sr", 2) == 0 ||
            strncmp(name, "fd", 2) == 0 ||
            strcmp(name, ".") == 0 ||
            strcmp(name, "..") == 0)
        {
            continue;
        }

        // Get disk size and format label.
        unsigned long long size = get_disk_size(name);
        if (size == 0)
        {
            continue;
        }

        // Format size string.
        char size_str[32];
        format_disk_size(size, size_str, sizeof(size_str));

        // Mark removable disks in the label.
        const char *removable_tag = is_disk_removable(name) ? " [Removable]" : "";

        // Populate option entry.
        snprintf(out_options[count].value, sizeof(out_options[count].value), "/dev/%s", name);
        snprintf(
            out_options[count].label,
            sizeof(out_options[count].label),
            "/dev/%s - %s%s",
            name, size_str, removable_tag
        );
        count++;
    }

    // Close the directory previously opened for reading block devices.
    closedir(dir);

    // Ensure at least one fallback option exists.
    if (count == 0)
    {
        snprintf(out_options[0].value, sizeof(out_options[0].value), "/dev/sda");
        snprintf(out_options[0].label, sizeof(out_options[0].label), "/dev/sda (No disks detected)");
        return 1;
    }

    return count;
}

int run_disk_step(WINDOW *modal)
{
    Store *store = get_store();
    StepOption options[STEPS_MAX_OPTIONS];

    // Populate options with available disks.
    int count = populate_disk_options(options, STEPS_MAX_OPTIONS);

    // Mark previously selected disk if any.
    int selected = 0;
    if (store->disk[0] != '\0')
    {
        for (int i = 0; i < count; i++)
        {
            if (strcmp(options[i].value, store->disk) == 0)
            {
                selected = i;
                // Append "*" to the label.
                size_t len = strlen(options[i].label);
                if (len + 3 < sizeof(options[i].label))
                {
                    strcat(options[i].label, " *");
                }
                break;
            }
        }
    }

    // Run selection step for disk choice.
    int result = run_selection_step(
        modal,                                      // Modal window.
        "Disk Selection",                           // Step title.
        3,                                          // Step ID.
        "Select the target disk for installation:", // Step prompt.
        options,                                    // Options array.
        count,                                      // Number of options.
        &selected,                                  // Selected index pointer.
        1                                           // Allow multi-select.
    );
    if (result)
    {
        // Store the selected disk in global store.
        snprintf(store->disk, sizeof(store->disk), "%s", options[selected].value);
    }

    return result;
}
