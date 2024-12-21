/*
 * aoc.c
 *
 *  Created on: Dec 2, 2024
 *      Author: pat
 */
#include "interactive.h"
#include "aoc.h"
#include "hash.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <search.h>
#include <string.h>

#ifdef INTERACTIVE
#define INTERACT(...) __VA_ARGS__
#else
#define INTERACT(...)
#endif

struct data* read_data(const char *path);

int day = 19;
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

struct data {
	char **codes;
	num codes_count;
	size_t max_codes_count;
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
	for (num y = 0; y < data->codes_count; ++y) {
		fprintf(str, "%s\n", data->codes[y]);
	}
	fputs(interactive ? STEP_FINISHED : RESET, str);
}

struct result {
	char *str;
	uint64_t above_count;
	uint64_t result;
};

static uint64_t r_h(const void *a) {
	const struct result *r = a;
	uint64_t result = r->above_count;
	for (const char *c = r->str; *c; ++c) {
		result = result * 31 + *c;
	}
	return result;
}

static int r_eq(const void *a, const void *b) {
	const struct result *r0 = a, *r1 = b;
	return r0->above_count == r1->above_count && !strcmp(r0->str, r1->str);
}

static void r_free(void *a) {
	free(((struct result*) a)->str);
	free(a);
}

static uint64_t calc_sortest(char *code, size_t above_count,
		struct hashset *cache) {
	struct result r0 = { .str = code, .above_count = above_count };
	struct result *r = hs_get(cache, &r0);
	if (r) {
		return r->result;
	}
	r = malloc(sizeof(struct result));
	r->str = strdup(code);
	r->above_count = above_count;
	struct pos A = { 2, (*code >= '0' && *code <= '9') ? 3 : 0 };
	struct pos hole = { 0, A.y };
	struct pos bot = A;
	uint64_t result = 0;
	for (; *code; ++code) {
		struct pos dst;
		switch (*code) {
		case 'A':
			dst = A;
			break;
		case '7':
			dst.x = 0;
			dst.y = 0;
			break;
		case '^':
		case '8':
			dst.x = 1;
			dst.y = 0;
			break;
		case '9':
			dst.x = 2;
			dst.y = 0;
			break;
		case '<':
		case '4':
			dst.x = 0;
			dst.y = 1;
			break;
		case 'v':
		case '5':
			dst.x = 1;
			dst.y = 1;
			break;
		case '>':
		case '6':
			dst.x = 2;
			dst.y = 1;
			break;
		case '1':
			dst.x = 0;
			dst.y = 2;
			break;
		case '2':
			dst.x = 1;
			dst.y = 2;
			break;
		case '3':
			dst.x = 2;
			dst.y = 2;
			break;
		case '0':
			dst.x = 1;
			dst.y = 3;
			break;
		default:
			exit(4);
		}
		uint64_t min_add = UINT64_MAX;
		num dx = dst.x - bot.x;
		num dy = dst.y - bot.y;
		if (dx < 0) {
			if (dst.x || (hole.y != dst.y && hole.y != bot.y)) {
				char myc = 'v';
				num ady = dy, adx = -dx;
				if (dy < 0) {
					myc = '^';
					ady = -dy;
				}
				size_t len = ady + adx;
				if (above_count > 1) {
					char buf[len + 2];
					memset(buf, '<', adx);
					memset(buf + adx, myc, ady);
					buf[len] = 'A';
					buf[len + 1] = 0;
					len = calc_sortest(buf, above_count - 1, cache);
				} else {
					++len;
				}
				if (len < min_add)
					min_add = len;
			}
		} else {
			char myc = 'v';
			num ady = dy, adx = dx;
			if (dy < 0) {
				myc = '^';
				ady = -dy;
			}
			size_t len = ady + adx;
			if (above_count > 1) {
				char buf[len + 2];
				memset(buf, '>', adx);
				memset(buf + adx, myc, ady);
				buf[len] = 'A';
				buf[len + 1] = 0;
				len = calc_sortest(buf, above_count - 1, cache);
			} else {
				++len;
			}
			if (len < min_add)
				min_add = len;
		}
		if ((bot.x || dst.y != hole.y)) {
			char myc = 'v', mxc = '>';
			num ady = dy, adx = dx;
			if (dy < 0) {
				myc = '^';
				ady = -dy;
			}
			if (dx < 0) {
				mxc = '<';
				adx = -dx;
			}
			size_t len = ady + adx;
			if (above_count > 1) {
				char buf[len + 2];
				memset(buf, myc, ady);
				memset(buf + ady, mxc, adx);
				buf[len] = 'A';
				buf[len + 1] = 0;
				len = calc_sortest(buf, above_count - 1, cache);
			} else {
				++len;
			}
			if (len < min_add)
				min_add = len;
		}
		result += min_add;
		bot = dst;
	}
	r->result = result;
	hs_set(cache, r);
	return result;
}

const char* solve(const char *path) {
	struct data *data = read_data(path);
	uint64_t result = 0;
	print(solution_out, data, result);
	struct hashset hs = { .hash = r_h, .equal = r_eq, .free = r_free };
	for (int i = 0; i < data->codes_count; ++i) {
		char *code = data->codes[i];
		char *end;
		uint64_t val = strtol(code, &end, 10);
		if (errno || end[0] != 'A' || end[1]) {
			fprintf(stderr, "invalid code %s\n", code);
			exit(2);
		}
		uint64_t len = calc_sortest(code, part == 1 ? 3 : 26, &hs);
		printf("the code is %s\n"
				"the add result %"I64"u = %"I64"u * %"I64"u\n", code, len * val,
				len, val);
		result += len * val;
	}
	free(data->codes);
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
		data = malloc(sizeof(struct data));
		data->codes = malloc(sizeof(char*) * 16);
		data->codes_count = 0;
		data->max_codes_count = 16;
	}
	char *end = line + strlen(line);
	while (isspace(*--end))
		;
	*++end = '\0';
	if (++data->codes_count > data->max_codes_count) {
		data->codes = reallocarray(data->codes, data->max_codes_count <<= 1,
				sizeof(char*));
	}
	data->codes[data->codes_count - 1] = strdup(line);
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
