/**
 * This code is responsible for orchestrating the full installation process
 * by invoking partitioning, rootfs extraction, bootloader setup, and locale
 * configuration in sequence.
 */

#include "../all.h"

#define NOTIFY(event, step, err) \
    if (progress_cb) progress_cb((event), (step), (err), context)

int run_install(install_progress_cb progress_cb, void *context)
{
    // Initialize install log file.
    init_install_log();

    // Enable input polling during command execution.
    set_install_poll_modal(context);
    set_command_poll_callback(install_poll_callback);

    NOTIFY(INSTALL_START, 0, 0);

    // Step 1: Create partitions and format disk.
    write_install_log_header("Partitioning");
    NOTIFY(INSTALL_STEP_BEGIN, STEP_PARTITIONS, 0);
    int result = create_partitions();
    if (result != 0)
    {
        NOTIFY(INSTALL_STEP_FAIL, STEP_PARTITIONS, result);
        cleanup_mounts();
        return -1;
    }
    NOTIFY(INSTALL_STEP_OK, STEP_PARTITIONS, 0);

    // Step 2: Extract rootfs archive to target.
    write_install_log_header("Extracting system files");
    NOTIFY(INSTALL_STEP_BEGIN, STEP_ROOTFS, 0);
    result = extract_rootfs();
    if (result != 0)
    {
        NOTIFY(INSTALL_STEP_FAIL, STEP_ROOTFS, result);
        cleanup_mounts();
        return -2;
    }
    NOTIFY(INSTALL_STEP_OK, STEP_ROOTFS, 0);

    // Step 2b: Generate fstab for target system.
    write_install_log_header("Generating fstab");
    result = generate_fstab();
    if (result != 0)
    {
        NOTIFY(INSTALL_STEP_FAIL, STEP_ROOTFS, result);
        cleanup_mounts();
        return -2;
    }

    // Step 3: Install and configure bootloader.
    write_install_log_header("Installing bootloader");
    NOTIFY(INSTALL_STEP_BEGIN, STEP_BOOTLOADER, 0);
    result = setup_bootloader();
    if (result != 0)
    {
        NOTIFY(INSTALL_STEP_FAIL, STEP_BOOTLOADER, result);
        cleanup_mounts();
        return -3;
    }
    NOTIFY(INSTALL_STEP_OK, STEP_BOOTLOADER, 0);

    // Step 4: Configure system locale.
    write_install_log_header("Configuring locale");
    NOTIFY(INSTALL_STEP_BEGIN, STEP_LOCALE, 0);
    result = configure_locale();
    if (result != 0)
    {
        NOTIFY(INSTALL_STEP_FAIL, STEP_LOCALE, result);
        cleanup_mounts();
        return -4;
    }
    NOTIFY(INSTALL_STEP_OK, STEP_LOCALE, 0);

    // Step 5: Configure user accounts and hostname.
    write_install_log_header("Configuring users");
    NOTIFY(INSTALL_STEP_BEGIN, STEP_USERS, 0);
    result = configure_users();
    if (result != 0)
    {
        NOTIFY(INSTALL_STEP_FAIL, STEP_USERS, result);
        cleanup_mounts();
        return -5;
    }
    NOTIFY(INSTALL_STEP_OK, STEP_USERS, 0);

    // Cleanup.
    cleanup_mounts();

    NOTIFY(INSTALL_COMPLETE, 0, 0);
    NOTIFY(INSTALL_AWAIT_REBOOT, 0, 0);

    // Disable input polling before reboot.
    set_command_poll_callback(NULL);

    run_command("reboot >>" INSTALL_LOG_PATH " 2>&1");
    close_dry_run_log();

    return 0;
}
