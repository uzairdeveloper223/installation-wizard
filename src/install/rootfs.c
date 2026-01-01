#include "../all.h"

int extract_rootfs(void)
{
    Store *store = get_store();
    char cmd[1024];

    // In non-dry-run mode, ensure the rootfs archive exists.
    if (!store->dry_run)
    {
        if (access("assets/placeholder-rootfs.gz", F_OK) != 0)
        {
            return -1;
        }
    }

    // Extract the rootfs archive to /mnt.
    if (run_command("tar -xzf assets/placeholder-rootfs.gz -C /mnt 2>/dev/null") != 0)
    {
        return -2;
    }

    // Mount the root filesystem.
    snprintf(cmd, sizeof(cmd), "mount %s /mnt 2>/dev/null", store->disk);
    if (run_command(cmd) != 0)
    {
        return 1;
    }

    return 0;
}
