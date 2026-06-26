# Monte Carlo Simulation Engine

A multi-process Monte Carlo simulation engine implemented in C. The program estimates π by distributing random-point trials across multiple worker processes, collecting results through pipes, and computing statistical output including the standard error and a 95% confidence interval.

## Overview

This project uses Monte Carlo simulation to estimate the value of π.

Random points are generated inside a unit square. The program counts how many points fall inside the quarter circle. Since the area ratio between the quarter circle and the square is related to π, the estimate is calculated as:

```text
pi = 4 * inside_count / total_trials
```

The project supports both single-worker and multi-worker execution. In the multi-process version, the parent process creates worker processes using `fork()`, assigns trials to each worker, receives results through anonymous pipes, and aggregates the final result.

## Key Features

- Multi-process execution using `fork()`
- Pipe-based inter-process communication
- Worker scheduling and deterministic trial distribution
- Monte Carlo estimation of π
- Standard error calculation
- 95% confidence interval calculation
- Modular C code structure
- Error handling for system calls such as `fork`, `pipe`, `read`, `write`, and `waitpid`

## Project Structure

```text
monte-carlo-simulation/
├── Makefile
├── README.md
└── src/
    ├── common.h
    ├── main.c
    ├── scheduler.c
    ├── scheduler.h
    ├── simulation.c
    ├── simulation.h
    ├── stats.c
    ├── stats.h
    ├── worker.c
    └── worker.h
```

## Main Components

- `main.c`: Parses command-line arguments and starts the simulation.
- `simulation.c`: Converts aggregated worker results into the final π estimate.
- `scheduler.c`: Creates worker processes, manages pipes, assigns trials, and aggregates worker results.
- `worker.c`: Runs Monte Carlo trials inside each worker process and sends results to the parent process.
- `stats.c`: Computes the standard error and 95% confidence interval.
- `common.h`: Defines shared data structures used across the program.

## Build Instructions

Compile the project with:

```bash
make
```

## Usage

Run the program with:

```bash
./Monte-Carlo_engine -n <trials> [-w <workers>] [--mode pi]
```

Examples:

```bash
./Monte-Carlo_engine -n 1000000
./Monte-Carlo_engine -n 1000000 -w 4
./Monte-Carlo_engine -n 9000 -w 25 --mode pi
```

Arguments:

- `-n <trials>`: Number of Monte Carlo trials to run.
- `-w <workers>`: Optional number of worker processes.
- `--mode pi`: Optional mode flag for π estimation.

## Example Output

The program prints the estimated value of π, the number of points inside the circle, the total number of trials, the standard error, and the 95% confidence interval.

Example output format:

```text
pi estimate: ...
inside count: ...
trials: ...
standard error: ...
95% confidence interval: [...]
```

## Technical Design

The program follows a parent-worker process model.

The parent process creates multiple worker processes using `fork()`. Each worker receives a portion of the total trial count and performs Monte Carlo sampling independently.

Each worker sends a result back to the parent process using a dedicated anonymous pipe. The result contains:

```c
typedef struct {
    long inside_count;
    long trials_done;
} WorkerResult;
```

The parent process reads each worker result, aggregates the total number of points inside the circle and the total number of trials, and then computes the final π estimate.

## Inter-Process Communication

This project uses pipe-based inter-process communication.

Each worker process writes one `WorkerResult` struct to its pipe. The parent process reads from the corresponding pipe and combines all worker results.

The communication direction is:

```text
Worker process -> pipe -> Parent process
```

This design avoids shared memory and keeps each worker process independent.

## Error Handling

The program checks important system calls and handles failure cases, including:

- Invalid command-line arguments
- Failed memory allocation
- Failed `pipe()` calls
- Failed `fork()` calls
- Failed or incomplete `read()` / `write()` operations
- Abnormal worker process termination
- `waitpid()` errors

This improves the robustness of the simulation engine and helps prevent resource leaks or zombie processes.

## What I Learned

This project strengthened my understanding of:

- Process creation and management in C
- Inter-process communication using pipes
- Worker scheduling and workload partitioning
- Safe reading and writing with system calls
- Monte Carlo simulation methods
- Statistical interpretation of simulation results
- Modular systems programming design

