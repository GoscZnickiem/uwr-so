#include "include/csapp.h"

static volatile int* busy;

static void lock_print() {
    while (__sync_lock_test_and_set(busy, 1));
}

static void unlock_print() {
    __sync_lock_release(busy);
}

static int conflict(int x1, int y1, int x2, int y2) {
	return x1 == x2
	|| y1 == y2
	|| x1 + y1 == x2 + y2
	|| x1 - y1 == x2 - y2;
}

static void print_line_sep(int size) {
	for (int i = 0; i < size; ++i) 
		printf("+---");
	printf("+\n");
}

static void print_board(int size, int board[size]) {
	lock_print();

	for (int i = 0; i < size; ++i) {
		print_line_sep(size);
		for (int j = 0; j < size; ++j)
			printf("|%s", board[i] == j ? " Q " : "   ");
		printf("|\n");
	}
	print_line_sep(size);
	printf("\n");
	fflush(stdout);

	unlock_print();
}

static int safe(int k, int board[], int col) {
    for (int i = 0; i < k; i++)
        if (conflict(i, board[i], k, col))
            return 0;
    return 1;
}

static void solve(int k, int board[], int size) {
	if (k == size) {
		print_board(size, board);
		exit(0);
	}

	pid_t pids[size];

	for (int col = 0; col < size; ++col) {
		if (!safe(k, board, col))
			continue;

		pid_t pid = fork();
		if (pid < 0) {
			unix_error("fork error");
		} else if (pid == 0) {
			int child_board[size];
			for (int i = 0; i < k; ++i)
				child_board[i] = board[i];
			child_board[k] = col;

			solve(k + 1, child_board, size);
			exit(0);
		} else {
			pids[col] = pid;
		}
	}

	for (int i = 0; i < size; ++i) {
		if (pids[i] > 0)
			waitpid(pids[i], NULL, 0);
	}

	exit(0);
}

int main(int argc, char **argv) {
	if (argc != 2)
		app_error("Usage: %s [SIZE]", argv[0]);

	int size = atoi(argv[1]);

	if (size < 3 || size > 9)
		app_error("Give board size in range from 4 to 9!");

	int board[size];
	for (int i = 0; i < size; i++)
		board[i] = -1;

	busy = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (busy == MAP_FAILED) unix_error("mmap failed");
	*busy = 0;

    solve(0, board, size);

	print_board(size, board);

	return 0;
}
