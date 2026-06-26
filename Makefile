CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -O2
LDFLAGS = -lm

# all source files
SRC = src/main.c src/simulation.c src/scheduler.c src/worker.c src/stats.c

# executable file name
OUT = Monte-Carlo_engine

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)