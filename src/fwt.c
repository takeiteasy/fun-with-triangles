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
#define SOKOL_IMPL
#define HANDMADE_MATH_IMPLEMENTATION
#include "internal.h"
#include "fwt.glsl.h"

#if defined(PLATFORM_WINDOWS)
#include <Windows.h>
#include <io.h>
#define F_OK 0
#define access _access
#include "getopt_win32.h"
#ifndef _MSC_VER
#pragma comment(lib, "Psapi.lib")
#endif
#define DLFCN_IMPLEMENTATION
#include "dlfcn_win32.h"
#else // PLATFORM_POSIX
#include <getopt.h>
#define _BSD_SOURCE // usleep()
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#endif

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

#ifdef __APPLE__
#include <Cocoa/Cocoa.h>
#endif

float fwtFrameBufferScaleFactor(void) {
#ifdef __APPLE__
    return [[NSScreen mainScreen] backingScaleFactor];
#else
    return 1.f; // I don't know if this is a thing for Windows/Linux so return 1
#endif
}

fwt_t fwt = {
    .running = 0,
    .mouse_hidden = 0,
    .mouse_locked = 0,
    .app_desc = (sapp_desc) {
        .width = DEFAULT_WINDOW_WIDTH,
        .height = DEFAULT_WINDOW_WIDTH,
        .window_title = DEFAULT_WINDOW_TITLE
    }
};

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
        int flags;
        char *path;
    } args;
} state;

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
    {"resizable", no_argument, NULL, 'r'},
    {"top", no_argument, NULL, 'a'},
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
    puts("      -r/--resizable Enable resizable window");
    puts("      -a/--top       Enable window always on top");
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

sapp_desc sokol_main(int argc, char* argv[]) {
    extern char* optarg;
    extern int optopt;
    extern int optind;
    int opt;
    while ((opt = getopt_long(argc, argv, ":w:h:t:uar", long_options, NULL)) != -1) {
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
            case 'r':
                state.args.flags |= FWT_RESIZABLE;
                break;
            case 'a':
                state.args.flags |= FWT_ALWAYS_ON_TOP;
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

    return (sapp_desc){
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
}
