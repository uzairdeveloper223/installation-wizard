#pragma once

/**
 * Unmounts all filesystems mounted during installation.
 * Should be called on installation failure or completion.
 *
 * Unmounts in reverse order:
 * 1. Chroot bind mounts (/mnt/sys, /mnt/proc, /mnt/dev)
 * 2. EFI partition (/mnt/boot/efi)
 * 3. Swap partitions
 * 4. Other mount points (in reverse order for nested mounts)
 * 5. Root partition (/mnt)
 *
 * @return - `0` on success.
 * @return - `non-zero` if some unmounts failed (non-fatal).
 */
int cleanup_mounts(void);
