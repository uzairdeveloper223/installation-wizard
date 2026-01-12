/**
 * This code is responsible for disk-related utility functions including
 * size formatting, device detection, and partition path construction.
 */

#include "../all.h"

static int is_valid_device_name(const char *device)
{
    // Ensure device name is not empty.
    if (device == NULL || device[0] == '\0')
    {
        return 0;
    }

    // Allow only alphanumeric characters and underscores.
    for (const char *character = device; *character != '\0'; character++)
    {
        if (!isalnum((unsigned char)*character) && *character != '_')
        {
            return 0;
        }
    }

    return 1;
}

void format_disk_size(unsigned long long bytes, char *out_buffer, size_t buffer_size)
{
    // Format size with appropriate unit based on magnitude.
    double size = (double)bytes;
    if (size >= 1e12)
    {
        snprintf(out_buffer, buffer_size, "%.0f TB", size / 1e12);
    }
    else if (size >= 1e9)
    {
        snprintf(out_buffer, buffer_size, "%.0f GB", size / 1e9);
    }
    else if (size >= 1e6)
    {
        snprintf(out_buffer, buffer_size, "%.0f MB", size / 1e6);
    }
    else
    {
        snprintf(out_buffer, buffer_size, "%llu B", bytes);
    }
}

unsigned long long get_disk_size(const char *disk_path)
{
    // Extract device name if full path provided (e.g., "/dev/sda" -> "sda").
    const char *device = strrchr(disk_path, '/');
    if (device == NULL)
    {
        device = disk_path;
    }
    else
    {
        device++;
    }

    // Validate device name to prevent path traversal.
    if (!is_valid_device_name(device))
    {
        return 0;
    }

    // Prepare path to size file in `/sys/block`.
    char path[256];
    snprintf(path, sizeof(path), "/sys/block/%s/size", device);

    // Open the file and read the number of sectors.
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        return 0;
    }
    unsigned long long sectors = 0;
    if (fscanf(file, "%llu", &sectors) != 1)
    {
        sectors = 0;
    }
    fclose(file);

    // Return sector size in bytes, 1 sector = 512 bytes.
    return sectors * 512;
}

int is_disk_removable(const char *device)
{
    // Validate device name to prevent path traversal.
    if (!is_valid_device_name(device))
    {
        return 0;
    }

    // Prepare path to removable status file in `/sys/block`.
    char path[256];
    snprintf(path, sizeof(path), "/sys/block/%s/removable", device);

    // Open the file and read the removable flag.
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        return 0;
    }
    int removable = 0;
    if (fscanf(file, "%d", &removable) != 1)
    {
        removable = 0;
    }
    fclose(file);

    return removable;
}

unsigned long long sum_partition_sizes(const struct Partition *partitions, int count)
{
    unsigned long long total = 0;
    for (int i = 0; i < count; i++)
    {
        total += partitions[i].size_bytes;
    }
    return total;
}

void get_partition_device(
    const char *disk, int partition_number, char *out_buffer, size_t buffer_size
)
{
    // Use 'p' separator for NVMe and MMC devices
    // (e.g., `/dev/nvme0n1p1`, `/dev/mmcblk0p1`).
    if (strstr(disk, "nvme") || strstr(disk, "mmcblk"))
    {
        snprintf(out_buffer, buffer_size, "%sp%d", disk, partition_number);
    }
    else
    {
        // Append number directly for standard devices (e.g., /dev/sda1).
        snprintf(out_buffer, buffer_size, "%s%d", disk, partition_number);
    }
}

FirmwareType detect_firmware_type(void)
{
    Store *store = get_store();

    // Check for forced firmware type (for testing).
    if (store->force_uefi == 1)
    {
        return FIRMWARE_UEFI;
    }
    if (store->force_uefi == 2)
    {
        return FIRMWARE_BIOS;
    }

    // Auto-detect by checking for EFI firmware directory.
    if (access("/sys/firmware/efi", F_OK) == 0)
    {
        return FIRMWARE_UEFI;
    }
    return FIRMWARE_BIOS;
}

DiskLabel get_disk_label(void)
{
    Store *store = get_store();

    // Check for forced disk label (for testing).
    if (store->force_disk_label == 1)
    {
        return DISK_LABEL_GPT;
    }
    if (store->force_disk_label == 2)
    {
        return DISK_LABEL_MBR;
    }

    // Default to GPT for modern systems.
    return DISK_LABEL_GPT;
}
