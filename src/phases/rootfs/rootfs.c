/**
 * This code is responsible for extracting the root filesystem archive
 * to the target mount point during installation.
 */

#include "../../all.h"

int extract_rootfs(void)
{
    Store *store = get_store();

    // In non-dry-run mode, ensure the rootfs archive exists.
    if (!store->dry_run)
    {
        write_install_log("Checking for rootfs archive at %s", CONFIG_ROOTFS_TARBALL_PATH);
        if (access(CONFIG_ROOTFS_TARBALL_PATH, F_OK) != 0)
        {
            write_install_log("Rootfs archive not found");
            return -1;
        }
    }

    // Extract the rootfs archive to /mnt.
    // Note: Root partition is already mounted by create_partitions().
    write_install_log("Extracting rootfs to /mnt");
    if (run_command("tar -xzf " CONFIG_ROOTFS_TARBALL_PATH " -C /mnt >>" CONFIG_INSTALL_LOG_PATH " 2>&1") != 0)
    {
        write_install_log("Rootfs extraction failed");
        return -2;
    }

    write_install_log("Rootfs extraction complete");
    return 0;
}
