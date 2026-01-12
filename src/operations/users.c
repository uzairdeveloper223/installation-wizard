/**
 * This code is responsible for configuring user accounts and the hostname
 * on the target system during installation.
 */

#include "../all.h"

static int set_hostname(const char *hostname)
{
    // Escape hostname for shell safety.
    char escaped_hostname[256];
    if (shell_escape(hostname, escaped_hostname, sizeof(escaped_hostname)) != 0)
    {
        return -1;
    }

    // Write hostname to target system.
    char command[512];
    snprintf(
        command, sizeof(command),
        "echo %s > /mnt/etc/hostname",
        escaped_hostname
    );
    if (run_command(command) != 0)
    {
        return -1;
    }

    return 0;
}

static int create_user(const User *user)
{
    // Escape username for shell safety.
    char escaped_username[256];
    if (shell_escape(user->username, escaped_username, sizeof(escaped_username)) != 0)
    {
        return -2;
    }

    // Create user with home directory and bash shell.
    char command[512];
    snprintf(
        command, sizeof(command),
        "chroot /mnt useradd -m -s /bin/bash %s >>" INSTALL_LOG_PATH " 2>&1",
        escaped_username
    );
    if (run_command(command) != 0)
    {
        return -2;
    }

    return 0;
}

static int set_password(const User *user)
{
    // Escape username and password for shell safety.
    char escaped_username[256];
    char escaped_password[512];
    if (shell_escape(user->username, escaped_username, sizeof(escaped_username)) != 0)
    {
        return -3;
    }
    if (shell_escape(user->password, escaped_password, sizeof(escaped_password)) != 0)
    {
        return -3;
    }

    // Set password using chpasswd.
    char command[1024];
    snprintf(
        command, sizeof(command),
        "chroot /mnt sh -c 'echo %s:%s | chpasswd' >>" INSTALL_LOG_PATH " 2>&1",
        escaped_username, escaped_password
    );
    if (run_command(command) != 0)
    {
        return -3;
    }

    return 0;
}

static int add_to_admin_group(const User *user)
{
    // Escape username for shell safety.
    char escaped_username[256];
    if (shell_escape(user->username, escaped_username, sizeof(escaped_username)) != 0)
    {
        return -4;
    }

    // Add user to sudo group.
    char command[512];
    snprintf(
        command, sizeof(command),
        "chroot /mnt usermod -aG sudo %s >>" INSTALL_LOG_PATH " 2>&1",
        escaped_username
    );
    if (run_command(command) != 0)
    {
        return -4;
    }

    return 0;
}

int configure_users(void)
{
    Store *store = get_store();
    int result;

    // Validate at least one user exists.
    if (store->user_count < 1)
    {
        return -2;
    }

    // Set hostname on target system.
    result = set_hostname(store->hostname);
    if (result != 0)
    {
        return result;
    }

    // Configure each user account.
    for (int i = 0; i < store->user_count; i++)
    {
        User *user = &store->users[i];

        // Create user account.
        result = create_user(user);
        if (result != 0)
        {
            return result;
        }

        // Set user password.
        result = set_password(user);
        if (result != 0)
        {
            return result;
        }

        // Add to sudo group if admin.
        if (user->is_admin)
        {
            result = add_to_admin_group(user);
            if (result != 0)
            {
                return result;
            }
        }
    }

    return 0;
}
