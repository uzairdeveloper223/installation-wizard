#pragma once
#include "../../all.h"

/**
 * Runs the partition management step interactively.
 *
 * Allows users to add, edit, and remove partitions.
 *
 * @param modal The modal window to draw in.
 * @param step_index The index of this step in the wizard (0-based).
 *
 * @return - `1` - Indicates user confirmed partitions.
 * @return - `0` - Indicates user went back.
 */
int run_partition_step(WINDOW *modal, int step_index);
