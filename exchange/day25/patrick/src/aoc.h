/*
 * aoc.h
 *
 *  Created on: Dec 1, 2024
 *      Author: pat
 */

#ifndef SRC_AOC_H_
#define SRC_AOC_H_

#define AC_POSIX 1
#define AC_REARR 2
#define AC_STRCN 4
#define AC_GTRND 8
/* only set AOC_COMPAT if not set by the CFLAGS */
#ifndef AOC_COMPAT
#	if defined __gnu_linux__
#		define AOC_COMPAT (AC_POSIX | AC_REARR | AC_STRCN | AC_GTRND)
#	elif defined __FreeBSD__ || (defined __GNU__ || defined __GLIBC__)
#		define AOC_COMPAT (AC_POSIX | AC_REARR | AC_STRCN)
#	elif defined __OpenBSD__
#		define AOC_COMPAT (AC_POSIX | AC_REARR)
#	elif defined __NetBSD__
#		define AOC_COMPAT (AC_POSIX | AC_STRCN)
#	elif defined __linux__
#		define AOC_COMPAT (AC_POSIX | AC_GTRND)
#	elif defined __unix__ \
		|| (defined __bsdi__ || defined __DragonFly__) \
		|| (defined __APPLE__ && defined __MACH__)
#		define AOC_COMPAT (AC_POSIX | AC_PTHRD | AC_TERMS)
#	else
#		define AOC_COMPAT (0)
#	endif
#endif // AOC_COMPAT

#include <stdint.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>

#ifndef EWOULDBLOCK
#	define EWOULDBLOCK EAGAIN
#endif

#if UINT64_MAX == UCHAR_MAX
#	define I64 "hh"
#elif UINT64_MAX == USHRT_MAX
#	define I64 "h"
#elif UINT64_MAX == UINT_MAX
#	define I64 ""
#elif UINT64_MAX == ULONG_MAX
#	define I64 "l"
#elif UINT64_MAX == ULLONG_MAX
#	define I64 "ll"
#else
#	error "i don't know what the I64 type looks like! 37"
#endif

#define Z "z"
#ifdef _WIN32
#	ifndef __STDC_NO_THREADS__
#		define __STDC_NO_THREADS__
#	endif
#	undef I64
#	undef Z
#	define I64 "I64"
#	ifdef _WIN64
#		define Z I64
#	else
#		define Z "l"
#	endif
#endif // _WIN32

#if !(AOC_COMPAT & AC_POSIX)
#include <stddef.h>

ssize_t getline(char **line_buf, size_t *line_len, FILE *file);
#endif
#if !(AOC_COMPAT & AC_STRCN)
char* strchrnul(char *str, int c);
#endif
#if !(AOC_COMPAT & AC_REARR)
#include <stddef.h>

void* reallocarray(void*ptr, size_t nmemb, size_t size);
#endif

#ifdef INTERACTIVE

extern int day;
extern int part;
extern FILE *solution_out;
extern int interactive;
#endif

char* u64toa(uint64_t);
char* d64toa(int64_t);

struct data;

const char* solve(const char*file);

#endif /* SRC_AOC_H_ */
