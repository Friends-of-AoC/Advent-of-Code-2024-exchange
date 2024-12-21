// SPDX-License-Identifier: AGPL-3.0-or-later

/*
 * hash.h
 *
 *  Created on: Aug 14, 2024
 *      Author: pat
 */

#ifndef SRC_EXPORT_HASH_H_
#define SRC_EXPORT_HASH_H_

#include <stdint.h>
#include <stddef.h>

union hs_entry;

struct hashset {
	union hs_entry *data;
	size_t data_size;
	size_t entry_count;
	uint64_t (*hash)(const void*);
	int (*equal)(const void*, const void*);
	/* optional operation, called when an entry is removed from the set
	 * (hs_remove, hs_clear, hs_filter)
	 * if set the usage of hs_remove's return value may lead to UB */
	void (*free)(void*);
};

void* hs_get(struct hashset *hs, void *val);

void* hs_remove(struct hashset *hs, void *val);

void* hs_compute(struct hashset *hs, void *val, void *param,
		void* (*get_val)(void *param, void *old_val, void *new_val));

void* hs_compute_absent(struct hashset *hs, void *val, void *param,
		void* (*get_val)(void *param, void *val));

void* hs_set(struct hashset *hs, void *val);

//like set but does not overwrite existing values
void* hs_add(struct hashset *hs, void *val);

void hs_clear(struct hashset *hs);

int hs_for_each(struct hashset *hs, void *param,
		int (*visitor)(void *param, void *element));

void hs_filter(struct hashset *hs, void *param,
		int (*retain_element)(void *param, void *element));

#endif /* SRC_EXPORT_HASH_H_ */
