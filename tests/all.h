#pragma once
#include "../src/all.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

/*
 * Declarations for semistatic (semistatic = static in production, non-static
 * in testing) functions from the `src` files. These aren't declared in any `.h`
 * files, so we declare them here, so they can be used in the unit tests.
 */

/* src/steps/confirm/confirm.c */
int has_root_partition(Store *store);
int has_duplicate_mount_points(Store *store);

typedef enum {
    BOOT_OK = 0,
    BOOT_ERR_UEFI_NO_ESP,
    BOOT_ERR_UEFI_ESP_NOT_FAT32,
    BOOT_ERR_UEFI_ESP_WRONG_MOUNT,
    BOOT_ERR_UEFI_ESP_TOO_SMALL,
    BOOT_ERR_UEFI_HAS_BIOS_GRUB,
    BOOT_ERR_BIOS_GPT_NO_BIOS_GRUB,
    BOOT_ERR_BIOS_GPT_BIOS_GRUB_HAS_FS,
    BOOT_ERR_BIOS_GPT_BIOS_GRUB_HAS_MOUNT,
    BOOT_ERR_BIOS_GPT_BIOS_GRUB_TOO_SMALL,
    BOOT_ERR_BIOS_GPT_HAS_ESP,
    BOOT_ERR_BOOT_TOO_SMALL,
    BOOT_ERR_BOOT_NO_FS,
    BOOT_ERR_BOOT_IS_BIOS_GRUB
} BootValidationError;

BootValidationError validate_uefi_boot(Store *store);
BootValidationError validate_bios_gpt_boot(Store *store);
BootValidationError validate_bios_mbr_boot(Store *store);
BootValidationError validate_optional_boot(Store *store);
BootValidationError validate_boot_config(
    Store *store, FirmwareType firmware, DiskLabel disk_label
);

/* src/steps/partition/dialogs.c */
#define SIZE_COUNT 19
#define MOUNT_COUNT 6
extern const unsigned long long size_presets[];
extern const char *mount_options[];
int find_closest_size_index(unsigned long long size);
int find_mount_index(const char *mount);
int find_flag_index(int boot, int esp, int bios_grub);
