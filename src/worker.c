#include "worker.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>

/*
 * Initialize a worker with its unique id, the number of trials that this worker need to run,
 * and the file_descripter of write_pipe.
 */
void worker_init(Worker *worker_pt, int worker_id, long trials, int write_fd) {
    worker_pt->worker_id = worker_id;
    worker_pt->trials = trials;
    worker_pt->write_fd = write_fd;
}

/*
 * Count points inside the quarter circle x^2 + y^2 <= 1
 * for random points sampled uniformly in [0,1] x [0,1].
 */
long count_inside_circle(long trials) {
    long inside_dots = 0;

    for (long i = 0; i < trials; i++) {
        double x = (double) rand() / RAND_MAX;  // [0, 1]
        double y = (double) rand() / RAND_MAX;

        if (x * x + y * y <= 1.0) {  // r = 1
            inside_dots++;
        }
    }

    return inside_dots;
}

/*
 * If all bytes are written into the pipe then return the number of bytes,
 * otherwise return -1.
 */
static ssize_t write_all(int fd, const void *buf, size_t count) {
    size_t total_written = 0;  // >= 0
    const char *p = (const char *) buf;  // need const, char * counts in one byte

    while (total_written < count) {
        // start from address p + (total_written) bytes
        ssize_t n = write(fd, p + total_written, count - total_written);
        if (n < 0) {
            if (errno == EINTR) {  // interrupt
                continue;
            }
            return -1;
        }
        total_written += (size_t) n;
    }

    return (ssize_t) total_written;
}

/*
 * Run one child worker:
 * 1. seed random number generator
 * 2. count dots inside the circle
 * 3. write the result to parent through pipe
 * 4. close pipe and exit
 */
void worker_run_pi(const Worker *worker_pt) {
    srand((unsigned int) (time(NULL) ^ getpid()));  // random seed

    // count the number of dots fall in the circle
    long inside_count = count_inside_circle(worker_pt->trials);

    /*
     * WorkerResult is the message sent from worker to parent.
     * It contains both inside_count and trials_done.
     */
    WorkerResult result;
    result.inside_count = inside_count;
    result.trials_done = worker_pt->trials;

    if (write_all(worker_pt->write_fd, &result, sizeof(result))
       != (ssize_t) sizeof(result)) {
        fprintf(stderr, "Error: worker %d failed to write result\n",
                worker_pt->worker_id);
        close(worker_pt->write_fd);
        _exit(EXIT_FAILURE);
    }

    // finish writing, close the write end
    close(worker_pt->write_fd);
    _exit(EXIT_SUCCESS);
}