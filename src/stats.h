#ifndef STATS_H
#define STATS_H

/*
 * Compute the standard error for PI estimation.
 */
double pi_standard_error(long inside_count, long total_trials);

/*
 * Compute the 95% confidence interval for PI estimation.
 * The results are written to lower and upper.
 */
void pi_confidence_interval(long inside_count, long total_trials,
                            double *lower, double *upper);

#endif