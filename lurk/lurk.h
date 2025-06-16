/* lurk.h -- https://github.com/takeiteasy/lurk

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

#ifndef __LURK_H__
#define __LURK_H__
#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
#include <emscripten.h>
#define LURK_EMSCRIPTEN
#define LURK_DISABLE_HOTRELOAD
#endif

#define LURK_POSIX
#if defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
#define LURK_MAC
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define LURK_WINDOWS
#if !defined(LURK_FORCE_POSIX)
#undef LURK_POSIX
#endif
#elif defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
#define LURK_LINUX
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
#if defined(LURK_MAC)
#include <mach/mach_time.h>
#endif
#if defined(LURK_POSIX)
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
#if defined(LURK_SCENE) && defined(SOKOL_IMPL)
#undef SOKOL_IMPL
#endif
#include "sokol_args.h"
#include "sokol_time.h"
#include "sokol_gp.h"
#include "jim.h"
#include "mjson.h"
#include "dmon.h"
#define IMAP_INTERFACE
#include "imap.h"
#include "qoi.h"
#define STB_NO_GIF
#include "stb_image.h"
#include "stb_image_write.h"
#include "ez/ezimage.h"
#include "ez/ezmath.h"
#include "ez/ezrng.h"
#include "ez/ezfs.h"
#include "ez/ezstack.h"
#include "ez/ezecs.h"
#include "gamepad/Gamepad.h"

#include "config.h"

// Taken from: https://gist.github.com/61131/7a22ac46062ee292c2c8bd6d883d28de
#define N_ARGS(...) _NARG_(__VA_ARGS__, _RSEQ())
#define _NARG_(...) _SEQ(__VA_ARGS__)
#define _SEQ(_1, _2, _3, _4, _5, _6, _7, _8, _9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67,_68,_69,_70,_71,_72,_73,_74,_75,_76,_77,_78,_79,_80,_81,_82,_83,_84,_85,_86,_87,_88,_89,_90,_91,_92,_93,_94,_95,_96,_97,_98,_99,_100,_101,_102,_103,_104,_105,_106,_107,_108,_109,_110,_111,_112,_113,_114,_115,_116,_117,_118,_119,_120,_121,_122,_123,_124,_125,_126,_127,N,...) N
#define _RSEQ() 127,126,125,124,123,122,121,120,119,118,117,116,115,114,113,112,111,110,109,108,107,106,105,104,103,102,101,100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0

#if !defined(DEFAULT_CONFIG_NAME)
#if defined(LURK_POSIX)
#define DEFAULT_CONFIG_NAME ".lurk.json"
#else
#define DEFAULT_CONFIG_NAME "lurk.json"
#endif // LURK_POSX
#endif

#if !defined(DEFAULT_WINDOW_WIDTH)
#define DEFAULT_WINDOW_WIDTH 640
#endif

#if !defined(DEFAULT_WINDOW_HEIGHT)
#define DEFAULT_WINDOW_HEIGHT 480
#endif

#if !defined(DEFAULT_WINDOW_TITLE)
#define DEFAULT_WINDOW_TITLE "lurk"
#endif

#if !defined(MAX_TEXTURE_STACK)
#define MAX_TEXTURE_STACK 8
#endif

#if defined(LURK_RELEASE)
#define LURK_DISABLE_HOTRELOAD
#endif

#if !defined(DEFAULT_TARGET_FPS)
#define DEFAULT_TARGET_FPS 60.f
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

#define LURK_CLIPBOARD_PASTED SAPP_EVENTTYPE_CLIPBOARD_PASTED
#define LURK_WINDOW_FILES_DROPPED SAPP_EVENTTYPE_FILES_DROPPED
#define LURK_WINDOW_MOUSE_ENTER SAPP_EVENTTYPE_MOUSE_ENTER
#define LURK_WINDOW_MOUSE_LEAVE SAPP_EVENTTYPE_MOUSE_LEAVE
#define LURK_WINDOW_RESIZED SAPP_EVENTTYPE_RESIZED
#define LURK_WINDOW_ICONIFIED SAPP_EVENTTYPE_ICONIFIED
#define LURK_WINDOW_RESTORED SAPP_EVENTTYPE_RESTORED
#define LURK_WINDOW_FOCUSED SAPP_EVENTTYPE_FOCUSED
#define LURK_WINDOW_UNFOCUSED SAPP_EVENTTYPE_UNFOCUSED
#define LURK_WINDOW_SUSPENDED SAPP_EVENTTYPE_SUSPENDED
#define LURK_WINDOW_RESUMED SAPP_EVENTTYPE_RESUMED
#define LURK_WINDOW_QUIT_REQUESTED SAPP_EVENTTYPE_QUIT_REQUESTED
typedef sapp_event_type lurkEventType;

#if !defined(LURK_MAX_DROPPED_FILES)
#define LURK_MAX_DROPPED_FILES 1 // sapp default
#endif

#if !defined(LURK_CLIPBOARD_SIZE)
#define LURK_CLIPBOARD_SIZE 8192 // sapp default
#endif

typedef struct lurkVertex {
    Vec2f position, texcoord;
    Vec4f color;
} lurkVertex;

typedef struct lurkRect {
    float x, y, w, h;
} lurkRect;

typedef struct lurkTexture {
    sg_image internal;
    int w, h;
} lurkTexture;

typedef struct lurkScene lurkScene;
typedef struct lurkContext lurkContext;

typedef struct lurkState {
    const char *libraryPath;
    void *libraryHandle;
#if defined(LURK_POSIX)
    ino_t libraryHandleID;
#else
    FILETIME libraryWriteTime;
#endif
    lurkContext *libraryContext;
    lurkScene *libraryScene;
    const char *nextScene;

    imap_node_t *textureMap;
    int textureMapCapacity;
    int textureMapCount;
    ezStack commandQueue;
    sg_color clearColor;

    uint64_t timerFrequency;
    int64_t prevFrameTime;
    int64_t desiredFrameTime;
    int64_t snapFrequencies[7];
    int64_t timeAverager[4];
    int64_t maxVsyncError;
    int64_t frameAccumulator;
    double fixedDeltaTime;
    bool resync;
    bool unlockFramerate;
    int updateMultiplicity;

    bool running;
    bool mouseHidden;
    bool mouseLocked;
    bool fullscreen, fullscreenLast;
    bool cursorVisible, cursorVisibleLast;
    bool cursorLocked, cursorLockedLast;
    int windowWidth, windowHeight;
    sapp_desc desc;
    sg_pass_action pass_action;

    ezWorld *world;

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
    const char *dropped[LURK_MAX_DROPPED_FILES];
    int droppedCount;
    char clipboard[LURK_CLIPBOARD_SIZE];
} lurkState;

struct lurkScene {
    lurkContext*(*init)(lurkState*);
    void (*deinit)(lurkState*, lurkContext*);
    void (*reload)(lurkState*, lurkContext*);
    void (*unload)(lurkState*, lurkContext*);
    void (*event)(lurkState*, lurkContext*, lurkEventType);
    void (*preframe)(lurkState*, lurkContext*);
    bool (*update)(lurkState*, lurkContext*, float);
    bool (*fixedupdate)(lurkState*, lurkContext*, float);
    void (*frame)(lurkState*, lurkContext*, float);
    void (*postframe)(lurkState*, lurkContext*);
};

EXPORT void lurkSwapToScene(lurkState *state, const char *name);

EXPORT void lurkWindowSize(lurkState *state, int* width, int* height);
EXPORT int lurkIsWindowFullscreen(lurkState *state);
EXPORT void lurkToggleFullscreen(lurkState *state);
EXPORT int lurkIsCursorVisible(lurkState *state);
EXPORT void lurkToggleCursorVisible(lurkState *state);
EXPORT int lurkIsCursorLocked(lurkState *state);
EXPORT void lurkToggleCursorLock(lurkState *state);

EXPORT bool lurkIsKeyDown(lurkState *state, sapp_keycode key);
#define LURK_ALL_KEYS(STATE, ...) (lurkAreAllKeysDown((STATE),  N_ARGS(__VA_ARGS__), __VA_ARGS__))
#define LURK_ANY_KEYS(STATE, ...) (lurkAreAnyKeysDown((STATE),  N_ARGS(__VA_ARGS__), __VA_ARGS__))
EXPORT bool lurkAreAllKeysDown(lurkState *state, int count, ...);
EXPORT bool lurkAreAnyKeysDown(lurkState *state, int count, ...);
EXPORT bool lurkIsMouseButtonDown(lurkState *state, sapp_mousebutton button);
EXPORT void lurkMousePosition(lurkState *state, int* x, int* y);
EXPORT void lurkMouseDelta(lurkState *state, int *dx, int *dy);
EXPORT void lurkMouseScroll(lurkState *state, float *dx, float *dy);
#define LURK_TEST_MODIFIER(STATE, ...) (lurkAnyKeysDown((STATE),  N_ARGS(__VA_ARGS__), __VA_ARGS__))
EXPORT bool lurkTestKeyboardModifiers(lurkState *state, int count, ...);

EXPORT uint64_t lurkFindTexture(lurkState *state, const char *name);
EXPORT void lurkCreateTexture(lurkState *state, const char *name, ezImage *image);

EXPORT void lurkProject(lurkState* state, float left, float right, float top, float bottom);
EXPORT void lurkResetProject(lurkState* state);
EXPORT void lurkPushTransform(lurkState* state);
EXPORT void lurkPopTransform(lurkState* state);
EXPORT void lurkResetTransform(lurkState* state);
EXPORT void lurkTranslate(lurkState* state, float x, float y);
EXPORT void lurkRotate(lurkState* state, float theta);
EXPORT void lurkRotateAt(lurkState* state, float theta, float x, float y);
EXPORT void lurkScale(lurkState* state, float sx, float sy);
EXPORT void lurkScaleAt(lurkState* state, float sx, float sy, float x, float y);
EXPORT void lurkResetPipeline(lurkState* state);
EXPORT void lurkSetUniform(lurkState* state, void* data, int size);
EXPORT void lurkResetUniform(lurkState* state);
EXPORT void lurkSetBlendMode(lurkState* state, sgp_blend_mode blend_mode);
EXPORT void lurkResetBlendMode(lurkState* state);
EXPORT void lurkSetColor(lurkState* state, float r, float g, float b, float a);
EXPORT void lurkResetColor(lurkState* state);
EXPORT void lurkSetImage(lurkState* state, uint64_t texture_id, int channel);
EXPORT void lurkUnsetImage(lurkState* state, int channel);
EXPORT void lurkResetImage(lurkState* state, int channel);
EXPORT void lurkResetSampler(lurkState* state, int channel);
EXPORT void lurkViewport(lurkState* state, int x, int y, int w, int h);
EXPORT void lurkResetViewport(lurkState* state);
EXPORT void lurkScissor(lurkState* state, int x, int y, int w, int h);
EXPORT void lurkResetScissor(lurkState* state);
EXPORT void lurkResetState(lurkState* state);
EXPORT void lurkClear(lurkState* state);
EXPORT void lurkDrawPoints(lurkState* state, sgp_point* points, int count);
EXPORT void lurkDrawPoint(lurkState* state, float x, float y);
EXPORT void lurkDrawLines(lurkState* state, sgp_line* lines, int count);
EXPORT void lurkDrawLine(lurkState* state, float ax, float ay, float bx, float by);
EXPORT void lurkDrawLinesStrip(lurkState* state, sgp_point* points, int count);
EXPORT void lurkDrawFilledTriangles(lurkState* state, sgp_triangle* triangles, int count);
EXPORT void lurkDrawFilledTriangle(lurkState* state, float ax, float ay, float bx, float by, float cx, float cy);
EXPORT void lurkDrawFilledTrianglesStrip(lurkState* state, sgp_point* points, int count);
EXPORT void lurkDrawFilledRects(lurkState* state, sgp_rect* rects, int count);
EXPORT void lurkDrawFilledRect(lurkState* state, float x, float y, float w, float h);
EXPORT void lurkDrawTexturedRects(lurkState* state, int channel, sgp_textured_rect* rects, int count);
EXPORT void lurkDrawTexturedRect(lurkState* state, int channel, sgp_rect dest_rect, sgp_rect src_rect);

extern lurkState state;

#if defined(__cplusplus)
}
#endif
#endif // __LURK_H__
