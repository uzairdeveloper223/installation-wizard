/**
 * This code is responsible for configuring the system locale settings
 * by enabling the selected locale and generating locale data.
 */

#include "../all.h"

static int is_valid_locale(const char *locale)
{
    if (locale == NULL || locale[0] == '\0')
    {
        return 0;
    }

    // Check length to prevent overly long inputs.
    size_t len = strlen(locale);
    if (len > 32)
    {
        return 0;
    }

    // Validate characters: only allow alphanumeric, underscore, dot, hyphen, 
    // and at-sign. These are the only characters valid in locale identifiers, 
    // ensuring both semantic validity and shell/sed safety without needing
    // additional escaping.
    int has_underscore = 0;
    for (size_t i = 0; i < len; i++)
    {
        char c = locale[i];
        if (c == '_')
        {
            has_underscore = 1;
        }
        if (!isalnum((unsigned char)c) && c != '_' && c != '.' && c != '-' && c != '@')
        {
            return 0;
        }
    }

    return has_underscore;
}

int configure_locale(void)
{
    Store *store = get_store();

    // Validate locale format and characters. The character validation ensures 
    // shell/sed safety for this constrained input.
    if (!is_valid_locale(store->locale))
    {
        return -1;
    }

    char cmd[512];

    // Enable the selected locale in /etc/locale.gen.
    // This uncomments the line matching the locale.
    snprintf(cmd, sizeof(cmd),
        "sed -i '/^# %s/s/^# //' /mnt/etc/locale.gen >>" INSTALL_LOG_PATH " 2>&1",
        store->locale);
    if (run_command(cmd) != 0)
    {
        return -2;
    }

    // Generate locales inside the chroot.
    if (run_command("chroot /mnt /usr/sbin/locale-gen >>" INSTALL_LOG_PATH " 2>&1") != 0)
    {
        return -3;
    }

    // Set the default locale in /etc/default/locale.
    snprintf(cmd, sizeof(cmd),
        "echo 'LANG=%s' > /mnt/etc/default/locale",
        store->locale);
    if (run_command(cmd) != 0)
    {
        return -4;
    }

    return 0;
}
