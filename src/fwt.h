/* fwt.h -- https://github.com/takeiteasy/fwt

 The MIT License (MIT)

 Copyright (c) 2022 George Watson

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

#ifndef __FWT_H__
#define __FWT_H__
#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
#include <emscripten.h>
#define FWT_EMSCRIPTEN
#define FWT_DISABLE_HOTRELOAD
#endif

#define FWT_POSIX
#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
#define FWT_MAC
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define FWT_WINDOWS
#if !defined(FWT_FORCE_POSIX)
#undef FWT_POSIX
#endif
#elif defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
#define FWT_LINUX
#else
#error "Unsupported operating system"
#endif

#if defined(PP_WINDOWS) && !defined(PP_NO_EXPORT)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#if !defined(__clang__) && (!defined(__GNUC__) || !defined(__GNUG__))
#error Unsupported compiler! This library relies on Clang/GCC extensions
#endif

#if defined(_MSC_VER) && _MSC_VER < 1800
#include <windef.h>
#define bool BOOL
#define true 1
#define false 0
#else
#if defined(__STDC__) && __STDC_VERSION__ < 199901L
typedef enum bool {
    false = 0,
    true = !false
} bool;
#else
#include <stdbool.h>
#endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <errno.h>
#include <assert.h>
#if defined(FWT_MAC)
#include <mach/mach_time.h>
#endif
#if defined(FWT_POSIX)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <dlfcn.h>
#else
#include "dlfcn_win32.h"
#ifndef _MSC_VER
#pragma comment(lib, "Psapi.lib")
#endif
#endif

#include "sokol_gfx.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#if defined(FWT_SCENE) && defined(SOKOL_IMPL)
#undef SOKOL_IMPL
#endif
#include "sokol_args.h"
#include "sokol_time.h"
#include "sokol_gp.h"
#include "jim.h"
#include "mjson.h"
#include "dmon.h"
#include "qoi.h"
#define STB_NO_GIF
#include "stb_image.h"

#include "fwt_config.h"

#if !defined(DEFAULT_CONFIG_NAME)
#if defined(FWT_POSIX)
#define DEFAULT_CONFIG_NAME ".fwt.json"
#else
#define DEFAULT_CONFIG_NAME "fwt.json"
#endif // FWT_POSX
#endif

#if !defined(DEFAULT_WINDOW_WIDTH)
#define DEFAULT_WINDOW_WIDTH 640
#endif

#if !defined(DEFAULT_WINDOW_HEIGHT)
#define DEFAULT_WINDOW_HEIGHT 480
#endif

#if !defined(DEFAULT_WINDOW_TITLE)
#define DEFAULT_WINDOW_TITLE "fwt"
#endif

#if !defined(MAX_TEXTURE_STACK)
#define MAX_TEXTURE_STACK 8
#endif

#if defined(FWT_RELEASE)
#define FWT_DISABLE_HOTRELOAD
#endif

#if !defined(DEFAULT_TARGET_FPS)
#define DEFAULT_TARGET_FPS 60.f
#endif

#ifndef MAX_PATH
#if defined(FWT_MAC)
#define MAX_PATH 255
#elif defined(FWT_WINDOWS)
#define MAX_PATH 256
#elif defined(FWT_LINUX)
#define MAX_PATH 4096
#endif
#endif

#define SETTINGS                                                                                 \
    X("width", integer, width, DEFAULT_WINDOW_WIDTH, "Set window width")                         \
    X("height", integer, height, DEFAULT_WINDOW_HEIGHT, "Set window height")                     \
    X("sampleCount", integer, sample_count, 4, "Set the MSAA sample count of the   framebuffer") \
    X("swapInterval", integer, swap_interval, 1, "Set the preferred swap interval")              \
    X("highDPI", boolean, high_dpi, true, "Enable high-dpi compatability")                       \
    X("fullscreen", boolean, fullscreen, false, "Set fullscreen")                                \
    X("alpha", boolean, alpha, false, "Enable/disable alpha channel on framebuffers")            \
    X("clipboard", boolean, enable_clipboard, false, "Enable clipboard support")                 \
    X("clipboardSize", integer, clipboard_size, 1024, "Size of clipboard buffer (in bytes)")     \
    X("drapAndDrop", boolean, enable_dragndrop, false, "Enable drag-and-drop files")             \
    X("maxDroppedFiles", integer, max_dropped_files, 1, "Max number of dropped files")           \
    X("maxDroppedFilesPathLength", integer, max_dropped_file_path_length, MAX_PATH, "Max path length for dropped files")

#define FWT_CLIPBOARD_PASTED SAPP_EVENTTYPE_CLIPBOARD_PASTED
#define FWT_WINDOW_FILES_DROPPED SAPP_EVENTTYPE_FILES_DROPPED
#define FWT_WINDOW_MOUSE_ENTER SAPP_EVENTTYPE_MOUSE_ENTER
#define FWT_WINDOW_MOUSE_LEAVE SAPP_EVENTTYPE_MOUSE_LEAVE
#define FWT_WINDOW_RESIZED SAPP_EVENTTYPE_RESIZED
#define FWT_WINDOW_ICONIFIED SAPP_EVENTTYPE_ICONIFIED
#define FWT_WINDOW_RESTORED SAPP_EVENTTYPE_RESTORED
#define FWT_WINDOW_FOCUSED SAPP_EVENTTYPE_FOCUSED
#define FWT_WINDOW_UNFOCUSED SAPP_EVENTTYPE_UNFOCUSED
#define FWT_WINDOW_SUSPENDED SAPP_EVENTTYPE_SUSPENDED
#define FWT_WINDOW_RESUMED SAPP_EVENTTYPE_RESUMED
#define FWT_WINDOW_QUIT_REQUESTED SAPP_EVENTTYPE_QUIT_REQUESTED
typedef sapp_event_type fwtEventType;

#if !defined(FWT_MAX_DROPPED_FILES)
#define FWT_MAX_DROPPED_FILES 1 // sapp default
#endif

#if !defined(FWT_CLIPBOARD_SIZE)
#define FWT_CLIPBOARD_SIZE 8192 // sapp default
#endif

typedef struct fwtVertex {
    Vec2f position, texcoord;
    Vec4f color;
} fwtVertex;

typedef struct fwtRect {
    float x, y, w, h;
} fwtRect;

typedef struct fwtTexture {
    sg_image internal;
    int w, h;
} fwtTexture;

typedef struct fwtScene fwtScene;
typedef struct fwtContext fwtContext;

typedef struct fwtState {
    const char *libraryPath;
    void *libraryHandle;
#if defined(FWT_POSIX)
    ino_t libraryHandleID;
#else
    FILETIME libraryWriteTime;
#endif
    fwtContext *libraryContext;
    fwtScene *libraryScene;
    const char *nextScene;

    imap_node_t *textureMap;
    int textureMapCapacity;
    int textureMapCount;
    ezStack commandQueue;
    sg_color clearColor;

    bool running;
    bool mouseHidden;
    bool mouseLocked;
    bool fullscreen, fullscreenLast;
    bool cursorVisible, cursorVisibleLast;
    bool cursorLocked, cursorLockedLast;
    int windowWidth, windowHeight;
    sapp_desc desc;
    sg_pass_action pass_action;

    struct {
        bool down;
        uint64_t timestamp;
    } keyboard[SAPP_MAX_KEYCODES];
    struct {
        struct {
            bool down;
            uint64_t timestamp;
        } buttons[3]; // left, right, middle
        struct {
            int x, y;
        } position, lastPosition;
        struct {
            float x, y;
        } scroll;
    } mouse;
    uint32_t modifiers;
    const char *dropped[FWT_MAX_DROPPED_FILES];
    int droppedCount;
    char clipboard[FWT_CLIPBOARD_SIZE];
} fwtState;

struct fwtScene {
    fwtContext*(*init)(fwtState*);
    void (*deinit)(fwtState*, fwtContext*);
    void (*reload)(fwtState*, fwtContext*);
    void (*unload)(fwtState*, fwtContext*);
    void (*event)(fwtState*, fwtContext*, fwtEventType);
    void (*preframe)(fwtState*, fwtContext*);
    bool (*update)(fwtState*, fwtContext*, float);
    bool (*fixedupdate)(fwtState*, fwtContext*, float);
    void (*frame)(fwtState*, fwtContext*, float);
    void (*postframe)(fwtState*, fwtContext*);
};

EXPORT void fwtSwapToScene(fwtState *state, const char *name);

EXPORT void fwtWindowSize(fwtState *state, int* width, int* height);
EXPORT int fwtIsWindowFullscreen(fwtState *state);
EXPORT void fwtToggleFullscreen(fwtState *state);
EXPORT int fwtIsCursorVisible(fwtState *state);
EXPORT void fwtToggleCursorVisible(fwtState *state);
EXPORT int fwtIsCursorLocked(fwtState *state);
EXPORT void fwtToggleCursorLock(fwtState *state);

EXPORT bool fwtIsKeyDown(fwtState *state, sapp_keycode key);
EXPORT bool fwtAreAllKeysDown(fwtState *state, int count, ...);
EXPORT bool fwtAreAnyKeysDown(fwtState *state, int count, ...);
EXPORT bool fwtIsMouseButtonDown(fwtState *state, sapp_mousebutton button);
EXPORT void fwtMousePosition(fwtState *state, int* x, int* y);
EXPORT void fwtMouseDelta(fwtState *state, int *dx, int *dy);
EXPORT void fwtMouseScroll(fwtState *state, float *dx, float *dy);
EXPORT bool fwtTestKeyboardModifiers(fwtState *state, int count, ...);

EXPORT uint64_t fwtFindTexture(fwtState *state, const char *name);
EXPORT void fwtCreateTexture(fwtState *state, const char *name, ezImage *image);

EXPORT void fwtProject(fwtState* state, float left, float right, float top, float bottom);
EXPORT void fwtResetProject(fwtState* state);
EXPORT void fwtPushTransform(fwtState* state);
EXPORT void fwtPopTransform(fwtState* state);
EXPORT void fwtResetTransform(fwtState* state);
EXPORT void fwtTranslate(fwtState* state, float x, float y);
EXPORT void fwtRotate(fwtState* state, float theta);
EXPORT void fwtRotateAt(fwtState* state, float theta, float x, float y);
EXPORT void fwtScale(fwtState* state, float sx, float sy);
EXPORT void fwtScaleAt(fwtState* state, float sx, float sy, float x, float y);
EXPORT void fwtResetPipeline(fwtState* state);
EXPORT void fwtSetUniform(fwtState* state, void* data, int size);
EXPORT void fwtResetUniform(fwtState* state);
EXPORT void fwtSetBlendMode(fwtState* state, sgp_blend_mode blend_mode);
EXPORT void fwtResetBlendMode(fwtState* state);
EXPORT void fwtSetColor(fwtState* state, float r, float g, float b, float a);
EXPORT void fwtResetColor(fwtState* state);
EXPORT void fwtSetImage(fwtState* state, uint64_t texture_id, int channel);
EXPORT void fwtUnsetImage(fwtState* state, int channel);
EXPORT void fwtResetImage(fwtState* state, int channel);
EXPORT void fwtResetSampler(fwtState* state, int channel);
EXPORT void fwtViewport(fwtState* state, int x, int y, int w, int h);
EXPORT void fwtResetViewport(fwtState* state);
EXPORT void fwtScissor(fwtState* state, int x, int y, int w, int h);
EXPORT void fwtResetScissor(fwtState* state);
EXPORT void fwtResetState(fwtState* state);
EXPORT void fwtClear(fwtState* state);
EXPORT void fwtDrawPoints(fwtState* state, sgp_point* points, int count);
EXPORT void fwtDrawPoint(fwtState* state, float x, float y);
EXPORT void fwtDrawLines(fwtState* state, sgp_line* lines, int count);
EXPORT void fwtDrawLine(fwtState* state, float ax, float ay, float bx, float by);
EXPORT void fwtDrawLinesStrip(fwtState* state, sgp_point* points, int count);
EXPORT void fwtDrawFilledTriangles(fwtState* state, sgp_triangle* triangles, int count);
EXPORT void fwtDrawFilledTriangle(fwtState* state, float ax, float ay, float bx, float by, float cx, float cy);
EXPORT void fwtDrawFilledTrianglesStrip(fwtState* state, sgp_point* points, int count);
EXPORT void fwtDrawFilledRects(fwtState* state, sgp_rect* rects, int count);
EXPORT void fwtDrawFilledRect(fwtState* state, float x, float y, float w, float h);
EXPORT void fwtDrawTexturedRects(fwtState* state, int channel, sgp_textured_rect* rects, int count);
EXPORT void fwtDrawTexturedRect(fwtState* state, int channel, sgp_rect dest_rect, sgp_rect src_rect);

extern fwtState state;

#if defined(__cplusplus)
}
#endif
#endif // __FWT_H__
