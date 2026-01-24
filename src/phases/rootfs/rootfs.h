#pragma once
#include "../all.h"

/**
 * Extracts the root filesystem archive to the target mount point.
 *
 * @return - `0` - on success.
 * @return - `-1` - if the rootfs archive does not exist.
 * @return - `-2` - if extraction fails.
 * @return - `-3` - if chroot mount setup fails.
 */
int extract_rootfs(void);
