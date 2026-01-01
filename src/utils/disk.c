#include "../all.h"

void format_disk_size(unsigned long long bytes, char *out_buffer, size_t buffer_size)
{
    double size = (double)bytes;
    if (size >= 1e12) {
        snprintf(out_buffer, buffer_size, "%.1f TB", size / 1e12);
    } else if (size >= 1e9) {
        snprintf(out_buffer, buffer_size, "%.1f GB", size / 1e9);
    } else if (size >= 1e6) {
        snprintf(out_buffer, buffer_size, "%.1f MB", size / 1e6);
    } else {
        snprintf(out_buffer, buffer_size, "%llu B", bytes);
    }
}

unsigned long long get_disk_size(const char *disk_path)
{
    // Extract device name if full path provided (e.g., "/dev/sda" -> "sda").
    const char *device = strrchr(disk_path, '/');
    if (device == NULL) {
        device = disk_path;
    } else {
        device++;
    }

    // Prepare path to size file in `/sys/block`.
    char path[256];
    snprintf(path, sizeof(path), "/sys/block/%s/size", device);

    // Open the file and read the number of sectors.
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return 0;
    }
    unsigned long long sectors = 0;
    if (fscanf(file, "%llu", &sectors) != 1) {
        sectors = 0;
    }
    fclose(file);

    // Return sector size in bytes, 1 sector = 512 bytes.
    return sectors * 512;
}

int is_disk_removable(const char *device)
{
    // Prepare path to removable status file in `/sys/block`.
    char path[256];
    snprintf(path, sizeof(path), "/sys/block/%s/removable", device);

    // Open the file and read the removable flag.
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return 0;
    }
    int removable = 0;
    if (fscanf(file, "%d", &removable) != 1) {
        removable = 0;
    }
    fclose(file);

    return removable;
}

unsigned long long sum_partition_sizes(const struct Partition *partitions, int count)
{
    unsigned long long total = 0;
    for (int i = 0; i < count; i++) {
        total += partitions[i].size_bytes;
    }
    return total;
}

void get_partition_device(
    const char *disk, int part_num, char *out_buffer, size_t buffer_size
)
{
    // NVMe and MMC devices use 'p' separator (e.g., /dev/nvme0n1p1, /dev/mmcblk0p1).
    if (strstr(disk, "nvme") || strstr(disk, "mmcblk"))
    {
        snprintf(out_buffer, buffer_size, "%sp%d", disk, part_num);
    }
    else
    {
        // Standard devices append number directly (e.g., /dev/sda1).
        snprintf(out_buffer, buffer_size, "%s%d", disk, part_num);
    }
}
