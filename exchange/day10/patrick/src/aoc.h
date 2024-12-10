/*
 * aoc.h
 *
 *  Created on: Dec 1, 2024
 *      Author: pat
 */

#ifndef SRC_AOC_H_
#define SRC_AOC_H_

#include <stdint.h>

#ifndef __unix__
ssize_t getline(char **line_buf, size_t *line_len, FILE *file);
char* strchrnul(char *str, char c);
void* reallocarray(void*ptr, size_t nmemb, size_t size);
#endif

extern int day;
extern int part;

char* u64toa(uint64_t);
char* d64toa(int64_t);

struct data;

static struct data* read_data(const char *path);

#endif /* SRC_AOC_H_ */
