#pragma once
#include "../all.h"

/**
 * Formats a byte count into a human-readable size string.
 *
 * @param bytes The size in bytes to format.
 * @param out_buffer Output buffer for the formatted string.
 * @param buffer_size Size of the output buffer.
 */
void format_disk_size(
    unsigned long long bytes, char *out_buffer, size_t buffer_size
);

/**
 * Gets the size of a disk in bytes by reading from /sys/block.
 * Accepts either a device name (e.g., "sda") or full path (e.g., "/dev/sda").
 *
 * @param disk_path Device name or full path to the disk.
 *
 * @return - `>0` - Size in bytes.
 * @return - `0` - Size unavailable.
 */
unsigned long long get_disk_size(const char *disk_path);

/**
 * Checks if a block device is removable.
 *
 * @param device Device name (e.g., "sda").
 *
 * @return - `1` - The device is removable.
 * @return - `0` - The device is not removable.
 */
int is_disk_removable(const char *device);

/**
 * Sums the sizes of all partitions in an array.
 *
 * @param partitions Array of partition structures.
 * @param count Number of partitions.
 *
 * @return Total size in bytes.
 */
unsigned long long sum_partition_sizes(
    const struct Partition *partitions, int count
);

/**
 * Constructs a partition device path from a disk path and partition number.
 * Handles NVMe and MMC device naming conventions (e.g., /dev/nvme0n1p1).
 *
 * @param disk             The disk device path (e.g., "/dev/sda" or "/dev/nvme0n1").
 * @param partition_number The partition number (1-indexed).
 * @param out_buffer       Buffer to store the partition path.
 * @param buffer_size      Size of the output buffer.
 */
void get_partition_device(
    const char *disk, int partition_number, char *out_buffer, size_t buffer_size
);

/**
 * Detects the system firmware type (UEFI or BIOS).
 * Checks for the presence of /sys/firmware/efi to determine UEFI.
 * Respects the store's force_uefi setting for testing.
 *
 * @return The detected firmware type.
 */
FirmwareType detect_firmware_type(void);

/**
 * Gets the disk label type that will be used for partitioning.
 * Currently defaults to GPT. Respects the store's force_disk_label setting.
 *
 * @return The disk label type to use.
 */
DiskLabel get_disk_label(void);
