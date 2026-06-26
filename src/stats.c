#include "stats.h"

#include <math.h>
#include <stddef.h>

/*
 * PI estimation comes from 4 * p_hat where p_hat is the sample proportion
 * of points falling inside the quarter circle. For a Bernoulli sample,
 * Var(p_hat) = p_hat * (1 - p_hat) / n.
 */
double pi_standard_error(long inside_count, long total_trials) {
    if (total_trials <= 0) {
        return 0.0;
    }

    double p_hat = (double) inside_count / (double) total_trials;
    double variance = p_hat * (1.0 - p_hat) / (double) total_trials;

    return 4.0 * sqrt(variance);
}

/*
 * Build a normal-approximation 95% confidence interval using
 * estimate +/- 1.96 * standard_error.
 */
void pi_confidence_interval(long inside_count, long total_trials,
                            double *lower, double *upper) {
    if (lower == NULL || upper == NULL) {
        return;
    }

    if (total_trials <= 0) {
        *lower = 0.0;
        *upper = 0.0;
        return;
    }

    double pi_hat = 4.0 * (double) inside_count / (double) total_trials;
    double se = pi_standard_error(inside_count, total_trials);
    double margin = 1.96 * se;

    *lower = pi_hat - margin;
    *upper = pi_hat + margin;
}