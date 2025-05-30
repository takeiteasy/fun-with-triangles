/* garry.h -- https://github.com/takeiteasy/garry

 MIT License

 Copyright (c) 2024 George Watson

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE. */

// Based on stetchy_buffer.h by nothings https://github.com/nothings/stb/blob/master/deprecated/stretchy_buffer.h

#ifndef GARRY_HEADER
#define GARRY_HEADER
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>

#ifndef GARRY_REALLOC
#define GARRY_REALLOC(...) realloc(__VA_ARGS__)
#endif

#ifndef GARRY_FREE
#define GARRY_FREE(...) free(__VA_ARGS__)
#endif

#define __garry_raw(a)              ((int *) (a) - 2)
#define __garry_m(a)                __garry_raw(a)[0]
#define __garry_n(a)                __garry_raw(a)[1]

#define __garry_needgrow(a,n)       ((a)==0 || __garry_n(a)+n >= __garry_m(a))
#define __garry_grow(a,n)           __garry_growf((void **) &(a), (n), sizeof(*(a)))
#define __garry_maybegrow(a,n)      (__garry_needgrow(a,(n)) ? __garry_grow(a,n) : 0)

#define __garry_needshrink(a)       (__garry_m(a) > 4 && __garry_n(a) <= __garry_m(a) / 4)
#define __garry_maybeshrink(a)      (__garry_needshrink(a) ? __garry_shrink(a) : 0)
#define __garry_shrink(a)           __garry_shrinkf((void **) &(a), sizeof(*(a)))

#define garry_free(a)               ((a) ? GARRY_FREE(__garry_raw(a)),0 : 0)
#define garry_append(a, v)          (__garry_maybegrow(a,1), (a)[__garry_n(a)++] = (v))
#define garry_count(a)              ((a) ? __garry_n(a) : 0)
#define garry_insert(a, idx, v)     (__garry_maybegrow(a,1), __garry_memmove(&a[idx+1], &a[idx], (__garry_n(a)++ - idx) * sizeof(*(a))), a[idx] = (v))
#define garry_push(a, v)            (garry_insert(a,0,v))
#define garry_cdr(a)                (void*)(garry_count(a) > 1 ? &(a+1) : NULL)
#define garry_car(a)                (void*)((a) ? &(a)[0] : NULL)
#define garry_last(a)               (void*)((a) ? &(a)[__garry_n(a)-1] : NULL)
#define garry_pop(a)                (--__garry_n(a), __garry_maybeshrink(a))
#define garry_remove_at(a, idx)     (idx == __garry_n(a)-1 ? __garry_memmove(&a[idx], &a[idx+1], (--__garry_n(a) - idx) * sizeof(*(a))) : garry_pop(a), __garry_maybeshrink(a))
#define garry_shift(a)              (garry_remove_at(a, 0))
#define garry_clear(a)              ((a) ? (__garry_n(a) = 0) : 0, __garry_shrink(a))

void  __garry_growf(void **arr, int increment, int itemsize);
void  __garry_shrinkf(void **arr, int itemsize);
void* __garry_memmove(void *dst, const void *src, size_t n);

#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(__VA_ARGS__)))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL3(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL4(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL(...)  EVAL4(EVAL4(EVAL4(__VA_ARGS__)))

#define MAP_END(...)
#define MAP_OUT
#define MAP_COMMA ,

#define MAP_GET_END2() 0, MAP_END
#define MAP_GET_END1(...) MAP_GET_END2
#define MAP_GET_END(...) MAP_GET_END1
#define MAP_NEXT0(test, next, ...) next MAP_OUT

#define MAP_LIST_NEXT1(test, next) MAP_NEXT0(test, MAP_COMMA next, 0)
#define MAP_LIST_NEXT(test, next)  MAP_LIST_NEXT1(MAP_GET_END test, next)

#define MAP_LIST0_UD(f, userdata, x, peek, ...) f(x, userdata) MAP_LIST_NEXT(peek, MAP_LIST1_UD)(f, userdata, peek, __VA_ARGS__)
#define MAP_LIST1_UD(f, userdata, x, peek, ...) f(x, userdata) MAP_LIST_NEXT(peek, MAP_LIST0_UD)(f, userdata, peek, __VA_ARGS__)

#define MAP_LIST_UD(f, userdata, ...) EVAL(MAP_LIST1_UD(f, userdata, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

#define __garry_append_va(v, a)  (garry_append(a, v))
#define garry_append_va(a, ...) MAP_LIST_UD(__garry_append_va, a, __VA_ARGS__)
#define __garry_push_va(v, a)  (garry_push(a, v))
#define garry_push_va(a, ...) MAP_LIST_UD(__garry_push_va, a, __VA_ARGS__)

#if defined(__clang__) || defined(__GNUC__)
#ifndef __has_feature
#define __has_feature(x) 0
#endif
#if __has_feature(blocks)
#define BLOCKS
#endif
#if __STDC_VERSION__ >= 202311L
#define TYPEOF(X) typeof((X))
#elif (defined(__GNUC__) && (__GNUC__ > 10 || (__GNUC__ == 10 && __GNUC_MINOR__ >= 4))) || (defined(__clang__) && (__clang_major__ >= 15))
#define TYPEOF(X) __typeof__((X))
#endif
#endif

#if defined(BLOCKS) && defined(TYPEOF)
#define garry_shuffle(a)                    \
    do {                                    \
        int i, j, n = garry_count(a);       \
        TYPEOF(a[0]) tmp;                   \
        for (i = n - 1; i > 0; i--) {       \
            j = rand() % (i + 1);           \
            tmp  = a[j];                    \
            a[j] = a[i];                    \
            a[i] = tmp;                     \
        }                                   \
    } while(0)

#define garry_shuffled(a)                               \
    ^(TYPEOF(a) _a) {                                   \
        TYPEOF(a) result = NULL;                        \
        size_t n = garry_count(a);                      \
        int *indices = NULL;                            \
        for (int i = 0; i < n; i++)                     \
            garry_append(indices, i);                   \
        garry_shuffle(indices);                         \
        for (int i = 0; i < n; i++)                     \
            garry_append(result, a[indices[i]]);        \
        garry_free(indices);                            \
        return result;                                  \
    }(a)

#define garry(TYPE, ...)                                \
    ^(void) {                                           \
        TYPE *result = NULL;                            \
        garry_append_va(result, __VA_ARGS__);           \
        return result;                                  \
    }()

#define garry_map(a, fn)                                \
    ^(TYPEOF(a) _a) {                                   \
        TYPEOF(_a) result = NULL;                       \
        for (int i = 0; i < garry_count(_a); i++)       \
            garry_append(result, fn(_a[i]));            \
        return result;                                  \
    }(a)

#define garry_map_enumerate(a, fn)                      \
    ^(TYPEOF(a) _a) {                                   \
        TYPEOF(_a) result = NULL;                       \
        for (int i = 0; i < garry_count(_a); i++)       \
            garry_append(result, fn(i, _a[i]));         \
        return result;                                  \
    }(a)

#define garry_reduce(a, initial, fn)                    \
    ^(TYPEOF(a) _a) {                                   \
        TYPEOF(_a[0]) result = initial;                 \
        for (int i = 0; i < garry_count(_a); i++)       \
            fn(&result, _a[i]);                         \
        return result;                                  \
    }(a)

#define garry_reduce_enumerate(a, initial, fn)          \
    ^(TYPEOF(a) _a) {                                   \
        TYPEOF(_a[0]) result = initial;                 \
        for (int i = 0; i < garry_count(_a); i++)       \
            fn(i, &result, _a[i]);                      \
        return result;                                  \
    }(a)

#define garry_filter(a, v)                              \
    ^(TYPEOF(a) _a) {                                   \
        TYPEOF(a) result = NULL;                        \
        for (int i = 0; i < garry_count(_a); i++)       \
            if (_a[i] == v)                             \
                garry_append(result, _a[i]);            \
        return result;                                  \
    }(a)

#define garry_filter_fn(a, fn)                          \
    ^(TYPEOF(a) _a) {                                   \
        TYPEOF(a) result = NULL;                        \
        for (int i = 0; i < garry_count(_a); i++)       \
            if (fn(_a[i]))                              \
                garry_append(result, _a[i]);            \
        return result;                                  \
    }(a)

#define garry_filter_indices(a, v)                      \
    ^(TYPEOF(a) _a) {                                   \
        int *result = NULL;                             \
        for (int i = 0; i < garry_count(_a); i++)       \
            if (_a[i] == v)                             \
                garry_append(result, i);                \
        return result; \
    }(a)

#define garry_filter_indices_fn(a, fn)                  \
    ^(TYPEOF(a) _a) {                                   \
        int *result = NULL;                             \
        for (int i = 0; i < garry_count(_a); i++)       \
            if (fn(i, _a[i]))                           \
                garry_append(result, i);                \
        return result;                                  \
    }(a)

#define garry_sample(a, n) \
    ^(TYPEOF(a) _a) {                               \
        TYPEOF(a) result = NULL;                    \
        int l = garry_count(a);                     \
        if (n > l || !n || !l)                      \
            return result;                          \
        if (n == l)                                 \
            return a;                               \
        int x[n], j = 0;                            \
        for (int i = 0; i < n; i++)                 \
            x[i] = 0;                               \
        for (int i = 0; i < n; i++) {               \
            int y = rand() / (RAND_MAX / n + 1);    \
            int alreadyFound = 0;                   \
            for (int k = 0; k < j; k++)             \
                if (x[k] == y) {                    \
                    alreadyFound = 1;               \
                    break;                          \
                }                                   \
            if (alreadyFound)                       \
                i--;                                \
            else {                                  \
                garry_append(result, _a[y]);        \
                x[j++] = y;                         \
            }                                       \
        }                                           \
        return result;                              \
    }(a)

#define garry_slice(a, start, end)              \
    ^(TYPEOF(a) _a, int _min, int _max) {       \
        TYPEOF(a) result = NULL;                \
        if (_min < 0)                           \
            _min = 0;                           \
        int l = garry_count(_a);                \
        if (_max > l)                           \
            _max = l;                           \
        if (_min >= _max)                       \
            return result;                      \
        for (int i = _min; i < _max; i++)       \
            garry_append(result, _a[i]);        \
        return result;                          \
    }(a, start, end)

#define garry_unique(a)                             \
    ^(TYPEOF(a) _a) {                               \
        TYPEOF(a) result = NULL;                    \
        int i, j;                                   \
        for (i = 0; i < garry_count(_a); i++) {     \
            for (j = 0; j < i; j++)                 \
                if (_a[i] == _a[j])                 \
                    break;                          \
            if (i == j)                             \
                garry_append(result, _a[i]);        \
        }                                           \
        return result;                              \
    }(a)

#define garry_unique_fn(a, fn)                      \
    ^(TYPEOF(a) _a) {                               \
        TYPEOF(a) result = NULL;                    \
        int i, j;                                   \
        for (i = 0; i < garry_count(_a); i++) { \
            for (j = 0; j < i; j++)                 \
                if (fn(_a[i], _a[j]))               \
                    break;                          \
            if (i == j)                             \
                garry_append(result, _a[i]);        \
        }                                           \
        return result;                              \
    }(a)

#define garry_clone(a)                              \
    ^(TYPEOF(a) _a) {                               \
        TYPEOF(_a) result = NULL;                   \
        for (int i = 0; i < garry_count(_a); i++)   \
            garry_append(result, _a[i]);            \
        return result;                              \
    }(a)

#define garry_reverse(a)                \
    do {                                \
        TYPEOF(a[0]) tmp;               \
        int len = garry_count(a);       \
        for (int i = 0; i < len; i++) { \
            int j = (len - i) - 1;      \
            if (i == j)                 \
                continue;               \
            tmp  = a[i];                \
            a[i] = a[j];                \
            a[j] = tmp;                 \
        }                               \
    } while(0)

#define garry_reversed(a)                    \
    ^(TYPEOF(a) _a) {                        \
        TYPEOF(_a) result = garry_clone(_a); \
        garry_reverse(result);               \
        return result;                       \
    }(a)
#endif

#ifdef __cplusplus
}
#endif
#endif // GARRY_HEADER

#ifdef GARRY_IMPLEMENTATION
void __garry_growf(void **arr, int increment, int itemsize) {
    int m = *arr ? 2 * __garry_m(*arr) + increment : increment + 1;
    void *p = (void*)GARRY_REALLOC(*arr ? __garry_raw(*arr) : 0, itemsize * m + sizeof(int) * 2);
    assert(p);
    if (p) {
        if (!*arr)
            ((int *)p)[1] = 0;
        *arr = (void*)((int*)p + 2);
        __garry_m(*arr) = m;
    }
}

void __garry_shrinkf(void **arr, int itemsize) {
    int m = *arr ? __garry_m(*arr) / 2 : 0;
    void *p = (void*)GARRY_REALLOC(*arr ? __garry_raw(*arr) : 0, itemsize * m + sizeof(int) * 2);
    assert(p);
    if (p) {
        *arr = (void*)((int*)p + 2);
        __garry_m(*arr) = m;
    }
}

void* __garry_memmove(void *dst, const void *src, size_t n) {
    if (!dst || !src)
        return (void*)0;
    if (!n)
        return dst;
    char *_dest = (char*)dst;
    char *_src = (char*)src;
    if (dst <= src)
        while (n--)
            *_dest++ = *_src++;
    else if (dst > src) {
        _dest += n - 1;
        _src += n - 1;
        while (n--)
            *_dest-- = *_src--;
    }
    return dst;
}
#endif
