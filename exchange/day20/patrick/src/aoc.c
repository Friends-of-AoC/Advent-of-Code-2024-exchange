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
	char **lines;
	num line_length;
	num line_count;
	size_t max_line_count;
	struct pos start;
};

static int do_print = 0;

static void print(FILE *str, struct data *data, uint64_t result,
		struct hashset *used_cheats, num lastx, num lasty) {
	if (result) {
		fprintf(str, "%sresult=%"I64"u\n%s", STEP_HEADER, result, STEP_BODY);
	} else {
		fputs(STEP_BODY, str);
	}
	if (!do_print && !interactive) {
		return;
	}
	struct pos p;
	for (p.y = 0; p.y < data->line_count; ++p.y) {
		char last = 0, *line = data->lines[p.y];
		for (p.x = 0; p.x < data->line_length; ++p.x) {
			if (p.y == lasty && p.x == lastx) {
				fputs(CSI C_DEF_INTENSE C_SEP //
						C_FC_PREFIX C_CYAN C_END "#", str);
				last = 0;
				continue;
			}
			char c = line[p.x];
			if (hs_get(used_cheats, &p)) {
				fprintf(str, CSI C_DEF_INTENSE C_SEP //
						C_FC_PREFIX C_RED C_END "%c", c);
				last = 0;
				continue;
			}
			if (c == last) {
				fputc(c, str);
			} else if (c == '#') {
				fprintf(str, CSI C_LESS_INTENSE C_SEP //
						C_FC_PREFIX C_RGB_GRAY C_END "%c", c);
			} else {
				fprintf(str, CSI C_DEF_INTENSE C_SEP C_FC_DEF C_END "%c", c);
			}
			last = c;
		}
		fputc('\n', str);
	}
	fputs(interactive ? STEP_FINISHED : RESET, str);
}

static void find_free_pos(struct data *data, struct pos *p, int set_dir) {
	if (data->lines[p->y][p->x - 1] == '.'
			|| data->lines[p->y][p->x - 1] == 'E') {
		if (set_dir)
			data->lines[p->y][p->x] = '<';
		else
			exit(4);
		--p->x;
	} else if (data->lines[p->y][p->x + 1] == '.'
			|| data->lines[p->y][p->x + 1] == 'E') {
		if (set_dir)
			data->lines[p->y][p->x] = '>';
		else
			exit(4);
		++p->x;
	} else if (data->lines[p->y - 1][p->x] == '.'
			|| data->lines[p->y - 1][p->x] == 'E') {
		if (set_dir)
			data->lines[p->y][p->x] = '^';
		else
			exit(4);
		--p->y;
	} else if (data->lines[p->y + 1][p->x] == '.'
			|| data->lines[p->y + 1][p->x] == 'E') {
		if (set_dir)
			data->lines[p->y][p->x] = 'v';
		else
			exit(4);
		++p->y;
	} else {
		fprintf(stderr, "invalid position %"I64"d,%"I64"u\n", p->x, p->y);
		exit(3);
	}
}

static num calc_turns(struct data *data, num startx, num starty) {
	struct pos p = { startx, starty };
	for (num cnt = 0; 77; ++cnt) {
		switch (data->lines[p.y][p.x]) {
		case 'E':
			return cnt;
		case 'S':
		case '.':
			find_free_pos(data, &p, 109);
			break;
		case '>':
			if (data->lines[p.y][p.x + 1] != '#') {
				++p.x;
			} else {
				find_free_pos(data, &p, 0);
			}
			break;
		case '<':
			if (data->lines[p.y][p.x - 1] != '#') {
				--p.x;
			} else {
				find_free_pos(data, &p, 0);
			}
			break;
		case '^':
			if (data->lines[p.y - 1][p.x] != '#') {
				--p.y;
			} else {
				find_free_pos(data, &p, 0);
			}
			break;
		case 'v':
			if (data->lines[p.y + 1][p.x] != '#') {
				++p.y;
			} else {
				find_free_pos(data, &p, 0);
			}
			break;
		default:
			fprintf(stderr, "invalid char %c\n", data->lines[p.y][p.x]);
			exit(3);
		}
	}
}

const num min_saved = 1;

static num cheat(struct data *data, num cheat_turns, num srcx, num srcy,
		num dstx, num dsty, struct hashset *used_cheats) {
	char old = data->lines[dsty][dstx];
	data->lines[dsty][dstx] = 'E';
	uint64_t saved = calc_turns(data, srcx, srcy) - cheat_turns;
	data->lines[dsty][dstx] = old;
	if (!interactive) {
		printf("saved %"I64"u turns\n", saved);
	} else if (saved >= min_saved) {
		struct pos *p = malloc(sizeof(struct pos));
		p->x = dstx;
		p->y = dsty;
		if (hs_set(used_cheats, p)) {
			fprintf(stderr, "same cheat used multiple times!\n");
			exit(4);
		}
	}
	return saved;
}

static uint64_t p_h(const void *a) {
	const struct pos *p = a;
	return p->x * 31 + p->y;
}
static int p_eq(const void *a, const void *b) {
	const struct pos *p0 = a, *p1 = b;
	return p0->x == p1->x && p0->y == p1->y;
}

static void recursive_cheat(struct data *data, struct hashset *cheats,
		num startx, num starty, num x, num y, uint64_t *counter,
		struct hashset *used_cheats) {
	if (x < 0 || y < 0 || x >= data->line_length || y >= data->line_count) {
		return;
	}
	struct pos *p = malloc(sizeof(struct pos));
	p->x = x;
	p->y = y;
	if (hs_set(cheats, p)) {
		return;
	}
	uint64_t turns = 0;
	turns += startx > x ? startx - x : x - startx;
	turns += starty > y ? starty - y : y - starty;
	if (turns < 20) {
		recursive_cheat(data, cheats, startx, starty, x + 1, y, counter,
				used_cheats);
		recursive_cheat(data, cheats, startx, starty, x - 1, y, counter,
				used_cheats);
		recursive_cheat(data, cheats, startx, starty, x, y + 1, counter,
				used_cheats);
		recursive_cheat(data, cheats, startx, starty, x, y - 1, counter,
				used_cheats);
	}
	if (data->lines[y][x] != '#') {
		if (cheat(data, turns, startx, starty, x, y, used_cheats)
				>= min_saved) {
			++*counter;
		}
	}
}

const char* solve(const char *path) {
	struct data *data = read_data(path);
	if (data->start.x < 0) {
		fprintf(stderr, "no start location!\n");
		exit(2);
	}
	uint64_t result = 0;
	struct hashset cheats = { .hash = p_h, .equal = p_eq, .free = free };
	struct hashset used_cheats = { .hash = p_h, .equal = p_eq, .free = free };
	print(solution_out, data, result, &used_cheats, -1, -1);
	num x = data->start.x, y = data->start.y;
	uint64_t turns = calc_turns(data, x, y);
	int cont = 160;
	num lastx = -1, lasty = -1;
	while (cont) {
		print(solution_out, data, result, &used_cheats, lastx, lasty);
		hs_clear(&used_cheats);
		char c = data->lines[y][x];
		if (part == 1) {
			if (y >= 2 && data->lines[y - 2][x] != '#'
					&& data->lines[y - 1][x] == '#'
					&& cheat(data, 2, x, y, x, y - 2, &used_cheats)
							>= min_saved) {
				result++;
			}
			if (y + 2 < data->line_count && data->lines[y + 2][x] != '#'
					&& data->lines[y + 1][x] == '#'
					&& cheat(data, 2, x, y, x, y + 2, &used_cheats)
							>= min_saved) {
				result++;
			}
			if (x >= 2 && data->lines[y][x - 2] != '#'
					&& data->lines[y][x - 1] == '#'
					&& cheat(data, 2, x, y, x - 2, y, &used_cheats)
							>= min_saved) {
				result++;
			}
			if (x + 2 < data->line_length && data->lines[y][x + 2] != '#'
					&& data->lines[y][x + 1] == '#'
					&& cheat(data, 2, x, y, x + 2, y, &used_cheats)
							>= min_saved) {
				result++;
			}
		} else {
			struct pos *p = malloc(sizeof(struct pos));
			hs_set(&cheats, p);
			recursive_cheat(data, &cheats, x, y, x + 1, y, &result,
					&used_cheats);
			recursive_cheat(data, &cheats, x, y, x - 1, y, &result,
					&used_cheats);
			recursive_cheat(data, &cheats, x, y, x, y + 1, &result,
					&used_cheats);
			recursive_cheat(data, &cheats, x, y, x, y - 1, &result,
					&used_cheats);
			hs_clear(&cheats);
		}
		data->lines[y][x] = '#';
		lastx = x;
		lasty = y;
		switch (c) {
		case '>':
			++x;
			break;
		case '<':
			--x;
			break;
		case '^':
			--y;
			break;
		case 'v':
			++y;
			break;
		case 'E':
			cont = 0;
			break;
		default:
			fprintf(stderr, "invalid char %c\n", data->lines[y][x]);
		}
	}
	print(solution_out, data, result, &used_cheats, lastx, lasty);
	hs_clear(&used_cheats);
	for (num l = 0; l < data->line_count; ++l) {
		free(data->lines[l]);
	}
	free(data->lines);
	free(data);
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
	*end = '\0';
	if (!data) {
		data = malloc(sizeof(struct data));
		data->lines = malloc(sizeof(char*) * 16);
		data->line_count = 0;
		data->max_line_count = 16;
		data->line_length = end - line;
		data->start.x = -1;
		data->start.y = -1;
	}
	if (data->line_length != end - line) {
		fprintf(stderr, "world is no recrangle!\n");
		exit(2);
	}
	if (*line != '#' || end[-1] != '#') {
		fprintf(stderr, "world is no invalid: %s\n", line);
		exit(2);
	}
	if (++data->line_count > data->max_line_count) {
		data->lines = reallocarray(data->lines, data->max_line_count <<= 1,
				sizeof(char*));
	}
	data->lines[data->line_count - 1] = strdup(line);
	char *start = strchr(line, 'S');
	if (start) {
		if (data->start.x >= 0) {
			fprintf(stderr, "multiple start locations!\n");
			exit(2);
		}
		data->start.x = start - line;
		data->start.y = data->line_count - 1;
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
