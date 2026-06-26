#ifndef WORKER_H
#define WORKER_H

#include "common.h"

/*
 * Worker information for one child process.
 * worker_id: unique worker index
 * trials: how many random points this worker should simulate
 * write_fd: pipe write end used to send result back to parent
 */
typedef struct {
    int worker_id;
    long trials;
    int write_fd;
} Worker;

void worker_init(Worker *worker_pt, int worker_id, long trials, int write_fd);
long count_inside_circle(long trials);
void worker_run_pi(const Worker *worker_pt);

#endif // WORKER_H