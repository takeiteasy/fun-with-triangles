/* fwt.c -- https://github.com/takeiteasy/fun-with-triangles

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
#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"
#define SOKOL_NO_ENTRY
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_app.h"
#include "sokol/sokol_glue.h"
#include "sokol/sokol_time.h"
#include "sokol/sokol_log.h"
#include "shader.glsl.h"
#define GARRY_IMPLEMENTATION
#include "garry.h"
#define QOI_IMPLEMENTATION
#include "qoi.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_NO_GIF
#include "stb_image.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define ZIP_C
#include "zip.h"
#define DIR_C
#include "dir.h"
#pragma clang diagnostic pop

#define PLATFORM_POSIX
#if defined(__APPLE__) || defined(__MACH__)
#define PLATFORM_MAC
#elif defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#if !defined(PLATFORM_FORCE_POSIX)
#undef PLATFORM_POSIX
#endif
#elif defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
#define PLATFORM_LINUX
#else
#error Unknown platform
#endif

#include <assert.h>
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <io.h>
#include <dirent.h>
#define F_OK 0
#define access _access
#define chdir _chdir
#include "getopt_win32.h"
#ifndef _MSC_VER
#pragma comment(lib, "Psapi.lib")
#endif
#else
#include <getopt.h>
#define _BSD_SOURCE // usleep()
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#ifdef PLATFORM_MAC
#include <Cocoa/Cocoa.h>
#endif
#endif

#if !defined(DEFAULT_WINDOW_WIDTH)
#define DEFAULT_WINDOW_WIDTH 640
#endif

#if !defined(DEFAULT_WINDOW_HEIGHT)
#define DEFAULT_WINDOW_HEIGHT 480
#endif

#if !defined(DEFAULT_WINDOW_TITLE)
#define DEFAULT_WINDOW_TITLE "fun-with-triangles"
#endif

#if !defined(DEFAULT_KEY_HOLD_DELAY)
#define DEFAULT_KEY_HOLD_DELAY 1
#endif

#if !defined(MAX_MATRIX_STACK)
#define MAX_MATRIX_STACK 32
#endif

typedef struct {
    hmm_mat4 stack[MAX_MATRIX_STACK];
    int count;
} fwt_matrix_stack_t;

typedef struct {
    hmm_vec4 x, y, z, w;
} fwt_vs_inst_t;

typedef struct {
    hmm_vec4 position;
    hmm_vec3 normal;
    hmm_vec2 texcoord;
    hmm_vec4 color;
} fwt_vertex_t;

enum {
    FWT_CMD_VIEWPORT,
    FWT_CMD_SCISSOR_RECT,
    FWT_CMD_DRAW_CALL
};

typedef struct {
    float x, y, w, h;
} fwt_rect_t;

typedef struct {
    fwt_vertex_t *vertices;
    int vcount;
    fwt_vs_inst_t *instances;
    int icount;
    hmm_mat4 projection, texture_matrix;
    sg_pipeline pip;
    sg_bindings bind;
    sg_image texture;
} fwt_draw_call_t;

typedef struct {
    fwt_matrix_stack_t matrix_stack[FWT_MATRIXMODE_COUNT];
    int matrix_mode;
    sg_color clear_color;
    fwt_draw_call_t draw_call;
    fwt_vertex_t current_vertex;
    sg_pipeline_desc pip_desc;
    sg_blend_state blend;
    int blend_mode;
    sg_image current_texture;
    sg_sampler_desc sampler_desc;
} fwt_state_t;

typedef struct fwt_command_t {
    void *data;
    int type;
    struct fwt_command_t *next;
} fwt_command_t;

typedef struct {
    fwt_command_t *head, *tail;
} fwt_command_queue_t;

typedef struct vfs_dir {
    char* path;
    // const
    zip* archive;
    int is_directory;
    struct vfs_dir *next;
} vfs_dir;

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

typedef struct {
    int running;
    int mouse_hidden;
    int mouse_locked;
    sapp_desc app_desc;
    fwt_state_t state;
    fwt_command_queue_t commands;
    sg_shader shader;
    vfs_dir *dir_head;
    sapp_input_t input_prev, input_current;
} fwt_t;

static fwt_t fwt = {0};

void sapp_input_clear(void);
void sapp_input_update(void);
void sapp_input_handler(const sapp_event* e);

void fwt_set_current_matrix(hmm_mat4 mat);
hmm_mat4* fwt_current_matrix(void);
void fwt_push_command(int type, void *data);
void fwt_push_vertex(void);
hmm_mat4* fwt_matrix_stack_head(int mode);

#ifdef FWT_MAIN_PROGRAM
static struct {
#if defined(PLATFORM_WINDOWS)
    FILETIME writeTime;
#else
    ino_t handleID;
#endif
    void *handle;
    fwtState *state;
    fwtScene *scene;
    struct {
        unsigned int width;
        unsigned int height;
        const char *title;
        char *path;
    } args;
} state;

static void init(void) {
    sg_desc desc = {
        .environment = sglue_environment(),
        .logger.func = slog_func,
        .buffer_pool_size = 256
    };
    sg_setup(&desc);
    stm_setup();

#ifdef FWT_DEFAULT_WORKING_PATH
    fwtSetWorkingPath(FWT_DEFAULT_WORKING_PATH);
#endif

    fwt.shader = sg_make_shader(fwt_shader_desc(sg_query_backend()));
    fwt.state.pip_desc = (sg_pipeline_desc) {
        .layout = {
            .buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE,
            .attrs = {
                [ATTR_fwt_position] = { .format=SG_VERTEXFORMAT_FLOAT4, .buffer_index=0 },
                [ATTR_fwt_normal] = { .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index=0 },
                [ATTR_fwt_texcoord] = { .format=SG_VERTEXFORMAT_FLOAT2, .buffer_index=0 },
                [ATTR_fwt_color_v] = { .format=SG_VERTEXFORMAT_FLOAT4, .buffer_index=0 },
                [ATTR_fwt_inst_mat_x] = { .format=SG_VERTEXFORMAT_FLOAT4, .buffer_index=1 },
                [ATTR_fwt_inst_mat_y] = { .format=SG_VERTEXFORMAT_FLOAT4, .buffer_index=1 },
                [ATTR_fwt_inst_mat_z] = { .format=SG_VERTEXFORMAT_FLOAT4, .buffer_index=1 },
                [ATTR_fwt_inst_mat_w] = { .format=SG_VERTEXFORMAT_FLOAT4, .buffer_index=1 }
            }
        },
        .shader = fwt.shader,
        .cull_mode = SG_CULLMODE_BACK,
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true
        }
    };

    for (int i = 0; i < FWT_MATRIXMODE_COUNT; i++) {
        fwt_matrix_stack_t *stack = &fwt.state.matrix_stack[i];
        stack->count = 1;
        stack->stack[0] = HMM_Mat4();
    }

    fwt.state.matrix_stack[FWT_MATRIXMODE_TEXTURE].stack[0] = HMM_Mat4d(1.f);
    fwt.state.blend_mode = -1;
    fwtBlendMode(FWT_BLEND_DEFAULT);
    fwt.state.pip_desc.depth.compare = -1;
    fwtDepthFunc(FWT_CMP_DEFAULT);
    fwt.state.pip_desc.cull_mode = -1;
    fwtCullMode(FWT_CULL_DEFAULT);

    sapp_input_clear();
    if (state.scene->init)
        state.scene->init();
}

static void frame(void) {
    const float t = (float)(sapp_frame_duration() * 60.);
    if (state.scene->tick)
        state.scene->tick(state.state, t);

    sg_begin_pass(&(sg_pass) {
        .action = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = fwt.state.clear_color
            }
        },
        .swapchain = sglue_swapchain()
    });
    fwt_command_t *cursor = fwt.commands.head;
    while (cursor) {
        fwt_command_t *tmp = cursor->next;
        switch (cursor->type) {
            case FWT_CMD_VIEWPORT:
            case FWT_CMD_SCISSOR_RECT:;
                fwt_rect_t *rect = (fwt_rect_t*)cursor->data;
                if (cursor->type == FWT_CMD_VIEWPORT)
                    fwtViewport(rect->x, rect->y, rect->w, rect->h);
                else
                    fwtScissor(rect->x, rect->y, rect->w, rect->h);
                break;
            case FWT_CMD_DRAW_CALL:;
                fwt_draw_call_t *call = (fwt_draw_call_t*)cursor->data;
                sg_apply_pipeline(call->pip);
                sg_apply_bindings(&call->bind);
                vs_params_t vs_params;
                vs_params.texture_matrix = call->texture_matrix;
                vs_params.projection = call->projection;
                sg_apply_uniforms(UB_vs_params, &SG_RANGE(vs_params));
                sg_draw(0, call->vcount, call->icount);
                sg_destroy_buffer(call->bind.vertex_buffers[0]);
                sg_destroy_buffer(call->bind.vertex_buffers[1]);
                sg_destroy_sampler(call->bind.samplers[SMP_sampler_v]);
                sg_destroy_pipeline(call->pip);
                break;
            default:
                abort();
        }
        free(cursor->data);
        free(cursor);
        cursor = tmp;
    }
    fwt.commands.head = fwt.commands.tail = NULL;
    sg_end_pass();
    sg_commit();
    sapp_input_update();
}

static void event(const sapp_event *e) {
    sapp_input_handler(e);
}

static void cleanup(void) {
    if (state.scene->deinit)
        state.scene->deinit(state.state);
    if (state.handle)
        dlclose(state.handle);
#if !defined(PLATFORM_WINDOWS)
    free(state.args.path);
#endif
    sg_shutdown();
}

static struct option long_options[] = {
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"title", required_argument, NULL, 't'},
    {"usage", no_argument, NULL, 'u'},
    {"path", required_argument, NULL, 'p'},
    {NULL, 0, NULL, 0}
};

static void usage(int exit_code) {
    puts(" usage: fwt [path to dylib] [options]");
    puts("");
    puts(" fun-with-triangles  Copyright (C) 2025  George Watson");
    puts(" This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.");
    puts(" This is free software, and you are welcome to redistribute it");
    puts(" under certain conditions; type `show c' for details.");
    puts("");
    puts("  Options:");
    puts("      -w/--width     Window width [default: 640]");
    puts("      -h/--height    Window height [default: 480]");
    puts("      -t/--title     Window title [default: \"fwp\"]");
    puts("      -u/--usage     Display this message");
    exit(exit_code);
}

#if defined(PLATFORM_WINDOWS)
static FILETIME Win32GetLastWriteTime(char* path) {
    FILETIME time;
    WIN32_FILE_ATTRIBUTE_DATA data;

    if (GetFileAttributesEx(path, GetFileExInfoStandard, &data))
        time = data.ftLastWriteTime;

    return time;
}
#endif

static int ShouldReloadLibrary(void) {
#if defined(PLATFORM_WINDOWS)
    FILETIME newTime = Win32GetLastWriteTime(state.args.path);
    int result = CompareFileTime(&newTime, &state.writeTime);
    if (result)
        state.writeTime = newTime;
    return result;
#else
    struct stat attr;
    int result = !stat(state.args.path, &attr) && state.handleID != attr.st_ino;
    if (result)
        state.handleID = attr.st_ino;
    return result;
#endif
}

#if defined(PLATFORM_WINDOWS)
char* RemoveExt(char* path) {
    char *ret = malloc(strlen(path) + 1);
    if (!ret)
        return NULL;
    strcpy(ret, path);
    char *ext = strrchr(ret, '.');
    if (ext)
        *ext = '\0';
    return ret;
}
#endif

static int ReloadLibrary(const char *path) {
    if (!ShouldReloadLibrary())
        return 1;

    if (state.handle) {
        if (state.scene->unload)
            state.scene->unload(state.state);
        dlclose(state.handle);
    }

#if defined(PLATFORM_WINDOWS)
    size_t newPathSize = strlen(path) + 4;
    char *newPath = malloc(sizeof(char) * newPathSize);
    char *noExt = RemoveExt(path);
    sprintf(newPath, "%s.tmp.dll", noExt);
    CopyFile(path, newPath, 0);
    handle = dlopen(newPath, RTLD_NOW);
    free(newPath);
    free(noExt);
    if (!state.handle)
#else
    if (!(state.handle = dlopen(path, RTLD_NOW)))
#endif
        goto BAIL;
    if (!(state.scene = dlsym(state.handle, "scene")))
        goto BAIL;
    if (!state.state) {
        if (state.scene->windowWidth > 0 && state.scene->windowHeight > 0)
            fwtSetWindowSize(state.scene->windowWidth, state.scene->windowHeight);
        if (state.scene->windowTitle)
            fwtSetWindowTitle(state.scene->windowTitle);
        if (!(state.state = state.scene->init()))
            goto BAIL;
    } else {
        if (state.scene->windowWidth > 0 && state.scene->windowHeight > 0)
            fwtSetWindowSize(state.scene->windowWidth, state.scene->windowHeight);
        if (state.scene->windowTitle)
            fwtSetWindowTitle(state.scene->windowTitle);
        if (state.scene->reload)
            state.scene->reload(state.state);
    }
    return 1;

BAIL:
    if (state.handle)
        dlclose(state.handle);
    state.handle = NULL;
#if defined(PLATFORM_WINDOWS)
    memset(&state.writeTime, 0, sizeof(FILETIME));
#else
    state.handleID = 0;
#endif
    return 0;
}

int main(int argc, char* argv[]) {
    extern char* optarg;
    extern int optopt;
    extern int optind;
    int opt;
    while ((opt = getopt_long(argc, argv, ":w:h:t:u", long_options, NULL)) != -1) {
        switch (opt) {
            case 'w':
                state.args.width = atoi(optarg);
                break;
            case 'h':
                state.args.height = atoi(optarg);
                break;
            case 't':
                state.args.title = optarg;
                break;
            case ':':
                printf("ERROR: \"-%c\" requires an value!\n", optopt);
                usage(1);
            case '?':
                printf("ERROR: Unknown argument \"-%c\"\n", optopt);
                usage(0);
            case 'u':
                usage(0);
            default:
                usage(0);
        }
    }

    if (optind >= argc) {
        puts("ERROR: No path to dynamic library provided");
        usage(0);
    } else {
        state.args.path = argv[optind];
    }
    if (state.args.path) {
#if !defined(PLATFORM_WINDOWS)
        if (state.args.path[0] != '.' || state.args.path[1] != '/') {
            char *tmp = malloc(strlen(state.args.path) + 2 * sizeof(char));
            sprintf(tmp, "./%s", state.args.path);
            state.args.path = tmp;
        } else
            state.args.path = strdup(state.args.path);
#endif
    }

    if (access(state.args.path, F_OK)) {
        printf("ERROR: No file found at path \"%s\"\n", state.args.path);
        exit(0);
    }

    if (!state.args.width)
        state.args.width = DEFAULT_WINDOW_WIDTH;
    if (!state.args.height)
        state.args.height = DEFAULT_WINDOW_HEIGHT;
    if (!state.args.title)
        state.args.title = DEFAULT_WINDOW_TITLE;

    if (!ReloadLibrary(state.args.path)) {
        printf("ERROR: Faield to load library at path \"%s\"\n", state.args.path ? state.args.path : "NULL");
        exit(0);
    }

    sapp_desc desc = (sapp_desc) {
        .init_cb = init,
        .frame_cb = frame,
        .event_cb = event,
        .cleanup_cb = cleanup,
        .width = 640,
        .height = 480,
        .window_title = "test",
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
    sapp_run(&desc);
    return 0;
}
#endif // FWT_MAIN_PROGRAM

int fwtFrameBufferWidth(void) {
    return sapp_width() * (int)fwtFrameBufferScaleFactor();
}

int fwtFrameBufferHeight(void) {
    return sapp_height() * (int)fwtFrameBufferScaleFactor();
}

float fwtFrameBufferWidthf(void) {
    return sapp_widthf() * fwtFrameBufferScaleFactor();
}

float fwtFrameBufferHeightf(void) {
    return sapp_heightf() * fwtFrameBufferScaleFactor();
}

float fwtFrameBufferScaleFactor(void) {
#ifdef __APPLE__
    return [[NSScreen mainScreen] backingScaleFactor];
#else
    return 1.f; // I don't know if this is a thing for Windows/Linux so return 1
#endif
}

void fwtSetWindowSize(int width, int height) {
    if (!fwt.running) {
        if (width)
            fwt.app_desc.width = width;
        if (height)
            fwt.app_desc.height = height;
    }
}

void fwtSetWindowTitle(const char *title) {
    if (fwt.running)
        sapp_set_window_title(title);
    else
        fwt.app_desc.window_title = title;
}

int fwtWindowWidth(void) {
    return fwt.running ? sapp_width() : -1;
}

int fwtWindowHeight(void) {
    return fwt.running ? sapp_height() : -1;
}

float fwtWindowAspectRatio(void) {
    return sapp_widthf() / sapp_heightf();
}

int fwtIsWindowFullscreen(void) {
    return fwt.running ? sapp_is_fullscreen() : fwt.app_desc.fullscreen;
}

void fwtToggleFullscreen(void) {
    if (!fwt.running)
        fwt.app_desc.fullscreen = !fwt.app_desc.fullscreen;
    else
        sapp_toggle_fullscreen();
}

int fwtIsCursorVisible(void) {
    return fwt.running ? sapp_mouse_shown() : fwt.mouse_hidden;
}

void fwtToggleCursorVisible(void) {
    if (fwt.running)
        sapp_show_mouse(!fwt.mouse_hidden);
    fwt.mouse_hidden = !fwt.mouse_hidden;
}

int fwtIsCursorLocked(void) {
    return fwt.running? sapp_mouse_locked() : fwt.mouse_locked;
}

void fwtToggleCursorLock(void) {
    if (fwt.running)
        sapp_lock_mouse(!fwt.mouse_locked);
    fwt.mouse_locked = !fwt.mouse_locked;
}

void sapp_input_clear(void) {
    memset(&fwt.input_prev,    0, sizeof(sapp_input_t));
    memset(&fwt.input_current, 0, sizeof(sapp_input_t));
}

void sapp_input_handler(const sapp_event* e) {
    switch (e->type) {
        case SAPP_EVENTTYPE_KEY_UP:
        case SAPP_EVENTTYPE_KEY_DOWN:
            fwt.input_current.keys[e->key_code].down = e->type == SAPP_EVENTTYPE_KEY_DOWN;
            fwt.input_current.keys[e->key_code].timestamp = stm_now();
            fwt.input_current.modifier = e->modifiers;
            break;
        case SAPP_EVENTTYPE_MOUSE_UP:
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            fwt.input_current.buttons[e->mouse_button].down = e->type == SAPP_EVENTTYPE_MOUSE_DOWN;
            fwt.input_current.buttons[e->mouse_button].timestamp = stm_now();
            break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            fwt.input_current.cursor.x = e->mouse_x;
            fwt.input_current.cursor.y = e->mouse_y;
            break;
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            fwt.input_current.scroll.x = e->scroll_x;
            fwt.input_current.scroll.y = e->scroll_y;
            break;
        default:
            fwt.input_current.modifier = e->modifiers;
            break;
    }
}

void sapp_input_update(void) {
    memcpy(&fwt.input_prev, &fwt.input_current, sizeof(sapp_input_t));
    fwt.input_current.scroll.x = fwt.input_current.scroll.y = 0.f;
}

bool fwtIsKeyDown(int key) {
    return fwt.input_current.keys[key].down == 1;
}

bool fwtIsKeyHeld(int key) {
    return fwtIsKeyDown(key) && stm_sec(stm_since(fwt.input_current.keys[key].timestamp)) > 1;
}

bool fwtWasKeyPressed(int key) {
    return fwtIsKeyDown(key) && !fwt.input_prev.keys[key].down;
}

bool fwtWasKeyReleased(int key) {
    return !fwtIsKeyDown(key) && fwt.input_prev.keys[key].down;
}

bool fwtIsKeyReleased(int key) {
    return !fwtIsKeyDown(key) && fwt.input_prev.keys[key].down;
}

bool __fwtAreKeysDown(int n, ...) {
    va_list args;
    va_start(args, n);
    int result = 1;
    for (int i = 0; i < n; i++)
        if (!fwt.input_current.keys[va_arg(args, int)].down) {
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
        if (fwt.input_current.keys[va_arg(args, int)].down) {
            result = 1;
            goto BAIL;
        }
BAIL:
    va_end(args);
    return result;
}

bool fwtIsButtonDown(int button) {
    return fwt.input_current.buttons[button].down;
}

bool fwtIsButtonHeld(int button) {
    return fwtIsButtonDown(button) && stm_sec(stm_since(fwt.input_current.buttons[button].timestamp)) > 1;
}

bool fwtWasButtonPressed(int button) {
    return fwtIsButtonDown(button) && !fwt.input_prev.buttons[button].down;
}

bool fwtWasButtonReleased(int button) {
    return !fwtIsButtonDown(button) && fwt.input_prev.buttons[button].down;
}

bool __fwtAreButtonsDown(int n, ...) {
    va_list args;
    va_start(args, n);
    int result = 1;
    for (int i = 0; i < n; i++)
        if (!fwt.input_current.buttons[va_arg(args, int)].down) {
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
        if (fwt.input_current.buttons[va_arg(args, int)].down) {
            result = 1;
            goto BAIL;
        }
BAIL:
    va_end(args);
    return result;
}

bool fwtModifiedEquals(int mods) {
    return fwt.input_current.modifier == mods;
}

bool fwtModifierDown(int mod) {
    return fwt.input_current.modifier & mod;
}

bool fwtHasMouseMove(void) {
    return fwt.input_current.cursor.x != fwt.input_prev.cursor.x || fwt.input_current.cursor.y != fwt.input_prev.cursor.y;
}

int fwtCursorX(void) {
    return fwt.input_current.cursor.x;
}

int fwtCursorY(void) {
    return fwt.input_current.cursor.y;
}

bool fwtCursorDeltaX(void) {
    return fwt.input_current.cursor.x - fwt.input_prev.cursor.x;
}

bool fwtCursorDeltaY(void) {
    return fwt.input_current.cursor.y - fwt.input_prev.cursor.y;
}

bool fwtCheckScrolled(void) {
    return fwt.input_current.scroll.x != 0 || fwt.input_current.scroll.y != 0;
}

float fwtScrollX(void) {
    return fwt.input_current.scroll.x;
}

float fwtScrollY(void) {
    return fwt.input_current.scroll.y;
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

#ifdef MIN
#undef MIN
#endif
#define MIN(a, b)  ((a) < (b) ? (a) : (b))
#ifdef MAX
#undef MAX
#endif
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

hmm_mat4* fwt_matrix_stack_head(int mode) {
    assert(mode >= 0 && mode < FWT_MATRIXMODE_COUNT);
    fwt_matrix_stack_t *stack = &fwt.state.matrix_stack[mode];
    return stack->count ? &stack->stack[stack->count-1] : NULL;
}

hmm_mat4* fwt_current_matrix(void) {
    return fwt_matrix_stack_head(fwt.state.matrix_mode);
}

void fwt_set_current_matrix(hmm_mat4 mat) {
    hmm_mat4 *head = fwt_current_matrix();
    *head = mat;
}

void fwt_push_vertex(void) {
    fwt.state.draw_call.vertices = realloc(fwt.state.draw_call.vertices, ++fwt.state.draw_call.vcount * sizeof(fwt_vertex_t));
    memcpy(&fwt.state.draw_call.vertices[fwt.state.draw_call.vcount-1], &fwt.state.current_vertex, sizeof(fwt_vertex_t));
}

void fwt_push_command(int type, void *data) {
    fwt_command_t *command = malloc(sizeof(fwt_command_t));
    command->type = type;
    command->data = data;
    command->next = NULL;

    if (!fwt.commands.head && !fwt.commands.tail)
        fwt.commands.head = fwt.commands.tail = command;
    else {
        fwt.commands.tail->next = command;
        fwt.commands.tail = command;
    }
}

void fwtMatrixMode(int mode) {
    switch (mode) {
        case FWT_MATRIXMODE_PROJECTION:
        case FWT_MATRIXMODE_TEXTURE:
        case FWT_MATRIXMODE_MODELVIEW:
            fwt.state.matrix_mode = mode;
            break;
        default:
            abort(); // unknown mode
    }
}

void fwtPushMatrix(void) {
    fwt_matrix_stack_t *cs = &fwt.state.matrix_stack[fwt.state.matrix_mode];
    assert(cs->count + 1 < MAX_MATRIX_STACK);
    cs->stack[cs->count] = cs->stack[cs->count-1];
    cs->count++;
}

void fwtPopMatrix(void) {
    fwt_matrix_stack_t *cs = &fwt.state.matrix_stack[fwt.state.matrix_mode];
    if (cs->count == 1)
        memset(&cs->stack, 0, MAX_MATRIX_STACK * sizeof(hmm_mat4));
    else
        cs->stack[--cs->count] = HMM_Mat4();
}

void fwtLoadIdentity(void) {
    fwt_set_current_matrix(HMM_Mat4d(1.f));
}

void fwtMulMatrix(const float *matf) {
    hmm_mat4 right;
    memcpy(&right, matf, sizeof(float) * 16);
    hmm_m4 result = HMM_MultiplyMat4(*fwt_current_matrix(), right);
    fwt_set_current_matrix(result);
}

#define FWT_MUL_MATRIX(M) fwtMulMatrix((float*)&(M))

void fwtTranslate(float x, float y, float z) {
    hmm_mat4 mat = HMM_Translate(HMM_Vec3(x, y, z));
    FWT_MUL_MATRIX(mat);
}

void fwtRotate(float angle, float x, float y, float z) {
    hmm_mat4 mat = HMM_Rotate(angle, HMM_Vec3(x, y, z));
    FWT_MUL_MATRIX(mat);
}

void fwtScale(float x, float y, float z) {
    hmm_mat4 mat = HMM_Scale(HMM_Vec3(x, y, z));
    FWT_MUL_MATRIX(mat);
}

void fwtOrtho(double left, double right, double bottom, double top, double znear, double zfar) {
    hmm_mat4 mat = HMM_Orthographic(left, right, bottom, top, znear, zfar);
    FWT_MUL_MATRIX(mat);
}

void fwtPerspective(float fovy, float aspect_ratio, float znear, float zfar) {
    hmm_mat4 mat = HMM_Perspective(fovy, aspect_ratio, znear, zfar);
    FWT_MUL_MATRIX(mat);
}

void fwtLookAt(float eyeX, float eyeY, float eyeZ,
                 float targetX, float targetY, float targetZ,
                 float upX, float upY, float upZ) {
    hmm_mat4 mat = HMM_LookAt(HMM_Vec3(eyeX, eyeY, eyeZ),
                              HMM_Vec3(targetX, targetY, targetZ),
                              HMM_Vec3(upX, upY, upZ));
    FWT_MUL_MATRIX(mat);
}

void fwtClearColor(float r, float g, float b, float a) {
    fwt.state.clear_color = (sg_color){r, g, b, a};
}

void fwtViewport(int x, int y, int width, int height) {
    fwt_rect_t *rect = malloc(sizeof(fwt_rect_t));
    rect->x = x;
    rect->y = y;
    rect->w = width;
    rect->h = height;
    fwt_push_command(FWT_CMD_VIEWPORT, rect);
}

void fwtScissor(int x, int y, int width, int height) {
    fwt_rect_t *rect = malloc(sizeof(fwt_rect_t));
    rect->x = x;
    rect->y = y;
    rect->w = width;
    rect->h = height;
    fwt_push_command(FWT_CMD_SCISSOR_RECT, rect);
}

void fwtBlendMode(int mode) {
    if (mode == fwt.state.blend_mode)
        return;
    sg_blend_state *blend = &fwt.state.blend;
    switch (mode) {
        case FWT_BLEND_NONE:
            blend->enabled = false;
            blend->src_factor_rgb = SG_BLENDFACTOR_ONE;
            blend->dst_factor_rgb = SG_BLENDFACTOR_ZERO;
            blend->op_rgb = SG_BLENDOP_ADD;
            blend->src_factor_alpha = SG_BLENDFACTOR_ONE;
            blend->dst_factor_alpha = SG_BLENDFACTOR_ZERO;
            blend->op_alpha = SG_BLENDOP_ADD;
            break;
        case FWT_BLEND_BLEND:
            blend->enabled = true;
            blend->src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
            blend->dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            blend->op_rgb = SG_BLENDOP_ADD;
            blend->src_factor_alpha = SG_BLENDFACTOR_ONE;
            blend->dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            blend->op_alpha = SG_BLENDOP_ADD;
            break;
        case FWT_BLEND_ADD:
            blend->enabled = true;
            blend->src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
            blend->dst_factor_rgb = SG_BLENDFACTOR_ONE;
            blend->op_rgb = SG_BLENDOP_ADD;
            blend->src_factor_alpha = SG_BLENDFACTOR_ZERO;
            blend->dst_factor_alpha = SG_BLENDFACTOR_ONE;
            blend->op_alpha = SG_BLENDOP_ADD;
            break;
        case FWT_BLEND_MOD:
            blend->enabled = true;
            blend->src_factor_rgb = SG_BLENDFACTOR_DST_COLOR;
            blend->dst_factor_rgb = SG_BLENDFACTOR_ZERO;
            blend->op_rgb = SG_BLENDOP_ADD;
            blend->src_factor_alpha = SG_BLENDFACTOR_ZERO;
            blend->dst_factor_alpha = SG_BLENDFACTOR_ONE;
            blend->op_alpha = SG_BLENDOP_ADD;
            break;
        default:
        case FWT_BLEND_MUL:
            blend->enabled = true;
            blend->src_factor_rgb = SG_BLENDFACTOR_DST_COLOR;
            blend->dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            blend->op_rgb = SG_BLENDOP_ADD;
            blend->src_factor_alpha = SG_BLENDFACTOR_DST_ALPHA;
            blend->dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            blend->op_alpha = SG_BLENDOP_ADD;
            break;
    }
    fwt.state.blend_mode = mode;
}

void fwtDepthFunc(int func) {
    if (func == fwt.state.pip_desc.depth.compare)
        return;
    switch (func) {
        default:
        case FWT_CMP_DEFAULT:
            func = FWT_CMP_LESS_EQUAL;
        case FWT_CMP_NEVER:
        case FWT_CMP_LESS:
        case FWT_CMP_EQUAL:
        case FWT_CMP_LESS_EQUAL:
        case FWT_CMP_GREATER:
        case FWT_CMP_NOT_EQUAL:
        case FWT_CMP_GREATER_EQUAL:
        case FWT_CMP_ALWAYS:
        case FWT_CMP_NUM:
            fwt.state.pip_desc.depth.compare = (sg_compare_func)func;
            break;
    }
}

void fwtCullMode(int mode) {
    if (mode == fwt.state.pip_desc.cull_mode)
        return;
    switch (mode) {
        default:
        case FWT_CULL_DEFAULT:
            mode = FWT_CULL_BACK;
        case FWT_CULL_NONE:
        case FWT_CULL_FRONT:
        case FWT_CULL_BACK:
            fwt.state.pip_desc.cull_mode = mode;
            break;
    }
}

void fwtBegin(int mode) {
    assert(!fwt.state.draw_call.vertices && !fwt.state.draw_call.instances);
    if (fwt.state.draw_call.vertices)
        fwtEnd();
    fwt.state.draw_call.vertices = malloc(0);
    fwt.state.draw_call.vcount = 0;
    fwt.state.draw_call.instances = malloc(0);
    fwt.state.draw_call.icount = 0;
    switch (mode) {
        default:
            mode = FWT_DRAW_TRIANGLES;
        case FWT_DRAW_TRIANGLES:
        case FWT_DRAW_POINTS:
        case FWT_DRAW_LINES:
        case FWT_DRAW_LINE_STRIP:
        case FWT_DRAW_TRIANGLE_STRIP:
            fwt.state.pip_desc.primitive_type = mode;
            break;
    }
}

void fwtVertex2i(int x, int y) {
    fwtVertex3f((float)x, (float)y, 0.f);
}

void fwtVertex2f(float x, float y) {
    fwtVertex3f(x, y, 0.f);
}

void fwtVertex3f(float x, float y, float z) {
    fwt.state.current_vertex.position = HMM_Vec4(x, y, z, 1.f);
    fwt_push_vertex();
}

void fwtTexCoord2f(float x, float y) {
    fwt.state.current_vertex.texcoord = HMM_Vec2(x, y);
}

void fwtNormal3f(float x, float y, float z) {
    fwt.state.current_vertex.normal = HMM_Vec3(x, y, z);
}

void fwtColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    fwt.state.current_vertex.color = HMM_Vec4((float)r / 255.f,
                                              (float)g / 255.f,
                                              (float)b / 255.f,
                                              (float)a / 255.f);
}

void fwtColor3f(float x, float y, float z) {
    fwt.state.current_vertex.color = HMM_Vec4(x, y, z, 1.f);
}

void fwtColor4f(float x, float y, float z, float w) {
    fwt.state.current_vertex.color = HMM_Vec4(x, y, z, w);
}

static void make_vs_inst(fwt_vs_inst_t *inst, hmm_mat4 model) {
    inst->x = HMM_Vec4(model.Elements[0][0],
                       model.Elements[1][0],
                       model.Elements[2][0],
                       model.Elements[3][0]);
    inst->y = HMM_Vec4(model.Elements[0][1],
                       model.Elements[1][1],
                       model.Elements[2][1],
                       model.Elements[3][1]);
    inst->z = HMM_Vec4(model.Elements[0][2],
                       model.Elements[1][2],
                       model.Elements[2][2],
                       model.Elements[3][2]);
    inst->w = HMM_Vec4(model.Elements[0][3],
                       model.Elements[1][3],
                       model.Elements[2][3],
                       model.Elements[3][3]);
}

void fwtDraw(void) {
    fwt.state.draw_call.instances = realloc(fwt.state.draw_call.instances, ++fwt.state.draw_call.icount * sizeof(fwt_vs_inst_t));
    fwt_vs_inst_t *inst = &fwt.state.draw_call.instances[fwt.state.draw_call.icount-1];
    hmm_mat4 *m = fwt_matrix_stack_head(FWT_MATRIXMODE_MODELVIEW);
    make_vs_inst(inst, m ? *m : HMM_Mat4());
}

void fwtEnd(void) {
    if (!fwt.state.draw_call.instances || !fwt.state.draw_call.icount)
        goto BAIL;
    if (!fwt.state.draw_call.vertices || !fwt.state.draw_call.vcount) {
//        if (sg_query_buffer_state(fwt.state.current_buffer) != SG_RESOURCESTATE_VALID)
            goto BAIL;
    }

    fwt.state.draw_call.pip = sg_make_pipeline(&fwt.state.pip_desc);
    fwt.state.draw_call.projection = *fwt_matrix_stack_head(FWT_MATRIXMODE_PROJECTION);
    fwt.state.draw_call.texture_matrix = *fwt_matrix_stack_head(FWT_MATRIXMODE_TEXTURE);

    sg_buffer_desc inst_desc = {
        .size = fwt.state.draw_call.icount * sizeof(fwt_vs_inst_t),
        .usage.stream_update = true
    };
    sg_buffer vbuf = (sg_buffer){.id=fwtStoreBuffer()};
    fwt.state.draw_call.bind = (sg_bindings) {
        .vertex_buffers[0] = vbuf,
        .vertex_buffers[1] = sg_make_buffer(&inst_desc),
        .images[IMG_texture_v] = fwt.state.current_texture,
        .samplers = sg_make_sampler(&fwt.state.sampler_desc)
    };
    sg_range r0 = {
        .ptr = fwt.state.draw_call.instances,
        .size = fwt.state.draw_call.icount * sizeof(fwt_vs_inst_t)
    };
    sg_update_buffer(fwt.state.draw_call.bind.vertex_buffers[1], &r0);
    fwt_draw_call_t *draw_call = malloc(sizeof(fwt_draw_call_t));
    memcpy(draw_call, &fwt.state.draw_call, sizeof(fwt_draw_call_t));
    fwt_push_command(FWT_CMD_DRAW_CALL, draw_call);

BAIL:
    if (fwt.state.draw_call.vertices) {
        free(fwt.state.draw_call.vertices);
        fwt.state.draw_call.vertices = NULL;
    }
    if (fwt.state.draw_call.instances) {
        free(fwt.state.draw_call.instances);
        fwt.state.draw_call.instances = NULL;
    }
    memset(&fwt.state.draw_call, 0, sizeof(fwt_draw_call_t));
    memset(&fwt.state.sampler_desc, 0, sizeof(sg_sampler_desc));
}

int fwtEmptyTexture(int width, int height) {
    assert(width && height);
    sg_image_desc desc = {
        .width = width,
        .height = height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .usage.stream_update = true
    };
    return sg_make_image(&desc).id;
}

void fwtPushTexture(int texture) {
    sg_image tmp = {.id = texture};
    assert(sg_query_image_state(tmp) == SG_RESOURCESTATE_VALID);
    fwt.state.current_texture = tmp;
}

void fwtPopTexture(void) {
    memset(&fwt.state.current_texture, 0, sizeof(sg_image));
}

static const char* file_extension(const char *path) {
    const char *dot = strrchr(path, '.');
    return !dot || dot == path ? NULL : dot + 1;
}

int fwtLoadTexturePath(const char *path) {
    assert(fwtFileExists(path));
    #define VALID_EXTS_SZ 11
    static const char *valid_extensions[VALID_EXTS_SZ] = {
        "jpg", "jpeg", "png", "bmp", "psd", "tga", "hdr", "pic", "ppm", "pgm", "qoi"
    };
    const char *ext = file_extension(path);
    unsigned long ext_length = strlen(ext);
    char *dup = strdup(ext);
    for (int i = 0; i < ext_length; i++)
        if (dup[i] >= 'A' && dup[i] <= 'Z')
            dup[i] += 32;
    int found = 0;
    for (int i = 0; i < VALID_EXTS_SZ; i++) {
        if (!strncmp(dup, valid_extensions[i], ext_length)) {
            found = 1;
            break;
        }
    }
    free(dup);
    if (!found)
        return SG_INVALID_ID;

    size_t sz = -1;
    unsigned char *data = fwtReadFile(path, &sz);
    assert(data && sz > 0);
    int result = fwtLoadTextureMemory(data, (int)sz);
    free(data);
    return result;
}

#define QOI_MAGIC (((unsigned int)'q') << 24 | ((unsigned int)'o') << 16 | ((unsigned int)'i') <<  8 | ((unsigned int)'f'))

static int check_if_qoi(unsigned char *data) {
    return (data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]) == QOI_MAGIC;
}

#define RGBA(R, G, B, A) (((unsigned int)(A) << 24) | ((unsigned int)(R) << 16) | ((unsigned int)(G) << 8) | (B))

static int* load_texture_data(unsigned char *data, int data_size, int *w, int *h) {
    assert(data && data_size);
    int _w, _h, c;
    unsigned char *in = NULL;
    if (check_if_qoi(data)) {
        qoi_desc desc;
        in = qoi_decode(data, data_size, &desc, 4);
        _w = desc.width;
        _h = desc.height;
    } else
        in = stbi_load_from_memory(data, data_size, &_w, &_h, &c, 4);
    assert(in && _w && _h);

    int *buf = malloc(_w * _h * sizeof(int));
    for (int x = 0; x < _w; x++)
        for (int y = 0; y < _h; y++) {
            unsigned char *p = in + (x + _w * y) * 4;
            buf[y * _w + x] = RGBA(p[0], p[1], p[2], p[3]);
        }
    free(in);
    if (w)
        *w = _w;
    if (h)
        *h = _h;
    return buf;
}

int fwtLoadTextureMemory(unsigned char *data, int data_size) {
    assert(data && data_size);
    int w, h;
    int *tmp = load_texture_data(data, data_size, &w, &h);
    assert(tmp && w && h);
    sg_image texture = { .id=fwtEmptyTexture(w, h) };
    sg_image_data desc = {
        .subimage[0][0] = (sg_range) {
            .ptr = tmp,
            .size = w * h * sizeof(int)
        }
    };
    sg_update_image(texture, &desc);
    free(tmp);
    return texture.id;
}

static void set_filter(sg_filter *dst, int val) {
    switch (val) {
        default:
        case FWT_FILTER_DEFAULT:
            val = FWT_FILTER_NONE;
        case FWT_FILTER_NONE:
        case FWT_FILTER_NEAREST:
        case FWT_FILTER_LINEAR:
            if (dst)
                *dst = val;
            break;
    }
}

void fwtSetTextureFilter(int min, int mag) {
    assert(sg_query_image_state(fwt.state.current_texture) == SG_RESOURCESTATE_VALID);
    set_filter(&fwt.state.sampler_desc.min_filter, min);
    set_filter(&fwt.state.sampler_desc.mag_filter, mag);
}

static void set_wrap(sg_wrap *dst, int val) {
    switch (val) {
        default:
        case FWT_WRAP_DEFAULT:
            val = FWT_WRAP_REPEAT;
        case FWT_WRAP_REPEAT:
        case FWT_WRAP_CLAMP_TO_EDGE:
        case FWT_WRAP_CLAMP_TO_BORDER:
        case FWT_WRAP_MIRRORED_REPEAT:
            if (dst)
                *dst = val;
            break;
    }
}

void fwtSetTextureWrap(int wrap_u, int wrap_v) {
    assert(sg_query_image_state(fwt.state.current_texture) == SG_RESOURCESTATE_VALID);
    set_wrap(&fwt.state.sampler_desc.wrap_u, wrap_u);
    set_wrap(&fwt.state.sampler_desc.wrap_v, wrap_v);
}

void fwtReleaseTexture(int texture) {
    sg_image tmp = {.id = texture};
    if (sg_query_image_state(tmp) == SG_RESOURCESTATE_VALID)
        sg_destroy_image(tmp);
}

int fwtStoreBuffer(void) {
    sg_buffer_desc desc = {
        .data = (sg_range) {
            .ptr = fwt.state.draw_call.vertices,
            .size = fwt.state.draw_call.vcount * sizeof(fwt_vertex_t)
        }
    };
    sg_buffer result = sg_make_buffer(&desc);
    assert(sg_query_buffer_state(result) == SG_RESOURCESTATE_VALID);
    return result.id;
}

bool fwtSetWorkingDir(const char *path) {
    return chdir(path);
}

unsigned char* read_file(const char *path, size_t *length) {
    unsigned char *result = NULL;
    size_t sz = -1;
    FILE *fh = fopen(path, "rb");
    if (!fh)
        goto BAIL;
    fseek(fh, 0, SEEK_END);
    sz = ftell(fh);
    fseek(fh, 0, SEEK_SET);

    result = malloc(sz * sizeof(unsigned char));
    fread(result, sz, 1, fh);
    fclose(fh);

BAIL:
    if (length)
        *length = sz;
    return result;
}

static bool file_exists(const char *path) {
    return !access(path, F_OK);
}

void fwtMountFileSystem(const char *path) {
    zip *z = NULL;
    int is_directory = ('/' == path[strlen(path)-1]);
    if( !is_directory ) z = zip_open(path, "rb");
    if( !is_directory && !z ) return;

    vfs_dir *prev = fwt.dir_head, zero = {0};
    *(fwt.dir_head = realloc(0, sizeof(vfs_dir))) = zero;
    fwt.dir_head->next = prev;
    fwt.dir_head->path = STRDUP(path);
    fwt.dir_head->archive = z;
    fwt.dir_head->is_directory = is_directory;
}

static bool vfs_has(vfs_dir *dir, const char *filename) {
    if (dir->is_directory) {
        char buf[512];
        snprintf(buf, sizeof(buf), "%s%s", dir->path, filename);
        if (file_exists(buf))
            return true;
    } else {
        int index = zip_find(dir->archive, filename);
        if (index != -1)
            return true;
    }
    return false;
}

bool fwtFileExists(const char *filename) {
    vfs_dir *dir = fwt.dir_head;
    if (!dir)
        return file_exists(filename);
    while (dir) {
        if (vfs_has(dir, filename))
            return true;
        dir = dir->next;
    }
    return false;
}

unsigned char* fwtReadFile(const char *filename, size_t *size) { // must free() after use
    vfs_dir *dir = fwt.dir_head;
    if (!dir)
        return read_file(filename, size);
    while (dir) {
        if (vfs_has(dir, filename)) {
            if (dir->is_directory)
                return read_file(filename, size);
            else {
                int index = zip_find(dir->archive, filename);
                unsigned char *data = zip_extract(dir->archive, index);
                if (size)
                    *size = zip_size(dir->archive, index);
                return data;
            }
        }
        dir = dir->next;
    }
    if (size)
        *size = 0;
    return NULL;
}

bool fwtWriteFile(const char *filename, unsigned char *data, size_t size) {
    // TODO: Write to all mounts, overwrite if existing
    return false;
}
