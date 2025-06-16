/* table.h -- https://github.com/takeiteasy/table.h

 table for C

 Copyright (C) 2024  George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.

 This library is based off https://github.com/billziss-gh/imap/
 Copyright (c) 2023 Bill Zissimopoulos. All rights reserved. */

#ifndef TABLE_HEADER
#define TABLE_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#if !defined(TABLE_ALLOC)
#define TABLE_ALLOC malloc
#endif
#if !defined(TABLE_FREE)
#define TABLE_FREE free
#endif

#ifndef TABLE_INITIAL_CAPACITY
#define TABLE_INITIAL_CAPACITY 8
#endif

typedef struct imap_node_t imap_node_t;

typedef struct unordered_map_pair {
    uint64_t key, *val;
} unordered_map_pair_t;

typedef uint64_t(*table_hash)(const void *data, size_t len, uint32_t seed);
typedef int(*unordered_map_iter_cb)(unordered_map_pair_t *pair, void *userdata);

typedef struct int_map {
    imap_node_t *tree;
    size_t count, capacity;
} unordered_map_t;

unordered_map_t unordered_map_new(void);
unordered_map_t unordered_map_make(uint32_t capacity);
void unordered_map_free(unordered_map_t *map);
int unordered_map_set(unordered_map_t *map, uint64_t key, uint64_t val);
int unordered_map_get(unordered_map_t *map, uint64_t key, uint64_t *val);
int unordered_map_has(unordered_map_t *map, uint64_t key);
int unordered_map_del(unordered_map_t *map, uint64_t key);
void unordered_map_foreach(unordered_map_t *map, unordered_map_iter_cb callback, void *userdata);

typedef struct table {
    unordered_map_t kmap, vmap;
    table_hash hashfn;
    uint64_t seed;
} table_t;

typedef struct table_pair {
    union {
        const char *string;
        uint64_t integer;
    } key;
    uint64_t value;
} table_pair_t;

table_t table_new(void);
table_t table_make(table_hash hashfn, uint32_t capacity, uint64_t seed);
void table_free(table_t *table);
void table_each(table_t *table, int(*callback)(table_pair_t *pair, void*), void *userdata);
void table_find(table_t *table, const char *wildcard, int(*callback)(const char*, void*, void*), void *userdata);
bool table_search(table_t *table, const char *wildcard);

// BEGIN HEADER

#define table_get(TABLE, KEY, VALUE) \
	_Generic((KEY), \
		char*: _Generic((VALUE), \
			char*: __table_get_c_s8, \
			short*: __table_get_c_s16, \
			int*: __table_get_c_s32, \
			long*: __table_get_c_sl32, \
			long long*: __table_get_c_s64, \
			unsigned char*: __table_get_c_u8, \
			unsigned short*: __table_get_c_u16, \
			unsigned int*: __table_get_c_u32, \
			unsigned long*: __table_get_c_ul32, \
			unsigned long long*: __table_get_c_u64, \
			void**: __table_get_c_v), \
		const char*: _Generic((VALUE), \
			char*: __table_get_cc_s8, \
			short*: __table_get_cc_s16, \
			int*: __table_get_cc_s32, \
			long*: __table_get_cc_sl32, \
			long long*: __table_get_cc_s64, \
			unsigned char*: __table_get_cc_u8, \
			unsigned short*: __table_get_cc_u16, \
			unsigned int*: __table_get_cc_u32, \
			unsigned long*: __table_get_cc_ul32, \
			unsigned long long*: __table_get_cc_u64, \
			void**: __table_get_cc_v), \
		unsigned char*: _Generic((VALUE), \
			char*: __table_get_uc_s8, \
			short*: __table_get_uc_s16, \
			int*: __table_get_uc_s32, \
			long*: __table_get_uc_sl32, \
			long long*: __table_get_uc_s64, \
			unsigned char*: __table_get_uc_u8, \
			unsigned short*: __table_get_uc_u16, \
			unsigned int*: __table_get_uc_u32, \
			unsigned long*: __table_get_uc_ul32, \
			unsigned long long*: __table_get_uc_u64, \
			void**: __table_get_uc_v), \
		const unsigned char*: _Generic((VALUE), \
			char*: __table_get_cuc_s8, \
			short*: __table_get_cuc_s16, \
			int*: __table_get_cuc_s32, \
			long*: __table_get_cuc_sl32, \
			long long*: __table_get_cuc_s64, \
			unsigned char*: __table_get_cuc_u8, \
			unsigned short*: __table_get_cuc_u16, \
			unsigned int*: __table_get_cuc_u32, \
			unsigned long*: __table_get_cuc_ul32, \
			unsigned long long*: __table_get_cuc_u64, \
			void**: __table_get_cuc_v), \
		char: _Generic((VALUE), \
			char*: __table_get_s8_s8, \
			short*: __table_get_s8_s16, \
			int*: __table_get_s8_s32, \
			long*: __table_get_s8_sl32, \
			long long*: __table_get_s8_s64, \
			unsigned char*: __table_get_s8_u8, \
			unsigned short*: __table_get_s8_u16, \
			unsigned int*: __table_get_s8_u32, \
			unsigned long*: __table_get_s8_ul32, \
			unsigned long long*: __table_get_s8_u64, \
			void**: __table_get_s8_v), \
		short: _Generic((VALUE), \
			char*: __table_get_s16_s8, \
			short*: __table_get_s16_s16, \
			int*: __table_get_s16_s32, \
			long*: __table_get_s16_sl32, \
			long long*: __table_get_s16_s64, \
			unsigned char*: __table_get_s16_u8, \
			unsigned short*: __table_get_s16_u16, \
			unsigned int*: __table_get_s16_u32, \
			unsigned long*: __table_get_s16_ul32, \
			unsigned long long*: __table_get_s16_u64, \
			void**: __table_get_s16_v), \
		int: _Generic((VALUE), \
			char*: __table_get_s32_s8, \
			short*: __table_get_s32_s16, \
			int*: __table_get_s32_s32, \
			long*: __table_get_s32_sl32, \
			long long*: __table_get_s32_s64, \
			unsigned char*: __table_get_s32_u8, \
			unsigned short*: __table_get_s32_u16, \
			unsigned int*: __table_get_s32_u32, \
			unsigned long*: __table_get_s32_ul32, \
			unsigned long long*: __table_get_s32_u64, \
			void**: __table_get_s32_v), \
		long: _Generic((VALUE), \
			char*: __table_get_sl32_s8, \
			short*: __table_get_sl32_s16, \
			int*: __table_get_sl32_s32, \
			long*: __table_get_sl32_sl32, \
			long long*: __table_get_sl32_s64, \
			unsigned char*: __table_get_sl32_u8, \
			unsigned short*: __table_get_sl32_u16, \
			unsigned int*: __table_get_sl32_u32, \
			unsigned long*: __table_get_sl32_ul32, \
			unsigned long long*: __table_get_sl32_u64, \
			void**: __table_get_sl32_v), \
		long long: _Generic((VALUE), \
			char*: __table_get_s64_s8, \
			short*: __table_get_s64_s16, \
			int*: __table_get_s64_s32, \
			long*: __table_get_s64_sl32, \
			long long*: __table_get_s64_s64, \
			unsigned char*: __table_get_s64_u8, \
			unsigned short*: __table_get_s64_u16, \
			unsigned int*: __table_get_s64_u32, \
			unsigned long*: __table_get_s64_ul32, \
			unsigned long long*: __table_get_s64_u64, \
			void**: __table_get_s64_v), \
		unsigned char: _Generic((VALUE), \
			char*: __table_get_u8_s8, \
			short*: __table_get_u8_s16, \
			int*: __table_get_u8_s32, \
			long*: __table_get_u8_sl32, \
			long long*: __table_get_u8_s64, \
			unsigned char*: __table_get_u8_u8, \
			unsigned short*: __table_get_u8_u16, \
			unsigned int*: __table_get_u8_u32, \
			unsigned long*: __table_get_u8_ul32, \
			unsigned long long*: __table_get_u8_u64, \
			void**: __table_get_u8_v), \
		unsigned short: _Generic((VALUE), \
			char*: __table_get_u16_s8, \
			short*: __table_get_u16_s16, \
			int*: __table_get_u16_s32, \
			long*: __table_get_u16_sl32, \
			long long*: __table_get_u16_s64, \
			unsigned char*: __table_get_u16_u8, \
			unsigned short*: __table_get_u16_u16, \
			unsigned int*: __table_get_u16_u32, \
			unsigned long*: __table_get_u16_ul32, \
			unsigned long long*: __table_get_u16_u64, \
			void**: __table_get_u16_v), \
		unsigned int: _Generic((VALUE), \
			char*: __table_get_u32_s8, \
			short*: __table_get_u32_s16, \
			int*: __table_get_u32_s32, \
			long*: __table_get_u32_sl32, \
			long long*: __table_get_u32_s64, \
			unsigned char*: __table_get_u32_u8, \
			unsigned short*: __table_get_u32_u16, \
			unsigned int*: __table_get_u32_u32, \
			unsigned long*: __table_get_u32_ul32, \
			unsigned long long*: __table_get_u32_u64, \
			void**: __table_get_u32_v), \
		unsigned long: _Generic((VALUE), \
			char*: __table_get_ul32_s8, \
			short*: __table_get_ul32_s16, \
			int*: __table_get_ul32_s32, \
			long*: __table_get_ul32_sl32, \
			long long*: __table_get_ul32_s64, \
			unsigned char*: __table_get_ul32_u8, \
			unsigned short*: __table_get_ul32_u16, \
			unsigned int*: __table_get_ul32_u32, \
			unsigned long*: __table_get_ul32_ul32, \
			unsigned long long*: __table_get_ul32_u64, \
			void**: __table_get_ul32_v), \
		unsigned long long: _Generic((VALUE), \
			char*: __table_get_u64_s8, \
			short*: __table_get_u64_s16, \
			int*: __table_get_u64_s32, \
			long*: __table_get_u64_sl32, \
			long long*: __table_get_u64_s64, \
			unsigned char*: __table_get_u64_u8, \
			unsigned short*: __table_get_u64_u16, \
			unsigned int*: __table_get_u64_u32, \
			unsigned long*: __table_get_u64_ul32, \
			unsigned long long*: __table_get_u64_u64, \
			void**: __table_get_u64_v), \
		void*: _Generic((VALUE), \
			char*: __table_get_v_s8, \
			short*: __table_get_v_s16, \
			int*: __table_get_v_s32, \
			long*: __table_get_v_sl32, \
			long long*: __table_get_v_s64, \
			unsigned char*: __table_get_v_u8, \
			unsigned short*: __table_get_v_u16, \
			unsigned int*: __table_get_v_u32, \
			unsigned long*: __table_get_v_ul32, \
			unsigned long long*: __table_get_v_u64, \
			void**: __table_get_v_v) \
	)(TABLE,KEY,VALUE)

#define table_set(TABLE, KEY, VALUE) \
	_Generic((KEY), \
		char*: _Generic((VALUE), \
			char: __table_set_c_s8, \
			short: __table_set_c_s16, \
			int: __table_set_c_s32, \
			long: __table_set_c_sl32, \
			long long: __table_set_c_s64, \
			unsigned char: __table_set_c_u8, \
			unsigned short: __table_set_c_u16, \
			unsigned int: __table_set_c_u32, \
			unsigned long: __table_set_c_ul32, \
			unsigned long long: __table_set_c_u64, \
			void*: __table_set_c_v), \
		const char*: _Generic((VALUE), \
			char: __table_set_cc_s8, \
			short: __table_set_cc_s16, \
			int: __table_set_cc_s32, \
			long: __table_set_cc_sl32, \
			long long: __table_set_cc_s64, \
			unsigned char: __table_set_cc_u8, \
			unsigned short: __table_set_cc_u16, \
			unsigned int: __table_set_cc_u32, \
			unsigned long: __table_set_cc_ul32, \
			unsigned long long: __table_set_cc_u64, \
			void*: __table_set_cc_v), \
		unsigned char*: _Generic((VALUE), \
			char: __table_set_uc_s8, \
			short: __table_set_uc_s16, \
			int: __table_set_uc_s32, \
			long: __table_set_uc_sl32, \
			long long: __table_set_uc_s64, \
			unsigned char: __table_set_uc_u8, \
			unsigned short: __table_set_uc_u16, \
			unsigned int: __table_set_uc_u32, \
			unsigned long: __table_set_uc_ul32, \
			unsigned long long: __table_set_uc_u64, \
			void*: __table_set_uc_v), \
		const unsigned char*: _Generic((VALUE), \
			char: __table_set_cuc_s8, \
			short: __table_set_cuc_s16, \
			int: __table_set_cuc_s32, \
			long: __table_set_cuc_sl32, \
			long long: __table_set_cuc_s64, \
			unsigned char: __table_set_cuc_u8, \
			unsigned short: __table_set_cuc_u16, \
			unsigned int: __table_set_cuc_u32, \
			unsigned long: __table_set_cuc_ul32, \
			unsigned long long: __table_set_cuc_u64, \
			void*: __table_set_cuc_v), \
		char: _Generic((VALUE), \
			char: __table_set_s8_s8, \
			short: __table_set_s8_s16, \
			int: __table_set_s8_s32, \
			long: __table_set_s8_sl32, \
			long long: __table_set_s8_s64, \
			unsigned char: __table_set_s8_u8, \
			unsigned short: __table_set_s8_u16, \
			unsigned int: __table_set_s8_u32, \
			unsigned long: __table_set_s8_ul32, \
			unsigned long long: __table_set_s8_u64, \
			void*: __table_set_s8_v), \
		short: _Generic((VALUE), \
			char: __table_set_s16_s8, \
			short: __table_set_s16_s16, \
			int: __table_set_s16_s32, \
			long: __table_set_s16_sl32, \
			long long: __table_set_s16_s64, \
			unsigned char: __table_set_s16_u8, \
			unsigned short: __table_set_s16_u16, \
			unsigned int: __table_set_s16_u32, \
			unsigned long: __table_set_s16_ul32, \
			unsigned long long: __table_set_s16_u64, \
			void*: __table_set_s16_v), \
		int: _Generic((VALUE), \
			char: __table_set_s32_s8, \
			short: __table_set_s32_s16, \
			int: __table_set_s32_s32, \
			long: __table_set_s32_sl32, \
			long long: __table_set_s32_s64, \
			unsigned char: __table_set_s32_u8, \
			unsigned short: __table_set_s32_u16, \
			unsigned int: __table_set_s32_u32, \
			unsigned long: __table_set_s32_ul32, \
			unsigned long long: __table_set_s32_u64, \
			void*: __table_set_s32_v), \
		long: _Generic((VALUE), \
			char: __table_set_sl32_s8, \
			short: __table_set_sl32_s16, \
			int: __table_set_sl32_s32, \
			long: __table_set_sl32_sl32, \
			long long: __table_set_sl32_s64, \
			unsigned char: __table_set_sl32_u8, \
			unsigned short: __table_set_sl32_u16, \
			unsigned int: __table_set_sl32_u32, \
			unsigned long: __table_set_sl32_ul32, \
			unsigned long long: __table_set_sl32_u64, \
			void*: __table_set_sl32_v), \
		long long: _Generic((VALUE), \
			char: __table_set_s64_s8, \
			short: __table_set_s64_s16, \
			int: __table_set_s64_s32, \
			long: __table_set_s64_sl32, \
			long long: __table_set_s64_s64, \
			unsigned char: __table_set_s64_u8, \
			unsigned short: __table_set_s64_u16, \
			unsigned int: __table_set_s64_u32, \
			unsigned long: __table_set_s64_ul32, \
			unsigned long long: __table_set_s64_u64, \
			void*: __table_set_s64_v), \
		unsigned char: _Generic((VALUE), \
			char: __table_set_u8_s8, \
			short: __table_set_u8_s16, \
			int: __table_set_u8_s32, \
			long: __table_set_u8_sl32, \
			long long: __table_set_u8_s64, \
			unsigned char: __table_set_u8_u8, \
			unsigned short: __table_set_u8_u16, \
			unsigned int: __table_set_u8_u32, \
			unsigned long: __table_set_u8_ul32, \
			unsigned long long: __table_set_u8_u64, \
			void*: __table_set_u8_v), \
		unsigned short: _Generic((VALUE), \
			char: __table_set_u16_s8, \
			short: __table_set_u16_s16, \
			int: __table_set_u16_s32, \
			long: __table_set_u16_sl32, \
			long long: __table_set_u16_s64, \
			unsigned char: __table_set_u16_u8, \
			unsigned short: __table_set_u16_u16, \
			unsigned int: __table_set_u16_u32, \
			unsigned long: __table_set_u16_ul32, \
			unsigned long long: __table_set_u16_u64, \
			void*: __table_set_u16_v), \
		unsigned int: _Generic((VALUE), \
			char: __table_set_u32_s8, \
			short: __table_set_u32_s16, \
			int: __table_set_u32_s32, \
			long: __table_set_u32_sl32, \
			long long: __table_set_u32_s64, \
			unsigned char: __table_set_u32_u8, \
			unsigned short: __table_set_u32_u16, \
			unsigned int: __table_set_u32_u32, \
			unsigned long: __table_set_u32_ul32, \
			unsigned long long: __table_set_u32_u64, \
			void*: __table_set_u32_v), \
		unsigned long: _Generic((VALUE), \
			char: __table_set_ul32_s8, \
			short: __table_set_ul32_s16, \
			int: __table_set_ul32_s32, \
			long: __table_set_ul32_sl32, \
			long long: __table_set_ul32_s64, \
			unsigned char: __table_set_ul32_u8, \
			unsigned short: __table_set_ul32_u16, \
			unsigned int: __table_set_ul32_u32, \
			unsigned long: __table_set_ul32_ul32, \
			unsigned long long: __table_set_ul32_u64, \
			void*: __table_set_ul32_v), \
		unsigned long long: _Generic((VALUE), \
			char: __table_set_u64_s8, \
			short: __table_set_u64_s16, \
			int: __table_set_u64_s32, \
			long: __table_set_u64_sl32, \
			long long: __table_set_u64_s64, \
			unsigned char: __table_set_u64_u8, \
			unsigned short: __table_set_u64_u16, \
			unsigned int: __table_set_u64_u32, \
			unsigned long: __table_set_u64_ul32, \
			unsigned long long: __table_set_u64_u64, \
			void*: __table_set_u64_v), \
		void*: _Generic((VALUE), \
			char: __table_set_v_s8, \
			short: __table_set_v_s16, \
			int: __table_set_v_s32, \
			long: __table_set_v_sl32, \
			long long: __table_set_v_s64, \
			unsigned char: __table_set_v_u8, \
			unsigned short: __table_set_v_u16, \
			unsigned int: __table_set_v_u32, \
			unsigned long: __table_set_v_ul32, \
			unsigned long long: __table_set_v_u64, \
			void*: __table_set_v_v) \
	)(TABLE,KEY,VALUE)

#define table_has(TABLE, KEY) \
	_Generic((KEY), \
		char*: __table_has_c, \
		const char*: __table_has_cc, \
		unsigned char*: __table_has_uc, \
		const unsigned char*: __table_has_cuc, \
		char: __table_has_s8, \
		short: __table_has_s16, \
		int: __table_has_s32, \
		long: __table_has_sl32, \
		long long: __table_has_s64, \
		unsigned char: __table_has_u8, \
		unsigned short: __table_has_u16, \
		unsigned int: __table_has_u32, \
		unsigned long: __table_has_ul32, \
		unsigned long long: __table_has_u64, \
		void*: __table_has_v) \
	(TABLE,KEY)

#define table_del(TABLE, KEY) \
	_Generic((KEY), \
		char*: __table_del_c, \
		const char*: __table_del_cc, \
		unsigned char*: __table_del_uc, \
		const unsigned char*: __table_del_cuc, \
		char: __table_del_s8, \
		short: __table_del_s16, \
		int: __table_del_s32, \
		long: __table_del_sl32, \
		long long: __table_del_s64, \
		unsigned char: __table_del_u8, \
		unsigned short: __table_del_u16, \
		unsigned int: __table_del_u32, \
		unsigned long: __table_del_ul32, \
		unsigned long long: __table_del_u64, \
		void*: __table_del_v) \
	(TABLE,KEY)

int __table_get_c_s8(table_t *table, char* key, char* val);
int __table_get_c_s16(table_t *table, char* key, short* val);
int __table_get_c_s32(table_t *table, char* key, int* val);
int __table_get_c_sl32(table_t *table, char* key, long* val);
int __table_get_c_s64(table_t *table, char* key, long long* val);
int __table_get_c_u8(table_t *table, char* key, unsigned char* val);
int __table_get_c_u16(table_t *table, char* key, unsigned short* val);
int __table_get_c_u32(table_t *table, char* key, unsigned int* val);
int __table_get_c_ul32(table_t *table, char* key, unsigned long* val);
int __table_get_c_u64(table_t *table, char* key, unsigned long long* val);
int __table_get_c_v(table_t *table, char* key, void** val);
int __table_get_cc_s8(table_t *table, const char* key, char* val);
int __table_get_cc_s16(table_t *table, const char* key, short* val);
int __table_get_cc_s32(table_t *table, const char* key, int* val);
int __table_get_cc_sl32(table_t *table, const char* key, long* val);
int __table_get_cc_s64(table_t *table, const char* key, long long* val);
int __table_get_cc_u8(table_t *table, const char* key, unsigned char* val);
int __table_get_cc_u16(table_t *table, const char* key, unsigned short* val);
int __table_get_cc_u32(table_t *table, const char* key, unsigned int* val);
int __table_get_cc_ul32(table_t *table, const char* key, unsigned long* val);
int __table_get_cc_u64(table_t *table, const char* key, unsigned long long* val);
int __table_get_cc_v(table_t *table, const char* key, void** val);
int __table_get_uc_s8(table_t *table, unsigned char* key, char* val);
int __table_get_uc_s16(table_t *table, unsigned char* key, short* val);
int __table_get_uc_s32(table_t *table, unsigned char* key, int* val);
int __table_get_uc_sl32(table_t *table, unsigned char* key, long* val);
int __table_get_uc_s64(table_t *table, unsigned char* key, long long* val);
int __table_get_uc_u8(table_t *table, unsigned char* key, unsigned char* val);
int __table_get_uc_u16(table_t *table, unsigned char* key, unsigned short* val);
int __table_get_uc_u32(table_t *table, unsigned char* key, unsigned int* val);
int __table_get_uc_ul32(table_t *table, unsigned char* key, unsigned long* val);
int __table_get_uc_u64(table_t *table, unsigned char* key, unsigned long long* val);
int __table_get_uc_v(table_t *table, unsigned char* key, void** val);
int __table_get_cuc_s8(table_t *table, const unsigned char* key, char* val);
int __table_get_cuc_s16(table_t *table, const unsigned char* key, short* val);
int __table_get_cuc_s32(table_t *table, const unsigned char* key, int* val);
int __table_get_cuc_sl32(table_t *table, const unsigned char* key, long* val);
int __table_get_cuc_s64(table_t *table, const unsigned char* key, long long* val);
int __table_get_cuc_u8(table_t *table, const unsigned char* key, unsigned char* val);
int __table_get_cuc_u16(table_t *table, const unsigned char* key, unsigned short* val);
int __table_get_cuc_u32(table_t *table, const unsigned char* key, unsigned int* val);
int __table_get_cuc_ul32(table_t *table, const unsigned char* key, unsigned long* val);
int __table_get_cuc_u64(table_t *table, const unsigned char* key, unsigned long long* val);
int __table_get_cuc_v(table_t *table, const unsigned char* key, void** val);
int __table_get_s8_s8(table_t *table, char key, char* val);
int __table_get_s8_s16(table_t *table, char key, short* val);
int __table_get_s8_s32(table_t *table, char key, int* val);
int __table_get_s8_sl32(table_t *table, char key, long* val);
int __table_get_s8_s64(table_t *table, char key, long long* val);
int __table_get_s8_u8(table_t *table, char key, unsigned char* val);
int __table_get_s8_u16(table_t *table, char key, unsigned short* val);
int __table_get_s8_u32(table_t *table, char key, unsigned int* val);
int __table_get_s8_ul32(table_t *table, char key, unsigned long* val);
int __table_get_s8_u64(table_t *table, char key, unsigned long long* val);
int __table_get_s8_v(table_t *table, char key, void** val);
int __table_get_s16_s8(table_t *table, short key, char* val);
int __table_get_s16_s16(table_t *table, short key, short* val);
int __table_get_s16_s32(table_t *table, short key, int* val);
int __table_get_s16_sl32(table_t *table, short key, long* val);
int __table_get_s16_s64(table_t *table, short key, long long* val);
int __table_get_s16_u8(table_t *table, short key, unsigned char* val);
int __table_get_s16_u16(table_t *table, short key, unsigned short* val);
int __table_get_s16_u32(table_t *table, short key, unsigned int* val);
int __table_get_s16_ul32(table_t *table, short key, unsigned long* val);
int __table_get_s16_u64(table_t *table, short key, unsigned long long* val);
int __table_get_s16_v(table_t *table, short key, void** val);
int __table_get_s32_s8(table_t *table, int key, char* val);
int __table_get_s32_s16(table_t *table, int key, short* val);
int __table_get_s32_s32(table_t *table, int key, int* val);
int __table_get_s32_sl32(table_t *table, int key, long* val);
int __table_get_s32_s64(table_t *table, int key, long long* val);
int __table_get_s32_u8(table_t *table, int key, unsigned char* val);
int __table_get_s32_u16(table_t *table, int key, unsigned short* val);
int __table_get_s32_u32(table_t *table, int key, unsigned int* val);
int __table_get_s32_ul32(table_t *table, int key, unsigned long* val);
int __table_get_s32_u64(table_t *table, int key, unsigned long long* val);
int __table_get_s32_v(table_t *table, int key, void** val);
int __table_get_sl32_s8(table_t *table, long key, char* val);
int __table_get_sl32_s16(table_t *table, long key, short* val);
int __table_get_sl32_s32(table_t *table, long key, int* val);
int __table_get_sl32_sl32(table_t *table, long key, long* val);
int __table_get_sl32_s64(table_t *table, long key, long long* val);
int __table_get_sl32_u8(table_t *table, long key, unsigned char* val);
int __table_get_sl32_u16(table_t *table, long key, unsigned short* val);
int __table_get_sl32_u32(table_t *table, long key, unsigned int* val);
int __table_get_sl32_ul32(table_t *table, long key, unsigned long* val);
int __table_get_sl32_u64(table_t *table, long key, unsigned long long* val);
int __table_get_sl32_v(table_t *table, long key, void** val);
int __table_get_s64_s8(table_t *table, long long key, char* val);
int __table_get_s64_s16(table_t *table, long long key, short* val);
int __table_get_s64_s32(table_t *table, long long key, int* val);
int __table_get_s64_sl32(table_t *table, long long key, long* val);
int __table_get_s64_s64(table_t *table, long long key, long long* val);
int __table_get_s64_u8(table_t *table, long long key, unsigned char* val);
int __table_get_s64_u16(table_t *table, long long key, unsigned short* val);
int __table_get_s64_u32(table_t *table, long long key, unsigned int* val);
int __table_get_s64_ul32(table_t *table, long long key, unsigned long* val);
int __table_get_s64_u64(table_t *table, long long key, unsigned long long* val);
int __table_get_s64_v(table_t *table, long long key, void** val);
int __table_get_u8_s8(table_t *table, unsigned char key, char* val);
int __table_get_u8_s16(table_t *table, unsigned char key, short* val);
int __table_get_u8_s32(table_t *table, unsigned char key, int* val);
int __table_get_u8_sl32(table_t *table, unsigned char key, long* val);
int __table_get_u8_s64(table_t *table, unsigned char key, long long* val);
int __table_get_u8_u8(table_t *table, unsigned char key, unsigned char* val);
int __table_get_u8_u16(table_t *table, unsigned char key, unsigned short* val);
int __table_get_u8_u32(table_t *table, unsigned char key, unsigned int* val);
int __table_get_u8_ul32(table_t *table, unsigned char key, unsigned long* val);
int __table_get_u8_u64(table_t *table, unsigned char key, unsigned long long* val);
int __table_get_u8_v(table_t *table, unsigned char key, void** val);
int __table_get_u16_s8(table_t *table, unsigned short key, char* val);
int __table_get_u16_s16(table_t *table, unsigned short key, short* val);
int __table_get_u16_s32(table_t *table, unsigned short key, int* val);
int __table_get_u16_sl32(table_t *table, unsigned short key, long* val);
int __table_get_u16_s64(table_t *table, unsigned short key, long long* val);
int __table_get_u16_u8(table_t *table, unsigned short key, unsigned char* val);
int __table_get_u16_u16(table_t *table, unsigned short key, unsigned short* val);
int __table_get_u16_u32(table_t *table, unsigned short key, unsigned int* val);
int __table_get_u16_ul32(table_t *table, unsigned short key, unsigned long* val);
int __table_get_u16_u64(table_t *table, unsigned short key, unsigned long long* val);
int __table_get_u16_v(table_t *table, unsigned short key, void** val);
int __table_get_u32_s8(table_t *table, unsigned int key, char* val);
int __table_get_u32_s16(table_t *table, unsigned int key, short* val);
int __table_get_u32_s32(table_t *table, unsigned int key, int* val);
int __table_get_u32_sl32(table_t *table, unsigned int key, long* val);
int __table_get_u32_s64(table_t *table, unsigned int key, long long* val);
int __table_get_u32_u8(table_t *table, unsigned int key, unsigned char* val);
int __table_get_u32_u16(table_t *table, unsigned int key, unsigned short* val);
int __table_get_u32_u32(table_t *table, unsigned int key, unsigned int* val);
int __table_get_u32_ul32(table_t *table, unsigned int key, unsigned long* val);
int __table_get_u32_u64(table_t *table, unsigned int key, unsigned long long* val);
int __table_get_u32_v(table_t *table, unsigned int key, void** val);
int __table_get_ul32_s8(table_t *table, unsigned long key, char* val);
int __table_get_ul32_s16(table_t *table, unsigned long key, short* val);
int __table_get_ul32_s32(table_t *table, unsigned long key, int* val);
int __table_get_ul32_sl32(table_t *table, unsigned long key, long* val);
int __table_get_ul32_s64(table_t *table, unsigned long key, long long* val);
int __table_get_ul32_u8(table_t *table, unsigned long key, unsigned char* val);
int __table_get_ul32_u16(table_t *table, unsigned long key, unsigned short* val);
int __table_get_ul32_u32(table_t *table, unsigned long key, unsigned int* val);
int __table_get_ul32_ul32(table_t *table, unsigned long key, unsigned long* val);
int __table_get_ul32_u64(table_t *table, unsigned long key, unsigned long long* val);
int __table_get_ul32_v(table_t *table, unsigned long key, void** val);
int __table_get_u64_s8(table_t *table, unsigned long long key, char* val);
int __table_get_u64_s16(table_t *table, unsigned long long key, short* val);
int __table_get_u64_s32(table_t *table, unsigned long long key, int* val);
int __table_get_u64_sl32(table_t *table, unsigned long long key, long* val);
int __table_get_u64_s64(table_t *table, unsigned long long key, long long* val);
int __table_get_u64_u8(table_t *table, unsigned long long key, unsigned char* val);
int __table_get_u64_u16(table_t *table, unsigned long long key, unsigned short* val);
int __table_get_u64_u32(table_t *table, unsigned long long key, unsigned int* val);
int __table_get_u64_ul32(table_t *table, unsigned long long key, unsigned long* val);
int __table_get_u64_u64(table_t *table, unsigned long long key, unsigned long long* val);
int __table_get_u64_v(table_t *table, unsigned long long key, void** val);
int __table_get_v_s8(table_t *table, void* key, char* val);
int __table_get_v_s16(table_t *table, void* key, short* val);
int __table_get_v_s32(table_t *table, void* key, int* val);
int __table_get_v_sl32(table_t *table, void* key, long* val);
int __table_get_v_s64(table_t *table, void* key, long long* val);
int __table_get_v_u8(table_t *table, void* key, unsigned char* val);
int __table_get_v_u16(table_t *table, void* key, unsigned short* val);
int __table_get_v_u32(table_t *table, void* key, unsigned int* val);
int __table_get_v_ul32(table_t *table, void* key, unsigned long* val);
int __table_get_v_u64(table_t *table, void* key, unsigned long long* val);
int __table_get_v_v(table_t *table, void* key, void** val);

int __table_set_c_s8(table_t *table, char* key, char val);
int __table_set_c_s16(table_t *table, char* key, short val);
int __table_set_c_s32(table_t *table, char* key, int val);
int __table_set_c_sl32(table_t *table, char* key, long val);
int __table_set_c_s64(table_t *table, char* key, long long val);
int __table_set_c_u8(table_t *table, char* key, unsigned char val);
int __table_set_c_u16(table_t *table, char* key, unsigned short val);
int __table_set_c_u32(table_t *table, char* key, unsigned int val);
int __table_set_c_ul32(table_t *table, char* key, unsigned long val);
int __table_set_c_u64(table_t *table, char* key, unsigned long long val);
int __table_set_c_v(table_t *table, char* key, void* val);
int __table_set_cc_s8(table_t *table, const char* key, char val);
int __table_set_cc_s16(table_t *table, const char* key, short val);
int __table_set_cc_s32(table_t *table, const char* key, int val);
int __table_set_cc_sl32(table_t *table, const char* key, long val);
int __table_set_cc_s64(table_t *table, const char* key, long long val);
int __table_set_cc_u8(table_t *table, const char* key, unsigned char val);
int __table_set_cc_u16(table_t *table, const char* key, unsigned short val);
int __table_set_cc_u32(table_t *table, const char* key, unsigned int val);
int __table_set_cc_ul32(table_t *table, const char* key, unsigned long val);
int __table_set_cc_u64(table_t *table, const char* key, unsigned long long val);
int __table_set_cc_v(table_t *table, const char* key, void* val);
int __table_set_uc_s8(table_t *table, unsigned char* key, char val);
int __table_set_uc_s16(table_t *table, unsigned char* key, short val);
int __table_set_uc_s32(table_t *table, unsigned char* key, int val);
int __table_set_uc_sl32(table_t *table, unsigned char* key, long val);
int __table_set_uc_s64(table_t *table, unsigned char* key, long long val);
int __table_set_uc_u8(table_t *table, unsigned char* key, unsigned char val);
int __table_set_uc_u16(table_t *table, unsigned char* key, unsigned short val);
int __table_set_uc_u32(table_t *table, unsigned char* key, unsigned int val);
int __table_set_uc_ul32(table_t *table, unsigned char* key, unsigned long val);
int __table_set_uc_u64(table_t *table, unsigned char* key, unsigned long long val);
int __table_set_uc_v(table_t *table, unsigned char* key, void* val);
int __table_set_cuc_s8(table_t *table, const unsigned char* key, char val);
int __table_set_cuc_s16(table_t *table, const unsigned char* key, short val);
int __table_set_cuc_s32(table_t *table, const unsigned char* key, int val);
int __table_set_cuc_sl32(table_t *table, const unsigned char* key, long val);
int __table_set_cuc_s64(table_t *table, const unsigned char* key, long long val);
int __table_set_cuc_u8(table_t *table, const unsigned char* key, unsigned char val);
int __table_set_cuc_u16(table_t *table, const unsigned char* key, unsigned short val);
int __table_set_cuc_u32(table_t *table, const unsigned char* key, unsigned int val);
int __table_set_cuc_ul32(table_t *table, const unsigned char* key, unsigned long val);
int __table_set_cuc_u64(table_t *table, const unsigned char* key, unsigned long long val);
int __table_set_cuc_v(table_t *table, const unsigned char* key, void* val);
int __table_set_s8_s8(table_t *table, char key, char val);
int __table_set_s8_s16(table_t *table, char key, short val);
int __table_set_s8_s32(table_t *table, char key, int val);
int __table_set_s8_sl32(table_t *table, char key, long val);
int __table_set_s8_s64(table_t *table, char key, long long val);
int __table_set_s8_u8(table_t *table, char key, unsigned char val);
int __table_set_s8_u16(table_t *table, char key, unsigned short val);
int __table_set_s8_u32(table_t *table, char key, unsigned int val);
int __table_set_s8_ul32(table_t *table, char key, unsigned long val);
int __table_set_s8_u64(table_t *table, char key, unsigned long long val);
int __table_set_s8_v(table_t *table, char key, void* val);
int __table_set_s16_s8(table_t *table, short key, char val);
int __table_set_s16_s16(table_t *table, short key, short val);
int __table_set_s16_s32(table_t *table, short key, int val);
int __table_set_s16_sl32(table_t *table, short key, long val);
int __table_set_s16_s64(table_t *table, short key, long long val);
int __table_set_s16_u8(table_t *table, short key, unsigned char val);
int __table_set_s16_u16(table_t *table, short key, unsigned short val);
int __table_set_s16_u32(table_t *table, short key, unsigned int val);
int __table_set_s16_ul32(table_t *table, short key, unsigned long val);
int __table_set_s16_u64(table_t *table, short key, unsigned long long val);
int __table_set_s16_v(table_t *table, short key, void* val);
int __table_set_s32_s8(table_t *table, int key, char val);
int __table_set_s32_s16(table_t *table, int key, short val);
int __table_set_s32_s32(table_t *table, int key, int val);
int __table_set_s32_sl32(table_t *table, int key, long val);
int __table_set_s32_s64(table_t *table, int key, long long val);
int __table_set_s32_u8(table_t *table, int key, unsigned char val);
int __table_set_s32_u16(table_t *table, int key, unsigned short val);
int __table_set_s32_u32(table_t *table, int key, unsigned int val);
int __table_set_s32_ul32(table_t *table, int key, unsigned long val);
int __table_set_s32_u64(table_t *table, int key, unsigned long long val);
int __table_set_s32_v(table_t *table, int key, void* val);
int __table_set_sl32_s8(table_t *table, long key, char val);
int __table_set_sl32_s16(table_t *table, long key, short val);
int __table_set_sl32_s32(table_t *table, long key, int val);
int __table_set_sl32_sl32(table_t *table, long key, long val);
int __table_set_sl32_s64(table_t *table, long key, long long val);
int __table_set_sl32_u8(table_t *table, long key, unsigned char val);
int __table_set_sl32_u16(table_t *table, long key, unsigned short val);
int __table_set_sl32_u32(table_t *table, long key, unsigned int val);
int __table_set_sl32_ul32(table_t *table, long key, unsigned long val);
int __table_set_sl32_u64(table_t *table, long key, unsigned long long val);
int __table_set_sl32_v(table_t *table, long key, void* val);
int __table_set_s64_s8(table_t *table, long long key, char val);
int __table_set_s64_s16(table_t *table, long long key, short val);
int __table_set_s64_s32(table_t *table, long long key, int val);
int __table_set_s64_sl32(table_t *table, long long key, long val);
int __table_set_s64_s64(table_t *table, long long key, long long val);
int __table_set_s64_u8(table_t *table, long long key, unsigned char val);
int __table_set_s64_u16(table_t *table, long long key, unsigned short val);
int __table_set_s64_u32(table_t *table, long long key, unsigned int val);
int __table_set_s64_ul32(table_t *table, long long key, unsigned long val);
int __table_set_s64_u64(table_t *table, long long key, unsigned long long val);
int __table_set_s64_v(table_t *table, long long key, void* val);
int __table_set_u8_s8(table_t *table, unsigned char key, char val);
int __table_set_u8_s16(table_t *table, unsigned char key, short val);
int __table_set_u8_s32(table_t *table, unsigned char key, int val);
int __table_set_u8_sl32(table_t *table, unsigned char key, long val);
int __table_set_u8_s64(table_t *table, unsigned char key, long long val);
int __table_set_u8_u8(table_t *table, unsigned char key, unsigned char val);
int __table_set_u8_u16(table_t *table, unsigned char key, unsigned short val);
int __table_set_u8_u32(table_t *table, unsigned char key, unsigned int val);
int __table_set_u8_ul32(table_t *table, unsigned char key, unsigned long val);
int __table_set_u8_u64(table_t *table, unsigned char key, unsigned long long val);
int __table_set_u8_v(table_t *table, unsigned char key, void* val);
int __table_set_u16_s8(table_t *table, unsigned short key, char val);
int __table_set_u16_s16(table_t *table, unsigned short key, short val);
int __table_set_u16_s32(table_t *table, unsigned short key, int val);
int __table_set_u16_sl32(table_t *table, unsigned short key, long val);
int __table_set_u16_s64(table_t *table, unsigned short key, long long val);
int __table_set_u16_u8(table_t *table, unsigned short key, unsigned char val);
int __table_set_u16_u16(table_t *table, unsigned short key, unsigned short val);
int __table_set_u16_u32(table_t *table, unsigned short key, unsigned int val);
int __table_set_u16_ul32(table_t *table, unsigned short key, unsigned long val);
int __table_set_u16_u64(table_t *table, unsigned short key, unsigned long long val);
int __table_set_u16_v(table_t *table, unsigned short key, void* val);
int __table_set_u32_s8(table_t *table, unsigned int key, char val);
int __table_set_u32_s16(table_t *table, unsigned int key, short val);
int __table_set_u32_s32(table_t *table, unsigned int key, int val);
int __table_set_u32_sl32(table_t *table, unsigned int key, long val);
int __table_set_u32_s64(table_t *table, unsigned int key, long long val);
int __table_set_u32_u8(table_t *table, unsigned int key, unsigned char val);
int __table_set_u32_u16(table_t *table, unsigned int key, unsigned short val);
int __table_set_u32_u32(table_t *table, unsigned int key, unsigned int val);
int __table_set_u32_ul32(table_t *table, unsigned int key, unsigned long val);
int __table_set_u32_u64(table_t *table, unsigned int key, unsigned long long val);
int __table_set_u32_v(table_t *table, unsigned int key, void* val);
int __table_set_ul32_s8(table_t *table, unsigned long key, char val);
int __table_set_ul32_s16(table_t *table, unsigned long key, short val);
int __table_set_ul32_s32(table_t *table, unsigned long key, int val);
int __table_set_ul32_sl32(table_t *table, unsigned long key, long val);
int __table_set_ul32_s64(table_t *table, unsigned long key, long long val);
int __table_set_ul32_u8(table_t *table, unsigned long key, unsigned char val);
int __table_set_ul32_u16(table_t *table, unsigned long key, unsigned short val);
int __table_set_ul32_u32(table_t *table, unsigned long key, unsigned int val);
int __table_set_ul32_ul32(table_t *table, unsigned long key, unsigned long val);
int __table_set_ul32_u64(table_t *table, unsigned long key, unsigned long long val);
int __table_set_ul32_v(table_t *table, unsigned long key, void* val);
int __table_set_u64_s8(table_t *table, unsigned long long key, char val);
int __table_set_u64_s16(table_t *table, unsigned long long key, short val);
int __table_set_u64_s32(table_t *table, unsigned long long key, int val);
int __table_set_u64_sl32(table_t *table, unsigned long long key, long val);
int __table_set_u64_s64(table_t *table, unsigned long long key, long long val);
int __table_set_u64_u8(table_t *table, unsigned long long key, unsigned char val);
int __table_set_u64_u16(table_t *table, unsigned long long key, unsigned short val);
int __table_set_u64_u32(table_t *table, unsigned long long key, unsigned int val);
int __table_set_u64_ul32(table_t *table, unsigned long long key, unsigned long val);
int __table_set_u64_u64(table_t *table, unsigned long long key, unsigned long long val);
int __table_set_u64_v(table_t *table, unsigned long long key, void* val);
int __table_set_v_s8(table_t *table, void* key, char val);
int __table_set_v_s16(table_t *table, void* key, short val);
int __table_set_v_s32(table_t *table, void* key, int val);
int __table_set_v_sl32(table_t *table, void* key, long val);
int __table_set_v_s64(table_t *table, void* key, long long val);
int __table_set_v_u8(table_t *table, void* key, unsigned char val);
int __table_set_v_u16(table_t *table, void* key, unsigned short val);
int __table_set_v_u32(table_t *table, void* key, unsigned int val);
int __table_set_v_ul32(table_t *table, void* key, unsigned long val);
int __table_set_v_u64(table_t *table, void* key, unsigned long long val);
int __table_set_v_v(table_t *table, void* key, void* val);

int __table_has_c(table_t *table, char* key);
int __table_has_cc(table_t *table, const char* key);
int __table_has_uc(table_t *table, unsigned char* key);
int __table_has_cuc(table_t *table, const unsigned char* key);
int __table_has_s8(table_t *table, char key);
int __table_has_s16(table_t *table, short key);
int __table_has_s32(table_t *table, int key);
int __table_has_sl32(table_t *table, long key);
int __table_has_s64(table_t *table, long long key);
int __table_has_u8(table_t *table, unsigned char key);
int __table_has_u16(table_t *table, unsigned short key);
int __table_has_u32(table_t *table, unsigned int key);
int __table_has_ul32(table_t *table, unsigned long key);
int __table_has_u64(table_t *table, unsigned long long key);
int __table_has_v(table_t *table, void* key);

int __table_del_c(table_t *table, char* key);
int __table_del_cc(table_t *table, const char* key);
int __table_del_uc(table_t *table, unsigned char* key);
int __table_del_cuc(table_t *table, const unsigned char* key);
int __table_del_s8(table_t *table, char key);
int __table_del_s16(table_t *table, short key);
int __table_del_s32(table_t *table, int key);
int __table_del_sl32(table_t *table, long key);
int __table_del_s64(table_t *table, long long key);
int __table_del_u8(table_t *table, unsigned char key);
int __table_del_u16(table_t *table, unsigned short key);
int __table_del_u32(table_t *table, unsigned int key);
int __table_del_ul32(table_t *table, unsigned long key);
int __table_del_u64(table_t *table, unsigned long long key);
int __table_del_v(table_t *table, void* key);

// END HEADER


#if defined(__cplusplus)
}
#endif
#endif // TABLE_HEADER

#if defined(TABLE_IMPLEMENTATION)
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct imap_node_t {
    union {
        uint32_t vec32[16];
        uint64_t vec64[8];
    };
};

#define imap__tree_root__           0
#define imap__tree_resv__           1
#define imap__tree_mark__           2
#define imap__tree_size__           3
#define imap__tree_nfre__           4
#define imap__tree_vfre__           5

#define imap__slot_pmask__          0x0000000f
#define imap__slot_node__           0x00000010
#define imap__slot_scalar__         0x00000020
#define imap__slot_value__          0xffffffe0
#define imap__slot_shift__          6
#define imap__slot_boxed__(sval)    (!((sval) & imap__slot_scalar__) && ((sval) >> imap__slot_shift__))

typedef struct {
    uint32_t stack[16];
    uint32_t stackp;
} imap_iter_t;

typedef struct {
    uint64_t x;
    uint32_t *slot;
} imap_pair_t;

#define imap__pair_zero__           ((imap_pair_t){0})
#define imap__pair__(x, slot)       ((imap_pair_t){(x), (slot)})
#define imap__node_zero__           ((imap_node_t){{{0}}})

#if defined(_MSC_VER)
static inline uint32_t imap__bsr__(uint64_t x) {
    return _BitScanReverse64((unsigned long *)&x, x | 1), (unsigned long)x;
}
#else
static inline uint32_t imap__bsr__(uint64_t x) {
    return 63 - __builtin_clzll(x | 1);
}
#endif

static inline uint32_t imap__xpos__(uint64_t x) {
    return imap__bsr__(x) >> 2;
}

static inline uint64_t imap__ceilpow2__(uint64_t x) {
    return 1ull << (imap__bsr__(x - 1) + 1);
}

static inline void *imap__aligned_alloc__(uint64_t alignment, uint64_t size) {
    void *p = TABLE_ALLOC(size + sizeof(void *) + alignment - 1);
    if (!p)
        return p;
    void **ap = (void**)(((uint64_t)p + sizeof(void *) + alignment - 1) & ~(alignment - 1));
    ap[-1] = p;
    return ap;
}

static inline void imap__aligned_free__(void *p) {
    if (p)
        TABLE_FREE(((void**)p)[-1]);
}

#define IMAP_ALIGNED_ALLOC(a, s)    (imap__aligned_alloc__(a, s))
#define IMAP_ALIGNED_FREE(p)        (imap__aligned_free__(p))

static inline imap_node_t* imap__node__(imap_node_t *tree, uint32_t val) {
    return (imap_node_t*)((uint8_t*)tree + val);
}

static inline uint32_t imap__node_pos__(imap_node_t *node) {
    return node->vec32[0] & 0xf;
}

static inline uint64_t imap__extract_lo4_port__(uint32_t vec32[16]) {
    union {
        uint32_t *vec32;
        uint64_t *vec64;
    } u;
    u.vec32 = vec32;
    return
        ((u.vec64[0] & 0xf0000000full)) |
        ((u.vec64[1] & 0xf0000000full) << 4) |
        ((u.vec64[2] & 0xf0000000full) << 8) |
        ((u.vec64[3] & 0xf0000000full) << 12) |
        ((u.vec64[4] & 0xf0000000full) << 16) |
        ((u.vec64[5] & 0xf0000000full) << 20) |
        ((u.vec64[6] & 0xf0000000full) << 24) |
        ((u.vec64[7] & 0xf0000000full) << 28);
}

static inline void imap__deposit_lo4_port__(uint32_t vec32[16], uint64_t value) {
    union {
        uint32_t *vec32;
        uint64_t *vec64;
    } u;
    u.vec32 = vec32;
    u.vec64[0] = (u.vec64[0] & ~0xf0000000full) | ((value) & 0xf0000000full);
    u.vec64[1] = (u.vec64[1] & ~0xf0000000full) | ((value >> 4) & 0xf0000000full);
    u.vec64[2] = (u.vec64[2] & ~0xf0000000full) | ((value >> 8) & 0xf0000000full);
    u.vec64[3] = (u.vec64[3] & ~0xf0000000full) | ((value >> 12) & 0xf0000000full);
    u.vec64[4] = (u.vec64[4] & ~0xf0000000full) | ((value >> 16) & 0xf0000000full);
    u.vec64[5] = (u.vec64[5] & ~0xf0000000full) | ((value >> 20) & 0xf0000000full);
    u.vec64[6] = (u.vec64[6] & ~0xf0000000full) | ((value >> 24) & 0xf0000000full);
    u.vec64[7] = (u.vec64[7] & ~0xf0000000full) | ((value >> 28) & 0xf0000000full);
}

static inline void imap__node_setprefix__(imap_node_t *node, uint64_t prefix) {
    imap__deposit_lo4_port__(node->vec32, prefix);
}

static inline uint64_t imap__node_prefix__(imap_node_t *node) {
    return imap__extract_lo4_port__(node->vec32);
}

static inline uint32_t imap__xdir__(uint64_t x, uint32_t pos) {
    return (x >> (pos << 2)) & 0xf;
}

static inline uint32_t imap__popcnt_hi28_port__(uint32_t vec32[16], uint32_t *p) {
    uint32_t pcnt = 0, sval, dirn;
    *p = 0;
    for (dirn = 0; 16 > dirn; dirn++)
    {
        sval = vec32[dirn];
        if (sval & ~0xf)
        {
            *p = sval;
            pcnt++;
        }
    }
    return pcnt;
}

static inline uint32_t imap__node_popcnt__(imap_node_t *node, uint32_t *p) {
    return imap__popcnt_hi28_port__(node->vec32, p);
}

static inline uint32_t imap__alloc_node__(imap_node_t *tree) {
    uint32_t mark = tree->vec32[imap__tree_nfre__];
    if (mark)
        tree->vec32[imap__tree_nfre__] = *(uint32_t*)((uint8_t *)tree + mark);
    else {
        mark = tree->vec32[imap__tree_mark__];
        assert(mark + sizeof(imap_node_t) <= tree->vec32[imap__tree_size__]);
        tree->vec32[imap__tree_mark__] = mark + sizeof(imap_node_t);
    }
    return mark;
}

static inline void imap__free_node__(imap_node_t *tree, uint32_t mark) {
    *(uint32_t *)((uint8_t *)tree + mark) = tree->vec32[imap__tree_nfre__];
    tree->vec32[imap__tree_nfre__] = mark;
}

static inline uint64_t imap__xpfx__(uint64_t x, uint32_t pos) {
    return x & (~0xfull << (pos << 2));
}

static inline imap_node_t* imap_ensure(imap_node_t *tree, uint32_t n) {
    imap_node_t *newtree;
    uint32_t hasnfre, hasvfre, newmark, oldsize, newsize;
    uint64_t newsize64;
    if (0 == n)
        return tree;
    if (0 == tree)
    {
        hasnfre = 0;
        hasvfre = 1;
        newmark = sizeof(imap_node_t);
        oldsize = 0;
    }
    else
    {
        hasnfre = !!tree->vec32[imap__tree_nfre__];
        hasvfre = !!tree->vec32[imap__tree_vfre__];
        newmark = tree->vec32[imap__tree_mark__];
        oldsize = tree->vec32[imap__tree_size__];
    }
    newmark += (n * 2 - hasnfre) * sizeof(imap_node_t) + (n - hasvfre) * sizeof(uint64_t);
    if (newmark <= oldsize)
        return tree;
    newsize64 = imap__ceilpow2__(newmark);
    if (0x20000000 < newsize64)
        return 0;
    newsize = (uint32_t)newsize64;
    newtree = (imap_node_t *)IMAP_ALIGNED_ALLOC(sizeof(imap_node_t), newsize);
    if (!newtree)
        return newtree;
    if (!tree) {
        newtree->vec32[imap__tree_root__] = 0;
        newtree->vec32[imap__tree_resv__] = 0;
        newtree->vec32[imap__tree_mark__] = sizeof(imap_node_t);
        newtree->vec32[imap__tree_size__] = newsize;
        newtree->vec32[imap__tree_nfre__] = 0;
        newtree->vec32[imap__tree_vfre__] = 3 << imap__slot_shift__;
        newtree->vec64[3] = 4 << imap__slot_shift__;
        newtree->vec64[4] = 5 << imap__slot_shift__;
        newtree->vec64[5] = 6 << imap__slot_shift__;
        newtree->vec64[6] = 7 << imap__slot_shift__;
        newtree->vec64[7] = 0;
    } else {
        memcpy(newtree, tree, tree->vec32[imap__tree_mark__]);
        IMAP_ALIGNED_FREE(tree);
        newtree->vec32[imap__tree_size__] = newsize;
    }
    return newtree;
}

static uint32_t *imap_lookup(imap_node_t *tree, uint64_t x) {
    imap_node_t *node = tree;
    uint32_t *slot;
    uint32_t sval, posn = 16, dirn = 0;
    for (;;) {
        slot = &node->vec32[dirn];
        sval = *slot;
        if (!(sval & imap__slot_node__)) {
            if ((sval & imap__slot_value__) && imap__node_prefix__(node) == (x & ~0xfull)) {
                assert(0 == posn);
                return slot;
            }
            return 0;
        }
        node = imap__node__(tree, sval & imap__slot_value__);
        posn = imap__node_pos__(node);
        dirn = imap__xdir__(x, posn);
    }
}

static uint32_t *imap_assign(imap_node_t *tree, uint64_t x) {
    uint32_t *slotstack[16 + 1];
    uint32_t posnstack[16 + 1];
    uint32_t stackp, stacki;
    imap_node_t *newnode, *node = tree;
    uint32_t *slot;
    uint32_t newmark, sval, diff, posn = 16, dirn = 0;
    uint64_t prfx;
    stackp = 0;
    for (;;) {
        slot = &node->vec32[dirn];
        sval = *slot;
        slotstack[stackp] = slot, posnstack[stackp++] = posn;
        if (!(sval & imap__slot_node__)) {
            prfx = imap__node_prefix__(node);
            if (0 == posn && prfx == (x & ~0xfull))
                return slot;
            diff = imap__xpos__(prfx ^ x);
            assert(diff < 16);
            for (stacki = stackp; diff > posn;)
                posn = posnstack[--stacki];
            if (stacki != stackp) {
                slot = slotstack[stacki];
                sval = *slot;
                assert(sval & imap__slot_node__);
                newmark = imap__alloc_node__(tree);
                *slot = (*slot & imap__slot_pmask__) | imap__slot_node__ | newmark;
                newnode = imap__node__(tree, newmark);
                *newnode = imap__node_zero__;
                newmark = imap__alloc_node__(tree);
                newnode->vec32[imap__xdir__(prfx, diff)] = sval;
                newnode->vec32[imap__xdir__(x, diff)] = imap__slot_node__ | newmark;
                imap__node_setprefix__(newnode, imap__xpfx__(prfx, diff) | diff);
            } else {
                newmark = imap__alloc_node__(tree);
                *slot = (*slot & imap__slot_pmask__) | imap__slot_node__ | newmark;
            }
            newnode = imap__node__(tree, newmark);
            *newnode = imap__node_zero__;
            imap__node_setprefix__(newnode, x & ~0xfull);
            return &newnode->vec32[x & 0xfull];
        }
        node = imap__node__(tree, sval & imap__slot_value__);
        posn = imap__node_pos__(node);
        dirn = imap__xdir__(x, posn);
    }
}

static inline uint32_t imap__alloc_val__(imap_node_t *tree) {
    uint32_t mark = imap__alloc_node__(tree);
    imap_node_t *node = imap__node__(tree, mark);
    mark <<= 3;
    tree->vec32[imap__tree_vfre__] = mark;
    node->vec64[0] = mark + (1 << imap__slot_shift__);
    node->vec64[1] = mark + (2 << imap__slot_shift__);
    node->vec64[2] = mark + (3 << imap__slot_shift__);
    node->vec64[3] = mark + (4 << imap__slot_shift__);
    node->vec64[4] = mark + (5 << imap__slot_shift__);
    node->vec64[5] = mark + (6 << imap__slot_shift__);
    node->vec64[6] = mark + (7 << imap__slot_shift__);
    node->vec64[7] = 0;
    return mark;
}

static uint64_t imap_getval64(imap_node_t *tree, uint32_t *slot) {
    assert(!(*slot & imap__slot_node__));
    uint32_t sval = *slot;
    return tree->vec64[sval >> imap__slot_shift__];
}

static void imap_setval64(imap_node_t *tree, uint32_t *slot, uint64_t y) {
    assert(!(*slot & imap__slot_node__));
    uint32_t sval = *slot;
    if (!(sval >> imap__slot_shift__))
    {
        sval = tree->vec32[imap__tree_vfre__];
        if (!sval)
            sval = imap__alloc_val__(tree);
        assert(sval >> imap__slot_shift__);
        tree->vec32[imap__tree_vfre__] = (uint32_t)tree->vec64[sval >> imap__slot_shift__];
    }
    assert(!(sval & imap__slot_node__));
    assert(imap__slot_boxed__(sval));
    *slot = (*slot & imap__slot_pmask__) | sval;
    tree->vec64[sval >> imap__slot_shift__] = y;
}

static void imap_delval(imap_node_t *tree, uint32_t *slot) {
    assert(!(*slot & imap__slot_node__));
    uint32_t sval = *slot;
    if (imap__slot_boxed__(sval)) {
        tree->vec64[sval >> imap__slot_shift__] = tree->vec32[imap__tree_vfre__];
        tree->vec32[imap__tree_vfre__] = sval & imap__slot_value__;
    }
    *slot &= imap__slot_pmask__;
}

static void imap_remove(imap_node_t *tree, uint64_t x) {
    uint32_t *slotstack[16 + 1];
    uint32_t stackp;
    imap_node_t *node = tree;
    uint32_t *slot;
    uint32_t sval, pval, posn = 16, dirn = 0;
    stackp = 0;
    for (;;) {
        slot = &node->vec32[dirn];
        sval = *slot;
        if (!(sval & imap__slot_node__)) {
            if ((sval & imap__slot_value__) && imap__node_prefix__(node) == (x & ~0xfull)) {
                assert(0 == posn);
                imap_delval(tree, slot);
            }
            while (stackp) {
                slot = slotstack[--stackp];
                sval = *slot;
                node = imap__node__(tree, sval & imap__slot_value__);
                posn = imap__node_pos__(node);
                if (!!posn != imap__node_popcnt__(node, &pval))
                    break;
                imap__free_node__(tree, sval & imap__slot_value__);
                *slot = (sval & imap__slot_pmask__) | (pval & ~imap__slot_pmask__);
            }
            return;
        }
        node = imap__node__(tree, sval & imap__slot_value__);
        posn = imap__node_pos__(node);
        dirn = imap__xdir__(x, posn);
        slotstack[stackp++] = slot;
    }
}

static imap_pair_t imap_iterate(imap_node_t *tree, imap_iter_t *iter, int restart) {
    imap_node_t *node;
    uint32_t *slot;
    uint32_t sval, dirn;
    if (restart) {
        iter->stackp = 0;
        sval = dirn = 0;
        goto enter;
    }
    // loop while stack is not empty
    while (iter->stackp) {
        // get slot value and increment direction
        sval = iter->stack[iter->stackp - 1]++;
        dirn = sval & 31;
        if (15 < dirn) {
            // if directions 0-15 have been examined, pop node from stack
            iter->stackp--;
            continue;
        }
    enter:
        node = imap__node__(tree, sval & imap__slot_value__);
        slot = &node->vec32[dirn];
        sval = *slot;
        if (sval & imap__slot_node__)
            // push node into stack
            iter->stack[iter->stackp++] = sval & imap__slot_value__;
        else if (sval & imap__slot_value__)
            return imap__pair__(imap__node_prefix__(node) | dirn, slot);
    }
    return imap__pair_zero__;
}

unordered_map_t unordered_map_new(void) {
    return unordered_map_make(TABLE_INITIAL_CAPACITY);
}

#define CAPACITY(C) ((C) > TABLE_INITIAL_CAPACITY ? (C) : TABLE_INITIAL_CAPACITY)

unordered_map_t unordered_map_make(uint32_t capacity) {
    uint64_t cap = CAPACITY(capacity);
    return (unordered_map_t) {
        .capacity = cap,
        .count = 0,
        .tree = imap_ensure(NULL, cap)
    };
}

void unordered_map_free(unordered_map_t *map) {
    IMAP_ALIGNED_FREE(map->tree);
    memset(map, 0, sizeof(unordered_map_t));
}

int unordered_map_set(unordered_map_t *map, uint64_t key, uint64_t val) {
    uint32_t *slot = imap_lookup(map->tree, key);
    if (slot) {
        imap_setval64(map->tree, slot, val);
        return 1;
    }
    if (map->count + 1 >= map->capacity) {
        map->capacity *= 2;
        map->tree = imap_ensure(map->tree, map->capacity);
    }
    if (!(slot = imap_assign(map->tree, key)))
        return 0;
    imap_setval64(map->tree, slot, val);
    map->count++;
    return 1;
}

int unordered_map_get(unordered_map_t *map, uint64_t key, uint64_t *val) {
    uint32_t *slot = imap_lookup(map->tree, key);
    if (!slot)
        return 0;
    if (val)
        *val = imap_getval64(map->tree, slot);
    return 1;
}

int unordered_map_has(unordered_map_t *map, uint64_t key) {
    return imap_lookup(map->tree, key) != NULL;
}

int unordered_map_del(unordered_map_t *map, uint64_t key) {
    if (!map->count)
        return 0;
    uint32_t *slot = imap_lookup(map->tree, key);
    if (!slot)
        return 0;
    imap_remove(map->tree, key);
    map->count--;
    return 1;
}

void unordered_map_foreach(unordered_map_t *map, unordered_map_iter_cb callback, void *userdata) {
    imap_iter_t iter;
    imap_pair_t pair = imap_iterate(map->tree, &iter, 1);
    size_t i = 0;
    for (;;) {
        if (!pair.slot)
            break;
        uint64_t val = imap_getval64(map->tree, pair.slot);
        unordered_map_pair_t p = {
            .key = pair.x,
            .val = &val
        };
        if (!callback(&p, userdata))
            break;
        else
            pair = imap_iterate(map->tree, &iter, 0);
    }
}

static void MM86128(const void *key, const int len, uint32_t seed, void *out) {
#define ROTL32(x, r) ((x << r) | (x >> (32 - r)))
#define FMIX32(h) h^=h>>16; h*=0x85ebca6b; h^=h>>13; h*=0xc2b2ae35; h^=h>>16;
    const uint8_t * data = (const uint8_t*)key;
    const int nblocks = len / 16;
    uint32_t h1 = seed;
    uint32_t h2 = seed;
    uint32_t h3 = seed;
    uint32_t h4 = seed;
    uint32_t c1 = 0x239b961b;
    uint32_t c2 = 0xab0e9789;
    uint32_t c3 = 0x38b34ae5;
    uint32_t c4 = 0xa1e38b93;
    const uint32_t * blocks = (const uint32_t *)(data + nblocks*16);
    for (int i = -nblocks; i; i++) {
        uint32_t k1 = blocks[i*4+0];
        uint32_t k2 = blocks[i*4+1];
        uint32_t k3 = blocks[i*4+2];
        uint32_t k4 = blocks[i*4+3];
        k1 *= c1; k1  = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
        h1 = ROTL32(h1,19); h1 += h2; h1 = h1*5+0x561ccd1b;
        k2 *= c2; k2  = ROTL32(k2,16); k2 *= c3; h2 ^= k2;
        h2 = ROTL32(h2,17); h2 += h3; h2 = h2*5+0x0bcaa747;
        k3 *= c3; k3  = ROTL32(k3,17); k3 *= c4; h3 ^= k3;
        h3 = ROTL32(h3,15); h3 += h4; h3 = h3*5+0x96cd1c35;
        k4 *= c4; k4  = ROTL32(k4,18); k4 *= c1; h4 ^= k4;
        h4 = ROTL32(h4,13); h4 += h1; h4 = h4*5+0x32ac3b17;
    }
    const uint8_t * tail = (const uint8_t*)(data + nblocks*16);
    uint32_t k1 = 0;
    uint32_t k2 = 0;
    uint32_t k3 = 0;
    uint32_t k4 = 0;
    switch(len & 15) {
        case 15:
            k4 ^= tail[14] << 16;
        case 14:
            k4 ^= tail[13] << 8;
        case 13:
            k4 ^= tail[12] << 0;
            k4 *= c4; k4  = ROTL32(k4,18); k4 *= c1; h4 ^= k4;
        case 12:
            k3 ^= tail[11] << 24;
        case 11:
            k3 ^= tail[10] << 16;
        case 10:
            k3 ^= tail[ 9] << 8;
        case 9:
            k3 ^= tail[ 8] << 0;
            k3 *= c3; k3  = ROTL32(k3,17); k3 *= c4; h3 ^= k3;
        case 8:
            k2 ^= tail[ 7] << 24;
        case 7:
            k2 ^= tail[ 6] << 16;
        case 6:
            k2 ^= tail[ 5] << 8;
        case 5:
            k2 ^= tail[ 4] << 0;
            k2 *= c2; k2  = ROTL32(k2,16); k2 *= c3; h2 ^= k2;
        case 4:
            k1 ^= tail[ 3] << 24;
        case 3:
            k1 ^= tail[ 2] << 16;
        case 2:
            k1 ^= tail[ 1] << 8;
        case 1:
            k1 ^= tail[ 0] << 0;
            k1 *= c1; k1  = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
    };
    h1 ^= len; h2 ^= len; h3 ^= len; h4 ^= len;
    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;
    FMIX32(h1); FMIX32(h2); FMIX32(h3); FMIX32(h4);
    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;
    ((uint32_t*)out)[0] = h1;
    ((uint32_t*)out)[1] = h2;
    ((uint32_t*)out)[2] = h3;
    ((uint32_t*)out)[3] = h4;
}

static uint64_t murmur(const void *data, size_t len, uint32_t seed) {
    char out[16];
    MM86128(data, (int)len, (uint32_t)seed, &out);
    return *(uint64_t*)out;
}

table_t table_new(void) {
    return table_make(murmur, TABLE_INITIAL_CAPACITY, 0);
}

table_t table_make(table_hash hashfn, uint32_t capacity, uint64_t seed) {
    uint64_t cap = CAPACITY(capacity);
    return (table_t) {
        .hashfn = hashfn,
        .seed = seed ? seed : (uint64_t)time(NULL),
        .kmap = unordered_map_make(cap),
        .vmap = unordered_map_make(cap)
    };
}

void table_free(table_t *table) {
    unordered_map_free(&table->kmap);
    unordered_map_free(&table->vmap);
    memset(table, 0, sizeof(table_t));
}

void table_each(table_t *table, int(*callback)(table_pair_t *pair, void*), void *userdata) {
    imap_iter_t iter;
    imap_pair_t pair = imap_iterate(table->kmap.tree, &iter, 1);
    size_t i = 0;
    table_pair_t p;
    for (;;) {
        if (!pair.slot)
            break;
        uint64_t tmp;
        if (unordered_map_get(&table->kmap, pair.x, &tmp))
            p.key.string = (const char*)tmp;
        else
            p.key.integer = (uint64_t)pair.x;
        if (!unordered_map_get(&table->vmap, pair.x, &tmp))
            continue;
        p.value = tmp;
        int result = callback(&p, userdata);
        if (!result)
            break;
        else
            pair = imap_iterate(table->kmap.tree, &iter, 0);
    }
}

// Copyright 2020 Joshua J Baker. All rights reserved.
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file
// https://github.com/tidwall/match.c/
static bool match(const char *pat, long plen, const char *str, long slen)  {
    if (plen < 0) plen = strlen(pat);
    if (slen < 0) slen = strlen(str);
    while (plen > 0) {
        if (pat[0] == '\\') {
            if (plen == 1) return false;
            pat++; plen--;
        } else if (pat[0] == '*') {
            if (plen == 1) return true;
            if (pat[1] == '*') {
                pat++; plen--;
                continue;
            }
            if (match(pat+1, plen-1, str, slen)) return true;
            if (slen == 0) return false;
            str++; slen--;
            continue;
        }
        if (slen == 0) return false;
        if (pat[0] != '?' && str[0] != pat[0]) return false;
        pat++; plen--;
        str++; slen--;
    }
    return slen == 0 && plen == 0;
}

void table_find(table_t *table, const char *wildcard, int(*callback)(const char*, void*, void*), void *userdata) {
    imap_iter_t iter;
    imap_pair_t pair = imap_iterate(table->kmap.tree, &iter, 1);
    for (;;) {
        if (!pair.slot)
            break;
        uint64_t tmp_key;
        const char *str = NULL;
        if (!unordered_map_get(&table->kmap, pair.x, &tmp_key))
            continue;
        str = (const char*)tmp_key;
        if (!match(wildcard, 0, str, 0))
            continue;
        if (!unordered_map_get(&table->vmap, pair.x, &tmp_key))
            continue;
        void *tmp = (void*)tmp_key;
        if (!callback(str, tmp, userdata))
            break;
        else
            pair = imap_iterate(table->kmap.tree, &iter, 0);
    }
}

bool table_search(table_t *table, const char *wildcard) {
    imap_iter_t iter;
    imap_pair_t pair = imap_iterate(table->kmap.tree, &iter, 1);
    for (;;) {
        if (!pair.slot)
            goto NEXT;
        uint64_t tmp_key;
        if (!unordered_map_get(&table->kmap, pair.x, &tmp_key))
            goto NEXT;
        if (match(wildcard, 0, (const char*)tmp_key, 0))
            return true;
    NEXT:
        pair = imap_iterate(table->kmap.tree, &iter, 0);
    }
    return false;
}

// BEGIN SOURCE

int __table_get_c_s8(table_t *table, char* key, char* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_c_s16(table_t *table, char* key, short* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_c_s32(table_t *table, char* key, int* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_c_sl32(table_t *table, char* key, long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_c_s64(table_t *table, char* key, long long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_c_u8(table_t *table, char* key, unsigned char* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_c_u16(table_t *table, char* key, unsigned short* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_c_u32(table_t *table, char* key, unsigned int* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_c_ul32(table_t *table, char* key, unsigned long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_c_u64(table_t *table, char* key, unsigned long long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_c_v(table_t *table, char* key, void** val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	void* tmp_out = NULL;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (void*)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cc_s8(table_t *table, const char* key, char* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cc_s16(table_t *table, const char* key, short* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cc_s32(table_t *table, const char* key, int* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cc_sl32(table_t *table, const char* key, long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cc_s64(table_t *table, const char* key, long long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cc_u8(table_t *table, const char* key, unsigned char* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cc_u16(table_t *table, const char* key, unsigned short* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cc_u32(table_t *table, const char* key, unsigned int* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cc_ul32(table_t *table, const char* key, unsigned long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cc_u64(table_t *table, const char* key, unsigned long long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cc_v(table_t *table, const char* key, void** val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	void* tmp_out = NULL;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (void*)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_uc_s8(table_t *table, unsigned char* key, char* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_uc_s16(table_t *table, unsigned char* key, short* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_uc_s32(table_t *table, unsigned char* key, int* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_uc_sl32(table_t *table, unsigned char* key, long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_uc_s64(table_t *table, unsigned char* key, long long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_uc_u8(table_t *table, unsigned char* key, unsigned char* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_uc_u16(table_t *table, unsigned char* key, unsigned short* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_uc_u32(table_t *table, unsigned char* key, unsigned int* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_uc_ul32(table_t *table, unsigned char* key, unsigned long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_uc_u64(table_t *table, unsigned char* key, unsigned long long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_uc_v(table_t *table, unsigned char* key, void** val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	void* tmp_out = NULL;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (void*)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cuc_s8(table_t *table, const unsigned char* key, char* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cuc_s16(table_t *table, const unsigned char* key, short* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cuc_s32(table_t *table, const unsigned char* key, int* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cuc_sl32(table_t *table, const unsigned char* key, long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cuc_s64(table_t *table, const unsigned char* key, long long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cuc_u8(table_t *table, const unsigned char* key, unsigned char* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cuc_u16(table_t *table, const unsigned char* key, unsigned short* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cuc_u32(table_t *table, const unsigned char* key, unsigned int* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cuc_ul32(table_t *table, const unsigned char* key, unsigned long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cuc_u64(table_t *table, const unsigned char* key, unsigned long long* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	unsigned long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_cuc_v(table_t *table, const unsigned char* key, void** val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	void* tmp_out = NULL;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (void*)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s8_s8(table_t *table, char key, char* val) {
	uint64_t k = (uint64_t)key;
	char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s8_s16(table_t *table, char key, short* val) {
	uint64_t k = (uint64_t)key;
	short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s8_s32(table_t *table, char key, int* val) {
	uint64_t k = (uint64_t)key;
	int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s8_sl32(table_t *table, char key, long* val) {
	uint64_t k = (uint64_t)key;
	long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s8_s64(table_t *table, char key, long long* val) {
	uint64_t k = (uint64_t)key;
	long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s8_u8(table_t *table, char key, unsigned char* val) {
	uint64_t k = (uint64_t)key;
	unsigned char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s8_u16(table_t *table, char key, unsigned short* val) {
	uint64_t k = (uint64_t)key;
	unsigned short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s8_u32(table_t *table, char key, unsigned int* val) {
	uint64_t k = (uint64_t)key;
	unsigned int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s8_ul32(table_t *table, char key, unsigned long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s8_u64(table_t *table, char key, unsigned long long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s8_v(table_t *table, char key, void** val) {
	uint64_t k = (uint64_t)key;
	void* tmp_out = NULL;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (void*)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s16_s8(table_t *table, short key, char* val) {
	uint64_t k = (uint64_t)key;
	char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s16_s16(table_t *table, short key, short* val) {
	uint64_t k = (uint64_t)key;
	short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s16_s32(table_t *table, short key, int* val) {
	uint64_t k = (uint64_t)key;
	int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s16_sl32(table_t *table, short key, long* val) {
	uint64_t k = (uint64_t)key;
	long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s16_s64(table_t *table, short key, long long* val) {
	uint64_t k = (uint64_t)key;
	long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s16_u8(table_t *table, short key, unsigned char* val) {
	uint64_t k = (uint64_t)key;
	unsigned char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s16_u16(table_t *table, short key, unsigned short* val) {
	uint64_t k = (uint64_t)key;
	unsigned short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s16_u32(table_t *table, short key, unsigned int* val) {
	uint64_t k = (uint64_t)key;
	unsigned int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s16_ul32(table_t *table, short key, unsigned long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s16_u64(table_t *table, short key, unsigned long long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s16_v(table_t *table, short key, void** val) {
	uint64_t k = (uint64_t)key;
	void* tmp_out = NULL;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (void*)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s32_s8(table_t *table, int key, char* val) {
	uint64_t k = (uint64_t)key;
	char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s32_s16(table_t *table, int key, short* val) {
	uint64_t k = (uint64_t)key;
	short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s32_s32(table_t *table, int key, int* val) {
	uint64_t k = (uint64_t)key;
	int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s32_sl32(table_t *table, int key, long* val) {
	uint64_t k = (uint64_t)key;
	long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s32_s64(table_t *table, int key, long long* val) {
	uint64_t k = (uint64_t)key;
	long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s32_u8(table_t *table, int key, unsigned char* val) {
	uint64_t k = (uint64_t)key;
	unsigned char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s32_u16(table_t *table, int key, unsigned short* val) {
	uint64_t k = (uint64_t)key;
	unsigned short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s32_u32(table_t *table, int key, unsigned int* val) {
	uint64_t k = (uint64_t)key;
	unsigned int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s32_ul32(table_t *table, int key, unsigned long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s32_u64(table_t *table, int key, unsigned long long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s32_v(table_t *table, int key, void** val) {
	uint64_t k = (uint64_t)key;
	void* tmp_out = NULL;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (void*)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_sl32_s8(table_t *table, long key, char* val) {
	uint64_t k = (uint64_t)key;
	char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_sl32_s16(table_t *table, long key, short* val) {
	uint64_t k = (uint64_t)key;
	short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_sl32_s32(table_t *table, long key, int* val) {
	uint64_t k = (uint64_t)key;
	int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_sl32_sl32(table_t *table, long key, long* val) {
	uint64_t k = (uint64_t)key;
	long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_sl32_s64(table_t *table, long key, long long* val) {
	uint64_t k = (uint64_t)key;
	long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_sl32_u8(table_t *table, long key, unsigned char* val) {
	uint64_t k = (uint64_t)key;
	unsigned char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_sl32_u16(table_t *table, long key, unsigned short* val) {
	uint64_t k = (uint64_t)key;
	unsigned short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_sl32_u32(table_t *table, long key, unsigned int* val) {
	uint64_t k = (uint64_t)key;
	unsigned int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_sl32_ul32(table_t *table, long key, unsigned long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_sl32_u64(table_t *table, long key, unsigned long long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_sl32_v(table_t *table, long key, void** val) {
	uint64_t k = (uint64_t)key;
	void* tmp_out = NULL;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (void*)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s64_s8(table_t *table, long long key, char* val) {
	uint64_t k = (uint64_t)key;
	char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s64_s16(table_t *table, long long key, short* val) {
	uint64_t k = (uint64_t)key;
	short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s64_s32(table_t *table, long long key, int* val) {
	uint64_t k = (uint64_t)key;
	int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s64_sl32(table_t *table, long long key, long* val) {
	uint64_t k = (uint64_t)key;
	long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s64_s64(table_t *table, long long key, long long* val) {
	uint64_t k = (uint64_t)key;
	long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s64_u8(table_t *table, long long key, unsigned char* val) {
	uint64_t k = (uint64_t)key;
	unsigned char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s64_u16(table_t *table, long long key, unsigned short* val) {
	uint64_t k = (uint64_t)key;
	unsigned short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s64_u32(table_t *table, long long key, unsigned int* val) {
	uint64_t k = (uint64_t)key;
	unsigned int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s64_ul32(table_t *table, long long key, unsigned long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s64_u64(table_t *table, long long key, unsigned long long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_s64_v(table_t *table, long long key, void** val) {
	uint64_t k = (uint64_t)key;
	void* tmp_out = NULL;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (void*)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u8_s8(table_t *table, unsigned char key, char* val) {
	uint64_t k = (uint64_t)key;
	char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u8_s16(table_t *table, unsigned char key, short* val) {
	uint64_t k = (uint64_t)key;
	short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u8_s32(table_t *table, unsigned char key, int* val) {
	uint64_t k = (uint64_t)key;
	int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u8_sl32(table_t *table, unsigned char key, long* val) {
	uint64_t k = (uint64_t)key;
	long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u8_s64(table_t *table, unsigned char key, long long* val) {
	uint64_t k = (uint64_t)key;
	long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u8_u8(table_t *table, unsigned char key, unsigned char* val) {
	uint64_t k = (uint64_t)key;
	unsigned char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u8_u16(table_t *table, unsigned char key, unsigned short* val) {
	uint64_t k = (uint64_t)key;
	unsigned short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u8_u32(table_t *table, unsigned char key, unsigned int* val) {
	uint64_t k = (uint64_t)key;
	unsigned int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u8_ul32(table_t *table, unsigned char key, unsigned long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u8_u64(table_t *table, unsigned char key, unsigned long long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u8_v(table_t *table, unsigned char key, void** val) {
	uint64_t k = (uint64_t)key;
	void* tmp_out = NULL;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (void*)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u16_s8(table_t *table, unsigned short key, char* val) {
	uint64_t k = (uint64_t)key;
	char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u16_s16(table_t *table, unsigned short key, short* val) {
	uint64_t k = (uint64_t)key;
	short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u16_s32(table_t *table, unsigned short key, int* val) {
	uint64_t k = (uint64_t)key;
	int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u16_sl32(table_t *table, unsigned short key, long* val) {
	uint64_t k = (uint64_t)key;
	long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u16_s64(table_t *table, unsigned short key, long long* val) {
	uint64_t k = (uint64_t)key;
	long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u16_u8(table_t *table, unsigned short key, unsigned char* val) {
	uint64_t k = (uint64_t)key;
	unsigned char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u16_u16(table_t *table, unsigned short key, unsigned short* val) {
	uint64_t k = (uint64_t)key;
	unsigned short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u16_u32(table_t *table, unsigned short key, unsigned int* val) {
	uint64_t k = (uint64_t)key;
	unsigned int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u16_ul32(table_t *table, unsigned short key, unsigned long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u16_u64(table_t *table, unsigned short key, unsigned long long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u16_v(table_t *table, unsigned short key, void** val) {
	uint64_t k = (uint64_t)key;
	void* tmp_out = NULL;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (void*)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u32_s8(table_t *table, unsigned int key, char* val) {
	uint64_t k = (uint64_t)key;
	char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u32_s16(table_t *table, unsigned int key, short* val) {
	uint64_t k = (uint64_t)key;
	short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u32_s32(table_t *table, unsigned int key, int* val) {
	uint64_t k = (uint64_t)key;
	int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u32_sl32(table_t *table, unsigned int key, long* val) {
	uint64_t k = (uint64_t)key;
	long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u32_s64(table_t *table, unsigned int key, long long* val) {
	uint64_t k = (uint64_t)key;
	long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u32_u8(table_t *table, unsigned int key, unsigned char* val) {
	uint64_t k = (uint64_t)key;
	unsigned char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u32_u16(table_t *table, unsigned int key, unsigned short* val) {
	uint64_t k = (uint64_t)key;
	unsigned short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u32_u32(table_t *table, unsigned int key, unsigned int* val) {
	uint64_t k = (uint64_t)key;
	unsigned int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u32_ul32(table_t *table, unsigned int key, unsigned long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u32_u64(table_t *table, unsigned int key, unsigned long long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u32_v(table_t *table, unsigned int key, void** val) {
	uint64_t k = (uint64_t)key;
	void* tmp_out = NULL;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (void*)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_ul32_s8(table_t *table, unsigned long key, char* val) {
	uint64_t k = (uint64_t)key;
	char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_ul32_s16(table_t *table, unsigned long key, short* val) {
	uint64_t k = (uint64_t)key;
	short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_ul32_s32(table_t *table, unsigned long key, int* val) {
	uint64_t k = (uint64_t)key;
	int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_ul32_sl32(table_t *table, unsigned long key, long* val) {
	uint64_t k = (uint64_t)key;
	long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_ul32_s64(table_t *table, unsigned long key, long long* val) {
	uint64_t k = (uint64_t)key;
	long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_ul32_u8(table_t *table, unsigned long key, unsigned char* val) {
	uint64_t k = (uint64_t)key;
	unsigned char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_ul32_u16(table_t *table, unsigned long key, unsigned short* val) {
	uint64_t k = (uint64_t)key;
	unsigned short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_ul32_u32(table_t *table, unsigned long key, unsigned int* val) {
	uint64_t k = (uint64_t)key;
	unsigned int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_ul32_ul32(table_t *table, unsigned long key, unsigned long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_ul32_u64(table_t *table, unsigned long key, unsigned long long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_ul32_v(table_t *table, unsigned long key, void** val) {
	uint64_t k = (uint64_t)key;
	void* tmp_out = NULL;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (void*)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u64_s8(table_t *table, unsigned long long key, char* val) {
	uint64_t k = (uint64_t)key;
	char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u64_s16(table_t *table, unsigned long long key, short* val) {
	uint64_t k = (uint64_t)key;
	short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u64_s32(table_t *table, unsigned long long key, int* val) {
	uint64_t k = (uint64_t)key;
	int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u64_sl32(table_t *table, unsigned long long key, long* val) {
	uint64_t k = (uint64_t)key;
	long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u64_s64(table_t *table, unsigned long long key, long long* val) {
	uint64_t k = (uint64_t)key;
	long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u64_u8(table_t *table, unsigned long long key, unsigned char* val) {
	uint64_t k = (uint64_t)key;
	unsigned char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u64_u16(table_t *table, unsigned long long key, unsigned short* val) {
	uint64_t k = (uint64_t)key;
	unsigned short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u64_u32(table_t *table, unsigned long long key, unsigned int* val) {
	uint64_t k = (uint64_t)key;
	unsigned int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u64_ul32(table_t *table, unsigned long long key, unsigned long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u64_u64(table_t *table, unsigned long long key, unsigned long long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_u64_v(table_t *table, unsigned long long key, void** val) {
	uint64_t k = (uint64_t)key;
	void* tmp_out = NULL;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (void*)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_v_s8(table_t *table, void* key, char* val) {
	uint64_t k = (uint64_t)key;
	char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_v_s16(table_t *table, void* key, short* val) {
	uint64_t k = (uint64_t)key;
	short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_v_s32(table_t *table, void* key, int* val) {
	uint64_t k = (uint64_t)key;
	int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_v_sl32(table_t *table, void* key, long* val) {
	uint64_t k = (uint64_t)key;
	long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_v_s64(table_t *table, void* key, long long* val) {
	uint64_t k = (uint64_t)key;
	long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_v_u8(table_t *table, void* key, unsigned char* val) {
	uint64_t k = (uint64_t)key;
	unsigned char tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned char)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_v_u16(table_t *table, void* key, unsigned short* val) {
	uint64_t k = (uint64_t)key;
	unsigned short tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned short)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_v_u32(table_t *table, void* key, unsigned int* val) {
	uint64_t k = (uint64_t)key;
	unsigned int tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned int)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_v_ul32(table_t *table, void* key, unsigned long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_v_u64(table_t *table, void* key, unsigned long long* val) {
	uint64_t k = (uint64_t)key;
	unsigned long long tmp_out = 0;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (unsigned long long)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_get_v_v(table_t *table, void* key, void** val) {
	uint64_t k = (uint64_t)key;
	void* tmp_out = NULL;
	int result = 1;
	if (!unordered_map_has(&table->vmap, k)) {
		result = 0;
		goto BAIL;
	}
	uint64_t tmp;
	if (!(result = unordered_map_get(&table->vmap, k, &tmp)))
		goto BAIL;
	tmp_out = (void*)tmp;
BAIL:
	if (val)
		*val = tmp_out;
	return result;
}

int __table_set_c_s8(table_t *table, char* key, char val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_c_s16(table_t *table, char* key, short val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_c_s32(table_t *table, char* key, int val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_c_sl32(table_t *table, char* key, long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_c_s64(table_t *table, char* key, long long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_c_u8(table_t *table, char* key, unsigned char val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_c_u16(table_t *table, char* key, unsigned short val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_c_u32(table_t *table, char* key, unsigned int val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_c_ul32(table_t *table, char* key, unsigned long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_c_u64(table_t *table, char* key, unsigned long long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_c_v(table_t *table, char* key, void* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cc_s8(table_t *table, const char* key, char val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cc_s16(table_t *table, const char* key, short val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cc_s32(table_t *table, const char* key, int val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cc_sl32(table_t *table, const char* key, long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cc_s64(table_t *table, const char* key, long long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cc_u8(table_t *table, const char* key, unsigned char val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cc_u16(table_t *table, const char* key, unsigned short val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cc_u32(table_t *table, const char* key, unsigned int val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cc_ul32(table_t *table, const char* key, unsigned long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cc_u64(table_t *table, const char* key, unsigned long long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cc_v(table_t *table, const char* key, void* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_uc_s8(table_t *table, unsigned char* key, char val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_uc_s16(table_t *table, unsigned char* key, short val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_uc_s32(table_t *table, unsigned char* key, int val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_uc_sl32(table_t *table, unsigned char* key, long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_uc_s64(table_t *table, unsigned char* key, long long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_uc_u8(table_t *table, unsigned char* key, unsigned char val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_uc_u16(table_t *table, unsigned char* key, unsigned short val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_uc_u32(table_t *table, unsigned char* key, unsigned int val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_uc_ul32(table_t *table, unsigned char* key, unsigned long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_uc_u64(table_t *table, unsigned char* key, unsigned long long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_uc_v(table_t *table, unsigned char* key, void* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cuc_s8(table_t *table, const unsigned char* key, char val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cuc_s16(table_t *table, const unsigned char* key, short val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cuc_s32(table_t *table, const unsigned char* key, int val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cuc_sl32(table_t *table, const unsigned char* key, long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cuc_s64(table_t *table, const unsigned char* key, long long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cuc_u8(table_t *table, const unsigned char* key, unsigned char val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cuc_u16(table_t *table, const unsigned char* key, unsigned short val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cuc_u32(table_t *table, const unsigned char* key, unsigned int val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cuc_ul32(table_t *table, const unsigned char* key, unsigned long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cuc_u64(table_t *table, const unsigned char* key, unsigned long long val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_cuc_v(table_t *table, const unsigned char* key, void* val) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k)) {
		const char *dup = strdup((const char*)key);
		unordered_map_set(&table->kmap, k, (uint64_t)dup);
	}
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s8_s8(table_t *table, char key, char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s8_s16(table_t *table, char key, short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s8_s32(table_t *table, char key, int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s8_sl32(table_t *table, char key, long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s8_s64(table_t *table, char key, long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s8_u8(table_t *table, char key, unsigned char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s8_u16(table_t *table, char key, unsigned short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s8_u32(table_t *table, char key, unsigned int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s8_ul32(table_t *table, char key, unsigned long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s8_u64(table_t *table, char key, unsigned long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s8_v(table_t *table, char key, void* val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s16_s8(table_t *table, short key, char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s16_s16(table_t *table, short key, short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s16_s32(table_t *table, short key, int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s16_sl32(table_t *table, short key, long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s16_s64(table_t *table, short key, long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s16_u8(table_t *table, short key, unsigned char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s16_u16(table_t *table, short key, unsigned short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s16_u32(table_t *table, short key, unsigned int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s16_ul32(table_t *table, short key, unsigned long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s16_u64(table_t *table, short key, unsigned long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s16_v(table_t *table, short key, void* val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s32_s8(table_t *table, int key, char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s32_s16(table_t *table, int key, short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s32_s32(table_t *table, int key, int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s32_sl32(table_t *table, int key, long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s32_s64(table_t *table, int key, long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s32_u8(table_t *table, int key, unsigned char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s32_u16(table_t *table, int key, unsigned short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s32_u32(table_t *table, int key, unsigned int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s32_ul32(table_t *table, int key, unsigned long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s32_u64(table_t *table, int key, unsigned long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s32_v(table_t *table, int key, void* val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_sl32_s8(table_t *table, long key, char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_sl32_s16(table_t *table, long key, short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_sl32_s32(table_t *table, long key, int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_sl32_sl32(table_t *table, long key, long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_sl32_s64(table_t *table, long key, long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_sl32_u8(table_t *table, long key, unsigned char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_sl32_u16(table_t *table, long key, unsigned short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_sl32_u32(table_t *table, long key, unsigned int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_sl32_ul32(table_t *table, long key, unsigned long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_sl32_u64(table_t *table, long key, unsigned long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_sl32_v(table_t *table, long key, void* val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s64_s8(table_t *table, long long key, char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s64_s16(table_t *table, long long key, short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s64_s32(table_t *table, long long key, int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s64_sl32(table_t *table, long long key, long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s64_s64(table_t *table, long long key, long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s64_u8(table_t *table, long long key, unsigned char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s64_u16(table_t *table, long long key, unsigned short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s64_u32(table_t *table, long long key, unsigned int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s64_ul32(table_t *table, long long key, unsigned long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s64_u64(table_t *table, long long key, unsigned long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_s64_v(table_t *table, long long key, void* val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u8_s8(table_t *table, unsigned char key, char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u8_s16(table_t *table, unsigned char key, short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u8_s32(table_t *table, unsigned char key, int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u8_sl32(table_t *table, unsigned char key, long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u8_s64(table_t *table, unsigned char key, long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u8_u8(table_t *table, unsigned char key, unsigned char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u8_u16(table_t *table, unsigned char key, unsigned short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u8_u32(table_t *table, unsigned char key, unsigned int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u8_ul32(table_t *table, unsigned char key, unsigned long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u8_u64(table_t *table, unsigned char key, unsigned long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u8_v(table_t *table, unsigned char key, void* val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u16_s8(table_t *table, unsigned short key, char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u16_s16(table_t *table, unsigned short key, short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u16_s32(table_t *table, unsigned short key, int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u16_sl32(table_t *table, unsigned short key, long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u16_s64(table_t *table, unsigned short key, long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u16_u8(table_t *table, unsigned short key, unsigned char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u16_u16(table_t *table, unsigned short key, unsigned short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u16_u32(table_t *table, unsigned short key, unsigned int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u16_ul32(table_t *table, unsigned short key, unsigned long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u16_u64(table_t *table, unsigned short key, unsigned long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u16_v(table_t *table, unsigned short key, void* val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u32_s8(table_t *table, unsigned int key, char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u32_s16(table_t *table, unsigned int key, short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u32_s32(table_t *table, unsigned int key, int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u32_sl32(table_t *table, unsigned int key, long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u32_s64(table_t *table, unsigned int key, long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u32_u8(table_t *table, unsigned int key, unsigned char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u32_u16(table_t *table, unsigned int key, unsigned short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u32_u32(table_t *table, unsigned int key, unsigned int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u32_ul32(table_t *table, unsigned int key, unsigned long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u32_u64(table_t *table, unsigned int key, unsigned long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u32_v(table_t *table, unsigned int key, void* val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_ul32_s8(table_t *table, unsigned long key, char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_ul32_s16(table_t *table, unsigned long key, short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_ul32_s32(table_t *table, unsigned long key, int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_ul32_sl32(table_t *table, unsigned long key, long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_ul32_s64(table_t *table, unsigned long key, long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_ul32_u8(table_t *table, unsigned long key, unsigned char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_ul32_u16(table_t *table, unsigned long key, unsigned short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_ul32_u32(table_t *table, unsigned long key, unsigned int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_ul32_ul32(table_t *table, unsigned long key, unsigned long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_ul32_u64(table_t *table, unsigned long key, unsigned long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_ul32_v(table_t *table, unsigned long key, void* val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u64_s8(table_t *table, unsigned long long key, char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u64_s16(table_t *table, unsigned long long key, short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u64_s32(table_t *table, unsigned long long key, int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u64_sl32(table_t *table, unsigned long long key, long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u64_s64(table_t *table, unsigned long long key, long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u64_u8(table_t *table, unsigned long long key, unsigned char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u64_u16(table_t *table, unsigned long long key, unsigned short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u64_u32(table_t *table, unsigned long long key, unsigned int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u64_ul32(table_t *table, unsigned long long key, unsigned long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u64_u64(table_t *table, unsigned long long key, unsigned long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_u64_v(table_t *table, unsigned long long key, void* val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_v_s8(table_t *table, void* key, char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_v_s16(table_t *table, void* key, short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_v_s32(table_t *table, void* key, int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_v_sl32(table_t *table, void* key, long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_v_s64(table_t *table, void* key, long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_v_u8(table_t *table, void* key, unsigned char val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_v_u16(table_t *table, void* key, unsigned short val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_v_u32(table_t *table, void* key, unsigned int val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_v_ul32(table_t *table, void* key, unsigned long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_v_u64(table_t *table, void* key, unsigned long long val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_set_v_v(table_t *table, void* key, void* val) {
	uint64_t k = (uint64_t)key;
	return unordered_map_set(&table->vmap, k, (uint64_t)val);
}

int __table_has_c(table_t *table, char* key) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	return unordered_map_has(&table->vmap, k);
}

int __table_has_cc(table_t *table, const char* key) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	return unordered_map_has(&table->vmap, k);
}

int __table_has_uc(table_t *table, unsigned char* key) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	return unordered_map_has(&table->vmap, k);
}

int __table_has_cuc(table_t *table, const unsigned char* key) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	return unordered_map_has(&table->vmap, k);
}

int __table_has_s8(table_t *table, char key) {
	uint64_t k = (uint64_t)key;
	return unordered_map_has(&table->vmap, k);
}

int __table_has_s16(table_t *table, short key) {
	uint64_t k = (uint64_t)key;
	return unordered_map_has(&table->vmap, k);
}

int __table_has_s32(table_t *table, int key) {
	uint64_t k = (uint64_t)key;
	return unordered_map_has(&table->vmap, k);
}

int __table_has_sl32(table_t *table, long key) {
	uint64_t k = (uint64_t)key;
	return unordered_map_has(&table->vmap, k);
}

int __table_has_s64(table_t *table, long long key) {
	uint64_t k = (uint64_t)key;
	return unordered_map_has(&table->vmap, k);
}

int __table_has_u8(table_t *table, unsigned char key) {
	uint64_t k = (uint64_t)key;
	return unordered_map_has(&table->vmap, k);
}

int __table_has_u16(table_t *table, unsigned short key) {
	uint64_t k = (uint64_t)key;
	return unordered_map_has(&table->vmap, k);
}

int __table_has_u32(table_t *table, unsigned int key) {
	uint64_t k = (uint64_t)key;
	return unordered_map_has(&table->vmap, k);
}

int __table_has_ul32(table_t *table, unsigned long key) {
	uint64_t k = (uint64_t)key;
	return unordered_map_has(&table->vmap, k);
}

int __table_has_u64(table_t *table, unsigned long long key) {
	uint64_t k = (uint64_t)key;
	return unordered_map_has(&table->vmap, k);
}

int __table_has_v(table_t *table, void* key) {
	uint64_t k = (uint64_t)key;
	return unordered_map_has(&table->vmap, k);
}

int __table_del_c(table_t *table, char* key) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k))
		return 0;
	if (!unordered_map_del(&table->vmap, k))
		return 0;
	uint64_t tmp;
	if (!unordered_map_get(&table->kmap, k, &tmp))
		return 0;
	free((void*)tmp);
	return unordered_map_del(&table->kmap, k);
}

int __table_del_cc(table_t *table, const char* key) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k))
		return 0;
	if (!unordered_map_del(&table->vmap, k))
		return 0;
	uint64_t tmp;
	if (!unordered_map_get(&table->kmap, k, &tmp))
		return 0;
	free((void*)tmp);
	return unordered_map_del(&table->kmap, k);
}

int __table_del_uc(table_t *table, unsigned char* key) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k))
		return 0;
	if (!unordered_map_del(&table->vmap, k))
		return 0;
	uint64_t tmp;
	if (!unordered_map_get(&table->kmap, k, &tmp))
		return 0;
	free((void*)tmp);
	return unordered_map_del(&table->kmap, k);
}

int __table_del_cuc(table_t *table, const unsigned char* key) {
	uint64_t k = table->hashfn((void*)key, strlen((const char*)key), table->seed);
	if (!unordered_map_has(&table->vmap, k))
		return 0;
	if (!unordered_map_del(&table->vmap, k))
		return 0;
	uint64_t tmp;
	if (!unordered_map_get(&table->kmap, k, &tmp))
		return 0;
	free((void*)tmp);
	return unordered_map_del(&table->kmap, k);
}

int __table_del_s8(table_t *table, char key) {
	uint64_t k = (uint64_t)key;
	if (!unordered_map_has(&table->vmap, k))
		return 0;
	if (!unordered_map_del(&table->vmap, k))
		return 0;
	uint64_t tmp;
	if (!unordered_map_get(&table->kmap, k, &tmp))
		return 0;
	free((void*)tmp);
	return unordered_map_del(&table->kmap, k);
}

int __table_del_s16(table_t *table, short key) {
	uint64_t k = (uint64_t)key;
	if (!unordered_map_has(&table->vmap, k))
		return 0;
	if (!unordered_map_del(&table->vmap, k))
		return 0;
	uint64_t tmp;
	if (!unordered_map_get(&table->kmap, k, &tmp))
		return 0;
	free((void*)tmp);
	return unordered_map_del(&table->kmap, k);
}

int __table_del_s32(table_t *table, int key) {
	uint64_t k = (uint64_t)key;
	if (!unordered_map_has(&table->vmap, k))
		return 0;
	if (!unordered_map_del(&table->vmap, k))
		return 0;
	uint64_t tmp;
	if (!unordered_map_get(&table->kmap, k, &tmp))
		return 0;
	free((void*)tmp);
	return unordered_map_del(&table->kmap, k);
}

int __table_del_sl32(table_t *table, long key) {
	uint64_t k = (uint64_t)key;
	if (!unordered_map_has(&table->vmap, k))
		return 0;
	if (!unordered_map_del(&table->vmap, k))
		return 0;
	uint64_t tmp;
	if (!unordered_map_get(&table->kmap, k, &tmp))
		return 0;
	free((void*)tmp);
	return unordered_map_del(&table->kmap, k);
}

int __table_del_s64(table_t *table, long long key) {
	uint64_t k = (uint64_t)key;
	if (!unordered_map_has(&table->vmap, k))
		return 0;
	if (!unordered_map_del(&table->vmap, k))
		return 0;
	uint64_t tmp;
	if (!unordered_map_get(&table->kmap, k, &tmp))
		return 0;
	free((void*)tmp);
	return unordered_map_del(&table->kmap, k);
}

int __table_del_u8(table_t *table, unsigned char key) {
	uint64_t k = (uint64_t)key;
	if (!unordered_map_has(&table->vmap, k))
		return 0;
	if (!unordered_map_del(&table->vmap, k))
		return 0;
	uint64_t tmp;
	if (!unordered_map_get(&table->kmap, k, &tmp))
		return 0;
	free((void*)tmp);
	return unordered_map_del(&table->kmap, k);
}

int __table_del_u16(table_t *table, unsigned short key) {
	uint64_t k = (uint64_t)key;
	if (!unordered_map_has(&table->vmap, k))
		return 0;
	if (!unordered_map_del(&table->vmap, k))
		return 0;
	uint64_t tmp;
	if (!unordered_map_get(&table->kmap, k, &tmp))
		return 0;
	free((void*)tmp);
	return unordered_map_del(&table->kmap, k);
}

int __table_del_u32(table_t *table, unsigned int key) {
	uint64_t k = (uint64_t)key;
	if (!unordered_map_has(&table->vmap, k))
		return 0;
	if (!unordered_map_del(&table->vmap, k))
		return 0;
	uint64_t tmp;
	if (!unordered_map_get(&table->kmap, k, &tmp))
		return 0;
	free((void*)tmp);
	return unordered_map_del(&table->kmap, k);
}

int __table_del_ul32(table_t *table, unsigned long key) {
	uint64_t k = (uint64_t)key;
	if (!unordered_map_has(&table->vmap, k))
		return 0;
	if (!unordered_map_del(&table->vmap, k))
		return 0;
	uint64_t tmp;
	if (!unordered_map_get(&table->kmap, k, &tmp))
		return 0;
	free((void*)tmp);
	return unordered_map_del(&table->kmap, k);
}

int __table_del_u64(table_t *table, unsigned long long key) {
	uint64_t k = (uint64_t)key;
	if (!unordered_map_has(&table->vmap, k))
		return 0;
	if (!unordered_map_del(&table->vmap, k))
		return 0;
	uint64_t tmp;
	if (!unordered_map_get(&table->kmap, k, &tmp))
		return 0;
	free((void*)tmp);
	return unordered_map_del(&table->kmap, k);
}

int __table_del_v(table_t *table, void* key) {
	uint64_t k = (uint64_t)key;
	if (!unordered_map_has(&table->vmap, k))
		return 0;
	if (!unordered_map_del(&table->vmap, k))
		return 0;
	uint64_t tmp;
	if (!unordered_map_get(&table->kmap, k, &tmp))
		return 0;
	free((void*)tmp);
	return unordered_map_del(&table->kmap, k);
}

// END SOURCE



#endif
