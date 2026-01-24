#pragma once

/**
 * Generates /etc/fstab on the target system.
 *
 * Creates fstab entries for all configured partitions including
 * root, swap, and any additional mount points.
 *
 * @return - `0` - Success.
 * @return - `-1` - Failed to create fstab file.
 */
int generate_fstab(void);
