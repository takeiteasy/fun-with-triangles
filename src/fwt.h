/* fwt.h -- https://github.com/takeiteasy/fun-with-triangles

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

#ifndef FWT_H
#define FWT_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifndef N_ARGS
#define N_ARGS(...) _NARG_(__VA_ARGS__, _RSEQ())
#define _NARG_(...) _SEQ(__VA_ARGS__)
#define _SEQ(_1, _2, _3, _4, _5, _6, _7, _8, _9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67,_68,_69,_70,_71,_72,_73,_74,_75,_76,_77,_78,_79,_80,_81,_82,_83,_84,_85,_86,_87,_88,_89,_90,_91,_92,_93,_94,_95,_96,_97,_98,_99,_100,_101,_102,_103,_104,_105,_106,_107,_108,_109,_110,_111,_112,_113,_114,_115,_116,_117,_118,_119,_120,_121,_122,_123,_124,_125,_126,_127,N,...) N
#define _RSEQ() 127,126,125,124,123,122,121,120,119,118,117,116,115,114,113,112,111,110,109,108,107,106,105,104,103,102,101,100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
#endif

typedef struct fwtState fwtState;

typedef struct fwtScene {
    int windowWidth;
    int windowHeight;
    const char *windowTitle;
    fwtState*(*init)(void);
    void(*deinit)(fwtState*);
    void(*reload)(fwtState*);
    void(*unload)(fwtState*);
    int(*tick)(fwtState*, double);
} fwtScene;

// returns sapp_width + scale
int fwtFrameBufferWidth(void);
// returns sapp_height + scale
int fwtFrameBufferHeight(void);
// returns sapp_widthf + scale
float fwtFrameBufferWidthf(void);
// returns sapp_heightf + scale
float fwtFrameBufferHeightf(void);
// returns monitor scale factor (Mac) or 1.f (other platforms)
float fwtFrameBufferScaleFactor(void);

#ifndef MAX_INPUT_STATE_KEYS
#define MAX_INPUT_STATE_KEYS 8
#endif

typedef struct input_state {
    int keys[MAX_INPUT_STATE_KEYS];
    int modifiers;
} fwtInputState;

typedef enum fwtKeyCode {
    FWT_KEY_INVALID          = 0,
    FWT_KEY_SPACE            = 32,
    FWT_KEY_APOSTROPHE       = 39,  /* ' */
    FWT_KEY_COMMA            = 44,  /* , */
    FWT_KEY_MINUS            = 45,  /* - */
    FWT_KEY_PERIOD           = 46,  /* . */
    FWT_KEY_SLASH            = 47,  /* / */
    FWT_KEY_0                = 48,
    FWT_KEY_1                = 49,
    FWT_KEY_2                = 50,
    FWT_KEY_3                = 51,
    FWT_KEY_4                = 52,
    FWT_KEY_5                = 53,
    FWT_KEY_6                = 54,
    FWT_KEY_7                = 55,
    FWT_KEY_8                = 56,
    FWT_KEY_9                = 57,
    FWT_KEY_SEMICOLON        = 59,  /* ; */
    FWT_KEY_EQUAL            = 61,  /* = */
    FWT_KEY_A                = 65,
    FWT_KEY_B                = 66,
    FWT_KEY_C                = 67,
    FWT_KEY_D                = 68,
    FWT_KEY_E                = 69,
    FWT_KEY_F                = 70,
    FWT_KEY_G                = 71,
    FWT_KEY_H                = 72,
    FWT_KEY_I                = 73,
    FWT_KEY_J                = 74,
    FWT_KEY_K                = 75,
    FWT_KEY_L                = 76,
    FWT_KEY_M                = 77,
    FWT_KEY_N                = 78,
    FWT_KEY_O                = 79,
    FWT_KEY_P                = 80,
    FWT_KEY_Q                = 81,
    FWT_KEY_R                = 82,
    FWT_KEY_S                = 83,
    FWT_KEY_T                = 84,
    FWT_KEY_U                = 85,
    FWT_KEY_V                = 86,
    FWT_KEY_W                = 87,
    FWT_KEY_X                = 88,
    FWT_KEY_Y                = 89,
    FWT_KEY_Z                = 90,
    FWT_KEY_LEFT_BRACKET     = 91,  /* [ */
    FWT_KEY_BACKSLASH        = 92,  /* \ */
    FWT_KEY_RIGHT_BRACKET    = 93,  /* ] */
    FWT_KEY_GRAVE_ACCENT     = 96,  /* ` */
    FWT_KEY_WORLD_1          = 161, /* non-US #1 */
    FWT_KEY_WORLD_2          = 162, /* non-US #2 */
    FWT_KEY_ESCAPE           = 256,
    FWT_KEY_ENTER            = 257,
    FWT_KEY_TAB              = 258,
    FWT_KEY_BACKSPACE        = 259,
    FWT_KEY_INSERT           = 260,
    FWT_KEY_DELETE           = 261,
    FWT_KEY_RIGHT            = 262,
    FWT_KEY_LEFT             = 263,
    FWT_KEY_DOWN             = 264,
    FWT_KEY_UP               = 265,
    FWT_KEY_PAGE_UP          = 266,
    FWT_KEY_PAGE_DOWN        = 267,
    FWT_KEY_HOME             = 268,
    FWT_KEY_END              = 269,
    FWT_KEY_CAPS_LOCK        = 280,
    FWT_KEY_SCROLL_LOCK      = 281,
    FWT_KEY_NUM_LOCK         = 282,
    FWT_KEY_PRINT_SCREEN     = 283,
    FWT_KEY_PAUSE            = 284,
    FWT_KEY_F1               = 290,
    FWT_KEY_F2               = 291,
    FWT_KEY_F3               = 292,
    FWT_KEY_F4               = 293,
    FWT_KEY_F5               = 294,
    FWT_KEY_F6               = 295,
    FWT_KEY_F7               = 296,
    FWT_KEY_F8               = 297,
    FWT_KEY_F9               = 298,
    FWT_KEY_F10              = 299,
    FWT_KEY_F11              = 300,
    FWT_KEY_F12              = 301,
    FWT_KEY_F13              = 302,
    FWT_KEY_F14              = 303,
    FWT_KEY_F15              = 304,
    FWT_KEY_F16              = 305,
    FWT_KEY_F17              = 306,
    FWT_KEY_F18              = 307,
    FWT_KEY_F19              = 308,
    FWT_KEY_F20              = 309,
    FWT_KEY_F21              = 310,
    FWT_KEY_F22              = 311,
    FWT_KEY_F23              = 312,
    FWT_KEY_F24              = 313,
    FWT_KEY_F25              = 314,
    FWT_KEY_KP_0             = 320,
    FWT_KEY_KP_1             = 321,
    FWT_KEY_KP_2             = 322,
    FWT_KEY_KP_3             = 323,
    FWT_KEY_KP_4             = 324,
    FWT_KEY_KP_5             = 325,
    FWT_KEY_KP_6             = 326,
    FWT_KEY_KP_7             = 327,
    FWT_KEY_KP_8             = 328,
    FWT_KEY_KP_9             = 329,
    FWT_KEY_KP_DECIMAL       = 330,
    FWT_KEY_KP_DIVIDE        = 331,
    FWT_KEY_KP_MULTIPLY      = 332,
    FWT_KEY_KP_SUBTRACT      = 333,
    FWT_KEY_KP_ADD           = 334,
    FWT_KEY_KP_ENTER         = 335,
    FWT_KEY_KP_EQUAL         = 336,
    FWT_KEY_LEFT_SHIFT       = 340,
    FWT_KEY_LEFT_CONTROL     = 341,
    FWT_KEY_LEFT_ALT         = 342,
    FWT_KEY_LEFT_SUPER       = 343,
    FWT_KEY_RIGHT_SHIFT      = 344,
    FWT_KEY_RIGHT_CONTROL    = 345,
    FWT_KEY_RIGHT_ALT        = 346,
    FWT_KEY_RIGHT_SUPER      = 347,
    FWT_KEY_MENU             = 348,
} fwtKeyCode;

typedef enum fwtModifier {
    FWT_MODIFIER_SHIFT = 0x1,      // left or right shift key
    FWT_MODIFIER_CTRL  = 0x2,      // left or right control key
    FWT_MODIFIER_ALT   = 0x4,      // left or right alt key
    FWT_MODIFIER_SUPER = 0x8,      // left or right 'super' key
    FWT_MODIFIER_LMB   = 0x100,    // left mouse button
    FWT_MODIFIER_RMB   = 0x200,    // right mouse button
    FWT_MODIFIER_MMB   = 0x400,    // middle mouse button
} fwtModifier;

typedef enum fwtMouseButton {
    FWT_BUTTON_LEFT = 0x0,
    FWT_BUTTON_RIGHT = 0x1,
    FWT_BUTTON_MIDDLE = 0x2,
    FWT_BUTTON_INVALID = 0x100,
} fwtMouseButton;

bool fwtIsKeyDown(int key);
// This will be true if a key is held for more than 1 second
// TODO: Make the duration configurable
bool fwtIsKeyHeld(int key);
// Returns true if key is down this frame and was up last frame
bool fwtWasKeyPressed(int key);
// Returns true if key is up and key was down last frame
bool fwtWasKeyReleased(int key);
// If any of the keys passed are not down returns false
bool __fwtAreKeysDown(int n, ...);
#define fwtAreKeysDown(...) __fwtAreKeysDown(N_ARGS(__VA_ARGS__), __VA_ARGS__)
// If none of the keys passed are down returns false
bool __fwtAnyKeysDown(int n, ...);
#define fwtAnyKeysDown(...) __fwtAnyKeysDown(N_ARGS(__VA_ARGS__), __VA_ARGS__)
bool fwtIsButtonDown(int button);
bool fwtIsButtonHeld(int button);
bool fwtWasButtonPressed(int button);
bool fwtWasButtonReleased(int button);
bool __fwtAreButtonsDown(int n, ...);
#define fwtAreButtonsDown(...) __fwtAreButtonsDown(N_ARGS(__VA_ARGS__), __VA_ARGS__)
bool __fwtAnyButtonsDown(int n, ...);
#define fwtAnyButtonsDown(...) __fwtAnyButtonsDown(N_ARGS(__VA_ARGS__), __VA_ARGS__)
bool fwtHasMouseMove(void);
bool fwtModifiedEquals(int mods);
bool fwtModifierDown(int mod);
bool fwtCheckInput(const char *str); // WIP
void __fwtCreateInput(fwtInputState *dst, int modifiers, int n, ...);
#define fwtCreateInput(dst, mods ...) __fwtCreateInput(dst, mods, N_ARGS(__VA_ARGS__), __VA_ARGS__)
bool fwtCreateInputFromString(fwtInputState *dst, const char *str);
bool fwtCheckInputState(fwtInputState *state);
int fwtCursorX(void);
int fwtCursorY(void);
bool fwtCursorDeltaX(void);
bool fwtCursorDeltaY(void);
bool fwtCheckScrolled(void);
float fwtScrollX(void);
float fwtScrollY(void);

enum {
    FWT_MATRIXMODE_MODELVIEW  = 0,
    FWT_MATRIXMODE_PROJECTION,
    FWT_MATRIXMODE_TEXTURE,
    FWT_MATRIXMODE_COUNT
};

enum {
    FWT_DRAW_POINTS = 0x0001,
    FWT_DRAW_LINES = 0x0002,
    FWT_DRAW_LINE_STRIP = 0x0003,
    FWT_DRAW_TRIANGLES = 0x0004,
    FWT_DRAW_TRIANGLE_STRIP = 0x0005
};

enum {
    FWT_CMP_DEFAULT = 0,
    FWT_CMP_NEVER,
    FWT_CMP_LESS,
    FWT_CMP_EQUAL,
    FWT_CMP_LESS_EQUAL,
    FWT_CMP_GREATER,
    FWT_CMP_NOT_EQUAL,
    FWT_CMP_GREATER_EQUAL,
    FWT_CMP_ALWAYS,
    FWT_CMP_NUM,
};

enum {
    FWT_BLEND_DEFAULT = 0,
    FWT_BLEND_NONE,
    FWT_BLEND_BLEND,
    FWT_BLEND_ADD,
    FWT_BLEND_MOD,
    FWT_BLEND_MUL,
};

enum {
    FWT_CULL_DEFAULT = 0,
    FWT_CULL_NONE,
    FWT_CULL_FRONT,
    FWT_CULL_BACK
};

enum {
    FWT_FILTER_DEFAULT = 0,
    FWT_FILTER_NONE,
    FWT_FILTER_NEAREST,
    FWT_FILTER_LINEAR
};

enum {
    FWT_WRAP_DEFAULT = 0,
    FWT_WRAP_REPEAT,
    FWT_WRAP_CLAMP_TO_EDGE,
    FWT_WRAP_CLAMP_TO_BORDER,
    FWT_WRAP_MIRRORED_REPEAT
};

void fwtSetWindowSize(int width, int height);
void fwtSetWindowTitle(const char *title);
int fwtWindowWidth(void);
int fwtWindowHeight(void);
float fwtAspectRatio(void);
int fwtIsWindowFullscreen(void);
void fwtToggleFullscreen(void);
int fwtIsCursorVisible(void);
void fwtToggleCursorVisible(void);
int fwtIsCursorLocked(void);
void fwtToggleCursorLock(void);

void fwtMatrixMode(int mode);
void fwtPushMatrix(void);
void fwtPopMatrix(void);
void fwtLoadIdentity(void);
void fwtMulMatrix(const float *mat);
void fwtTranslate(float x, float y, float z);
void fwtRotate(float angle, float x, float y, float z);
void fwtScale(float x, float y, float z);
void fwtFrustum(double left, double right, double bottom, double top, double znear, double zfar);
void fwtOrtho(double left, double right, double bottom, double top, double znear, double zfar);
void fwtPerspective(float fovy, float aspect_ratio, float znear, float zfar);
void fwtLookAt(float eyeX, float eyeY, float eyeZ, float targetX, float targetY, float targetZ, float upX, float upY, float upZ);

void fwtClearColor(float r, float g, float b, float a);
void fwtViewport(int x, int y, int width, int height);
void fwtScissor(int x, int y, int width, int height);
void fwtBlendMode(int mode);
void fwtDepthFunc(int func);
void fwtCullMode(int mode);

void fwtBegin(int mode);
void fwtVertex2i(int x, int y);
void fwtVertex2f(float x, float y);
void fwtVertex3f(float x, float y, float z);
void fwtTexCoord2f(float x, float y);
void fwtTexCoord2f(float x, float y);
void fwtNormal3f(float x, float y, float z);
void fwtColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void fwtColor3f(float x, float y, float z);
void fwtColor4f(float x, float y, float z, float w);
void fwtDraw(void);
void fwtEnd(void);

int fwtEmptyTexture(int width, int height);
void fwtPushTexture(int texture);
void fwtPopTexture(void);
int fwtLoadTexturePath(const char *path);
int fwtLoadTextureMemory(unsigned char *data, int data_size);
void fwtSetTextureFilter(int min, int mag);
void fwtSetTextureWrap(int wrap_u, int wrap_v);
void fwtReleaseTexture(int texture);

int fwtStoreBuffer(void);
void fwtLoadBuffer(int buffer);
void fwtReleaseBuffer(int buffer);

#ifdef __cplusplus
}
#endif
#endif
