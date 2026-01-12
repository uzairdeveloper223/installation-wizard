#pragma once
#include "../all.h"

/** A type representing system chassis types. */
typedef enum
{
    CHASSIS_DESKTOP,
    CHASSIS_LAPTOP,
    CHASSIS_UNKNOWN
} ChassisType;

/**
 * Detects the system chassis type by reading DMI information.
 * Checks /sys/class/dmi/id/chassis_type for the chassis type code.
 *
 * Common chassis type codes:
 * - Desktop: 3 (Desktop), 4 (Low Profile Desktop), 6 (Mini Tower),
 *            7 (Tower), 13 (All-in-One), 35 (Mini PC)
 * - Laptop: 8 (Portable), 9 (Laptop), 10 (Notebook), 14 (Sub Notebook),
 *           31 (Convertible), 32 (Detachable)
 *
 * @return The detected chassis type.
 */
ChassisType detect_chassis_type(void);

/**
 * Returns the default hostname suffix based on chassis type.
 *
 * @return "laptop" for laptops, "pc" for desktops and unknown systems.
 */
const char *get_default_hostname_suffix(void);

/**
 * Generates a hostname from the given username and chassis type.
 *
 * @param username The username to use as hostname prefix.
 * @param out_hostname Buffer to write the generated hostname.
 * @param out_hostname_size Size of the output buffer.
 */
void generate_hostname(const char *username, char *out_hostname, size_t out_hostname_size);
