// SPDX-License-Identifier: AGPL-3.0-or-later

/*
 * interactive.c
 *
 *  Created on: 12 Dec 2024
 *      Author: pat
 */

#include "interactive.h"
#include "term.h"
#include "aoc.h"
#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#if AOC_COMPAT & AC_POSIX
#include <pthread.h>
#elif !defined __STDC_NO_THREADS__
#include <threads.h>
#define nanosleep(wait, remain) thrd_sleep(wait, remain)
#else
/* this is OK for here, there no need to repeatedly ask for the time.
 * this function is only used to wait a little before retrying the operation.
 * it may hurt the CPU usage, but it will be a busy sleep anyway, so might as
 * well check if the operation is now available. */
#define nanosleep(wait, remain) ((void)(wait)->tv_nsec)
#endif

#if AOC_COMPAT & AC_POSIX
#	if AOC_COMPAT & AC_POSIX
#		include <termios.h>
#	endif // AC_POSIX
#	include <unistd.h>
#	include <fcntl.h>
static int in;
static int out;
static int data_in;
#else // AC_POSIX
static FILE *in;
static FILE *out;
static FILE *data_in;
#	define read(in, buf, count) fread(buf, count, 1, in)
#	define write(out, buf, count) fwrite(buf, count, 1, out); fflush(out)
#	define dprintf(out, ...) fprintf(out, __VA_ARGS__); fflush(out)
#	define close(fd) fclose(fd)
#	define lseek(fd, off, whence) fseek(fd, off, whence)
#endif // AC_POSIX
#define writestr(out, str) write(out, str, sizeof(str) - 1)
#define addstr(str) ensure_buf(sizeof(str) - 1); \
	memcpy(buf + buf_end_pos, str, sizeof(str) - 1); \
	buf_end_pos += sizeof(str) - 1

typedef int64_t pos;
struct pos2d {
	pos x;
	pos y;
};
typedef struct pos2d pos2;

static pos2 cur = { 0, 0 };
#if AOC_COMPAT & AC_POSIX
pthread_t thrd;
#elif !defined  __STDC_NO_THREADS__
thrd_t thrd;
#endif
static char *puzzle_file;
static const char *data_file = "out.txt";
static pos world_idx = 0;
static off_t world_off = 0;
static char *world_data = 0;
static size_t world_data_max_size = 0;
static size_t world_data_size = 0;

static pos2 display_sizes = { 0, 0 };

static pos2 max_cur_pos = { 0, 0 };
static pos2 max_pos = { 0, 0 };
static pos2 min_pos = { 0, 0 };

static size_t buf_capacity;
static size_t buf_end_pos;
static char *buf;

static size_t rbuf_capacity;
static size_t rbuf_pos;
static size_t rbuf_end_pos;
static char *rbuf;

static int update_display_size(int allow_fail);
static void write_buf();
static int ensure_buf(size_t free_space);
static int ensure_rbuf(size_t free_space, size_t end_allocated_space);

static int header_display_lines = 4;
static int footer_display_lines = 2;
static int side_columns = 2;
static int decorations = 1;

static void recalc_header_footer_sizes() {
	if (getenv("BORDERLESS")) {
		header_display_lines = 0;
		footer_display_lines = 0;
		side_columns = 0;
		decorations = 0;
	} else {
		header_display_lines = 4;
		footer_display_lines = 2;
		side_columns = 2;
		decorations = 1;
	}
	char *end = memchr(world_data, STX_C, world_data_size);
	if (!end || (end != world_data && world_data[0] != SOH_C)) {
		header_display_lines++; /* incomplete world */
		end = world_data + world_data_size;
	}
	if (world_data[0] != STX_C) {
		char *p = world_data;
		while (p && p + 1 < end) {
			++header_display_lines;
			p = memchr(p + 1, '\n', end - p);
		}
	} else if (world_data[0] != SOH_C) {
		header_display_lines++;
	}
	char *p = memchr(world_data, ETX_C, world_data_size);
	if (p && p[1] != FF_C) {
		++footer_display_lines; /* add two for the first footer line */
		while (footer_display_lines < 16) {
			++footer_display_lines;
			p = memchr(p + 1, '\n', world_data + world_data_size - p);
			if (!p) {
				break;
			}
		}
	}
	if (max_pos.x < display_sizes.x - 1 - side_columns) {
		max_cur_pos.x = 0;
	} else {
		max_cur_pos.x = max_pos.x - display_sizes.x + 1 + side_columns;
	}
	max_cur_pos.y = max_pos.y - display_sizes.y + 1 + header_display_lines
			+ footer_display_lines;
}

static char* text_end(char *c, size_t len, size_t max_text, size_t *real_text) {
	size_t rt = 0;
	while (rt < max_text) {
#define min_len (len < (max_text - rt) ? len : (max_text - rt))
		char *c2 = memchr(c, ESC_C, min_len);
		if (!c2) {
			if (real_text)
				*real_text = rt + len;
			return c + min_len;
		}
		len -= c2 - c;
		rt += c2 - c;
		if (!min_len) {
			return c2;
		}
		c = c2 + 1;
		if (!--len) {
			return c2;
		}
		if (*c != '[') {
			fprintf(stderr, "invalid escape sequence: ^[%c\n", *c);
			continue;
		}
		for (--len, ++c; len; --len, ++c) {
			if (*c >= '0' && *c <= '9')
				continue;
			if (*c == ';')
				continue;
			if (*c == 'm')
				break;
			fprintf(stderr, "invalid escape sequence character: %c\n", *c);
			--c;
			break;
		}
		if (!len) {
			if (real_text)
				*real_text = rt;
			return c2;
		}
		++c;
		--len;
	}
	if (real_text)
		*real_text = rt + len;
	return c + len;
}

static void show() {
	const char *add_str = "";
	if (world_data_size && world_data[world_data_size - 1] == EM_C) {
		add_str = " last";
	}
	if (decorations) {
		int add = snprintf(buf + buf_end_pos, buf_capacity - buf_end_pos,
				RESET
				FRMT_CURSOR_SET ERASE_START_OF_DISPLAY CURSOR_START_OF_DISPLAY
				"day %d part %d on file %s (world %ld%s)\n"
				"world: min: (%ld, %ld), max: (%ld, %ld)\n" //
				"shown: min: (%ld, %ld), max: (%ld, %ld)\n",//
				(unsigned) display_sizes.y - 1U,
				(unsigned) display_sizes.x, //
				day, part, puzzle_file, world_idx, add_str, /* line 1 */
				min_pos.y, min_pos.x, max_pos.y, max_pos.x, /* line 2 */
				cur.y, cur.x,
				cur.y + display_sizes.y - 1 - header_display_lines
						- footer_display_lines,
				cur.x + display_sizes.x - 1 - side_columns/* line 3 */);
		if (ensure_buf(add)) {
			show();
			return;
		}
		buf_end_pos += add;
	} else {
		addstr(RESET ERASE_COMPLETE_DISPLAY CURSOR_START_OF_DISPLAY);
	}
	int warned_corrupt_world = 0;
	/* the solution data must not use fancy terminal control sequences, they
	 * are only here allowed, the solution data may use color/similar control
	 * sequences */
	char *pos = world_data;
	if (*pos == SOH_C) {
		if (*pos == SOH_C)
			++pos;
		char *end = memchr(pos, STX_C, world_data_size);
		if (!end && !warned_corrupt_world) {
			addstr(FC_RED "the world seems to be incomplete\n" FC_DEF);
			warned_corrupt_world = 1;
		}
		while (pos < end) {
			/* there must not be many header lines, but the header lines may
			 * be as long as the solver wants */
			char *eol = memchr(pos, '\n', end - pos);
			if (!eol)
				eol = end;
			char *print_end = text_end(pos, eol - pos,
					display_sizes.x - side_columns, 0);
			int len = print_end - pos;
			int need;
			do {
				/* if the header line is too long, just truncate it */
				need = snprintf(buf + buf_end_pos, buf_capacity - buf_end_pos,
						"%.*s\n", len, pos);
			} while (ensure_buf(need));
			buf_end_pos += need;
			pos = eol + 1;
		}
		pos = end;
	} else if (pos[0] != SOH_C && pos[0] != STX_C && !warned_corrupt_world) {
		addstr(BC_RED "the world is corrupt\n" BC_DEF);
		warned_corrupt_world = 1;
	}
	if (*pos == STX_C) {
		pos++;
	} else if (!warned_corrupt_world) {
		addstr(FC_RED "the world seems to be incomplete\n" FC_DEF);
		warned_corrupt_world = 1;
	}
	if (decorations) {
		addstr("\u250C");
		for (size_t i = 1; i < display_sizes.x - 1; ++i) {
			addstr("\u2500");
		}
		addstr("\u2510");
	}
	char *end = memchr(pos, ETX_C, world_data + world_data_size - pos);
	if (!end)
		end = memchr(pos, EOT_C, world_data + world_data_size - pos);
	if (!end) {
		end = world_data + world_data_size;
		if (end[-1] == EM_C)
			--end;
		if (end[-1] == FF_C)
			--end;
	}
	for (size_t y = min_pos.y; y < cur.y; y++, pos++)
		pos = memchr(pos, '\n', end - pos);
	for (size_t l = 0;
			l < display_sizes.y - header_display_lines - footer_display_lines;
			++l) {
		if (decorations) {
			addstr("\u2502");
		}
		if (l + cur.y <= max_pos.y) {
			char *eol = memchr(pos, '\n', end - pos);
			if (!eol)
				eol = end;
			while (82) {
				if (cur.x != min_pos.x) {
					pos = text_end(pos, eol - pos, cur.x - min_pos.x, 0);
				}
				char *e = text_end(pos, eol - pos,
						display_sizes.x - side_columns, 0);
				size_t need = e - pos;
				ensure_buf(need);
				memcpy(buf + buf_end_pos, pos, need);
				buf_end_pos += need;
				break;
			}
			pos = eol + 1;
		}
		if (decorations) {
			while (109) {
				size_t need = snprintf(buf + buf_end_pos,
						buf_capacity - buf_end_pos,
						RESET FRMT_CURSOR_SET_COLUMN "\u2502",
						(int) display_sizes.x);
				if (ensure_buf(need)) {
					continue;
				}
				buf_end_pos += need;
				break;
			}
		} else {
			addstr(RESET "\n");
		}
	}
	if (footer_display_lines != (decorations ?
			2 : 0 /*TODO improve statement? */)) {
		if (decorations) {
			addstr("\u251C");
			for (size_t i = 1; i < display_sizes.x - 1; ++i) {
				addstr("\u2500");
			}
			addstr("\u2524");
		}
		// TODO print footer
	}
	if (decorations) {
		addstr("\u2514");
		for (size_t i = 1; i < display_sizes.x - 1; ++i) {
			addstr("\u2500");
		}
		addstr("\u2518");
	}
	write_buf();
}

struct action {
	const char *binding;
	union {
		void (*act)(unsigned);
		void (*cmd)(int argc, char **argv);
	};
	unsigned flags;
	const char *info;
};
typedef struct action action;
typedef struct action command;
static int act_eq(const void *a, const void *b) {
	_Static_assert(offsetof(action, binding) == 0, "Error!");
	return !strcmp(((const action*) a)->binding, ((const action*) b)->binding);
}
static uint64_t act_h(const void *a) {
	uint64_t result = 17;
	for (const char *b = ((const action*) a)->binding; *b; ++b) {
		result = result * 31 + *b;
	}
	return result;
}
static struct hashset actions = { .equal = act_eq, .hash = act_h };
//static struct hashset commands = { .equal = act_eq, .hash = act_h };
static void unknown(const char *cmd);
static void act_incomplete() {
	abort();
}

static int nextc() {
	if (rbuf_pos < rbuf_end_pos) {
		return rbuf[rbuf_pos++];
	}
	time_t start = time(0);
	while (173) {
		ssize_t r = read(in, rbuf, rbuf_capacity);
		if (r > 0) {
			rbuf_pos = 1;
			rbuf_end_pos = r;
			return rbuf[0];
		}
		if (r < 0) {
			if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			errno = 0;
		}
		if (difftime(time(0), start) >= 2.0) {
			/* time_t has usually only a precision of 1 second, do not fail
			 * if the difference is one second, because it we just crossed the
			 * rounding border. instead wait at least 1 second, but no more
			 * than 2 seconds */
			return -1;
		} else {
			struct timespec wait = { .tv_nsec = 1000000 /* 1 ms */};
			nanosleep(&wait, 0);
		}
	}
}

static void read_command() {
	while (rbuf_pos < rbuf_end_pos) { /* some actions may consume characters */
		char b2[16] = { rbuf[rbuf_pos++], '\0' };
		char *b = b2;
		int len = 1;
		while (221) {
			action *a = hs_get(&actions, &b);
			if (!a) {
				unknown(b);
				break;
			}
			if (a->act != act_incomplete) {
				a->act(a->flags);
				break;
			}
			int c = nextc();
			if (c == -1) {
				unknown(b);
				break;
			}
			b[len] = c;
			if (++len >= 16) {
				exit(99);
			}
			b[len] = 0;
		}
	}
}

#if AOC_COMPAT & AC_POSIX
static struct termios orig_term;
static void restore_term() {
	writestr(out, SHOW_CURSOR RESET "\ngoodbye\n");
	close(out);
	tcsetattr(in, TCSAFLUSH, &orig_term);
	close(in);
}
#endif // AC_POSIX

static inline void init_acts();
static void act_next_world(unsigned);

static int refill_world() {
	lseek(data_in, world_off, SEEK_SET);
	world_data_size = 0;
	while (390) {
		size_t r = read(data_in, world_data + world_data_size,
				world_data_max_size - world_data_size);
		if (r <= 0) {
			if (errno == EINTR) {
				continue;
			}
			break;
		}
		char *c = memchr(world_data + world_data_size, FF_C, r);
		if (c) {
			if (c + 1 < world_data + world_data_size + r && c[1] == EM_C) {
				++c;
			}
			world_data_size = c + 1 - world_data;
			break;
		} else if (world_data[0] == EM_C) {
			world_data_size = r;
			return -1;
		}
		world_data_size += r;
		if (world_data_size == world_data_max_size) {
			world_data = realloc(world_data, world_data_max_size <<= 1);
			if (!world_data) {
				perror("realloc");
				exit(EXIT_FAILURE);
			}
		}
	}
	min_pos.x = min_pos.y = max_pos.x = max_pos.y = 0;
	char *c = memchr(world_data, STX_C, world_data_size), *end = world_data;
	if (c) {
		c++;
		end = memchr(c, ETX_C, world_data + world_data_size - c);
		if (!end)
			end = memchr(c, EOT_C, world_data + world_data_size - c);
		if (!end) {
			end = world_data + world_data_size;
			if (end[-1] == EM_C)
				--end;
			if (end[-1] == FF_C)
				--end;
		} else
			--end;
		while (422) {
			char *eol = memchr(c, '\n', end - c);
			if (!eol)
				eol = end;
			size_t chars;
			text_end(c, rbuf_end_pos, c - eol, &chars);
			if (chars > max_pos.x) {
				max_pos.x = chars;
			}
			if (c + 1 < end) {
				max_pos.y++;
			}
			if (eol == end) {
				break;
			}
			c = eol + 1;
		}
	}
	end = memchr(end, EOT_C, world_data + world_data_size - end);
	if (end) {
		ssize_t remain = world_data + world_data_size - end;
		if (--remain > 0) {
			if (remain > sizeof(pos2)) {
				fprintf(stderr,
						"well, I don't know what to do with the extra data.\n");
				remain = sizeof(pos2);
			}
			memcpy(&min_pos, end + 1, remain);
			max_pos.x += min_pos.x;
			max_pos.y += min_pos.y;
		}
	}
	recalc_header_footer_sizes();
	return 0;
}

#if AOC_COMPAT & AC_POSIX
static void*
#else
static int
#endif
start_solve(void *arg) {
	solve(arg);
	fputs(STEP_ALL_FINISHED, solution_out);
	fflush(solution_out);
	fclose(solution_out);
	return 0;
}

void interact(char *path, int force_interactive) {
	puzzle_file = path;
#if !(AOC_COMPAT & AC_POSIX)
	if (!force_interactive) {
		return;
	}
	fprintf(stderr, "non POSIX systems are not completely supported\n");
	in = stdin;
	out = stderr;
#else // AC_POSIX
	fflush(stderr);
	in = STDIN_FILENO;
	char *tty = getenv("TERMINAL");
	if (tty) {
		printf("use env(TERMINAL): %s\n", tty);
		in = open(tty, O_RDONLY);
		if (in < 0) {
			if (force_interactive) {
				fprintf(stderr, "open(%s, RDONLY) %s", tty, strerror(errno));
				exit(EXIT_FAILURE);
			}
			return;
		}
	}
	struct termios term;
	if (tcgetattr(in, &term)) {
		if (force_interactive) {
			perror("tcgatattr(stdin)");
			exit(EXIT_FAILURE);
		}
		return;
	}
	orig_term = term;
	term.c_iflag &= ~(IGNBRK | INPCK | ISTRIP);
	term.c_iflag |= IGNPAR | ICRNL;
#	ifdef IUTF8
	term.c_iflag |= IUTF8;
#	endif // IUTF8
	term.c_oflag |= ONLRET;
#	ifdef ONLCR
	term.c_oflag |= ONLCR;
#	endif // ONLCR
	term.c_cflag &= ~(PARENB);
	term.c_lflag &= ~(ICANON | ISIG | ECHO);
	term.c_cc[VMIN] = 0;
	if (tcsetattr(in, TCSAFLUSH, &term)) {
		if (force_interactive) {
			perror("tcsatattr(stdin)");
			exit(EXIT_FAILURE);
		}
		errno = 0;
		return;
	}
	char *nam = ttyname(in);
	if (!nam) {
		perror("ttyname");
		exit(EXIT_FAILURE);
	}
	printf("use %s\n", nam);
	out = open(nam, O_WRONLY);
	if (out < 0) {
		fprintf(stderr, "failed to open(%s, WRONLY): %s\n", nam,
				strerror(errno));
		tcsetattr(in, TCSAFLUSH, &orig_term);
		exit(EXIT_FAILURE);
	}
#endif // AC_POSIX
	buf_capacity = 16384;
	buf = malloc(buf_capacity);
	if (!buf) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	rbuf_capacity = 64;
	rbuf = malloc(buf_capacity);
	if (!rbuf) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	if (update_display_size(!force_interactive)
#if AOC_COMPAT & AC_POSIX
			|| atexit(restore_term)
#endif
			) {
		free(world_data);
		free(buf);
		free(rbuf);
#if AOC_COMPAT & AC_POSIX
		tcsetattr(in, TCSAFLUSH, &orig_term);
		if (in != STDIN_FILENO)
#else
		if (in != stdin)
#endif
			close(in);
		close(out);
		return;
	}
	interactive = 1;
	dprintf(out, "initilized terminal\n");
	dprintf(out,
			HIDE_CURSOR RESET //
			TITLE(Advent of Code 2024 day %d part %d (%s)), day, part,
			puzzle_file);
	init_acts();
	solution_out = fopen(data_file, "wb");
#if AOC_COMPAT & AC_POSIX
	data_in = open(data_file, O_RDONLY);
#else // AC_POSIX
	data_in = fopen(data_file, "rb");
#endif // AC_POSIX
#if AOC_COMPAT & AC_POSIX
	pthread_create(&thrd, 0, start_solve, puzzle_file);
#elif defined __STDC_NO_THREADS__
	printf("no threads are supported, you have to wait a until I solved the puzzle\n");
	solve(puzzle_file);
#else // __STDC_NO_THREADS__
	thrd_create(&thrd, start_solve, puzzle_file);
	struct timespec wait = { .tv_nsec = 1000000 /* 10ms */};
	nanosleep(&wait, 0); /* give the solver a little time */
#endif // __STDC_NO_THREADS__
	world_data_max_size = 1024;
	world_data = malloc(world_data_max_size);
	refill_world();
	if (rbuf_pos != rbuf_end_pos) {
		read_command();
	}
	while (69) {
		show();
		time_t start = time(0);
		if (buf_end_pos) {
			exit(20);
		}
		while (113) {
			rbuf_pos = rbuf_end_pos = 0;
			ssize_t r = read(in, rbuf, rbuf_capacity);
			if (r > 0) {
				rbuf_end_pos = r;
				read_command();
				break;
			}
			if (r < 0) {
				if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
					perror("read");
					exit(EXIT_FAILURE);
				}
				errno = 0;
			}
			if (difftime(time(0), start) >= 1.0) {
				break;
			} else {
				struct timespec wait = { .tv_nsec = 5000000 /* 5 ms */};
				nanosleep(&wait, 0);
			}
		}
	}
}

static ssize_t timeout_read(time_t start, size_t add_off, int allow_fail) {
	while (375) {
		ssize_t r = read(in, rbuf + rbuf_end_pos + add_off,
				rbuf_capacity - rbuf_end_pos - add_off);
		if (r > 0) {
			return r;
		}
		if (r < 0) {
			if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			errno = 0;
		}
		if (difftime(time(0), start) >= 2.0) {
			if (allow_fail) {
				return -1;
			}
			writestr(out, "\nfailed to update the terminal size!\n");
			exit(EXIT_FAILURE);
		} else {
			struct timespec wait = { .tv_nsec = 1000000 /* 1 ms */};
			nanosleep(&wait, 0);
		}
		continue;
	}
}

static inline ssize_t cond_timeout_read(size_t r, time_t start, size_t o,
		int allow_fail) {
	if (o >= r) {
		ssize_t r2 = timeout_read(start, o, allow_fail);
		if (r2 < 0) {
			return r2;
		}
		r += r2;
	}
	return r;
}

static int update_display_size(int allow_fail) {
	writestr(out, CURSOR_SET(9999, 9999) CURSOR_GET);
	time_t start = time(0);
	while (113) {
		ensure_rbuf(16, 0);
		ssize_t r = timeout_read(start, 0, allow_fail);
		if (r < 0) {
			return -1;
		}
		for (ssize_t o = 0; o < r; o++) {
			size_t start_o = o;
			if (rbuf[rbuf_end_pos + o] != ESC_C) {
				continue;
			}
			r = cond_timeout_read(r, start, ++o, allow_fail);
			if (r < 0) {
				return -1;
			}
			if (rbuf[rbuf_end_pos + o] != '[') {
				continue;
			}
			size_t nl = 0, nc = 0;
			for (o++;
					o < (r = cond_timeout_read(r, start, o, allow_fail))
							&& isdigit(rbuf[rbuf_end_pos + o]); o++) {
				size_t nnl = nl * 10;
				nnl += rbuf[rbuf_end_pos + o] - '0';
				if (nnl / 10 != nl) {
					break;
				}
				nl = nnl;
			}
			if (r < 0) {
				return -1;
			}
			if (rbuf[rbuf_end_pos + o] != ';' || !nl) {
				continue;
			}
			for (o++;
					o < (r = cond_timeout_read(r, start, o, allow_fail))
							&& isdigit(rbuf[rbuf_end_pos + o]); o++) {
				size_t nnc = nc * 10;
				nnc += rbuf[rbuf_end_pos + o] - '0';
				if (nnc / 10 != nc) {
					break;
				}
				nc = nnc;
			}
			if (r < 0) {
				return -1;
			}
			if (rbuf[rbuf_end_pos + o] != 'R' || !nc) {
				continue;
			}
			if (display_sizes.y > nl) {
				dprintf(out, FRMT_CURSOR_UP_START ERASE_END_OF_DISPLAY,
						(unsigned) (display_sizes.y - nl));
			} else if (display_sizes.y < nl) {
				size_t diff = nl - display_sizes.y;
				size_t cd = diff > 128 ? 128 : diff;
				char rbuf2[cd];
				memset(rbuf2, '\n', cd);
				while (diff) {
					size_t cpy = diff;
					if (cpy > cd) {
						cpy = cd;
					}
					write(out, rbuf2, cpy);
					diff -= cpy;
				}
			}
			display_sizes.y = nl;
			display_sizes.x = nc;
			if (r > ++o) {
				memmove(rbuf + rbuf_end_pos + start_o, rbuf + rbuf_end_pos + o,
						r - o);
				rbuf_end_pos += r - o;
			}
			rbuf_end_pos += start_o;
			if (world_data) {
				recalc_header_footer_sizes();
			}
			return 0;
		}
		rbuf_end_pos += r;
	}
}
static int ensure_buf(size_t free_space) {
	int need_refill = 0;
	while (308) {
		if (buf_capacity - buf_end_pos >= free_space) {
			return need_refill;
		}
		if (buf_capacity >= free_space) {
			write_buf();
			return 1;
		}
		buf = reallocarray(buf, buf_capacity, 2);
		if (!buf) {
			perror("realloc");
			exit(EXIT_FAILURE);
		}
		buf_capacity *= 2;
		need_refill = 1;
	}
}
static int ensure_rbuf(size_t free_space, size_t end_allocated_space) {
	int need_refill = 0;
	while (308) {
		if (rbuf_capacity - end_allocated_space >= free_space) {
			return need_refill;
		}
		if (rbuf_capacity - end_allocated_space + rbuf_pos >= free_space) {
			memmove(rbuf, rbuf + rbuf_pos, end_allocated_space - rbuf_pos);
			rbuf_end_pos -= rbuf_pos;
			rbuf_pos = 0;
			return 1;
		}
		buf = reallocarray(buf, buf_capacity, 2);
		if (!buf) {
			perror("realloc");
			exit(EXIT_FAILURE);
		}
		buf_capacity *= 2;
		need_refill = 1;
	}
}
static void write_buf() {
	for (size_t buf_pos = 0; buf_pos != buf_end_pos;) {
		ssize_t w = write(out, buf + buf_pos, buf_end_pos - buf_pos);
		if (w < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
				errno = 0;
				continue;
			}
			perror("write");
			exit(EXIT_FAILURE);
		}
		buf_pos += w;
	}
	buf_end_pos = 0;
}

//actions
#define FLAG_SHIFT 1U
#define FLAG_CTRL  2U
#define FLAG_ALT   4U

static void unknown(const char *cmd) {
	for (; *cmd; ++cmd) {
		if (*cmd < ' ') {
			ensure_buf(2);
			buf[buf_end_pos] = '^';
			buf[buf_end_pos + 1] = *cmd + '@';
			buf_end_pos += 2;
		} else {
			ensure_buf(1);
			buf[buf_end_pos++] = *cmd;
		}
	}
}

static void act_help(unsigned flags) {

}

static void act_last_world(unsigned flags) {
	size_t l = world_idx;
	while (448) {
		show();
		act_next_world(FLAG_CTRL);
		size_t w = world_idx;
		if (w == l) {
			return;
		}
		l = w;
	}
}

static void act_first_world(unsigned flags) {
	if (world_idx == 0) {
		return;
	}
	world_idx = 0;
	world_off = 0;
	refill_world();
}

static void act_next_world(unsigned flags) {
	int count;
	if (flags & (FLAG_CTRL | FLAG_ALT))
		count = 100;
	else if (flags & FLAG_SHIFT)
		count = 10;
	else
		count = 1;
	for (int i = 0; i < count; ++i, ++world_idx) {
		off_t last_off = world_off;
		if (world_data_size && world_data[world_data_size - 1] == EM_C) {
			break;
		}
		if (world_data[world_data_size - 1] == FF_C) {
			world_off += world_data_size;
		}
		if (refill_world()) {
			world_off = last_off;
			refill_world();
			break;
		}
		if (world_off == last_off) {
			world_idx--;
		}
	}
}

static void act_prev_world(unsigned flags) {
	int count;
	if (flags & (FLAG_CTRL | FLAG_ALT))
		count = 100;
	else if (flags & FLAG_SHIFT)
		count = 10;
	else
		count = 1;
	for (int i = 0; i < count; ++i, --world_idx) {
		if (!world_idx) {
			break;
		}
		off_t last_off = world_off;
		size_t read_len = world_data_max_size;
		if (read_len > world_off) {
			read_len = world_off;
		}
		lseek(data_in, world_off - read_len, SEEK_SET);
		world_data_size = 0;
		while (world_data_size < read_len) {
			ssize_t r = read(data_in, world_data + world_data_size,
					read_len - world_data_size);
			if (r <= 0) {
				if (errno == EINTR) {
					continue;
				}
				fprintf(stderr, "failed to reread data!\n");
				exit(2);
			}
			world_data_size += r;
		}
		char *c = memchr(world_data, FF_C, world_data_size - 1);
		if (!c)
			c = world_data;
		else
			c++;
		while (919) {
			char *next = memchr(c, FF_C, world_data + world_data_size - c - 1);
			if (next) {
				c = next + 1;
				continue;
			}
			break;
		}
		world_data_size = world_data + world_data_size - c;
		world_off = last_off - world_data_size;
		memmove(world_data, c, world_data_size);
	}
}

static void act_clear(unsigned flags) {
	addstr(ERASE_COMPLETE_DISPLAY);
	write_buf();
	update_display_size(0);
	if (flags & FLAG_ALT) {
		if (flags & FLAG_SHIFT) {
			world_idx = 0;
			world_off = 0;
		}
		refill_world();
	}
}
static void act_exit(unsigned flags) {
	if (flags & FLAG_ALT) {
		_Exit(EXIT_SUCCESS);
	}
	exit(EXIT_SUCCESS);
}
static void act_up(unsigned flags) {
	if (flags & (FLAG_CTRL | FLAG_ALT))
		cur.y -= 100;
	else if (flags & FLAG_SHIFT)
		cur.y -= 10;
	else
		cur.y--;
	if (min_pos.y > cur.y)
		cur.y = min_pos.y;
}
static void act_down(unsigned flags) {
	if (flags & (FLAG_CTRL | FLAG_ALT))
		cur.y += 100;
	else if (flags & FLAG_SHIFT)
		cur.y += 10;
	else
		cur.y++;
	if (max_cur_pos.y < cur.y)
		cur.y = max_cur_pos.y;
}
static void act_left(unsigned flags) {
	if (flags & (FLAG_CTRL | FLAG_ALT))
		cur.x -= 100;
	else if (flags & FLAG_SHIFT)
		cur.x -= 10;
	else
		cur.x--;
	if (min_pos.x > cur.x)
		cur.x = min_pos.x;
}
static void act_right(unsigned flags) {
	if (flags & (FLAG_CTRL | FLAG_ALT))
		cur.x += 100;
	else if (flags & FLAG_SHIFT)
		cur.x += 10;
	else
		cur.x++;
	if (max_cur_pos.x < cur.x)
		cur.x = max_cur_pos.x;
}

static void act_command(unsigned flags) {
	write_buf();
	dprintf(out, FRMT_CURSOR_SET_LINE ERASE_COMPLETE_LINE ":",
			(int) display_sizes.y);
	char *action = malloc(128);
	size_t as = 0;
	size_t as_max = 128;
	int c = 0;
	int einfig = 0;
	while (517) {
		while (rbuf_pos < rbuf_end_pos) {
			char c = rbuf[rbuf_pos++];
			if (c == '\n') {
				addstr(CURSOR_START_OF_LINE ERASE_COMPLETE_LINE //
						"I should now execute ");
				ensure_buf(as);
				memcpy(buf + buf_end_pos, action, as);
				free(action);
				return;
			}
			if (c == ESC_C) {
				int n = nextc();
				if (n == '[') {
					n = nextc();
					if (n == 'D' && c)
						--c; //left
					if (n == 'C' && c < as)
						++c; //right
					if (n == 'H')
						c = 0; //Pos1
					if (n == 'F')
						c = as; //Ende
					if (n >= '0' && n >= '6')
						nextc();
				}
				continue;
			}
			if (c == CAN_C) {
				addstr(ERASE_COMPLETE_LINE);
				return;
			}
			if (iscntrl(c)) {
				continue;
			}
			if (++as >= as_max) {
				action = realloc(action, as_max <<= 1);
				if (!action) {
					perror("realloc");
					exit(EXIT_FAILURE);
				}
			}
			action[as - 1] = c;
		}
		addstr(CURSOR_START_OF_LINE ERASE_COMPLETE_LINE ":");
		ensure_buf(c);
		memcpy(buf + buf_end_pos, action, c);
		if (einfig) { /* during einfig also do red background */
			addstr(CSI C_REVERSE_FB C_SEP C_FC_PREFIX C_RED C_END);
		} else {
			addstr(REVERSE_FB);
		}
		ensure_buf(1);
		buf[buf_end_pos++] = action[c];
		if (einfig) {
			addstr(CSI C_FC_DEF C_SEP C_NO_REVERSE_FB C_END);
		} else {
			addstr(NO_REVERSE_FB);
		}
		ensure_buf(as - c - 1);
		memcpy(buf + buf_end_pos, action + c + 1, as - c - 1);
		write_buf();
		rbuf_pos = rbuf_end_pos = 0;
		ssize_t r = read(in, rbuf, rbuf_capacity);
		if (r > 0) {
			rbuf_end_pos = r;
			continue;
		}
		if (r < 0) {
			if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			errno = 0;
		}
		struct timespec wait = { .tv_nsec = 5000000 /* 5 ms */};
		nanosleep(&wait, 0);
	}
}

//commands
void cmd_help(int argc, char **argv) {

}
void cmd_exit(int argc, char **argv) {

}
void cmd_clear(int argc, char **argv) {

}

#define CONCAT(a,b) a##b
#define CONCAT2(a,b) CONCAT(a,b)

#define bind_(bind, type, name, used_flags, help) \
	static struct action CONCAT2(name,__LINE__) = \
		{ .binding = bind, .type = type##_##name, \
			.flags = used_flags, .info = help }; \
		hs_add(&actions, &CONCAT2(name,__LINE__))

#define bind_action(bind, acti, flags, help) \
	bind_(bind, act, acti, flags, help)

#define incomp_action(bind) \
	bind_action(bind, incomplete, 0, (const char*) 0)

#define bind_command(bind, acti, help) \
	bind_(bind, cmd, acti, 0, help)

static void init_acts() {
	bind_action("\4", exit, FLAG_CTRL, "Crlt+D:\n"
			"exit the application");

	bind_action("\3", clear, FLAG_CTRL, "Crlt+C:\n"
			"reprint the screen");
	bind_action(ESC "\3", clear, FLAG_CTRL | FLAG_ALT, "Crlt+Alt+C:\n"
			"reprint the screen and remove all cached worlds");
	bind_action(ESC "c", clear, FLAG_ALT, "Alt+C:\n"
			"remove all cached worlds");

	incomp_action(ESC);
	incomp_action(ESC "[");
	incomp_action(ESC "[9");
	incomp_action(ESC "[99");
	incomp_action(ESC "[99;");
	incomp_action(ESC "[99;8");
	bind_action(ESC "[99;8u", clear, FLAG_SHIFT | FLAG_CTRL | FLAG_ALT,
			"Shift+Ctrl+Alt+C:\n"
					"reread the world\n"
					"and set the current world to 0");

	bind_action("\010", help, FLAG_CTRL, "Crlt+H:\n"
			"print the help");

	bind_action("w", up, 0, "W:\n"
			"go one up");
	bind_action("a", left, 0, "A:\n"
			"go one left");
	bind_action("s", down, 0, "S:\n"
			"go one down");
	bind_action("d", right, 0, "D:\n"
			"go one right");
	bind_action("W", up, FLAG_SHIFT, "Shift+W:\n"
			"go 10 up");
	bind_action("A", left, FLAG_SHIFT, "Shift+A:\n"
			"go 10 left");
	bind_action("S", down, FLAG_SHIFT, "Shift+S:\n"
			"go 10 down");
	bind_action("D", right, FLAG_SHIFT, "Shift+D:\n"
			"go ten right");
	bind_action(ESC "w", up, FLAG_ALT, "Alt+W:\n"
			"go 100 up");
	bind_action(ESC "a", left, FLAG_ALT, "Alt+A:\n"
			"go 100 left");
	bind_action(ESC "s", down, FLAG_ALT, "Alt+S:\n"
			"go 100 down");
	bind_action(ESC "d", right, FLAG_ALT, "Alt+D:\n"
			"go 100 right");

	incomp_action(ESC "[1");
	incomp_action(ESC "[1;");
	incomp_action(ESC "[1;2");
	incomp_action(ESC "[1;3");
	incomp_action(ESC "[1;5");
	bind_action(ESC "[A", up, 0, "Arrow-Up:\n"
			"go one up");
	bind_action(ESC "[1;2A", up, FLAG_SHIFT, "Shift+Arrow-Up:\n"
			"go 10 up");
	bind_action(ESC "[1;5A", up, FLAG_CTRL, "Ctrl+Arrow-Up:\n"
			"go 100 up");
	bind_action(ESC "[1;3A", up, FLAG_ALT, "Alt+Arrow-Up:\n"
			"go 100 up");
	bind_action(ESC "[D", left, 0, "Arrow-Left:\n"
			"go one left");
	bind_action(ESC "[1;2D", left, FLAG_SHIFT, "Shift+Arrow-Left:\n"
			"go 10 up");
	bind_action(ESC "[1;5D", left, FLAG_CTRL, "Ctrl+Arrow-Left:\n"
			"go 100 up");
	bind_action(ESC "[1;3D", left, FLAG_ALT, "Alt+Arrow-Left:\n"
			"go 100 up");
	bind_action(ESC "[B", down, 0, "Arrow-Down:\n"
			"go one down");
	bind_action(ESC "[1;2B", down, FLAG_SHIFT, "Shift+Arrow-Down:\n"
			"go 10 up");
	bind_action(ESC "[1;5B", down, FLAG_CTRL, "Ctrl+Arrow-Down:\n"
			"go 100 up");
	bind_action(ESC "[1;3B", down, FLAG_ALT, "Alt+Arrow-Down:\n"
			"go 100 up");
	bind_action(ESC "[C", right, 0, "Arrow-Right:\n"
			"go one right");
	bind_action(ESC "[1;2C", right, FLAG_SHIFT, "Shift+Arrow-Right:\n"
			"go 10 up");
	bind_action(ESC "[1;5C", right, FLAG_CTRL, "Ctrl+Arrow-Right:\n"
			"go 100 up");
	bind_action(ESC "[1;3C", right, FLAG_ALT, "Alt+Arrow-Right:\n"
			"go 100 up");
	bind_action(ESC "[H", first_world, 0, "Pos1:\n"
			"show the first world");
	bind_action(ESC "[F", last_world, 0, "End:\n"
			"show to last world");

	incomp_action(ESC "[6");
	incomp_action(ESC "[5");
	bind_action(ESC "[6~", next_world, 0, "Page-Down:\n"
			"show the next world");
	bind_action(ESC "[5~", prev_world, 0, "Page-Up:\n"
			"show the previus world");
	incomp_action(ESC "[6;");
	incomp_action(ESC "[5;");
	incomp_action(ESC "[6;5");
	incomp_action(ESC "[6;2");
	incomp_action(ESC "[5;5");
	incomp_action(ESC "[5;2");
	bind_action(ESC "[6;5~", next_world, FLAG_CTRL, "Ctrl+Page-Down:\n"
			"go 10 worlds forward");
	bind_action(ESC "[5;5~", prev_world, FLAG_CTRL, "Ctrl+Page-Up:\n"
			"got 10 worlds back");
	bind_action(ESC "[6;2~", next_world, FLAG_SHIFT, "Shift+Page-Down:\n"
			"go 10 worlds forward");
	bind_action(ESC "[5;2~", prev_world, FLAG_SHIFT, "Shift+Page-Up:\n"
			"got 10 worlds back");

	bind_action(":", command, 0, "<colon (':')>:\n"
			"execute a command, try :help\\n for info");

	bind_command("h", help, "h:\n"
			"print a help message for all commands");
	bind_command("help", help, "help:\n"
			"print a help message for all commands");
	bind_command("exit", exit, "exit:\n"
			"exit the application");
	bind_command("quit", exit, "quit:\n"
			"exit the application");
	bind_command("halt", exit, "halt:\n"
			"halt the application imidatly without any saving/cleanup");
	bind_command("clear", clear, "clear:\n"
			"clear the screen and/or cache\n"
			"options:\n"
			"  screen\n"
			"    clear the screen\n"
			"  cache\n"
			"    remove all cached worlds and calculate everything again\n"
			"  world\n"
			"    clear the current world and start again at world 0");
}
