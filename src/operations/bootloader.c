/**
 * This code is responsible for installing and configuring the GRUB bootloader
 * on the target system, supporting both UEFI and BIOS boot modes.
 */

#include "../all.h"

static int verify_chroot_works(void)
{
    const char *marker = "/mnt/tmp/.chroot_verify";
    
    // Escape the marker path for shell command.
    char escaped_marker[256];
    if (shell_escape(marker, escaped_marker, sizeof(escaped_marker)) != 0)
    {
        return -1;
    }
    
    // Create marker file inside chroot /mnt.
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "echo 'limeos' > %s", escaped_marker);
    if (run_command(cmd) != 0)
    {
        return -1;
    }

    // Verify chroot can see the marker at /tmp/.chroot_verify (not /mnt/tmp).
    // If chroot fails silently, cat would look at the host's /tmp and fail.
    int result = run_command("chroot /mnt cat /tmp/.chroot_verify >/dev/null 2>&1");

    // Clean up marker file.
    snprintf(cmd, sizeof(cmd), "rm -f %s", escaped_marker);
    run_command(cmd);

    return (result == 0) ? 0 : -1;
}

static int detect_uefi_mode(Store *store)
{
    if (store->force_uefi != 0)
    {
        return (store->force_uefi > 0);
    }
    return (access("/sys/firmware/efi", F_OK) == 0);
}

static int find_esp_partition_index(Store *store)
{
    for (int i = 0; i < store->partition_count; i++)
    {
        if (store->partitions[i].flag_esp)
        {
            return i + 1;
        }
    }
    return -1;
}

static int mount_efi_partition(const char *disk, int esp_partition_index)
{
    // Construct the ESP partition device path.
    char esp_device[128];
    get_partition_device(disk, esp_partition_index, esp_device, sizeof(esp_device));

    // Escape the device path for shell command.
    char escaped_device[256];
    if (shell_escape(esp_device, escaped_device, sizeof(escaped_device)) != 0)
    {
        return -1;
    }

    // Create the EFI mount point directory.
    if (run_command("mkdir -p /mnt/boot/efi") != 0)
    {
        return -1;
    }

    // Mount the EFI system partition.
    char mount_cmd[256];
    snprintf(
        mount_cmd, sizeof(mount_cmd),
        "mount -t vfat %s /mnt/boot/efi", escaped_device
    );
    if (run_command(mount_cmd) != 0)
    {
        return -2;
    }

    return 0;
}

static int mount_chroot_system_dirs(void)
{
    if (run_command("mount --bind /dev /mnt/dev") != 0)
    {
        return -3;
    }
    if (run_command("mount -t proc proc /mnt/proc") != 0)
    {
        run_command("umount /mnt/dev");
        return -4;
    }
    if (run_command("mount -t sysfs sys /mnt/sys") != 0)
    {
        run_command("umount /mnt/proc");
        run_command("umount /mnt/dev");
        return -5;
    }
    return 0;
}

static void unmount_chroot_system_dirs(void)
{
    run_command("umount /mnt/sys");
    run_command("umount /mnt/proc");
    run_command("umount /mnt/dev");
}

static int install_grub_packages(int is_uefi)
{
    const char *pkg_dir = is_uefi
    ? "/usr/share/limeos/packages/efi"
    : "/usr/share/limeos/packages/bios";
    
    // Escape the package directory path for shell command.
    char escaped_dir[256];
    if (shell_escape(pkg_dir, escaped_dir, sizeof(escaped_dir)) != 0)
    {
        return -7;
    }
    
    // Copy all .deb files to chroot.
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "cp %s/*.deb /mnt/tmp/", escaped_dir);
    if (run_command(cmd) != 0)
    {
        return -7;
    }

    // Install packages inside chroot.
    if (run_command("chroot /mnt sh -c 'dpkg -i /tmp/*.deb' >>" INSTALL_LOG_PATH " 2>&1") != 0)
    {
        run_command("rm -f /mnt/tmp/*.deb");
        return -8;
    }

    // Clean up package files.
    run_command("rm -f /mnt/tmp/*.deb");
    return 0;
}

static int run_grub_install(const char *disk, int is_uefi)
{
    char cmd[512];

    if (is_uefi)
    {
        snprintf(cmd, sizeof(cmd),
            "chroot /mnt /usr/sbin/grub-install "
            "--target=x86_64-efi --efi-directory=/boot/efi --bootloader-id=GRUB "
            ">>" INSTALL_LOG_PATH " 2>&1");
        if (run_command(cmd) != 0)
        {
            return -9;
        }
    }
    else
    {
        char escaped_disk[256];
        if (shell_escape(disk, escaped_disk, sizeof(escaped_disk)) != 0)
        {
            return -10;
        }
        snprintf(
            cmd, sizeof(cmd),
            "chroot /mnt /usr/sbin/grub-install %s >>" INSTALL_LOG_PATH " 2>&1", 
            escaped_disk
        );
        if (run_command(cmd) != 0)
        {
            return -10;
        }
    }

    return 0;
}

static int run_update_grub(void)
{
    if (run_command("chroot /mnt /usr/sbin/update-grub >>" INSTALL_LOG_PATH " 2>&1") != 0)
    {
        return -11;
    }
    return 0;
}

int setup_bootloader(void)
{
    Store *store = get_store();
    const char *disk = store->disk;
    int result;

    // Detect boot mode.
    int is_uefi = detect_uefi_mode(store);

    // Mount EFI partition if running in UEFI mode.
    if (is_uefi)
    {
        int esp_index = find_esp_partition_index(store);
        if (esp_index > 0)
        {
            result = mount_efi_partition(disk, esp_index);
            if (result != 0)
            {
                return result;
            }
        }
    }

    // Bind mount system directories for chroot.
    result = mount_chroot_system_dirs();
    if (result != 0)
    {
        return result;
    }

    // Verify chroot environment is functional.
    if (verify_chroot_works() != 0)
    {
        unmount_chroot_system_dirs();
        return -6;
    }

    // Install GRUB packages.
    result = install_grub_packages(is_uefi);
    if (result != 0)
    {
        return result;
    }

    // Run grub-install.
    result = run_grub_install(disk, is_uefi);
    if (result != 0)
    {
        return result;
    }

    // Generate GRUB configuration.
    result = run_update_grub();
    if (result != 0)
    {
        return result;
    }

    return 0;
}
