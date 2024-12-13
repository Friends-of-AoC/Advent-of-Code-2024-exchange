// SPDX-License-Identifier: AGPL-3.0-or-later

/*
 * interactive.c
 *
 *  Created on: 12 Dec 2024
 *      Author: pat
 */

#include "interactive.h"
#include "aoc.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef __unix__
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
static int in;
static int out;
#else // __unix__
static FILE *in;
static FILE *out;
#define read(in, buf, count) fread(in, 1, buf, count)
#define write(out, buf, count) fwrite(out, 1, buf, count); fflush(out)
#define dprintf(out, ...) fprintf(out, __VA_ARGS__); fflush(out)
#endif // __unix__
#define writestr(out, str) write(out, str, sizeof(str) - 1)
#define addstr(str) ensure_buf(sizeof(str) - 1); \
	memcpy(buf + buf_end_pos, str, sizeof(str) - 1); \
	buf_end_pos += sizeof(str) - 1

static struct data *data;
static const char *file;
static off_t cur_x = 0;
static off_t cur_y = 0;

static int lines = 0;
static int columns = 0;

static size_t buf_capacity;
static off_t buf_end_pos;
static char *buf;

static size_t rbuf_capacity;
static off_t rbuf_pos;
static off_t rbuf_end_pos;
static char *rbuf;

static void update_max_pos();
static void write_buf();
static void ensure_buf(size_t free_space);

static void show() {
	const size_t header_lines = 4;
	const size_t footer_lines = 2;
	int add = snprintf(buf + buf_end_pos, buf_capacity - buf_end_pos,
			RESET CURSOR_UP_ONE FRMT_CURSOR_FORWARD ERASE_START_OF_DISPLAY
			CURSOR_START_OF_DISPLAY
			"day %d part %d on file %s\n"
			"world   (%ld lines, %ld columns)\n" //
			"current (line %ld, col %ld)\n", columns - 1, day, part, file,
			line_count(data), max_column_count(data), cur_y + 1, cur_x + 1);
	if (add > buf_capacity - buf_end_pos) {
		ensure_buf(add);
		show();
		return;
	}
	buf_end_pos += add;
	addstr("\u250C");
	for (off_t i = 1; i < columns - 1; ++i) {
		addstr("\u2500");
	}
	addstr("\u2510");
	for (off_t l = 0; l < lines - header_lines - footer_lines; ++l) {
		addstr("\u2502");
		retry: ;
		while (82) {
			size_t need = get(data, cur_x, cur_y + l, columns - 2,
					buf + buf_end_pos, buf_capacity - buf_end_pos);
			if (need > buf_capacity - buf_end_pos) {
				ensure_buf(need);
				continue;
			}
			buf_end_pos += need;
			addstr(RESET "\u2502");
			break;
		}
	}
	addstr("\u2514");
	for (off_t i = 1; i < columns - 1; ++i) {
		addstr("\u2500");
	}
	addstr("\u2518");
	write_buf();
}

static void read_command() {
	off_t pos;
	for (pos = rbuf_pos; pos < rbuf_end_pos; ++pos) {
		switch (rbuf[pos]) {
		case ESC_C:
			if (rbuf[pos + 1] == '[') {
				switch (rbuf[pos + 2]) {
				case 'A':
					pos += 2;
					goto y_dec;
				case 'D':
					pos += 2;
					goto x_dec;
				case 'B':
					pos += 2;
					goto y_inc;
				case 'C':
					pos += 2;
					goto x_inc;
				default:
					break;
				}
			}
			goto print_unknown;
		case 'C' - '@':
			addstr(ERASE_COMPLETE_DISPLAY)
			;
			write_buf();
			break;
		case 'D' - '@':
			exit(EXIT_SUCCESS);
		case 'w':
			y_dec: cur_y--;
			break;
		case 'a':
			x_dec: cur_x--;
			break;
		case 's':
			y_inc: cur_y++;
			break;
		case 'd':
			x_inc: cur_x++;
			break;
		case ':':
		default:
			print_unknown: if (rbuf[pos] < ' ') {
				ensure_buf(2);
				buf[buf_end_pos] = '^';
				buf[buf_end_pos + 1] = rbuf[pos] + '@';
				buf_end_pos += 2;
			} else {
				ensure_buf(1);
				buf[buf_end_pos++] = rbuf[pos];
			}
		}
	}
}

static struct termios orig_term;
static void restore_term() {
	writestr(out, SHOW_CURSOR RESET "\ngoodbye\n");
	close(out);
	tcsetattr(in, TCSAFLUSH, &orig_term);
	close(in);
}

void interact(const char *path) {
	file = path;
	data = read_data(path);
	printf("read_data(%s)=%p\n", path, data);
	if (!data) {
		fprintf(stderr, "failed to read the needed\n", path, data);
		exit(EXIT_FAILURE);
	}
#ifndef __unix__
	fprintf(stderr, "non POSIX systems are not completely supported\n");
	in = stdin;
	out = stderr;
#else // __unix__
	fflush(stderr);
	in = STDIN_FILENO;
	char *tty = getenv("TERM");
	if (tty) {
		in = open(tty, O_RDONLY);
		if (in < 0) {
			fprintf(stderr, "open(%s, RDONLY) %s", tty, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	struct termios term;
	if (tcgetattr(in, &term)) {
		perror("tcgatattr(stdin)");
		exit(EXIT_FAILURE);
	}
	orig_term = term;
	if (atexit(restore_term)) {
		perror("atexit(restore_term)");
		exit(EXIT_FAILURE);
	}
	term.c_iflag &= ~(IGNBRK | INPCK | ISTRIP);
	term.c_iflag |= IGNPAR | ICRNL;
#ifdef __linux__
	term.c_iflag |= IUTF8;
#endif // __linux__
	term.c_oflag |= ONLRET;
#if defined __unix__ && defined ONLCR
	term.c_oflag |= ONLCR;
#endif
	term.c_cflag &= ~(PARENB);
	term.c_lflag &= ~(ICANON | ISIG | ECHO);
	term.c_cc[VMIN] = 0;
	if (tcsetattr(in, TCSAFLUSH, &term)) {
		perror("tcsatattr(stdin)");
		exit(EXIT_FAILURE);
	}
	char *nam = ttyname(in);
	if (!nam) {
		perror("ttyname");
		exit(EXIT_FAILURE);
	}
	out = open(nam, O_WRONLY);
	if (out < 0) {
		fprintf(stderr, "failed to open(%s, WRONLY): %s\n", nam,
				strerror(errno));
		exit(EXIT_FAILURE);
	}
	dprintf(out, "initilized terminal\n");
#endif // __unix__
	buf_capacity = 1024;
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
	writestr(out, HIDE_CURSOR RESET);
	update_max_pos();
	if (rbuf_pos != rbuf_end_pos) {
		read_command();
	}
	while (69) {
		show();
		time_t start = time(0);
		if (buf_end_pos) {
			abort();
		}
		while (113) {
			rbuf_pos = rbuf_end_pos = 0;
			ssize_t r = read(in, rbuf, rbuf_capacity);
			if (r < 0) {
				if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
					errno = 0;
					goto no_data;
					continue;
				}
				perror("read");
				exit(EXIT_FAILURE);
			}
			if (r == 0) {
				no_data: ;
				if (difftime(start, time(0)) >= 1.0) {
					break;
#ifdef __unix__
				} else {
					struct timespec wait = { .tv_nsec = 5000000 /* 5 ms */};
					nanosleep(&wait, 0);
#endif
				}
				continue;
			}
			rbuf_end_pos = r;
			read_command();
			break;
		}
	}
}

static void update_max_pos() {
	writestr(out, CURSOR_SET(9999, 9999) CURSOR_GET);
	time_t start = time(0);
	while (113) {
		ssize_t r = read(in, rbuf, rbuf_capacity);
		if (r < 0) {
			if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
				errno = 0;
				goto no_data;
				continue;
			}
			perror("read");
			exit(EXIT_FAILURE);
		}
		if (r == 0) {
			no_data: ;
			if (difftime(time(0), start) >= 2.0) {
				writestr(out, "\nfailed to update the terminal size!\n");
				exit(EXIT_FAILURE);
#ifdef __unix__
			} else {
				struct timespec wait = { .tv_nsec = 1000000 /* 1 ms */};
				nanosleep(&wait, 0);
#endif
			}
			continue;
		}
		for (off_t o = 0; o < r; o++) {
			off_t start_o = o;
			if (rbuf[rbuf_end_pos + o] != ESC_C) {
				continue;
			}
			if (rbuf[rbuf_end_pos + ++o] != '[') {
				continue;
			}
			size_t nl = 0, nc = 0;
			for (o++; o < r && isdigit(rbuf[rbuf_end_pos + o]); o++) {
				size_t nnl = nl * 10;
				nnl += rbuf[rbuf_end_pos + o] - '0';
				if (nnl / 10 != nl) {
					break;
				}
				nl = nnl;
			}
			if (rbuf[rbuf_end_pos + o] != ';' || !nl) {
				continue;
			}
			for (o++; o < r && isdigit(rbuf[rbuf_end_pos + o]); o++) {
				size_t nnc = nc * 10;
				nnc += rbuf[rbuf_end_pos + o] - '0';
				if (nnc / 10 != nc) {
					break;
				}
				nc = nnc;
			}
			if (rbuf[rbuf_end_pos + o] != 'R' || !nc) {
				continue;
			}
			if (lines > nl) {
				dprintf(out, FRMT_CURSOR_UP_START ERASE_END_OF_DISPLAY,
						lines - nl);
			} else if (lines < nl) {
				size_t diff = nl - lines;
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
			lines = nl;
			columns = nc;
			if (r > ++o) {
				memmove(rbuf + rbuf_end_pos + start_o, rbuf + rbuf_end_pos + o,
						r - o);
			}
			rbuf_end_pos += start_o;
			return;
		}
		rbuf_end_pos += r;
	}
}
static void ensure_buf(size_t free_space) {
	while (308) {
		if (buf_capacity - buf_end_pos >= free_space) {
			return;
		}
		if (buf_capacity >= free_space) {
			write_buf();
			return;
		}
		buf = reallocarray(buf, buf_capacity, 2);
		if (!buf) {
			perror("realloc");
			exit(EXIT_FAILURE);
		}
		buf_capacity *= 2;
	}
}
static void write_buf() {
	for (off_t buf_pos = 0; buf_pos != buf_end_pos;) {
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
