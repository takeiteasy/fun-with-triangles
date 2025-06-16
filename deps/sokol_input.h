/* sokol_input.h -- https://github.com/takeiteasy/suk

 The MIT License (MIT)
 
 Copyright (c) 2024 George Watson
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef SUK_INPUT
#define SUK_INPUT
#if defined(__cplusplus)
extern "C" {
#endif

#ifndef __has_include
#define __has_include(x) 1
#endif

#ifndef SOKOL_APP_INCLUDED
#if __has_include("sokol_app.h")
#include "sokol_app.h"
#else
#error Please include sokol_app.h before the sokol_input.h implementation
#endif
#endif

#ifndef SOKOL_TIME_INCLUDED
#if __has_include("sokol_time.h")
#include "sokol_time.h"
#else
#error Please include sokol_time.h before the sokol_input.h implementation
#endif
#endif

#include <stdarg.h>

// Assign sapp_desc.event_cb = sapp_input_event
// Or just pass the event to it inside the callback
void sapp_input_event(const sapp_event *event);
// Call this at the end of sapp frame callback
void sapp_input_flush(void);
// Clear the input internal state
void sapp_input_init(void);

bool sapp_is_key_down(int key);
// This will be true if a key is held for more than 1 second
// TODO: Make the duration configurable
bool sapp_is_key_held(int key);
// Returns true if key is down this frame and was up last frame
bool sapp_was_key_pressed(int key);
// Returns true if key is up and key was down last frame
bool sapp_was_key_released(int key);
// If any of the keys passed are not down returns false
bool sapp_are_keys_down(int n, ...);
// If none of the keys passed are down returns false
bool sapp_any_keys_down(int n, ...);
bool sapp_is_button_down(int button);
bool sapp_is_button_held(int button);
bool sapp_was_button_pressed(int button);
bool sapp_was_button_released(int button);
bool sapp_are_buttons_down(int n, ...);
bool sapp_any_buttons_down(int n, ...);
bool sapp_has_mouse_move(void);
bool sapp_modifier_equals(int mods);
bool sapp_modifier_down(int mod);
int sapp_cursor_x(void);
int sapp_cursor_y(void);
bool sapp_cursor_delta_x(void);
bool sapp_cursor_delta_y(void);
bool sapp_check_scrolled(void);
float sapp_scroll_x(void);
float sapp_scroll_y(void);

bool sapp_check_input_str_down(const char *str);
bool sapp_check_input_down(int modifiers, int n, ...);
bool sapp_check_input_str_released(const char *str);
bool sapp_check_input_released(int modifiers, int n, ...);
bool sapp_check_input_str_up(const char *str);
bool sapp_check_input_up(int modifiers, int n, ...);

#if defined(__cplusplus)
}
#endif
#endif // SUK_INPUT

#ifdef SUK_IMPL
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>

typedef struct {
    int down;
    uint64_t timestamp;
} input_action_state_t;

typedef struct {
    input_action_state_t keys[SAPP_KEYCODE_MENU+1];
    input_action_state_t buttons[3];
    int modifier;
    struct {
        int x, y;
    } cursor;
    struct {
        float x, y;
    } scroll;
} input_t;

#ifndef MAX_INPUT_STATE_KEYS
#define MAX_INPUT_STATE_KEYS 8
#endif

typedef struct input_state_t {
    int keys[MAX_INPUT_STATE_KEYS];
    int modifiers;
} input_state_t;

#ifndef DEFAULT_KEY_HOLD_DELAY
#define DEFAULT_KEY_HOLD_DELAY 1
#endif

static struct {
    input_t input_prev, input_current;
} state;

void sapp_input_init(void) {
    memset(&state.input_prev,    0, sizeof(input_t));
    memset(&state.input_current, 0, sizeof(input_t));
}

void sapp_input_event(const sapp_event* e) {
    switch (e->type) {
        case SAPP_EVENTTYPE_KEY_UP:
        case SAPP_EVENTTYPE_KEY_DOWN:
            state.input_current.keys[e->key_code].down = e->type == SAPP_EVENTTYPE_KEY_DOWN;
            state.input_current.keys[e->key_code].timestamp = stm_now();
            state.input_current.modifier = e->modifiers;
            break;
        case SAPP_EVENTTYPE_MOUSE_UP:
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            state.input_current.buttons[e->mouse_button].down = e->type == SAPP_EVENTTYPE_MOUSE_DOWN;
            state.input_current.buttons[e->mouse_button].timestamp = stm_now();
            break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            state.input_current.cursor.x = e->mouse_x;
            state.input_current.cursor.y = e->mouse_y;
            break;
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            state.input_current.scroll.x = e->scroll_x;
            state.input_current.scroll.y = e->scroll_y;
            break;
        default:
            state.input_current.modifier = e->modifiers;
            break;
    }
}

void sapp_input_flush(void) {
    memcpy(&state.input_prev, &state.input_current, sizeof(input_t));
    state.input_current.scroll.x = state.input_current.scroll.y = 0.f;
}

bool sapp_is_key_down(int key) {
    return state.input_current.keys[key].down == 1;
}

bool sapp_is_key_held(int key) {
    return sapp_is_key_down(key) && stm_sec(stm_since(state.input_current.keys[key].timestamp)) > 1;
}

bool sapp_was_key_pressed(int key) {
    return sapp_is_key_down(key) && !state.input_prev.keys[key].down;
}

bool sapp_was_key_released(int key) {
    return !sapp_is_key_down(key) && state.input_prev.keys[key].down;
}

bool sapp_are_keys_down(int n, ...) {
    va_list args;
    va_start(args, n);
    int result = 1;
    for (int i = 0; i < n; i++)
        if (!state.input_current.keys[va_arg(args, int)].down) {
            result = 0;
            goto BAIL;
        }
BAIL:
    va_end(args);
    return result;
}

bool sapp_any_keys_down(int n, ...) {
    va_list args;
    va_start(args, n);
    int result = 0;
    for (int i = 0; i < n; i++)
        if (state.input_current.keys[va_arg(args, int)].down) {
            result = 1;
            goto BAIL;
        }
BAIL:
    va_end(args);
    return result;
}

bool sapp_is_button_down(int button) {
    return state.input_current.buttons[button].down;
}

bool sapp_is_button_held(int button) {
    return sapp_is_button_down(button) && stm_sec(stm_since(state.input_current.buttons[button].timestamp)) > 1;
}

bool sapp_was_button_pressed(int button) {
    return sapp_is_button_down(button) && !state.input_prev.buttons[button].down;
}

bool sapp_was_button_released(int button) {
    return !sapp_is_button_down(button) && state.input_prev.buttons[button].down;
}

bool sapp_are_buttons_down(int n, ...) {
    va_list args;
    va_start(args, n);
    int result = 1;
    for (int i = 0; i < n; i++)
        if (!state.input_current.buttons[va_arg(args, int)].down) {
            result = 0;
            goto BAIL;
        }
BAIL:
    va_end(args);
    return result;
}

bool sapp_any_buttons_down(int n, ...) {
    va_list args;
    va_start(args, n);
    int result = 0;
    for (int i = 0; i < n; i++)
        if (state.input_current.buttons[va_arg(args, int)].down) {
            result = 1;
            goto BAIL;
        }
BAIL:
    va_end(args);
    return result;
}

bool sapp_modifier_equal(int mods) {
    return state.input_current.modifier == mods;
}

bool sapp_modifier_down(int mod) {
    return state.input_current.modifier & mod;
}

bool sapp_has_mouse_move(void) {
    return state.input_current.cursor.x != state.input_prev.cursor.x || state.input_current.cursor.y != state.input_prev.cursor.y;
}

int sapp_cursor_x(void) {
    return state.input_current.cursor.x;
}

int sapp_cursor_y(void) {
    return state.input_current.cursor.y;
}

bool sapp_cursor_delta_x(void) {
    return state.input_current.cursor.x - state.input_prev.cursor.x;
}

bool sapp_cursor_delta_y(void) {
    return state.input_current.cursor.y - state.input_prev.cursor.y;
}

bool sapp_check_scrolled(void) {
    return state.input_current.scroll.x != 0 || state.input_current.scroll.y != 0;
}

float sapp_scroll_x(void) {
    return state.input_current.scroll.x;
}

float sapp_scroll_y(void) {
    return state.input_current.scroll.y;
}

typedef struct input_str {
    const char *original;
    const char *offset;
    const char *cursor;
    int modifiers;
    int *keys;
} input_parser_t;

static bool parser_eof(input_parser_t *p) {
    return p->cursor || *p->cursor == '\0';
}

static char parser_advance(input_parser_t *p) {
    char c = *p->cursor;
    if (c != '\0')
        p->cursor++;
    return c;
}

static char parse_peek(input_parser_t *p) {
    return parser_eof(p) ? '\0' : *(p->cursor + 1);
}

static bool parser_is_whitespace(input_parser_t *p) {
    return *p->cursor == ' ';
}

static bool parse_word(input_parser_t *p) {
    p->offset = p->cursor;
    while (!parser_eof(p)) {
        switch (*p->cursor) {
            case 'a' ... 'z':
            case 'A' ... 'Z':
            case '0' ... '9':
                (void)parser_advance(p);
                break;
            default:
                if (!parser_is_whitespace(p))
                    return false;
            case '+':
            case ',':
            case ' ':
                return true;
        }
    }
    return true;
}

static int translate_char(input_parser_t *p) {
    switch (*p->offset) {
        case 'A' ... 'Z':
        case '0' ... '9':
            return (int)*p->offset;
        default:
            return -1;
    }
}

#define KEY_TESTS       \
    X(SPACE, 32),       \
    X(APOSTROPHE, 39),  \
    X(COMMA, 44),       \
    X(MINUS, 45),       \
    X(PERIOD, 46),      \
    X(SLASH, 47),       \
    X(SEMICOLON, 59),   \
    X(EQUAL, 61),       \
    X(LBRACKET, 91),    \
    X(BACKSLASH, 92),   \
    X(RBRACKET, 93),    \
    X(GRAVE, 96),       \
    X(WORLD1, 161),     \
    X(WORLD2, 162),     \
    X(ESCAPE, 256),     \
    X(ENTER, 257),      \
    X(TAB, 258),        \
    X(BACKSPACE, 259),  \
    X(INSERT, 260),     \
    X(DELETE, 261),     \
    X(RIGHT, 262),      \
    X(LEFT, 263),       \
    X(DOWN, 264),       \
    X(UP, 265),         \
    X(PGUP, 266),       \
    X(PGDN, 267),       \
    X(HOME, 268),       \
    X(END, 269),        \
    X(CAPSLOCK, 280),   \
    X(SCROLLLOCK, 281), \
    X(NUMLOCK, 282),    \
    X(PRNTSCRN, 283),   \
    X(PAUSE, 284),      \
    X(F1, 290),         \
    X(F2, 291),         \
    X(F3, 292),         \
    X(F4, 293),         \
    X(F5, 294),         \
    X(F6, 295),         \
    X(F7, 296),         \
    X(F8, 297),         \
    X(F9, 298),         \
    X(F10, 299),        \
    X(F11, 300),        \
    X(F12, 301),        \
    X(F13, 302),        \
    X(F14, 303),        \
    X(F15, 304),        \
    X(F16, 305),        \
    X(F17, 306),        \
    X(F18, 307),        \
    X(F19, 308),        \
    X(F20, 309),        \
    X(F21, 310),        \
    X(F22, 311),        \
    X(F23, 312),        \
    X(F24, 313),        \
    X(F25, 314),        \
    X(MENU, 348)

#define MOD_TESTS      \
    X(SHIFT, 340),     \
    X(CONTROL, 341),   \
    X(CTRL, 341),      \
    X(ALT, 342),       \
    X(SUPER, 343),     \
    X(CMD, 343),       \
    X(LSHIFT, 340),    \
    X(LCONTROL, 341),  \
    X(LCTRL, 341),     \
    X(LALT, 342),      \
    X(LSUPER, 343),    \
    X(LCMD, 343),      \
    X(RSHIFT, 344),    \
    X(RCONTROL, 345),  \
    X(RCTRL, 345),     \
    X(RALT, 346),      \
    X(RSUPER, 347),    \
    X(RCMD, 347)

static int translate_word(input_parser_t *p, bool *is_modifier) {
    int len = (int)(p->cursor - p->offset);
    char *buf = calloc(len + 1, sizeof(char));
    memcpy(buf, p->offset, sizeof(char));
    buf[len] = '\0';
    for (char *c = buf; *c != '\0'; c++)
        if (*c >= 'a' && *c <= 'z')
            *c -= 32;
#define X(NAME, VAL)               \
    if (!strncmp(buf, #NAME, len)) \
    {                              \
        if (is_modifier)           \
            *is_modifier = false;  \
        free(buf);                 \
        return VAL;                \
    }                              \
    KEY_TESTS
#undef X
#define X(NAME, VAL)               \
    if (!strncmp(buf, #NAME, len)) \
    {                              \
        if (is_modifier)           \
            *is_modifier = true;   \
        free(buf);                 \
        return VAL;                \
    }                              \
    MOD_TESTS
#undef X
    free(buf);
    return -1;
}

#define __vector_raw(a)              ((int *) (a) - 2)
#define __vector_m(a)                __vector_raw(a)[0]
#define __vector_n(a)                __vector_raw(a)[1]

#define __vector_needgrow(a,n)       ((a)==0 || __vector_n(a)+n >= __vector_m(a))
#define __vector_grow(a,n)           __grow_vector((void **) &(a), (n), sizeof(*(a)))
#define __vector_maybegrow(a,n)      (__vector_needgrow(a,(n)) ? __vector_grow(a,n) : 0)

#define vector_free(a)               ((a) ? free(__vector_raw(a)),0 : 0)
#define vector_append(a, v)          (__vector_maybegrow(a,1), (a)[__vector_n(a)++] = (v))
#define vector_count(a)              ((a) ? __vector_n(a) : 0)

static void __grow_vector(void **arr, int increment, int itemsize) {
    int m = *arr ? 2 * __vector_m(*arr) + increment : increment + 1;
    void *p = (void*)realloc(*arr ? __vector_raw(*arr) : 0, itemsize * m + sizeof(int) * 2);
    assert(p);
    if (p) {
        if (!*arr)
            ((int *)p)[1] = 0;
        *arr = (void*)((int*)p + 2);
        __vector_m(*arr) = m;
    }
}

static void parser_add_key(input_parser_t *p, int key) {
    bool found = false;
    for (int i = 0; i < vector_count(p->keys); i++)
        if (p->keys[i] == key) {
            found = true;
            break;
        }
    if (!found)
        vector_append(p->keys, key);
}

static bool parse_input_str(input_parser_t *p) {
    while (!parser_eof(p)) {
        p->offset = p->cursor;
        while (parser_is_whitespace(p))
            if (parser_advance(p) == '\0')
                return true;
        switch (*p->cursor) {
            case 'a' ... 'z':
            case 'A' ... 'Z':
                if (!parse_word(p))
                    goto BAIL;
                switch ((int)(p->cursor - p->offset)) {
                    case 0:
                        goto BAIL;
                    case 1:;
                        int key = translate_char(p);
                        if (key == -1)
                            goto BAIL;
                        parser_add_key(p, key);
                        break;
                    default:;
                        bool is_modifier = false;
                        int wkey = translate_word(p, &is_modifier);
                        if (wkey == -1)
                            goto BAIL;
                        if (is_modifier)
                            p->modifiers |= wkey;
                        else
                            parser_add_key(p, wkey);
                        break;
                }
                break;
            case '0' ... '9':;
                int nkey = translate_char(p);
                if (nkey == -1)
                    goto BAIL;
                parser_add_key(p, nkey);
                break;
            case '+':
            case ',':
                (void)parser_advance(p);
                break;
            default:
                goto BAIL;
        }
    }
    return true;
BAIL:
    fprintf(stderr, "[FAILED TO PARSE INPUT STRING] parse_input_str cannot read '%s'\n", p->original);
    if (p->keys)
        vector_free(p->keys);
    return false;
}

static int* _vaargs(int n, va_list args) {
    int *result = NULL;
    for (int i = 0; i < n; i++) {
        bool found = false;
        int key = va_arg(args, int);
        for (int j = 0; j < vector_count(result); j++)
            if (key == result[j]) {
                found = true;
                break;
            }
        if (!found)
            vector_append(result, key);
    }
    va_end(args);
    return result;
}

#define _MIN(A, B) ((A) < (B) ? (A) : (B))

static bool sapp_create_input(input_state_t *dst, int modifiers, int n, ...) {
    dst->modifiers = modifiers;
    va_list args;
    va_start(args, n);
    int *tmp = _vaargs(n, args);
    if (tmp) {
        vector_free(tmp);
        return false;
    }
    for (int i = 0; i < MAX_INPUT_STATE_KEYS; i++)
        dst->keys[i] = -1;
    int max = _MIN(vector_count(tmp), MAX_INPUT_STATE_KEYS);
    memcpy(dst->keys, tmp, max * sizeof(int));
    vector_free(tmp);
    return true;
}

static bool sapp_create_input_str(input_state_t *dst, const char *str) {
    dst->modifiers = -1;
    for (int i = 0; i < MAX_INPUT_STATE_KEYS; i++)
        dst->keys[i] = -1;

    input_parser_t p;
    memset(&p, 0, sizeof(input_parser_t));
    p.original = p.offset = p.cursor = str;
    if (!parse_input_str(&p) || (!p.modifiers && !p.keys))
        return false;
    if (p.modifiers)
        dst->modifiers = p.modifiers;
    if (p.keys) {
        int max = _MIN(vector_count(p.keys), MAX_INPUT_STATE_KEYS);
        memcpy(dst->keys, p.keys, max * sizeof(int));
        vector_free(p.keys);
    }
    return true;
}

bool sapp_check_input_str_down(const char *str) {
    input_parser_t p;
    memset(&p, 0, sizeof(input_parser_t));
    p.original = p.offset = p.cursor = str;
    if (!parse_input_str(&p) || (!p.modifiers && !p.keys))
        return false;
    bool mod_check = true;
    bool key_check = true;
    if (p.modifiers)
        mod_check = sapp_modifier_equal(p.modifiers);
    if (p.keys) {
        for (int i = 0; i < vector_count(p.keys); i++)
            if (!sapp_is_key_down(p.keys[i])) {
                key_check = false;
                break;
            }
        vector_free(p.keys);
    }
    return mod_check && key_check;
}

bool sapp_check_input_down(int modifiers, int n, ...) {
    int *tmp = NULL;
    bool result = false;
    if (modifiers != 0)
        if (!sapp_modifier_equal(modifiers))
            goto BAIL;
    if (!n)
        goto BAIL;
    va_list args;
    va_start(args, n);
    if (!(tmp = _vaargs(n, args)))
        goto BAIL;
    bool check = true;
    for (int i = 0; i < vector_count(args); i++)
        if (!sapp_is_key_down(tmp[i])) {
            check = false;
            break;
        }
    va_end(args);
    if (check)
        result = true;
BAIL:
    if (tmp)
        vector_free(tmp);
    return result;
}

bool sapp_check_input_str_released(const char *str) {
    input_parser_t p;
    memset(&p, 0, sizeof(input_parser_t));
    p.original = p.offset = p.cursor = str;
    if (!parse_input_str(&p) || (!p.modifiers && !p.keys))
        return false;
    bool mod_check = true;
    bool key_check = true;
    if (p.modifiers)
        mod_check = sapp_modifier_equal(p.modifiers);
    if (p.keys) {
        for (int i = 0; i < vector_count(p.keys); i++)
            if (!sapp_was_key_released(p.keys[i])) {
                key_check = false;
                break;
            }
        vector_free(p.keys);
    }
    return mod_check || key_check;
}

bool sapp_check_input_released(int modifiers, int n, ...) {
    int *tmp = NULL;
    bool result = false;
    if (modifiers != 0)
        if (!sapp_modifier_equal(modifiers))
            goto BAIL;
    if (!n)
        goto BAIL;
    va_list args;
    va_start(args, n);
    if (!(tmp = _vaargs(n, args)))
        goto BAIL;
    bool check = true;
    for (int i = 0; i < vector_count(args); i++)
        if (!sapp_was_key_released(tmp[i])) {
            check = false;
            break;
        }
    va_end(args);
    if (check)
        result = true;
BAIL:
    if (tmp)
        vector_free(tmp);
    return result;
}

bool sapp_check_input_str_up(const char *str) {
    input_parser_t p;
    memset(&p, 0, sizeof(input_parser_t));
    p.original = p.offset = p.cursor = str;
    if (!parse_input_str(&p) || (!p.modifiers && !p.keys))
        return false;
    bool mod_check = true;
    bool key_check = true;
    if (p.modifiers)
        if ((mod_check = sapp_modifier_equal(p.modifiers)))
            return false;
    if (p.keys) {
        for (int i = 0; i < vector_count(p.keys); i++)
            if (sapp_is_key_down(p.keys[i])) {
                key_check = false;
                break;
            }
        vector_free(p.keys);
    }
    return mod_check && key_check;
}

bool sapp_check_input_up(int modifiers, int n, ...) {
    int *tmp = NULL;
    bool result = false;
    if (modifiers != 0)
        if (sapp_modifier_equal(modifiers))
            goto BAIL;
    if (!n)
        goto BAIL;
    va_list args;
    va_start(args, n);
    if (!(tmp = _vaargs(n, args)))
        goto BAIL;
    bool check = true;
    for (int i = 0; i < vector_count(args); i++)
        if (sapp_is_key_down(tmp[i])) {
            check = false;
            break;
        }
    va_end(args);
    if (check)
        result = true;
BAIL:
    if (tmp)
        vector_free(tmp);
    return result;
}
#endif // SUK_IMPL
