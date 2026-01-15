/**
 * This code is responsible for generating /etc/fstab on the target system
 * to ensure partitions are mounted correctly on boot.
 */

#include "../all.h"

static const char *get_fs_type_string(PartitionFS fs)
{
    switch (fs)
    {
        case FS_EXT4:  return "ext4";
        case FS_FAT32: return "vfat";
        case FS_SWAP:  return "swap";
        default:       return NULL;
    }
}

static const char *get_mount_options(PartitionFS fs, const char *mount_point)
{
    if (fs == FS_SWAP)
    {
        return "sw";
    }
    if (fs == FS_FAT32)
    {
        return "umask=0077";
    }
    if (strcmp(mount_point, "/") == 0)
    {
        return "errors=remount-ro";
    }
    return "defaults";
}

static int get_fs_passno(const char *mount_point, PartitionFS fs)
{
    if (fs == FS_SWAP) return 0;
    if (strcmp(mount_point, "/") == 0) return 1;
    return 2;
}

int generate_fstab(void)
{
    Store *store = get_store();
    const char *disk = store->disk;

    // Open fstab file for writing.
    FILE *fstab = fopen("/mnt/etc/fstab", "w");
    if (!fstab)
    {
        return -1;
    }

    // Write header comment.
    fprintf(fstab, "# /etc/fstab: static file system information.\n");
    fprintf(fstab, "# <device>  <mount>  <type>  <options>  <dump>  <pass>\n\n");

    // Generate entry for each partition.
    for (int i = 0; i < store->partition_count; i++)
    {
        Partition *partition = &store->partitions[i];

        // Skip partitions without a filesystem or mount point.
        const char *fs_type = get_fs_type_string(partition->filesystem);
        if (!fs_type) continue;

        // Get partition device path.
        char device[128];
        get_partition_device(disk, i + 1, device, sizeof(device));

        // Determine mount point (swap uses "none").
        const char *mount = (partition->filesystem == FS_SWAP) 
            ? "none" 
            : partition->mount_point;

        // Skip if no valid mount point.
        if (!mount || mount[0] == '\0') continue;

        // Get mount options and pass number.
        const char *options = get_mount_options(partition->filesystem, partition->mount_point);
        int passno = get_fs_passno(partition->mount_point, partition->filesystem);

        // Write fstab entry.
        fprintf(fstab, "%s\t%s\t%s\t%s\t0\t%d\n",
            device, mount, fs_type, options, passno);
    }

    fclose(fstab);
    return 0;
}
