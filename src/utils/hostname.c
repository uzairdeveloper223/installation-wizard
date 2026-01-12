/**
 * This code is responsible for detecting system hardware characteristics
 * such as chassis type (laptop, desktop, etc.) using DMI information.
 */

#include "../all.h"

ChassisType detect_chassis_type(void)
{
    // Open the chassis type file from DMI information.
    FILE *file = fopen("/sys/class/dmi/id/chassis_type", "r");
    if (!file)
    {
        return CHASSIS_UNKNOWN;
    }

    // Read the chassis type code.
    int chassis_code = 0;
    if (fscanf(file, "%d", &chassis_code) != 1)
    {
        fclose(file);
        return CHASSIS_UNKNOWN;
    }
    fclose(file);

    // Return chassis type based on code.
    switch (chassis_code)
    {
        case 8:   // Portable
        case 9:   // Laptop
        case 10:  // Notebook
        case 14:  // Sub Notebook
        case 31:  // Convertible
        case 32:  // Detachable
            return CHASSIS_LAPTOP;

        case 3:   // Desktop
        case 4:   // Low Profile Desktop
        case 6:   // Mini Tower
        case 7:   // Tower
        case 13:  // All-in-One
        case 35:  // Mini PC
            return CHASSIS_DESKTOP;

        default:
            return CHASSIS_UNKNOWN;
    }
}

const char *get_default_hostname_suffix(void)
{
    // Detect the system chassis type.
    ChassisType type = detect_chassis_type();

    // Return suffix based on chassis type.
    if (type == CHASSIS_LAPTOP)
    {
        return "laptop";
    }
    return "pc";
}

void generate_hostname(const char *username, char *out_hostname, size_t out_hostname_size)
{
    // Get chassis suffix.
    const char *suffix = get_default_hostname_suffix();

    // Generate hostname as username-chassis.
    snprintf(out_hostname, out_hostname_size, "%s-%s", username, suffix);
}
