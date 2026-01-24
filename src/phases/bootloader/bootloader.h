#pragma once
#include "../all.h"

/**
 * Installs and configures the bootloader on the target disk.
 *
 * @return - `0` - on success.
 * @return - `-1` - on failure.
 */
int setup_bootloader(void);
