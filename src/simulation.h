#ifndef SIMULATION_H
#define SIMULATION_H

#include "common.h"

/*
 * Convert inside_count and total_trials into a PI estimate.
 */
double estimate_pi(long inside_count, long total_trials);

/*
 * Run the PI simulation using only one worker.
 */
double run_single_pi_simulation(long trials);

/*
 * Run the PI simulation using one or more workers.
 */
double run_parallel_pi_simulation(long trials, int num_workers);

#endif