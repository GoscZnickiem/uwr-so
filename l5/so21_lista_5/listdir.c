#include "include/csapp.h"
#include <sys/sysmacros.h>

#define DIRBUFSZ 256

static void print_mode(mode_t m) {
	char t;

	if (S_ISDIR(m))
		t = 'd';
	else if (S_ISCHR(m))
		t = 'c';
	else if (S_ISBLK(m))
		t = 'b';
	else if (S_ISREG(m))
		t = '-';
	else if (S_ISFIFO(m))
		t = 'f';
	else if (S_ISLNK(m))
		t = 'l';
	else if (S_ISSOCK(m))
		t = 's';
	else
		t = '?';

	char ur = (m & S_IRUSR) ? 'r' : '-';
	char uw = (m & S_IWUSR) ? 'w' : '-';
	char ux = (m & S_IXUSR) ? 'x' : '-';
	char gr = (m & S_IRGRP) ? 'r' : '-';
	char gw = (m & S_IWGRP) ? 'w' : '-';
	char gx = (m & S_IXGRP) ? 'x' : '-';
	char or = (m & S_IROTH) ? 'r' : '-';
	char ow = (m & S_IWOTH) ? 'w' : '-';
	char ox = (m & S_IXOTH) ? 'x' : '-';

	if (m & S_ISUID) ux = (ux == 'x') ? 's' : 'S';
    if (m & S_ISGID) gx = (gx == 'x') ? 's' : 'S';
    if (m & S_ISVTX) ox = (ox == 'x') ? 't' : 'T';

	printf("%c%c%c%c%c%c%c%c%c%c", t, ur, uw, ux, gr, gw, gx, or, ow, ox);
}

static void print_uid(uid_t uid) {
	struct passwd *pw = getpwuid(uid);
	if (pw)
		printf(" %10s", pw->pw_name);
	else
		printf(" %10d", uid);
}

static void print_gid(gid_t gid) {
	struct group *gr = getgrgid(gid);
	if (gr)
		printf(" %10s", gr->gr_name);
	else
		printf(" %10d", gid);
}

static void file_info(int dirfd, const char *name) {
	struct stat sb[1];

	if (fstatat(dirfd, name, sb, AT_SYMLINK_NOFOLLOW) == -1) {
		perror("fstatat");
		return;
	}

	print_mode(sb->st_mode);
	printf("%4ld", sb->st_nlink);
	print_uid(sb->st_uid);
	print_gid(sb->st_gid);

	if (S_ISCHR(sb->st_mode) || S_ISBLK(sb->st_mode)) {
		printf(" %4u,%4u", major(sb->st_rdev), minor(sb->st_rdev));
	} else {
		printf(" %10ld", (long)sb->st_size);
	}

	char *now = ctime(&sb->st_mtime);
	now[strlen(now) - 1] = '\0';
	printf("%26s", now);

	printf("  %s", name);

	if (S_ISLNK(sb->st_mode)) {
		char link_target[PATH_MAX];
		ssize_t len = readlinkat(dirfd, name, link_target, sizeof(link_target) - 1);
		if (len != -1) {
			link_target[len] = '\0';
			printf(" -> %s", link_target);
		}
	}

	putchar('\n');
}

int main(int argc, char *argv[]) {
	if (!argv[1])
		argv[1] = ".";

	int dirfd = Open(argv[1], O_RDONLY | O_DIRECTORY, 0);
	char buf[DIRBUFSZ];
	int n;

	while ((n = Getdents(dirfd, (void *)buf, DIRBUFSZ))) {
		int bpos = 0;
		while(bpos < n) {
			struct linux_dirent* d = (struct linux_dirent*) (buf + bpos);
			file_info(dirfd, d->d_name);
			bpos += d->d_reclen;
		}
	}

	Close(dirfd);
	return EXIT_SUCCESS;
}
