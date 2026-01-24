#pragma once

/**
 * Installs LimeOS components and configures X11 on the target system.
 *
 * This function:
 * 1. Copies component binaries from live system to target
 * 2. Installs bundled component dependencies if present
 * 3. Writes X11 configuration files (xinitrc, startx.sh) so the installed
 *    system auto-starts X
 *
 * @return 0 on success, non-zero on failure.
 */
int install_components(void);
