#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "common.h"

/*
 * Run the PI experiment with num_workers workers and collect the final
 * WorkerResult from all child workers.
 */
WorkerResult run_parallel_pi_workers(long trials, int num_workers);

#endif // SCHEDULER_H