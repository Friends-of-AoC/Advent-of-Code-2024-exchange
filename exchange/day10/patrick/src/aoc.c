/*
 * aoc.c
 *
 *  Created on: Dec 2, 2024
 *      Author: pat
 */

#include "aoc.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <search.h>

#define DAY 9
int part = 2;

typedef int num;
#define NUMF "%d"

struct pos {
	num x;
	num y;
};

struct data {
	char **lines;
	size_t line_count;
	size_t max_line_count;
	size_t line_length;
};

static void print(FILE *str, struct data *data, uint64_t result) {
	fprintf(str, "result=%s\n", u64toa(result));
	for (int l = 0; l < data->line_count; ++l) {
		fprintf(str, "%s\n", data->lines[l]);
	}
}

struct pos_list {
	struct pos pos;
	struct pos_list *next;
};

static uint64_t hiking_trails(struct data *data, num x, num y,
		struct pos_list **nines) {
	char cur = data->lines[y][x];
	if (cur == '9') {
		if (part == 1) {
			struct pos_list *pl = malloc(sizeof(struct pos_list));
			pl->pos.x = x;
			pl->pos.y = y;
			pl->next = *nines;
			*nines = pl;
			data->lines[y][x] = '.';
		}
		return 1;
	}
	char next = cur + 1;
	uint64_t result = 0;
	if (x && data->lines[y][x - 1] == next) {
		result += hiking_trails(data, x - 1, y, nines);
	}
	if (y && data->lines[y - 1][x] == next) {
		result += hiking_trails(data, x, y - 1, nines);
	}
	if (x + 1 < data->line_length && data->lines[y][x + 1] == next) {
		result += hiking_trails(data, x + 1, y, nines);
	}
	if (y + 1 < data->line_count && data->lines[y + 1][x] == next) {
		result += hiking_trails(data, x, y + 1, nines);
	}
	return result;
}

static char* solve(char *path) {
	struct data *data = read_data(path);
	uint64_t result = 0;
	for (num i = 0; i < data->line_count; ++i) {
		for (char *c = strchr(data->lines[i], '0'); c; c = strchr(c + 1, '0')) {
			struct pos_list *ninesl = 0;
			struct pos_list **nines = &ninesl;
			result += hiking_trails(data, c - data->lines[i], i, nines);
			while (ninesl) {
				data->lines[ninesl->pos.y][ninesl->pos.x] = '9';
				struct pos_list *nn = ninesl->next;
				free(ninesl);
				ninesl = nn;
			}
			print(stdout, data, result);
		}
	}
	print(stdout, data, result);
	return u64toa(result);
}

static struct data* parse_line(struct data *data, char *line) {
	for (; *line && isspace(*line); ++line)
		;
	if (!*line) {
		return data;
	}
	char *end = line + strlen(line);
	while (isspace(*--end))
		;
	end++;
	if (!data) {
		data = malloc(sizeof(struct data));
		data->lines = malloc(sizeof(char*) * 16);
		data->line_count = 0;
		data->max_line_count = 16;
		data->line_length = end - line;
	}
	if (data->line_length != end - line) {
		fprintf(stderr, "input is no rectangle!");
		abort();
	}
	if (++data->line_count >= data->max_line_count) {
		data->lines = reallocarray(data->lines, data->max_line_count <<= 1,
				sizeof(char*));
	}
	*end = '\0';
	data->lines[data->line_count - 1] = strdup(line);
	return data;
}

// common stuff

#ifndef __unix__
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
char* strchrnul(char *str, char c) {
	char *end = strchr(str, c);
	return end ? end : (str + strlen(str));
}
void* reallocarray(void*ptr, size_t nmemb, size_t size) {
	size_t s = nmemb * size;
	if (s / size != nmemb) {
		errno = EOVERFLOW;
		return 0;
	}
	return realloc(ptr, s);
}
#endif

char* u64toa(uint64_t value) {
	char *result = malloc(21);
	if (sprintf(result, "%llu", (unsigned long long) value) <= 0) {
		free(result);
		return 0;
	}
	return result;
}

char* d64toa(int64_t value) {
	char *result = malloc(21);
	if (sprintf(result, "%lld", (unsigned long long) value) <= 0) {
		free(result);
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
	char *me = argv[0];
	char *f = 0;
	if (argc > 1) {
		if (argc > 4) {
			fprintf(stderr, "usage: %s [p1|p2] [DATA]\n", me);
			return 1;
		}
		int idx = 1;
		if (!strcmp("help", argv[idx])) {
			fprintf(stderr, "usage: %s [p1|p2] [DATA]\n", me);
			return 1;
		} else if (!strcmp("p1", argv[idx])) {
			part = 1;
			f = argv[idx + 1] ? argv[idx + 1] : 0;
		} else if (!strcmp("p2", argv[idx])) {
			part = 2;
			f = argv[idx + 1] ? argv[idx + 1] : 0;
		} else if (argv[idx + 1]) {
			fprintf(stderr, "usage: %s [p1|p2] [DATA]\n", me);
			return 1;
		} else {
			f = argv[idx];
		}
	}
	parse_end: if (!f) {
		f = "rsrc/data.txt";
	} else if (!strchr(f, '/')) {
		char *f2 = malloc(64);
		if (snprintf(f2, 64, "rsrc/test%s.txt", f) <= 0) {
			perror("snprintf");
			abort();
		}
		f = f2;
	}
	printf("execute now day %d part %d on file %s\n", DAY, part, f);
	clock_t start = clock();
	char *result = solve(f);
	clock_t end = clock();
	if (result) {
		uint64_t diff = end - start;
		printf("the result is %s\n"
				"  I needed %lu.%.6lu seconds\n", result, diff / CLOCKS_PER_SEC,
				((diff % CLOCKS_PER_SEC) * 1000000LU) / CLOCKS_PER_SEC);
	} else {
		puts("there is no result");
	}
	return 0;
}
