/* draw.c -- https://github.com/takeiteasy/fun-with-triangles

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
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_app.h"
#include "sokol/sokol_glue.h"
#include "sokol/sokol_time.h"
#include "sokol/sokol_log.h"
#define SOKOL_IMPL
#include "sokol/util/sokol_shape.h"
#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"
#define QOI_IMPLEMENTATION
#include "qoi.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_NO_GIF
#include "stb_image.h"
#include "fwt.glsl.h"

#if defined(FWT_WINDOWS)
#include <windows.h>
#include <io.h>
#include <dirent.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif // FWT_WINDOWS

#if !defined(DEFAULT_WINDOW_WIDTH)
#define DEFAULT_WINDOW_WIDTH 640
#endif

#if !defined(DEFAULT_WINDOW_HEIGHT)
#define DEFAULT_WINDOW_HEIGHT 480
#endif

#if !defined(DEFAULT_WINDOW_TITLE)
#define DEFAULT_WINDOW_TITLE "SIM"
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

static struct fwt_t {
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
} sim = {
    .running = 0,
    .mouse_hidden = 0,
    .mouse_locked = 0,
    .app_desc = (sapp_desc) {
        .width = DEFAULT_WINDOW_WIDTH,
        .height = DEFAULT_WINDOW_WIDTH,
        .window_title = DEFAULT_WINDOW_TITLE
    },
    .userdata = NULL
};

static hmm_mat4* fwt_matrix_stack_head(int mode) {
    assert(mode >= 0 && mode < FWT_MATRIXMODE_COUNT);
    fwt_matrix_stack_t *stack = &sim.state.matrix_stack[mode];
    return stack->count ? &stack->stack[stack->count-1] : NULL;
}

static hmm_mat4* fwt_current_matrix(void) {
    return fwt_matrix_stack_head(sim.state.matrix_mode);
}

static void fwt_set_current_matrix(hmm_mat4 mat) {
    hmm_mat4 *head = fwt_current_matrix();
    *head = mat;
}

static void fwt_push_vertex(void) {
    sim.state.draw_call.vertices = realloc(sim.state.draw_call.vertices, ++sim.state.draw_call.vcount * sizeof(fwt_vertex_t));
    memcpy(&sim.state.draw_call.vertices[sim.state.draw_call.vcount-1], &sim.state.current_vertex, sizeof(fwt_vertex_t));
}

static void fwt_push_command(int type, void *data) {
    fwt_command_t *command = malloc(sizeof(fwt_command_t));
    command->type = type;
    command->data = data;
    command->next = NULL;

    if (!sim.commands.head && !sim.commands.tail)
        sim.commands.head = sim.commands.tail = command;
    else {
        sim.commands.tail->next = command;
        sim.commands.tail = command;
    }
}

extern void sapp_input_clear(void);
extern void sapp_input_update(void);
extern void sapp_input_handler(const sapp_event* e);

static void init(void) {
    sg_desc desc = {
        .environment = sglue_environment(),
        .logger.func = slog_func,
        .buffer_pool_size = 256
    };
    sg_setup(&desc);
    stm_setup();

    sim.shader = sg_make_shader(fwt_shader_desc(sg_query_backend()));
    sim.state.pip_desc = (sg_pipeline_desc) {
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
        .shader = sim.shader,
        .cull_mode = SG_CULLMODE_BACK,
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true
        }
    };

    for (int i = 0; i < FWT_MATRIXMODE_COUNT; i++) {
        fwt_matrix_stack_t *stack = &sim.state.matrix_stack[i];
        stack->count = 1;
        stack->stack[0] = HMM_Mat4();
    }

    sim.state.matrix_stack[FWT_MATRIXMODE_TEXTURE].stack[0] = HMM_Mat4d(1.f);
    sim.state.blend_mode = -1;
    fwtBlendMode(FWT_BLEND_DEFAULT);
    sim.state.pip_desc.depth.compare = -1;
    fwtDepthFunc(FWT_CMP_DEFAULT);
    sim.state.pip_desc.cull_mode = -1;
    fwtCullMode(FWT_CULL_DEFAULT);

    sapp_input_clear();
    if (sim.init)
        sim.init();
}

static void frame(void) {
    const float t = (float)(sapp_frame_duration() * 60.);
    sim.loop(t);

    sg_begin_pass(&(sg_pass) {
        .action = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = sim.state.clear_color
            }
        },
        .swapchain = sglue_swapchain()
    });
    fwt_command_t *cursor = sim.commands.head;
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
    sim.commands.head = sim.commands.tail = NULL;
    sg_end_pass();
    sg_commit();
    sapp_input_update();
}

static void event(const sapp_event *e) {
    sapp_input_handler(e);
}

static void cleanup(void) {
    if (sim.deinit)
        sim.deinit();
    sg_shutdown();
}

void fwtSetWindowSize(int width, int height) {
    if (!sim.running) {
        if (width)
            sim.app_desc.width = width;
        if (height)
            sim.app_desc.height = height;
    }
}

void fwtSetWindowTitle(const char *title) {
    if (sim.running)
        sapp_set_window_title(title);
    else
        sim.app_desc.window_title = title;
}

void fwtMatrixMode(int mode) {
    switch (mode) {
        case FWT_MATRIXMODE_PROJECTION:
        case FWT_MATRIXMODE_TEXTURE:
        case FWT_MATRIXMODE_MODELVIEW:
            sim.state.matrix_mode = mode;
            break;
        default:
            abort(); // unknown mode
    }
}

void fwtPushMatrix(void) {
    fwt_matrix_stack_t *cs = &sim.state.matrix_stack[sim.state.matrix_mode];
    assert(cs->count + 1 < MAX_MATRIX_STACK);
    cs->stack[cs->count] = cs->stack[cs->count-1];
    cs->count++;
}

void fwtPopMatrix(void) {
    fwt_matrix_stack_t *cs = &sim.state.matrix_stack[sim.state.matrix_mode];
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
    sim.state.clear_color = (sg_color){r, g, b, a};
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
    if (mode == sim.state.blend_mode)
        return;
    sg_blend_state *blend = &sim.state.blend;
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
    sim.state.blend_mode = mode;
}

void fwtDepthFunc(int func) {
    if (func == sim.state.pip_desc.depth.compare)
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
            sim.state.pip_desc.depth.compare = (sg_compare_func)func;
            break;
    }
}

void fwtCullMode(int mode) {
    if (mode == sim.state.pip_desc.cull_mode)
        return;
    switch (mode) {
        default:
        case FWT_CULL_DEFAULT:
            mode = FWT_CULL_BACK;
        case FWT_CULL_NONE:
        case FWT_CULL_FRONT:
        case FWT_CULL_BACK:
            sim.state.pip_desc.cull_mode = mode;
            break;
    }
}

void fwtBegin(int mode) {
    assert(!sim.state.draw_call.vertices && !sim.state.draw_call.instances);
    if (sim.state.draw_call.vertices)
        fwtEnd();
    sim.state.draw_call.vertices = malloc(0);
    sim.state.draw_call.vcount = 0;
    sim.state.draw_call.instances = malloc(0);
    sim.state.draw_call.icount = 0;
    switch (mode) {
        default:
            mode = FWT_DRAW_TRIANGLES;
        case FWT_DRAW_TRIANGLES:
        case FWT_DRAW_POINTS:
        case FWT_DRAW_LINES:
        case FWT_DRAW_LINE_STRIP:
        case FWT_DRAW_TRIANGLE_STRIP:
            sim.state.pip_desc.primitive_type = mode;
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
    sim.state.current_vertex.position = HMM_Vec4(x, y, z, 1.f);
    fwt_push_vertex();
}

void fwtTexCoord2f(float x, float y) {
    sim.state.current_vertex.texcoord = HMM_Vec2(x, y);
}

void fwtNormal3f(float x, float y, float z) {
    sim.state.current_vertex.normal = HMM_Vec3(x, y, z);
}

void fwtColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    sim.state.current_vertex.color = HMM_Vec4((float)r / 255.f,
                                              (float)g / 255.f,
                                              (float)b / 255.f,
                                              (float)a / 255.f);
}

void fwtColor3f(float x, float y, float z) {
    sim.state.current_vertex.color = HMM_Vec4(x, y, z, 1.f);
}

void fwtColor4f(float x, float y, float z, float w) {
    sim.state.current_vertex.color = HMM_Vec4(x, y, z, w);
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
    sim.state.draw_call.instances = realloc(sim.state.draw_call.instances, ++sim.state.draw_call.icount * sizeof(fwt_vs_inst_t));
    fwt_vs_inst_t *inst = &sim.state.draw_call.instances[sim.state.draw_call.icount-1];
    hmm_mat4 *m = fwt_matrix_stack_head(FWT_MATRIXMODE_MODELVIEW);
    make_vs_inst(inst, m ? *m : HMM_Mat4());
}

void fwtEnd(void) {
    if (!sim.state.draw_call.instances || !sim.state.draw_call.icount)
        goto BAIL;
    if (!sim.state.draw_call.vertices || !sim.state.draw_call.vcount) {
//        if (sg_query_buffer_state(sim.state.current_buffer) != SG_RESOURCESTATE_VALID)
            goto BAIL;
    }

    sim.state.draw_call.pip = sg_make_pipeline(&sim.state.pip_desc);
    sim.state.draw_call.projection = *fwt_matrix_stack_head(FWT_MATRIXMODE_PROJECTION);
    sim.state.draw_call.texture_matrix = *fwt_matrix_stack_head(FWT_MATRIXMODE_TEXTURE);

    sg_buffer_desc inst_desc = {
        .size = sim.state.draw_call.icount * sizeof(fwt_vs_inst_t),
        .usage.stream_update = true
    };
    sg_buffer vbuf = (sg_buffer){.id=fwtStoreBuffer()};
    sim.state.draw_call.bind = (sg_bindings) {
        .vertex_buffers[0] = vbuf,
        .vertex_buffers[1] = sg_make_buffer(&inst_desc),
        .images[IMG_texture_v] = sim.state.current_texture,
        .samplers = sg_make_sampler(&sim.state.sampler_desc)
    };
    sg_range r0 = {
        .ptr = sim.state.draw_call.instances,
        .size = sim.state.draw_call.icount * sizeof(fwt_vs_inst_t)
    };
    sg_update_buffer(sim.state.draw_call.bind.vertex_buffers[1], &r0);
    fwt_draw_call_t *draw_call = malloc(sizeof(fwt_draw_call_t));
    memcpy(draw_call, &sim.state.draw_call, sizeof(fwt_draw_call_t));
    fwt_push_command(FWT_CMD_DRAW_CALL, draw_call);

BAIL:
    if (sim.state.draw_call.vertices) {
        free(sim.state.draw_call.vertices);
        sim.state.draw_call.vertices = NULL;
    }
    if (sim.state.draw_call.instances) {
        free(sim.state.draw_call.instances);
        sim.state.draw_call.instances = NULL;
    }
    memset(&sim.state.draw_call, 0, sizeof(fwt_draw_call_t));
    memset(&sim.state.sampler_desc, 0, sizeof(sg_sampler_desc));
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
    sim.state.current_texture = tmp;
}

void fwtPopTexture(void) {
    memset(&sim.state.current_texture, 0, sizeof(sg_image));
}

static int does_file_exist(const char *path) {
    return !access(path, F_OK);
}

static const char* file_extension(const char *path) {
    const char *dot = strrchr(path, '.');
    return !dot || dot == path ? NULL : dot + 1;
}

int fwtLoadTexturePath(const char *path) {
    assert(does_file_exist(path));
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
    FILE *fh = fopen(path, "rb");
    assert(fh);
    fseek(fh, 0, SEEK_END);
    sz = ftell(fh);
    fseek(fh, 0, SEEK_SET);

    unsigned char *data = malloc(sz * sizeof(unsigned char));
    fread(data, sz, 1, fh);
    fclose(fh);
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
    assert(sg_query_image_state(sim.state.current_texture) == SG_RESOURCESTATE_VALID);
    set_filter(&sim.state.sampler_desc.min_filter, min);
    set_filter(&sim.state.sampler_desc.mag_filter, mag);
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
    assert(sg_query_image_state(sim.state.current_texture) == SG_RESOURCESTATE_VALID);
    set_wrap(&sim.state.sampler_desc.wrap_u, wrap_u);
    set_wrap(&sim.state.sampler_desc.wrap_v, wrap_v);
}

void fwtReleaseTexture(int texture) {
    sg_image tmp = {.id = texture};
    if (sg_query_image_state(tmp) == SG_RESOURCESTATE_VALID)
        sg_destroy_image(tmp);
}

int fwtStoreBuffer(void) {
    sg_buffer_desc desc = {
        .data = (sg_range) {
            .ptr = sim.state.draw_call.vertices,
            .size = sim.state.draw_call.vcount * sizeof(fwt_vertex_t)
        }
    };
    sg_buffer result = sg_make_buffer(&desc);
    assert(sg_query_buffer_state(result) == SG_RESOURCESTATE_VALID);
    return result.id;
}
