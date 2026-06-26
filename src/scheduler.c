#include "scheduler.h"
#include "worker.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

/*
 * If all bytes are read from the pipe then return the number of bytes,
 * otherwise return -1.
 * read_all stops when encounter EOF. That is, it might return smaller
 * number of bytes compared to the parameter count.
 */
static ssize_t read_all(int fd, void *buf, size_t count) {
    size_t total_read = 0;  // >= 0
    char *p = (char *) buf;  // char * counts in one byte

    while (total_read < count) {
        // start from address p + (total_read) bytes
        ssize_t n = read(fd, p + total_read, count - total_read);
        if (n < 0) {
            if (errno == EINTR) {  // interrupt
                continue;
            }
            return -1;
        }
        if (n == 0) {  // EOF
            break;
        }
        total_read += (size_t) n;
    }

    return (ssize_t) total_read;
}

/*
 * Close all still-open pipe ends and reset them to -1.
 */
static void close_open_pipes(int (*pipes)[2], int count) {
    if (pipes == NULL) {
        return;
    }

    for (int i = 0; i < count; i++) {
        if (pipes[i][0] >= 0) {
            close(pipes[i][0]);
            pipes[i][0] = -1;
        }
        if (pipes[i][1] >= 0) {
            close(pipes[i][1]);
            pipes[i][1] = -1;
        }
    }
}

/*
 * Reap children that have already been created.
 * This avoids leaving zombie processes on early failure.
 */
static void clean_children(pid_t *pids, int count) {
    if (pids == NULL) {
        return;
    }

    for (int i = 0; i < count; i++) {
        if (pids[i] > 0) {
            int status;
            while (waitpid(pids[i], &status, 0) == -1) {
                if (errno == EINTR) {
                    continue;
                }
                break;
            }
        }
    }
}

/*
 * Run the experiment using one parent and one or more workers.
 * The parent creates workers, collects every worker WorkerResult,
 * then returns the final combined result.
 */
WorkerResult run_parallel_pi_workers(long trials, int num_workers) {
    WorkerResult result;
    result.inside_count = 0;
    result.trials_done = 0;

    if (trials <= 0 || num_workers <= 0) {
        return result;
    }

    /*
     * For one worker, no fork is necessary.
     * Run directly in the parent and return the result.
     */
    if (num_workers == 1) {
        srand((unsigned int)(time(NULL) ^ getpid()));
        result.inside_count = count_inside_circle(trials);
        result.trials_done = trials;
        return result;
    }

    // [ [int, int], [pipe_read, pipe_write], ..., pipe #num_workers ]
    int (*pipes)[2] = malloc(sizeof(int[2]) * (size_t) num_workers);
    if (pipes == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // [pid_of_the_1st_child, ..., pid_of_the_n-th_child]
    pid_t *pids = malloc(sizeof(pid_t) * (size_t) num_workers);
    if (pids == NULL) {
        perror("malloc");
        free(pipes);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_workers; i++) {
        pipes[i][0] = -1;
        pipes[i][1] = -1;
        pids[i] = -1;
    }

    // create a pipe for every worker
    for (int i = 0; i < num_workers; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            close_open_pipes(pipes, num_workers);
            free(pipes);
            free(pids);
            exit(EXIT_FAILURE);
        }
    }

    long base_trial = trials / num_workers;
    long remainder = trials % num_workers;

    for (int i = 0; i < num_workers; i++) {
        // create child process
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close_open_pipes(pipes, num_workers);
            clean_children(pids, i);
            free(pipes);
            free(pids);
            exit(EXIT_FAILURE);
        }

        // child process starts
        if (pid == 0) {
            for (int j = 0; j < num_workers; j++) {  // only keep own write end
                if (pipes[j][0] >= 0) {
                    close(pipes[j][0]);
                }
                if (j != i && pipes[j][1] >= 0) {
                    close(pipes[j][1]);
                }
            }

            // some workers need to run an additional trial if i < remainder
            long worker_trials = base_trial + (i < remainder ? 1 : 0);
            Worker worker;
            worker_init(&worker, i, worker_trials, pipes[i][1]);
            worker_run_pi(&worker);  // include _exit for child
        }
        // child process ends

        pids[i] = pid;
    }

    // parent close all write ends
    for (int i = 0; i < num_workers; i++) {
        if (pipes[i][1] >= 0) {
            close(pipes[i][1]);
            pipes[i][1] = -1;
        }
    }

    // sum up all child process result
    WorkerResult total_result;
    total_result.inside_count = 0;
    total_result.trials_done = 0;

    for (int i = 0; i < num_workers; i++) {
        WorkerResult worker_result;
        worker_result.inside_count = 0;
        worker_result.trials_done = 0;

        ssize_t bytes_read = read_all(pipes[i][0], &worker_result, sizeof(worker_result));

        if (bytes_read < 0) {
            perror("read");
            close_open_pipes(pipes, num_workers);
            clean_children(pids, num_workers);
            free(pipes);
            free(pids);
            exit(EXIT_FAILURE);
        }

        if (bytes_read != (ssize_t) sizeof(worker_result)) {
            fprintf(stderr, "Error: unexpected EOF while reading worker %d result\n", i);
            close_open_pipes(pipes, num_workers);
            clean_children(pids, num_workers);
            free(pipes);
            free(pids);
            exit(EXIT_FAILURE);
        }

        total_result.inside_count += worker_result.inside_count;
        total_result.trials_done += worker_result.trials_done;

        if (pipes[i][0] >= 0) {
            close(pipes[i][0]);
            pipes[i][0] = -1;
        }
    }

    // check the child process ends normally
    for (int i = 0; i < num_workers; i++) {
        int status;

        while (waitpid(pids[i], &status, 0) == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("waitpid");
            close_open_pipes(pipes, num_workers);
            clean_children(pids, num_workers);
            free(pipes);
            free(pids);
            exit(EXIT_FAILURE);
        }

        pids[i] = -1;

        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Error: child %d failed\n", i);
            close_open_pipes(pipes, num_workers);
            clean_children(pids, num_workers);
            free(pipes);
            free(pids);
            exit(EXIT_FAILURE);
        }
    }

    free(pipes);
    free(pids);

    return total_result;
}