#include "include/csapp.h"

static pid_t spawn(void (*fn)(void)) {
	pid_t pid = Fork();
	if (pid == 0) {
		fn();
		printf("(%d) I'm done!\n", getpid());
		exit(EXIT_SUCCESS);
	}
	return pid;
}

static void grandchild(void) {
	printf("(%d) Waiting for signal!\n", getpid());
	pause();
	printf("(%d) Got the signal!\n", getpid());
}

static void child(void) {
	pid_t pid = spawn(grandchild);
	printf("(%d) Grandchild (%d) spawned!\n", getpid(), pid);
}

/* Runs command "ps -o pid,ppid,pgrp,stat,cmd" using execve(2). */
static void ps(void) {
	char *argv[] = {"ps", "-o", "pid,ppid,pgrp,stat,cmd", NULL};
	char *envp[] = {NULL};
	execve("/bin/ps", argv, envp);
	unix_error("execve error");
}

int main(void) {
#ifdef LINUX
	Prctl(PR_SET_CHILD_SUBREAPER, 1);
#endif
	printf("(%d) I'm a reaper now!\n", getpid());

	pid_t pid = spawn(child);

	Wait(NULL);

	spawn(ps);



	return EXIT_SUCCESS;
}
