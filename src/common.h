#ifndef COMMON_H
#define COMMON_H

/*
 * Maximum workers allowed from command line.
 * This limit avoids creating too many processes by mistake.
 */
#define MAX_WORKERS 64

/*
 * Current simulation mode.
 * Right now only PI estimation is supported.
 */
typedef enum {
    MODE_PI
} SimMode;

/*
 * Result collected from one full experiment.
 * inside_count stores number of points inside the quarter circle,
 * and trials_done stores total number of random trials executed.
 */
typedef struct {
    long inside_count;
    long trials_done;
} WorkerResult;

/*
 * Program configuration parsed from command line.
 */
typedef struct {
    SimMode mode;
    int num_workers;
    long total_trials;
} Config;

#endif