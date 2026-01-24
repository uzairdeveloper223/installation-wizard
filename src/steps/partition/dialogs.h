#pragma once
#include "../../all.h"

/**
 * Displays the add partition dialog and creates a new partition.
 *
 * @param modal     The modal window to draw in.
 * @param store     The global store to add partition to.
 * @param disk_size Total disk size in bytes.
 *
 * @return - `0` - Indicates partition was added.
 * @return - `-1` - Indicates maximum partition limit reached.
 * @return - `-2` - Indicates insufficient free space.
 * @return - `-3` - Indicates user cancelled form.
 */
int add_partition_dialog(
    WINDOW *modal, Store *store, unsigned long long disk_size
);

/**
 * Displays the edit partition dialog and modifies an existing partition.
 *
 * @param modal     The modal window to draw in.
 * @param store     The global store containing partitions.
 * @param disk_size Total disk size in bytes.
 *
 * @return - `0` - Indicates partition was modified.
 * @return - `-1` - Indicates no partitions to edit.
 * @return - `-2` - Indicates user cancelled selection.
 * @return - `-3` - Indicates user cancelled form.
 */
int edit_partition_dialog(
    WINDOW *modal, Store *store, unsigned long long disk_size
);

/**
 * Displays the remove partition dialog and removes a partition.
 *
 * @param modal     The modal window to draw in.
 * @param store     The global store containing partitions.
 * @param disk_size Total disk size in bytes.
 *
 * @return - `0` - Indicates partition was removed.
 * @return - `-1` - Indicates no partitions to remove.
 * @return - `-2` - Indicates user cancelled selection.
 */
int remove_partition_dialog(
    WINDOW *modal, Store *store, unsigned long long disk_size
);

/**
 * Automatically creates an optimal partition layout based on system type.
 *
 * Clears existing partitions and creates:
 * - Boot partition (ESP for UEFI, bios_grub for BIOS+GPT)
 * - Swap partition (sized based on system RAM)
 * - Root partition (remaining disk space)
 *
 * @param store     The global store to populate with partitions.
 * @param disk_size Total disk size in bytes.
 *
 * @return - `0` - Indicates partitions were created.
 */
int autofill_partitions(Store *store, unsigned long long disk_size);
