#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "simulation.h"
#include "scheduler.h"
#include "stats.h"

/*
 * print messages about how to correct use this program under correct form
 * if the user enters invalid arguments
 */
static void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s -n <trials> [-w <workers>] [--mode pi]\n", prog_name);
}

/*
 * parse arguments from users in form: ./program_name -n <trials> [-w <workers>] [--mode pi]
 */
static int parse_args(int argc, char *argv[], Config *config) {
    config->mode = MODE_PI;
    config->num_workers = 1;
    config->total_trials = 1000000;

    for (int i = 1; i < argc; i++) {  // arg[0] is program_name, start from 1

        // trial
        if (strcmp(argv[i], "-n") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: missing value after -n\n");
                return 0;
            }

            char *t_endptr;
            errno = 0;
            long trial_val = strtol(argv[++i], &t_endptr, 10);
            //check the end index pointer
            if (t_endptr == argv[i]) {
                fprintf(stderr, "Error: no digits found in '%s'\n", argv[i]);
                return 0;
            }
            if (*t_endptr != '\0') {
                fprintf(stderr, "Error: invalid characters in '%s'\n", argv[i]);
                return 0;
            }
            if (errno == ERANGE) {  // the value is too big or too small(long overflow)
                fprintf(stderr, "Error: number out of range '%s'\n", argv[i]);
                return 0;
            }
            if (trial_val <= 0) {
                fprintf(stderr, "Error: total trials must be > 0\n");
                return 0;
            }
            config->total_trials = trial_val;
        }

        // worker
        else if (strcmp(argv[i], "-w") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: missing value after -w\n");
                return 0;
            }

            char *w_endptr;
            errno = 0;
            long worker_val = strtol(argv[++i], &w_endptr, 10);
            //check the end index pointer
            if (w_endptr == argv[i]) {
                fprintf(stderr, "Error: no digits found in '%s'\n", argv[i]);
                return 0;
            }
            if (*w_endptr != '\0') {
                fprintf(stderr, "Error: invalid characters in '%s'\n", argv[i]);
                return 0;
            }
            if (errno == ERANGE) {  // the value is too big or too small(long overflow)
                fprintf(stderr, "Error: number out of range '%s'\n", argv[i]);
                return 0;
            }
            if (worker_val <= 0 || worker_val > MAX_WORKERS) {
                fprintf(stderr, "Error: workers must be between 1 and %d\n", MAX_WORKERS);
                return 0;
            }
            config->num_workers = (int)worker_val;
        }

        // mode
        else if (strcmp(argv[i], "--mode") == 0) {
            if (i + 1 >= argc) {  // missing mode
                fprintf(stderr, "Error: missing value after --mode\n");
                return 0;
            }

            const char *mode_str = argv[++i];
            if (strcmp(mode_str, "pi") == 0) {
                config->mode = MODE_PI;
            } else {
                fprintf(stderr, "Error: unsupported mode '%s'\n", mode_str);
                return 0;
            }
        }

        else {
            fprintf(stderr, "Error: unknown argument '%s'\n", argv[i]);
            return 0;
        }
    }

    return 1;  // success
}

int main(int argc, char *argv[]) {
    Config config;

    // parse arguments
    if (!parse_args(argc, argv, &config)) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // experiment info
    printf("Monte Carlo Engine\n");
    printf("Mode: PI estimation\n");
    printf("Workers requested: %d\n", config.num_workers);
    printf("Total trials: %ld\n", config.total_trials);

    // run the experiment and collect raw result
    WorkerResult result = run_parallel_pi_workers(config.total_trials,
        config.num_workers);

    // compute estimate and summary statistics
    double estimate = estimate_pi(result.inside_count, result.trials_done);
    double standard_error = pi_standard_error(result.inside_count, result.trials_done);
    double ci_lower = 0.0;
    double ci_upper = 0.0;
    pi_confidence_interval(result.inside_count, result.trials_done,
                           &ci_lower, &ci_upper);

    printf("\nResult:\n");
    printf("Estimated pi = %.10f\n", estimate);  // Round to 10 decimal places
    printf("Inside count = %ld\n", result.inside_count);
    printf("Trials done = %ld\n", result.trials_done);
    printf("Standard error = %.10f\n", standard_error);
    printf("95%% CI = [%.10f, %.10f]\n", ci_lower, ci_upper);

    return EXIT_SUCCESS;
}