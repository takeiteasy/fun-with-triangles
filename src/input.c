/* input.c -- https://github.com/takeiteasy/fun-with-triangles

 fun-with-triangles

 Copyright (C) 2025  George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include "fwt.h"
#define GARRY_IMPLEMENTATION
#include "garry.h"
#include "sokol/sokol_app.h"
#include "sokol/sokol_time.h"

typedef struct {
    bool down;
    uint64_t timestamp;
} sapp_keystate_t;

typedef struct {
    sapp_keystate_t keys[SAPP_KEYCODE_MENU+1];
    sapp_keystate_t buttons[3];
    int modifier;
    struct {
        int x, y;
    } cursor;
    struct {
        float x, y;
    } scroll;
} sapp_input_t;

static struct {
    sapp_input_t input_prev, input_current;
} state;

void sapp_input_clear(void) {
    memset(&state.input_prev,    0, sizeof(sapp_input_t));
    memset(&state.input_current, 0, sizeof(sapp_input_t));
}

void sapp_input_handler(const sapp_event* e) {
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

void sapp_input_update(void) {
    memcpy(&state.input_prev, &state.input_current, sizeof(sapp_input_t));
    state.input_current.scroll.x = state.input_current.scroll.y = 0.f;
}

bool fwtIsKeyDown(int key) {
    return state.input_current.keys[key].down == 1;
}

bool fwtIsKeyHeld(int key) {
    return fwtIsKeyDown(key) && stm_sec(stm_since(state.input_current.keys[key].timestamp)) > 1;
}

bool fwtWasKeyPressed(int key) {
    return fwtIsKeyDown(key) && !state.input_prev.keys[key].down;
}

bool fwtWasKeyReleased(int key) {
    return !fwtIsKeyDown(key) && state.input_prev.keys[key].down;
}

bool fwtIsKeyReleased(int key) {
    return !fwtIsKeyDown(key) && state.input_prev.keys[key].down;
}

bool __fwtAreKeysDown(int n, ...) {
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

bool __fwtAnyKeysDown(int n, ...) {
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

bool fwtIsButtonDown(int button) {
    return state.input_current.buttons[button].down;
}

bool fwtIsButtonHeld(int button) {
    return fwtIsButtonDown(button) && stm_sec(stm_since(state.input_current.buttons[button].timestamp)) > 1;
}

bool fwtWasButtonPressed(int button) {
    return fwtIsButtonDown(button) && !state.input_prev.buttons[button].down;
}

bool fwtWasButtonReleased(int button) {
    return !fwtIsButtonDown(button) && state.input_prev.buttons[button].down;
}

bool __fwtAreButtonsDown(int n, ...) {
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

bool __fwtAnyButtonsDown(int n, ...) {
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

bool fwtModifiedEquals(int mods) {
    return state.input_current.modifier == mods;
}

bool fwtModifierDown(int mod) {
    return state.input_current.modifier & mod;
}

bool fwtHasMouseMove(void) {
    return state.input_current.cursor.x != state.input_prev.cursor.x || state.input_current.cursor.y != state.input_prev.cursor.y;
}

int fwtCursorX(void) {
    return state.input_current.cursor.x;
}

int fwtCursorY(void) {
    return state.input_current.cursor.y;
}

bool fwtCursorDeltaX(void) {
    return state.input_current.cursor.x - state.input_prev.cursor.x;
}

bool fwtCursorDeltaY(void) {
    return state.input_current.cursor.y - state.input_prev.cursor.y;
}

bool fwtCheckScrolled(void) {
    return state.input_current.scroll.x != 0 || state.input_current.scroll.y != 0;
}

float fwtScrollX(void) {
    return state.input_current.scroll.x;
}

float fwtScrollY(void) {
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

static void parser_add_key(input_parser_t *p, int key) {
    bool found = false;
    for (int i = 0; i < garry_count(p->keys); i++)
        if (p->keys[i] == key) {
            found = true;
            break;
        }
    if (!found)
        garry_append(p->keys, key);
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
        garry_free(p->keys);
    return false;
}

bool fwtCheckInput(const char *str) {
    input_parser_t p;
    memset(&p, 0, sizeof(input_parser_t));
    p.original = p.offset = p.cursor = str;
    if (!parse_input_str(&p) || (!p.modifiers && !p.keys))
        return false;
    bool mod_check = true;
    bool key_check = true;
    if (p.modifiers)
        mod_check = fwtModifiedEquals(p.modifiers);
    if (p.keys) {
        for (int i = 0; i < garry_count(p.keys); i++)
            if (!fwtIsKeyDown(p.keys[i])) {
                key_check = false;
                break;
            }
        garry_free(p.keys);
    }
    return mod_check && key_check;
}

static int* _vaargs(int n, va_list args) {
    int *result = NULL;
    for (int i = 0; i < n; i++) {
        bool found = false;
        int key = va_arg(args, int);
        for (int j = 0; j < garry_count(result); j++)
            if (key == result[j]) {
                found = true;
                break;
            }
        if (!found)
            garry_append(result, key);
    }
    va_end(args);
    return result;
}

#define MIN(a, b)  ((a) < (b) ? (a) : (b))
#define MAX(a, b)  ((a) > (b) ? (a) : (b))

void __fwtCreateInput(fwtInputState *dst, int modifiers, int n, ...) {
    dst->modifiers = modifiers;
    va_list args;
    va_start(args, n);
    int *tmp = _vaargs(n, args);
    if (tmp)
        garry_free(tmp);
    for (int i = 0; i < MAX_INPUT_STATE_KEYS; i++)
        dst->keys[i] = -1;
    int max = MIN(garry_count(tmp), MAX_INPUT_STATE_KEYS);
    memcpy(dst->keys, tmp, max * sizeof(int));
    garry_free(tmp);
}

bool fwtCreateInputFromString(fwtInputState *dst, const char *str) {
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
        int max = MIN(garry_count(p.keys), MAX_INPUT_STATE_KEYS);
        memcpy(dst->keys, p.keys, max * sizeof(int));
        garry_free(p.keys);
    }
    return true;
}

bool fwtCheckInputState(fwtInputState *istate) {
    bool mod_check = true;
    bool key_check = true;
    if (istate->modifiers > 0)
        mod_check = fwtModifiedEquals(istate->modifiers);
    if (istate->keys > 0)
        for (int i = 0; i < garry_count(istate->keys); i++)
            if (!fwtIsKeyDown(istate->keys[i])) {
                key_check = false;
                break;
            }
    return mod_check && key_check;
}
