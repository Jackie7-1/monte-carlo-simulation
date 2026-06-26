#include "simulation.h"
#include "scheduler.h"

/*
 * Convert inside_count / total_trials to the final PI estimate.
 */
double estimate_pi(long inside_count, long total_trials) {
    if (total_trials <= 0) {
        return 0.0;
    }

    return 4.0 * (double) inside_count / (double) total_trials;
}

/*
 * Run one-worker PI simulation.
 * Scheduler returns the raw counts, and simulation.c only applies the formula.
 */
double run_single_pi_simulation(long trials) {
    WorkerResult result = run_parallel_pi_workers(trials, 1);
    return estimate_pi(result.inside_count, result.trials_done);
}

/*
 * Run PI simulation with one or more workers.
 * Scheduler handles processes and pipes, while simulation.c only computes PI.
 */
double run_parallel_pi_simulation(long trials, int num_workers) {
    WorkerResult result = run_parallel_pi_workers(trials, num_workers);
    return estimate_pi(result.inside_count, result.trials_done);
}