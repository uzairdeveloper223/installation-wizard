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
int has_required_boot_partition(Store *store, int is_uefi);
int is_boot_partition_too_small(Store *store, int is_uefi);

/* src/steps/partition/dialogs.c */
#define SIZE_COUNT 19
#define MOUNT_COUNT 6
extern const unsigned long long size_presets[];
extern const char *mount_options[];
int find_closest_size_index(unsigned long long size);
int find_mount_index(const char *mount);
int find_flag_index(int boot, int esp, int bios_grub);
