/*
 * aoc.c
 *
 *  Created on: Dec 2, 2024
 *      Author: pat
 */

#include "aoc.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "color.h"
#include "hash.h"
#include "interactive.h"

#ifdef INTERACTIVE
#define INTERACT(...) __VA_ARGS__
#else
#define INTERACT(...)
#endif

struct data* read_data(const char *path);

int day = 25;
int part = 2;
FILE *solution_out;
#ifdef INTERACTIVE
int interactive = 0;
#else
#define interactive 0
#endif

#define starts_with(str, start) !memcmp(str, start, sizeof(start) - 1)

typedef int64_t num;

struct pos {
	num x;
	num y;
};

#define LK_WIDTH 5
#define LK_HEIGHT 5

struct lock_or_key {
	uint8_t heighs[LK_WIDTH];
	uint8_t _pad[8 - LK_WIDTH];
};

typedef struct lock_or_key lock;
typedef struct lock_or_key key;

enum data_state {
	ds_none, ds_key, ds_lock
};

struct data {
	lock *locks;
	key *keys;
	uint64_t keys_count;
	uint64_t locks_count;
	uint64_t max_keys_count;
	uint64_t max_locks_count;
	enum data_state state;
	int row;
	struct lock_or_key current;
};

static int do_print = 1;

static void print(FILE *str, struct data *data, uint64_t result) {
	if (result) {
		fprintf(str, "%sresult=%"I64"u\n%s", STEP_HEADER, result, STEP_BODY);
	} else {
		fputs(STEP_BODY, str);
	}
	if (!do_print && !interactive) {
		return;
	}
	fputs("locks:", str);
	for (uint64_t i = 0; i < data->locks_count; ++i) {
		for (int r = -1; r <= LK_HEIGHT; ++r) {
			fputs("\n  ", str);
			for (int c = 0; c < LK_WIDTH; ++c) {
				if (data->locks[i].heighs[c] > r) {
					fputc('#', str);
				} else {
					fputc('.', str);
				}
			}
		}
		fputc('\n', str);
	}
	fputs("keys:", str);
	for (uint64_t i = 0; i < data->keys_count; ++i) {
		for (int r = 0; r <= LK_HEIGHT + 1; ++r) {
			fputs("\n  ", str);
			for (int c = 0; c < LK_WIDTH; ++c) {
				if (data->keys[i].heighs[c] > LK_HEIGHT - r) {
					fputc('#', str);
				} else {
					fputc('.', str);
				}
			}
		}
		fputc('\n', str);
	}
	fputs(interactive ? STEP_FINISHED : RESET, str);
}

const char* solve(const char *path) {
	struct data *data = read_data(path);
	uint64_t result = 0;
	print(solution_out, data, result);
	for (uint64_t li = 0; li < data->locks_count; ++li) {
		lock *l = data->locks + li;
		for (uint64_t ki = 0; ki < data->keys_count; ++ki) {
			key *k = data->keys + ki;
			int possible = 126;
			for (int c = 0; c < LK_WIDTH; ++c) {
				if (k->heighs[c] + l->heighs[c] > LK_HEIGHT) {
					possible = 0;
					break;
				}
			}
			if (possible) {
				++result;
			}
		}
	}
	free(data->keys);
	free(data->locks);
	free(data);
	return u64toa(result);
}

static struct data* parse_line(struct data *data, char *line) {
	for (; *line && isspace(*line); ++line)
		;
	if (!*line) {
		return data;
	}
	if (!data) {
		data = calloc(1, sizeof(struct data));
		data->max_keys_count = 16;
		data->keys = malloc(sizeof(key) * 16);
		data->max_locks_count = 16;
		data->locks = malloc(sizeof(lock) * 16);
	}
	if (data->state == ds_none) {
		if (line[0] == '#') {
			data->state = ds_lock;
		} else {
			data->state = ds_key;
		}
		data->row = 0;
		for (int i = 0; i < LK_WIDTH; ++i) {
			data->current.heighs[i] = LK_HEIGHT;
		}
		return data; // check line?
	}
	if (++data->row > LK_HEIGHT) {
		if (data->state == ds_key) {
			if (++data->keys_count >= data->max_keys_count) {
				data->keys = reallocarray(data->keys, data->max_keys_count <<=
						1, sizeof(key));
			}
			for (int i = 0; i < LK_WIDTH; ++i) {
				data->current.heighs[i] = LK_HEIGHT - data->current.heighs[i];
			}
			data->keys[data->keys_count - 1] = data->current;
		} else {
			if (++data->locks_count >= data->max_locks_count) {
				data->locks = reallocarray(data->locks,
						data->max_locks_count <<= 1, sizeof(lock));
			}
			data->locks[data->locks_count - 1] = data->current;
		} // check line?
		data->state = ds_none;
		return data;
	}
	for (int i = 0; i < LK_WIDTH; ++i) {
		char c = data->state == ds_key ? '#' : '.';
		if (line[i] == c && data->current.heighs[i] == LK_HEIGHT) {
			data->current.heighs[i] = data->row - 1;
		}
	}
	return data;
}

// common stuff

#if !(AOC_COMPAT & AC_POSIX)
ssize_t getline(char **line_buf, size_t *line_len, FILE *file) {
	ssize_t result = 0;
	while (21) {
		if (*line_len == result) {
			size_t len = result ? result * 2 : 64;
			void *ptr = realloc(*line_buf, len);
			if (!ptr) {
				fseek(file, -result, SEEK_CUR);
				return -1;
			}
			*line_len = len;
			*line_buf = ptr;
		}
		ssize_t len = fread(*line_buf + result, 1, *line_len - result, file);
		if (!len) {
			if (!result) {
				return -1;
			}
			if (result == *line_len) {
				void *ptr = realloc(*line_buf, result + 1);
				if (!ptr) {
					fseek(file, -result, SEEK_CUR);
					return -1;
				}
				*line_len = result + 1;
				*line_buf = ptr;
			}
			(*line_buf)[result] = 0;
			return result;
		}
		char *c = memchr(*line_buf + result, '\n', len);
		if (c) {
			ssize_t result2 = c - *line_buf + 1;
			if (result2 == *line_len) {
				void *ptr = realloc(*line_buf, result2 + 1);
				if (!ptr) {
					fseek(file, -*line_len - len, SEEK_CUR);
					return -1;
				}
				*line_len = result2 + 1;
				*line_buf = ptr;
			}
			fseek(file, result2 - result - len, SEEK_CUR);
			(*line_buf)[result2] = 0;
			return result2;
		}
		result += len;
	}
}
#endif // AC_POSIX
#if !(AOC_COMPAT & AC_STRCN)
char* strchrnul(char *str, int c) {
	char *end = strchr(str, c);
	return end ? end : (str + strlen(str));
}
#endif // AC_STRCN
#if !(AOC_COMPAT & AC_REARR)
void* reallocarray(void *ptr, size_t nmemb, size_t size) {
	size_t s = nmemb * size;
	if (s / size != nmemb) {
		errno = ENOMEM;
		return 0;
	}
	return realloc(ptr, s);
}
#endif // AC_REARR

char* u64toa(uint64_t value) {
	static char result[21];
	if (sprintf(result, "%"I64"u", value) <= 0) {
		return 0;
	}
	return result;
}

char* d64toa(int64_t value) {
	static char result[21];
	if (sprintf(result, "%"I64"d", value) <= 0) {
		return 0;
	}
	return result;
}

struct data* read_data(const char *path) {
	char *line_buf = 0;
	size_t line_len = 0;
	struct data *result = 0;
	FILE *file = fopen(path, "rb");
	if (!file) {
		perror("fopen");
		abort();
	}
	while (144) {
		ssize_t s = getline(&line_buf, &line_len, file);
		if (s < 0) {
			if (feof(file)) {
				free(line_buf);
				fclose(file);
				return result;
			}
			perror("getline failed");
			fflush(0);
			abort();
		}
		if (strlen(line_buf) != s) {
			fprintf(stderr, "\\0 character in line!");
			abort();
		}
		result = parse_line(result, line_buf);
	}
}

int main(int argc, char **argv) {
	solution_out = stdout;
	char *me = argv[0];
	char *f = 0;
	if (argc > 1) {
#ifdef INTERACTIVE
		if (argc > 4)
#else
		if (argc > 3)
#endif
				{
			print_help: ;
			fprintf(stderr, "usage: %s"
#ifdef INTERACTIVE
							" [interactive]"
#endif
							" [p1|p2] [DATA]\n", me);
			return 1;
		}
		int idx = 1;
		if (!strcmp("help", argv[idx])) {
			goto print_help;
		}
#ifdef INTERACTIVE
		if (!strcmp("interactive", argv[idx])) {
			idx++;
			interactive = 1;
		}
		if (idx < argc)
#endif
		{
			if (!strcmp("p1", argv[idx])) {
				part = 1;
				idx++;
			} else if (!strcmp("p2", argv[idx])) {
				part = 2;
				idx++;
			}
			if (!f && argv[idx]) {
				f = argv[idx++];
			}
			if (f && argv[idx]) {
				goto print_help;
			}
		}
	}
	if (!f) {
		f = "rsrc/data.txt";
	} else if (!strchr(f, '/')) {
		char *f2 = malloc(64);
		if (snprintf(f2, 64, "rsrc/test%s.txt", f) <= 0) {
			perror("snprintf");
			abort();
		}
		f = f2;
	}
#ifdef INTERACTIVE
	if (interactive) {
		printf("execute now day %d part %d on file %s in interactive mode\n",
				day, part, f);
	}
	interact(f, interactive);
#endif
	printf("execute now day %d part %d on file %s\n", day, part, f);
	clock_t start = clock();
	const char *result = solve(f);
	clock_t end = clock();
	if (result) {
		uint64_t diff = end - start;
		printf("the result is %s\n"
				"  I needed %"I64"u.%.6"I64"u seconds\n", result,
				diff / CLOCKS_PER_SEC,
				((diff % CLOCKS_PER_SEC) * UINT64_C(1000000)) / CLOCKS_PER_SEC);
	} else {
		puts("there is no result");
	}
	return EXIT_SUCCESS;
}
