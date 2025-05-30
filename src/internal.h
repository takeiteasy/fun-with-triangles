/* internal.h -- https://github.com/takeiteasy/fun-with-triangles

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

#ifndef INTERNAL_H
#define INTERNAL_H
#include <assert.h>
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_app.h"
#include "sokol/sokol_glue.h"
#include "sokol/sokol_time.h"
#include "sokol/sokol_log.h"
#include "fwt.glsl.h"

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

typedef struct {
    int running;
    int mouse_hidden;
    int mouse_locked;
    sapp_desc app_desc;
    void(*init)(void);
    void(*loop)(double);
    void(*deinit)(void);
    void *userdata;
    fwt_state_t state;
    fwt_command_queue_t commands;
    sg_shader shader;
} fwt_t;

void sapp_input_clear(void);
void sapp_input_update(void);
void sapp_input_handler(const sapp_event* e);

void fwt_set_current_matrix(hmm_mat4 mat);
hmm_mat4* fwt_current_matrix(void);
void fwt_push_command(int type, void *data);
void fwt_push_vertex(void);
hmm_mat4* fwt_matrix_stack_head(int mode);

#endif
