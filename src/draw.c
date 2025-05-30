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
#include "internal.h"
#define SOKOL_IMPL
#include "sokol/util/sokol_shape.h"
#define QOI_IMPLEMENTATION
#include "qoi.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_NO_GIF
#include "stb_image.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <io.h>
#include <dirent.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif

extern fwt_t fwt;

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
