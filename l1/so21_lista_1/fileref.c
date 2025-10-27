#include "include/csapp.h"

static char buf[256];

#define LINE1 49
#define LINE2 33
#define LINE3 78

static void do_read(int fd) {
	off_t pos;
	ssize_t n;

	pid_t pid = Fork();

	if (pid == 0) {
		pos = Lseek(fd, 0, SEEK_CUR);
		printf("[child %d] Cursor before read: %ld\n", getpid(), (long)pos);

		n = Read(fd, buf, 50);
		buf[n] = '\0';
		printf("[child %d] Read: '%s'\n", getpid(), buf);

		pos = Lseek(fd, 0, SEEK_CUR);
		printf("[child %d] Cursor after read: %ld\n", getpid(), (long)pos);
		exit(0);
	} else {
		pos = Lseek(fd, 0, SEEK_CUR);
		printf("[parent %d] Cursor before read: %ld\n", getpid(), (long)pos);

		n = Read(fd, buf, 50);
		buf[n] = '\0';
		printf("[parent %d] Read: '%s'\n", getpid(), buf);

		pos = Lseek(fd, 0, SEEK_CUR);
		printf("[parent %d] Cursor after read: %ld\n", getpid(), (long)pos);

		Wait(NULL);
	}
	exit(0);
}

static void do_close(int fd) {
	pid_t pid = Fork();

	if (pid == 0) {
		printf("[child %d] Closing file descriptor\n", getpid());
		Close(fd);
		exit(0);
	} else {
		Wait(NULL);
		printf("[parent %d] After child exited, trying to read...\n", getpid());

		ssize_t n = Read(fd, buf, 10);
		if (n > 0) {
			buf[n] = '\0';
			printf("[parent %d] Read succeeded: '%s'\n", getpid(), buf);
		} else {
			perror("[parent] Read failed");
		}
	}
  exit(0);
}

int main(int argc, char **argv) {
	if (argc != 2)
		app_error("Usage: %s [read|close]", argv[0]);

	int fd = Open("test.txt", O_RDONLY, 0);

	if (!strcmp(argv[1], "read"))
		do_read(fd);
	if (!strcmp(argv[1], "close"))
		do_close(fd);
	app_error("Unknown variant '%s'", argv[1]);
}
