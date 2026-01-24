#pragma once

// ---
// Logging Configuration
// ---

/** The path to the installation log file. */
#define CONFIG_INSTALL_LOG_PATH "/tmp/limeos-install.log"

/** The path to the dry run log file. */
#define CONFIG_DRY_RUN_LOG_PATH "dry-run.log"

// ---
// System Paths
// ---

/** The path where the rootfs tarball is stored on the live system. */
#define CONFIG_ROOTFS_TARBALL_PATH "/usr/share/limeos/rootfs.tar.gz"

/** The mount point for the target system during installation. */
#define CONFIG_TARGET_MOUNT_POINT "/mnt"

// ---
// Component Configuration
// ---

/** Path where components are installed on the live system. */
#define CONFIG_LIVE_COMPONENT_PATH "/usr/local/bin"

/** Path where components are installed on the target system. */
#define CONFIG_TARGET_COMPONENT_PATH CONFIG_TARGET_MOUNT_POINT "/usr/local/bin"

/** Live system path for bundled component dependencies. */
#define CONFIG_LIVE_COMPONENT_DEPS_PATH "/var/cache/limeos/components"

/** Target xinitrc path. */
#define CONFIG_TARGET_XINITRC_PATH CONFIG_TARGET_MOUNT_POINT "/etc/X11/xinit/xinitrc"

/** Target startx profile path. */
#define CONFIG_TARGET_STARTX_PROFILE_PATH CONFIG_TARGET_MOUNT_POINT "/etc/profile.d/startx.sh"

/** A type representing an installable component. */
typedef struct {
    const char *binary_name;
    const char *deps_directory;
    int x11_startup;
} Component;

/** Installable LimeOS components. */
static const Component CONFIG_COMPONENTS[] = {
    { "limeos-window-manager",  "window-manager",  1 },
    { "limeos-display-manager", "display-manager", 0 },
};

/** The number of installable components. */
#define CONFIG_COMPONENT_COUNT \
    (int)(sizeof(CONFIG_COMPONENTS) / sizeof(CONFIG_COMPONENTS[0]))

