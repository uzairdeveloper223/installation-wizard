/**
 * This code is responsible for installing LimeOS components and configuring
 * X11 on the target system, including copying component binaries, installing
 * bundled dependencies, and writing X11 startup configuration.
 */

#include "../../all.h"

static int component_exists(const Component *component)
{
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", CONFIG_LIVE_COMPONENT_PATH, component->binary_name);
    return access(path, F_OK) == 0;
}

static int copy_component_binary(const Component *component)
{
    char cmd[512];
    snprintf(
        cmd, sizeof(cmd),
        "cp %s/%s %s/ >>" CONFIG_INSTALL_LOG_PATH " 2>&1",
        CONFIG_LIVE_COMPONENT_PATH, component->binary_name, CONFIG_TARGET_COMPONENT_PATH
    );
    return run_command(cmd);
}

static int copy_component_binaries(void)
{
    // Check if any components exist.
    int any_exist = 0;
    for (int i = 0; i < CONFIG_COMPONENT_COUNT; i++)
    {
        if (component_exists(&CONFIG_COMPONENTS[i]))
        {
            any_exist = 1;
            break;
        }
    }

    // If no components exist, nothing to do.
    if (!any_exist)
    {
        return 0;
    }

    // Ensure target directory exists.
    if (run_command("mkdir -p " CONFIG_TARGET_COMPONENT_PATH " >>" CONFIG_INSTALL_LOG_PATH " 2>&1") != 0)
    {
        return -1;
    }

    // Copy each component that exists.
    for (int i = 0; i < CONFIG_COMPONENT_COUNT; i++)
    {
        const Component *component = &CONFIG_COMPONENTS[i];
        if (component_exists(component))
        {
            if (copy_component_binary(component) != 0)
            {
                return -(i + 2);
            }
        }
    }

    return 0;
}

static int install_component_packages(const Component *component)
{
    // Check if bundled dependencies exist for this component.
    char deps_path[256];
    snprintf(deps_path, sizeof(deps_path), "%s/%s", CONFIG_LIVE_COMPONENT_DEPS_PATH, component->deps_directory);

    if (access(deps_path, F_OK) != 0)
    {
        // No bundled dependencies for this component.
        return 0;
    }

    // Ensure target apt cache directory exists.
    if (run_command("mkdir -p " CONFIG_TARGET_MOUNT_POINT "/var/cache/apt/archives >>" CONFIG_INSTALL_LOG_PATH " 2>&1") != 0)
    {
        return -1;
    }

    // Copy bundled .deb packages to target apt cache.
    char cmd[512];
    snprintf(
        cmd, sizeof(cmd),
        "cp %s/*.deb " CONFIG_TARGET_MOUNT_POINT "/var/cache/apt/archives/ >>" CONFIG_INSTALL_LOG_PATH " 2>&1",
        deps_path
    );
    if (run_command(cmd) != 0)
    {
        return -2;
    }

    // Install packages using dpkg (run twice for dependency resolution).
    run_command("chroot " CONFIG_TARGET_MOUNT_POINT " dpkg -i /var/cache/apt/archives/*.deb >>" CONFIG_INSTALL_LOG_PATH " 2>&1");
    if (run_command("chroot " CONFIG_TARGET_MOUNT_POINT " dpkg --configure -a >>" CONFIG_INSTALL_LOG_PATH " 2>&1") != 0)
    {
        return -3;
    }

    return 0;
}

static int install_all_component_packages(void)
{
    for (int i = 0; i < CONFIG_COMPONENT_COUNT; i++)
    {
        const Component *component = &CONFIG_COMPONENTS[i];
        if (component_exists(component))
        {
            if (install_component_packages(component) != 0)
            {
                return -(i + 1);
            }
        }
    }

    return 0;
}

static int find_x11_startup_component(void)
{
    for (int i = 0; i < CONFIG_COMPONENT_COUNT; i++)
    {
        const Component *component = &CONFIG_COMPONENTS[i];
        if (component->x11_startup && component_exists(component))
        {
            return i;
        }
    }
    return -1;
}

static int write_xinitrc(const Component *component)
{
    // Ensure X11 xinit directory exists.
    if (run_command("mkdir -p " CONFIG_TARGET_MOUNT_POINT "/etc/X11/xinit >>" CONFIG_INSTALL_LOG_PATH " 2>&1") != 0)
    {
        return -1;
    }

    // Write xinitrc that starts the component.
    char xinitrc_content[256];
    snprintf(
        xinitrc_content, sizeof(xinitrc_content),
        "#!/bin/sh\nexec /usr/local/bin/%s\n",
        component->binary_name
    );

    char cmd[1024];
    snprintf(
        cmd, sizeof(cmd),
        "cat > " CONFIG_TARGET_XINITRC_PATH " << 'EOF'\n%sEOF",
        xinitrc_content
    );
    if (run_command(cmd) != 0)
    {
        return -2;
    }

    // Make xinitrc executable.
    if (run_command("chmod +x " CONFIG_TARGET_XINITRC_PATH " >>" CONFIG_INSTALL_LOG_PATH " 2>&1") != 0)
    {
        return -3;
    }

    return 0;
}

static int write_startx_profile(void)
{
    // Ensure profile.d directory exists.
    if (run_command("mkdir -p " CONFIG_TARGET_MOUNT_POINT "/etc/profile.d >>" CONFIG_INSTALL_LOG_PATH " 2>&1") != 0)
    {
        return -1;
    }

    // Write startx.sh that auto-starts X on tty1 login.
    const char *startx_content =
        "# Auto-start X on tty1 login\n"
        "if [ -z \"$DISPLAY\" ] && [ \"$(tty)\" = \"/dev/tty1\" ]; then\n"
        "    exec startx\n"
        "fi\n";

    char cmd[1024];
    snprintf(
        cmd, sizeof(cmd),
        "cat > " CONFIG_TARGET_STARTX_PROFILE_PATH " << 'EOF'\n%sEOF",
        startx_content
    );
    if (run_command(cmd) != 0)
    {
        return -2;
    }

    return 0;
}

int install_components(void)
{
    // Check if any components exist on the live system.
    int any_exist = 0;
    for (int i = 0; i < CONFIG_COMPONENT_COUNT; i++)
    {
        if (component_exists(&CONFIG_COMPONENTS[i]))
        {
            any_exist = 1;
            break;
        }
    }

    if (!any_exist)
    {
        // No components to install; skip this step entirely.
        return 0;
    }

    // Copy component binaries to target system.
    if (copy_component_binaries() != 0)
    {
        return -1;
    }

    // Install bundled component dependencies if present.
    if (install_all_component_packages() != 0)
    {
        return -2;
    }

    // Configure X11 if an X11 startup component is present.
    int startup_index = find_x11_startup_component();
    if (startup_index >= 0)
    {
        if (write_xinitrc(&CONFIG_COMPONENTS[startup_index]) != 0)
        {
            return -3;
        }

        if (write_startx_profile() != 0)
        {
            return -4;
        }
    }

    return 0;
}
