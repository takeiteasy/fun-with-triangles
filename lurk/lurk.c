/* lurk.c -- https://github.com/takeiteasy/lurk

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

#define SOKOL_IMPL
#define JIM_IMPLEMENTATION
#define MJSON_IMPLEMENTATION
#define HASHMAP_IMPL
#define QOI_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define EZ_IMPLEMENTATION
#define IMAP_IMPLEMENTATION
#define DMON_IMPL
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#include "lurk.h"
#pragma clang diagnostic pop
#if defined(LURK_WINDOW)
#include "dirent_win32.h"
#endif

#if !defined(LURK_SCENE)
static lurkTexture* NewTexture(sg_image_desc *desc) {
    lurkTexture *result = malloc(sizeof(lurkTexture));
    result->internal = sg_make_image(desc);
    result->w = desc->width;
    result->h = desc->height;
    return result;
}

static lurkTexture* EmptyTexture(unsigned int w, unsigned int h) {
    sg_image_desc desc = {
        .width = w,
        .height = h,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .usage = SG_USAGE_STREAM
    };
    return NewTexture(&desc);
}

static void DestroyTexture(lurkTexture *texture) {
    if (texture) {
        if (sg_query_image_state(texture->internal) == SG_RESOURCESTATE_VALID)
            sg_destroy_image(texture->internal);
        free(texture);
    }
}

#define QOI_MAGIC (((unsigned int)'q') << 24 | ((unsigned int)'o') << 16 | ((unsigned int)'i') <<  8 | ((unsigned int)'f'))

static bool CheckQOI(unsigned char *data) {
    return (data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]) == QOI_MAGIC;
}

#define RGBA(R, G, B, A) (((unsigned int)(A) << 24) | ((unsigned int)(R) << 16) | ((unsigned int)(G) << 8) | (B))

static int* LoadImage(unsigned char *data, int sizeOfData, int *w, int *h) {
    assert(data && sizeOfData);
    int _w, _h, c;
    unsigned char *in = NULL;
    if (CheckQOI(data)) {
        qoi_desc desc;
        in = qoi_decode(data, sizeOfData, &desc, 4);
        _w = desc.width;
        _h = desc.height;
    } else
        in = stbi_load_from_memory(data, sizeOfData, &_w, &_h, &c, 4);
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

static void UpdateTexture(lurkTexture *texture, int *data, int w, int h) {
    if (texture->w != w || texture->h != h) {
        DestroyTexture(texture);
        texture = EmptyTexture(w, h);
    }
    sg_image_data desc = {
        .subimage[0][0] = (sg_range) {
            .ptr = data,
            .size = w * h * sizeof(int)
        }
    };
    sg_update_image(texture->internal, &desc);
}

lurkState state = {
    .running = false,
    .desc = (sapp_desc) {
#define X(NAME, TYPE, VAL, DEFAULT, DOCS) .VAL = DEFAULT,
        SETTINGS
#undef X
        .window_title = DEFAULT_WINDOW_TITLE
    },
    .pass_action = {
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = {0}
        }
    }
};
#endif

//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
//-----------------------------------------------------------------------------
static void MM86128(const void *key, const int len, uint32_t seed, void *out) {
#define ROTL32(x, r) ((x << r) | (x >> (32 - r)))
#define FMIX32(h) h^=h>>16; h*=0x85ebca6b; h^=h>>13; h*=0xc2b2ae35; h^=h>>16;
    const uint8_t * data = (const uint8_t*)key;
    const int nblocks = len / 16;
    uint32_t h1 = seed;
    uint32_t h2 = seed;
    uint32_t h3 = seed;
    uint32_t h4 = seed;
    uint32_t c1 = 0x239b961b;
    uint32_t c2 = 0xab0e9789;
    uint32_t c3 = 0x38b34ae5;
    uint32_t c4 = 0xa1e38b93;
    const uint32_t * blocks = (const uint32_t *)(data + nblocks*16);
    for (int i = -nblocks; i; i++) {
        uint32_t k1 = blocks[i*4+0];
        uint32_t k2 = blocks[i*4+1];
        uint32_t k3 = blocks[i*4+2];
        uint32_t k4 = blocks[i*4+3];
        k1 *= c1; k1  = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
        h1 = ROTL32(h1,19); h1 += h2; h1 = h1*5+0x561ccd1b;
        k2 *= c2; k2  = ROTL32(k2,16); k2 *= c3; h2 ^= k2;
        h2 = ROTL32(h2,17); h2 += h3; h2 = h2*5+0x0bcaa747;
        k3 *= c3; k3  = ROTL32(k3,17); k3 *= c4; h3 ^= k3;
        h3 = ROTL32(h3,15); h3 += h4; h3 = h3*5+0x96cd1c35;
        k4 *= c4; k4  = ROTL32(k4,18); k4 *= c1; h4 ^= k4;
        h4 = ROTL32(h4,13); h4 += h1; h4 = h4*5+0x32ac3b17;
    }
    const uint8_t * tail = (const uint8_t*)(data + nblocks*16);
    uint32_t k1 = 0;
    uint32_t k2 = 0;
    uint32_t k3 = 0;
    uint32_t k4 = 0;
    switch(len & 15) {
        case 15:
            k4 ^= tail[14] << 16;
        case 14:
            k4 ^= tail[13] << 8;
        case 13:
            k4 ^= tail[12] << 0;
            k4 *= c4; k4  = ROTL32(k4,18); k4 *= c1; h4 ^= k4;
        case 12:
            k3 ^= tail[11] << 24;
        case 11:
            k3 ^= tail[10] << 16;
        case 10:
            k3 ^= tail[ 9] << 8;
        case 9:
            k3 ^= tail[ 8] << 0;
            k3 *= c3; k3  = ROTL32(k3,17); k3 *= c4; h3 ^= k3;
        case 8:
            k2 ^= tail[ 7] << 24;
        case 7:
            k2 ^= tail[ 6] << 16;
        case 6:
            k2 ^= tail[ 5] << 8;
        case 5:
            k2 ^= tail[ 4] << 0;
            k2 *= c2; k2  = ROTL32(k2,16); k2 *= c3; h2 ^= k2;
        case 4:
            k1 ^= tail[ 3] << 24;
        case 3:
            k1 ^= tail[ 2] << 16;
        case 2:
            k1 ^= tail[ 1] << 8;
        case 1:
            k1 ^= tail[ 0] << 0;
            k1 *= c1; k1  = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
    };
    h1 ^= len; h2 ^= len; h3 ^= len; h4 ^= len;
    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;
    FMIX32(h1); FMIX32(h2); FMIX32(h3); FMIX32(h4);
    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;
    ((uint32_t*)out)[0] = h1;
    ((uint32_t*)out)[1] = h2;
    ((uint32_t*)out)[2] = h3;
    ((uint32_t*)out)[3] = h4;
}

static uint64_t MurmurHash(const void *data, size_t len, uint32_t seed) {
    char out[16];
    MM86128(data, (int)len, (uint32_t)seed, &out);
    return *(uint64_t*)out;
}

typedef enum {
    lurkCommandProject,
    lurkCommandResetProject,
    lurkCommandPushTransform,
    lurkCommandPopTransform,
    lurkCommandResetTransform,
    lurkCommandTranslate,
    lurkCommandRotate,
    lurkCommandRotateAt,
    lurkCommandScale,
    lurkCommandScaleAt,
    lurkCommandResetPipeline,
    lurkCommandSetUniform,
    lurkCommandResetUniform,
    lurkCommandSetBlendMode,
    lurkCommandResetBlendMode,
    lurkCommandSetColor,
    lurkCommandResetColor,
    lurkCommandSetImage,
    lurkCommandUnsetImage,
    lurkCommandResetImage,
    lurkCommandResetSampler,
    lurkCommandViewport,
    lurkCommandResetViewport,
    lurkCommandScissor,
    lurkCommandResetScissor,
    lurkCommandResetState,
    lurkCommandClear,
    lurkCommandDrawPoints,
    lurkCommandDrawPoint,
    lurkCommandDrawLines,
    lurkCommandDrawLine,
    lurkCommandDrawLinesStrip,
    lurkCommandDrawFilledTriangles,
    lurkCommandDrawFilledTriangle,
    lurkCommandDrawFilledTrianglesStrip,
    lurkCommandDrawFilledRects,
    lurkCommandDrawFilledRect,
    lurkCommandDrawTexturedRects,
    lurkCommandDrawTexturedRect,
    lurkCommandCreateTexture
} lurkCommandType;

typedef struct {
    lurkCommandType type;
    void* data;
} lurkCommand;

static void PushCommand(lurkState* state, lurkCommand* command) {
    ezStackAppend(&state->commandQueue, command->type, (void*)command);
}

typedef struct {
    float left;
    float right;
    float top;
    float bottom;
} lurkProjectData;

void lurkProject(lurkState *state, float left, float right, float top, float bottom) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandProject;
    lurkProjectData* cmdData = malloc(sizeof(lurkProjectData));
    cmdData->left = left;
    cmdData->right = right;
    cmdData->top = top;
    cmdData->bottom = bottom;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

void lurkResetProject(lurkState *state) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandResetProject;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

void lurkPushTransform(lurkState *state) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandPushTransform;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

void lurkPopTransform(lurkState *state) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandPopTransform;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

void lurkResetTransform(lurkState *state) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandResetTransform;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

typedef struct {
    float x;
    float y;
} lurkTranslateData;

void lurkTranslate(lurkState *state, float x, float y) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandTranslate;
    lurkTranslateData* cmdData = malloc(sizeof(lurkTranslateData));
    cmdData->x = x;
    cmdData->y = y;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    float theta;
} lurkRotateData;

void lurkRotate(lurkState *state, float theta) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandRotate;
    lurkRotateData* cmdData = malloc(sizeof(lurkRotateData));
    cmdData->theta = theta;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    float theta;
    float x;
    float y;
} lurkRotateAtData;

void lurkRotateAt(lurkState *state, float theta, float x, float y) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandRotateAt;
    lurkRotateAtData* cmdData = malloc(sizeof(lurkRotateAtData));
    cmdData->theta = theta;
    cmdData->x = x;
    cmdData->y = y;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    float sx;
    float sy;
} lurkScaleData;

void lurkScale(lurkState *state, float sx, float sy) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandScale;
    lurkScaleData* cmdData = malloc(sizeof(lurkScaleData));
    cmdData->sx = sx;
    cmdData->sy = sy;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    float sx;
    float sy;
    float x;
    float y;
} lurkScaleAtData;

void lurkScaleAt(lurkState *state, float sx, float sy, float x, float y) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandScaleAt;
    lurkScaleAtData* cmdData = malloc(sizeof(lurkScaleAtData));
    cmdData->sx = sx;
    cmdData->sy = sy;
    cmdData->x = x;
    cmdData->y = y;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

void lurkResetPipeline(lurkState *state) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandResetPipeline;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

typedef struct {
    void* data;
    int size;
} lurkSetUniformData;

void lurkSetUniform(lurkState *state, void* data, int size) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandSetUniform;
    lurkSetUniformData* cmdData = malloc(sizeof(lurkSetUniformData));
    cmdData->data = data;
    cmdData->size = size;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

void lurkResetUniform(lurkState *state) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandResetUniform;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

typedef struct {
    sgp_blend_mode blend_mode;
} lurkSetBlendModeData;

void lurkSetBlendMode(lurkState *state, sgp_blend_mode blend_mode) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandSetBlendMode;
    lurkSetBlendModeData* cmdData = malloc(sizeof(lurkSetBlendModeData));
    cmdData->blend_mode = blend_mode;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

void lurkResetBlendMode(lurkState *state) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandResetBlendMode;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

typedef struct {
    float r;
    float g;
    float b;
    float a;
} lurkSetColorData;

void lurkSetColor(lurkState *state, float r, float g, float b, float a) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandSetColor;
    lurkSetColorData* cmdData = malloc(sizeof(lurkSetColorData));
    cmdData->r = r;
    cmdData->g = g;
    cmdData->b = b;
    cmdData->a = a;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

void lurkResetColor(lurkState *state) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandResetColor;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

typedef struct {
    int channel;
    lurkTexture* texture;
} lurkSetImageData;

void lurkSetImage(lurkState* state, uint64_t texture_id, int channel) {
    assert(texture_id);
    imap_slot_t* slot = imap_lookup(state->textureMap, texture_id);
    assert(slot);
    lurkTexture* texture = (lurkTexture*)imap_getval64(state->textureMap, slot);
    assert(texture);

    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandSetImage;
    lurkSetImageData* cmdData = malloc(sizeof(lurkSetImageData));
    cmdData->channel = channel;
    cmdData->texture = texture;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    int channel;
} lurkUnsetImageData;

void lurkUnsetImage(lurkState *state, int channel) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandUnsetImage;
    lurkUnsetImageData* cmdData = malloc(sizeof(lurkUnsetImageData));
    cmdData->channel = channel;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    int channel;
} lurkResetImageData;

void lurkResetImage(lurkState *state, int channel) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandResetImage;
    lurkResetImageData* cmdData = malloc(sizeof(lurkResetImageData));
    cmdData->channel = channel;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    int channel;
} lurkResetSamplerData;

void lurkResetSampler(lurkState *state, int channel) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandResetSampler;
    lurkResetSamplerData* cmdData = malloc(sizeof(lurkResetSamplerData));
    cmdData->channel = channel;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    int x;
    int y;
    int w;
    int h;
} lurkViewportData;

void lurkViewport(lurkState *state, int x, int y, int w, int h) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandViewport;
    lurkViewportData* cmdData = malloc(sizeof(lurkViewportData));
    cmdData->x = x;
    cmdData->y = y;
    cmdData->w = w;
    cmdData->h = h;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

void lurkResetViewport(lurkState *state) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandResetViewport;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

typedef struct {
    int x;
    int y;
    int w;
    int h;
} lurkScissorData;

void lurkScissor(lurkState *state, int x, int y, int w, int h) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandScissor;
    lurkScissorData* cmdData = malloc(sizeof(lurkScissorData));
    cmdData->x = x;
    cmdData->y = y;
    cmdData->w = w;
    cmdData->h = h;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

void lurkResetScissor(lurkState *state) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandResetScissor;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

void lurkResetState(lurkState *state) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandResetState;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

void lurkClear(lurkState *state) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandClear;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

typedef struct {
    sgp_point* points;
    int count;
} lurkDrawPointsData;

void lurkDrawPoints(lurkState *state, sgp_point* points, int count) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandDrawPoints;
    lurkDrawPointsData* cmdData = malloc(sizeof(lurkDrawPointsData));
    cmdData->points = points;
    cmdData->count = count;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    float x;
    float y;
} lurkDrawPointData;

void lurkDrawPoint(lurkState *state, float x, float y) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandDrawPoint;
    lurkDrawPointData* cmdData = malloc(sizeof(lurkDrawPointData));
    cmdData->x = x;
    cmdData->y = y;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    sgp_line* lines;
    int count;
} lurkDrawLinesData;

void lurkDrawLines(lurkState *state, sgp_line* lines, int count) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandDrawLines;
    lurkDrawLinesData* cmdData = malloc(sizeof(lurkDrawLinesData));
    cmdData->lines = lines;
    cmdData->count = count;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    float ax;
    float ay;
    float bx;
    float by;
} lurkDrawLineData;

void lurkDrawLine(lurkState *state, float ax, float ay, float bx, float by) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandDrawLine;
    lurkDrawLineData* cmdData = malloc(sizeof(lurkDrawLineData));
    cmdData->ax = ax;
    cmdData->ay = ay;
    cmdData->bx = bx;
    cmdData->by = by;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    sgp_point* points;
    int count;
} lurkDrawLinesStripData;

void lurkDrawLinesStrip(lurkState *state, sgp_point* points, int count) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandDrawLinesStrip;
    lurkDrawLinesStripData* cmdData = malloc(sizeof(lurkDrawLinesStripData));
    cmdData->points = points;
    cmdData->count = count;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    sgp_triangle* triangles;
    int count;
} lurkDrawFilledTrianglesData;

void lurkDrawFilledTriangles(lurkState *state, sgp_triangle* triangles, int count) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandDrawFilledTriangles;
    lurkDrawFilledTrianglesData* cmdData = malloc(sizeof(lurkDrawFilledTrianglesData));
    cmdData->triangles = triangles;
    cmdData->count = count;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    float ax;
    float ay;
    float bx;
    float by;
    float cx;
    float cy;
} lurkDrawFilledTriangleData;

void lurkDrawFilledTriangle(lurkState *state, float ax, float ay, float bx, float by, float cx, float cy) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandDrawFilledTriangle;
    lurkDrawFilledTriangleData* cmdData = malloc(sizeof(lurkDrawFilledTriangleData));
    cmdData->ax = ax;
    cmdData->ay = ay;
    cmdData->bx = bx;
    cmdData->by = by;
    cmdData->cx = cx;
    cmdData->cy = cy;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    sgp_point* points;
    int count;
} lurkDrawFilledTrianglesStripData;

void lurkDrawFilledTrianglesStrip(lurkState *state, sgp_point* points, int count) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandDrawFilledTrianglesStrip;
    lurkDrawFilledTrianglesStripData* cmdData = malloc(sizeof(lurkDrawFilledTrianglesStripData));
    cmdData->points = points;
    cmdData->count = count;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    sgp_rect* rects;
    int count;
} lurkDrawFilledRectsData;

void lurkDrawFilledRects(lurkState *state, sgp_rect* rects, int count) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandDrawFilledRects;
    lurkDrawFilledRectsData* cmdData = malloc(sizeof(lurkDrawFilledRectsData));
    cmdData->rects = rects;
    cmdData->count = count;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    float x;
    float y;
    float w;
    float h;
} lurkDrawFilledRectData;

void lurkDrawFilledRect(lurkState *state, float x, float y, float w, float h) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandDrawFilledRect;
    lurkDrawFilledRectData* cmdData = malloc(sizeof(lurkDrawFilledRectData));
    cmdData->x = x;
    cmdData->y = y;
    cmdData->w = w;
    cmdData->h = h;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    int channel;
    sgp_textured_rect* rects;
    int count;
} lurkDrawTexturedRectsData;

void lurkDrawTexturedRects(lurkState *state, int channel, sgp_textured_rect* rects, int count) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandDrawTexturedRects;
    lurkDrawTexturedRectsData* cmdData = malloc(sizeof(lurkDrawTexturedRectsData));
    cmdData->channel = channel;
    cmdData->rects = rects;
    cmdData->count = count;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    int channel;
    sgp_rect dest_rect;
    sgp_rect src_rect;
} lurkDrawTexturedRectData;

void lurkDrawTexturedRect(lurkState *state, int channel, sgp_rect dest_rect, sgp_rect src_rect) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandDrawTexturedRect;
    lurkDrawTexturedRectData* cmdData = malloc(sizeof(lurkDrawTexturedRectData));
    cmdData->channel = channel;
    cmdData->dest_rect = dest_rect;
    cmdData->src_rect = src_rect;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    ezImage *image;
    const char *name;
} lurkCreateTextureData;

void lurkCreateTexture(lurkState *state, const char *name, ezImage *image) {
    lurkCommand* cmd = malloc(sizeof(lurkCommand));
    cmd->type = lurkCommandDrawTexturedRect;
    lurkCreateTextureData* cmdData = malloc(sizeof(lurkCreateTextureData));
    cmdData->name = name;
    cmdData->image = image;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

#if !defined(LURK_SCENE)
static void FreeCommand(lurkCommand* command) {
    lurkCommandType type = command->type;
    switch
(type) {
    case lurkCommandProject: {
        lurkProjectData* data = (lurkProjectData*)command->data;
        free(data);
        break;
    }
    case lurkCommandTranslate: {
        lurkTranslateData* data = (lurkTranslateData*)command->data;
        free(data);
        break;
    }
    case lurkCommandRotate: {
        lurkRotateData* data = (lurkRotateData*)command->data;
        free(data);
        break;
    }
    case lurkCommandRotateAt: {
        lurkRotateAtData* data = (lurkRotateAtData*)command->data;
        free(data);
        break;
    }
    case lurkCommandScale: {
        lurkScaleData* data = (lurkScaleData*)command->data;
        free(data);
        break;
    }
    case lurkCommandScaleAt: {
        lurkScaleAtData* data = (lurkScaleAtData*)command->data;
        free(data);
        break;
    }
    case lurkCommandSetUniform: {
        lurkSetUniformData* data = (lurkSetUniformData*)command->data;
        free(data);
        break;
    }
    case lurkCommandSetBlendMode: {
        lurkSetBlendModeData* data = (lurkSetBlendModeData*)command->data;
        free(data);
        break;
    }
    case lurkCommandSetColor: {
        lurkSetColorData* data = (lurkSetColorData*)command->data;
        free(data);
        break;
    }
    case lurkCommandSetImage: {
        lurkSetImageData* data = (lurkSetImageData*)command->data;
        free(data);
        break;
    }
    case lurkCommandUnsetImage: {
        lurkUnsetImageData* data = (lurkUnsetImageData*)command->data;
        free(data);
        break;
    }
    case lurkCommandResetImage: {
        lurkResetImageData* data = (lurkResetImageData*)command->data;
        free(data);
        break;
    }
    case lurkCommandResetSampler: {
        lurkResetSamplerData* data = (lurkResetSamplerData*)command->data;
        free(data);
        break;
    }
    case lurkCommandViewport: {
        lurkViewportData* data = (lurkViewportData*)command->data;
        free(data);
        break;
    }
    case lurkCommandScissor: {
        lurkScissorData* data = (lurkScissorData*)command->data;
        free(data);
        break;
    }
    case lurkCommandDrawPoints: {
        lurkDrawPointsData* data = (lurkDrawPointsData*)command->data;
        free(data);
        break;
    }
    case lurkCommandDrawPoint: {
        lurkDrawPointData* data = (lurkDrawPointData*)command->data;
        free(data);
        break;
    }
    case lurkCommandDrawLines: {
        lurkDrawLinesData* data = (lurkDrawLinesData*)command->data;
        free(data);
        break;
    }
    case lurkCommandDrawLine: {
        lurkDrawLineData* data = (lurkDrawLineData*)command->data;
        free(data);
        break;
    }
    case lurkCommandDrawLinesStrip: {
        lurkDrawLinesStripData* data = (lurkDrawLinesStripData*)command->data;
        free(data);
        break;
    }
    case lurkCommandDrawFilledTriangles: {
        lurkDrawFilledTrianglesData* data = (lurkDrawFilledTrianglesData*)command->data;
        free(data);
        break;
    }
    case lurkCommandDrawFilledTriangle: {
        lurkDrawFilledTriangleData* data = (lurkDrawFilledTriangleData*)command->data;
        free(data);
        break;
    }
    case lurkCommandDrawFilledTrianglesStrip: {
        lurkDrawFilledTrianglesStripData* data = (lurkDrawFilledTrianglesStripData*)command->data;
        free(data);
        break;
    }
    case lurkCommandDrawFilledRects: {
        lurkDrawFilledRectsData* data = (lurkDrawFilledRectsData*)command->data;
        free(data);
        break;
    }
    case lurkCommandDrawFilledRect: {
        lurkDrawFilledRectData* data = (lurkDrawFilledRectData*)command->data;
        free(data);
        break;
    }
    case lurkCommandDrawTexturedRects: {
        lurkDrawTexturedRectsData* data = (lurkDrawTexturedRectsData*)command->data;
        free(data);
        break;
    }
    case lurkCommandDrawTexturedRect: {
        lurkDrawTexturedRectData* data = (lurkDrawTexturedRectData*)command->data;
        free(data);
        break;
    }
    default:
        break;
    }
    free(command);
}

static void ProcessCommand(lurkCommand* command) {
    lurkCommandType type = command->type;
    switch (type) {
    case lurkCommandProject: {
        lurkProjectData* data = (lurkProjectData*)command->data;
        sgp_project(data->left, data->right, data->top, data->bottom);
        break;
    }
    case lurkCommandResetProject:
        sgp_reset_project();
        break;
    case lurkCommandPushTransform:
        sgp_push_transform();
        break;
    case lurkCommandPopTransform:
        sgp_pop_transform();
        break;
    case lurkCommandResetTransform:
        sgp_reset_transform();
        break;
    case lurkCommandTranslate: {
        lurkTranslateData* data = (lurkTranslateData*)command->data;
        sgp_translate(data->x, data->y);
        break;
    }
    case lurkCommandRotate: {
        lurkRotateData* data = (lurkRotateData*)command->data;
        sgp_rotate(data->theta);
        break;
    }
    case lurkCommandRotateAt: {
        lurkRotateAtData* data = (lurkRotateAtData*)command->data;
        sgp_rotate_at(data->theta, data->x, data->y);
        break;
    }
    case lurkCommandScale: {
        lurkScaleData* data = (lurkScaleData*)command->data;
        sgp_scale(data->sx, data->sy);
        break;
    }
    case lurkCommandScaleAt: {
        lurkScaleAtData* data = (lurkScaleAtData*)command->data;
        sgp_scale_at(data->sx, data->sy, data->x, data->y);
        break;
    }
    case lurkCommandResetPipeline:
        sgp_reset_pipeline();
        break;
    case lurkCommandSetUniform: {
        lurkSetUniformData* data = (lurkSetUniformData*)command->data;
        sgp_set_uniform(data->data, data->size);
        break;
    }
    case lurkCommandResetUniform:
        sgp_reset_uniform();
        break;
    case lurkCommandSetBlendMode: {
        lurkSetBlendModeData* data = (lurkSetBlendModeData*)command->data;
        sgp_set_blend_mode(data->blend_mode);
        break;
    }
    case lurkCommandResetBlendMode:
        sgp_reset_blend_mode();
        break;
    case lurkCommandSetColor: {
        lurkSetColorData* data = (lurkSetColorData*)command->data;
        sgp_set_color(data->r, data->g, data->b, data->a);
        break;
    }
    case lurkCommandResetColor:
        sgp_reset_color();
        break;
    case lurkCommandSetImage: {
        lurkSetImageData* data = (lurkSetImageData*)command->data;
        sgp_set_image(data->channel, data->texture->internal);
        break;
    }
    case lurkCommandUnsetImage: {
        lurkUnsetImageData* data = (lurkUnsetImageData*)command->data;
        sgp_unset_image(data->channel);
        break;
    }
    case lurkCommandResetImage: {
        lurkResetImageData* data = (lurkResetImageData*)command->data;
        sgp_reset_image(data->channel);
        break;
    }
    case lurkCommandResetSampler: {
        lurkResetSamplerData* data = (lurkResetSamplerData*)command->data;
        sgp_reset_sampler(data->channel);
        break;
    }
    case lurkCommandViewport: {
        lurkViewportData* data = (lurkViewportData*)command->data;
        sgp_viewport(data->x, data->y, data->w, data->h);
        break;
    }
    case lurkCommandResetViewport:
        sgp_reset_viewport();
        break;
    case lurkCommandScissor: {
        lurkScissorData* data = (lurkScissorData*)command->data;
        sgp_scissor(data->x, data->y, data->w, data->h);
        break;
    }
    case lurkCommandResetScissor:
        sgp_reset_scissor();
        break;
    case lurkCommandResetState:
        sgp_reset_state();
        break;
    case lurkCommandClear:
        sgp_clear();
        break;
    case lurkCommandDrawPoints: {
        lurkDrawPointsData* data = (lurkDrawPointsData*)command->data;
        sgp_draw_points(data->points, data->count);
        break;
    }
    case lurkCommandDrawPoint: {
        lurkDrawPointData* data = (lurkDrawPointData*)command->data;
        sgp_draw_point(data->x, data->y);
        break;
    }
    case lurkCommandDrawLines: {
        lurkDrawLinesData* data = (lurkDrawLinesData*)command->data;
        sgp_draw_lines(data->lines, data->count);
        break;
    }
    case lurkCommandDrawLine: {
        lurkDrawLineData* data = (lurkDrawLineData*)command->data;
        sgp_draw_line(data->ax, data->ay, data->bx, data->by);
        break;
    }
    case lurkCommandDrawLinesStrip: {
        lurkDrawLinesStripData* data = (lurkDrawLinesStripData*)command->data;
        sgp_draw_lines_strip(data->points, data->count);
        break;
    }
    case lurkCommandDrawFilledTriangles: {
        lurkDrawFilledTrianglesData* data = (lurkDrawFilledTrianglesData*)command->data;
        sgp_draw_filled_triangles(data->triangles, data->count);
        break;
    }
    case lurkCommandDrawFilledTriangle: {
        lurkDrawFilledTriangleData* data = (lurkDrawFilledTriangleData*)command->data;
        sgp_draw_filled_triangle(data->ax, data->ay, data->bx, data->by, data->cx, data->cy);
        break;
    }
    case lurkCommandDrawFilledTrianglesStrip: {
        lurkDrawFilledTrianglesStripData* data = (lurkDrawFilledTrianglesStripData*)command->data;
        sgp_draw_filled_triangles_strip(data->points, data->count);
        break;
    }
    case lurkCommandDrawFilledRects: {
        lurkDrawFilledRectsData* data = (lurkDrawFilledRectsData*)command->data;
        sgp_draw_filled_rects(data->rects, data->count);
        break;
    }
    case lurkCommandDrawFilledRect: {
        lurkDrawFilledRectData* data = (lurkDrawFilledRectData*)command->data;
        sgp_draw_filled_rect(data->x, data->y, data->w, data->h);
        break;
    }
    case lurkCommandDrawTexturedRects: {
        lurkDrawTexturedRectsData* data = (lurkDrawTexturedRectsData*)command->data;
        sgp_draw_textured_rects(data->channel, data->rects, data->count);
        break;
    }
    case lurkCommandDrawTexturedRect: {
        lurkDrawTexturedRectData* data = (lurkDrawTexturedRectData*)command->data;
        sgp_draw_textured_rect(data->channel, data->dest_rect, data->src_rect);
        break;
    }
    case lurkCommandCreateTexture: {
        lurkCreateTextureData* data = (lurkCreateTextureData*)command->data;
        uint64_t hash = MurmurHash((void*)data->name, strlen(data->name), 0);
        imap_slot_t *slot = imap_assign(state.textureMap, hash);
        assert(!slot);
        lurkTexture* texture = EmptyTexture(data->image->w, data->image->h);
        UpdateTexture(texture, data->image->buf, data->image->w, data->image->h);
        imap_setval64(state.textureMap, slot, (uint64_t)texture);
        break;
    }
    default:
        abort();
    }
}
#endif

#if defined(LURK_WINDOWS)
static FILETIME Win32GetLastWriteTime(char* path) {
    FILETIME time;
    WIN32_FILE_ATTRIBUTE_DATA data;

    if (GetFileAttributesEx(path, GetFileExInfoStandard, &data))
        time = data.ftLastWriteTime;

    return time;
}
#endif

#if !defined(LURK_SCENE)
static bool ReloadLibrary(const char *path) {
#if defined(LURK_DISABLE_HOTRELOAD)
    return true;
#endif

#if defined(LURK_WINDOWS)
    FILETIME newTime = Win32GetLastWriteTime(path);
    bool result = CompareFileTime(&newTime, &state.libraryWriteTime);
    if (result)
        state.libraryWriteTime = newTime;
    else
        return true;
#else
    struct stat attr;
    bool result = !stat(path, &attr) && state.libraryHandleID != attr.st_ino;
    if (result)
        state.libraryHandleID = attr.st_ino;
    else
        return true;
#endif

    size_t libraryPathLength = state.libraryPath ? strlen(state.libraryPath) : 0;

    if (state.libraryHandle) {
        if (libraryPathLength != strlen(path) ||
            strncmp(state.libraryPath, path, libraryPathLength)) {
            if (state.libraryScene->deinit)
                state.libraryScene->deinit(&state, state.libraryContext);
        } else {
            if (state.libraryScene->unload)
                state.libraryScene->unload(&state, state.libraryContext);
        }
        dlclose(state.libraryHandle);
    }

#if defined(LURK_WINDOWS)
    size_t newPathSize = libraryPathLength + 4;
    char *newPath = malloc(sizeof(char) * newPathSize);
    char *noExt = RemoveExt(state.libraryPath);
    sprintf(newPath, "%s.tmp.dll", noExt);
    CopyFile(state.libraryPath, newPath, 0);
    if (!(state.libraryHandle = dlopen(newPath, RTLD_NOW)))
        goto BAIL;
    free(newPath);
    free(noExt);
    if (!state.libraryHandle)
#else
    if (!(state.libraryHandle = dlopen(path, RTLD_NOW)))
#endif
        goto BAIL;
    if (!(state.libraryScene = dlsym(state.libraryHandle, "scene")))
        goto BAIL;
    if (!state.libraryContext) {
        if (!(state.libraryContext = state.libraryScene->init(&state)))
            goto BAIL;
    } else {
        if (state.libraryScene->reload)
            state.libraryScene->reload(&state, state.libraryContext);
    }
    state.libraryPath = path;
    return true;

BAIL:
    if (state.libraryHandle)
        dlclose(state.libraryHandle);
    state.libraryHandle = NULL;
#if defined(LURK_WINDOWS)
    memset(&state.libraryWriteTime, 0, sizeof(FILETIME));
#else
    state.libraryHandleID = 0;
#endif
    return false;
}

static void Usage(const char *name) {
    printf("  usage: %s [options]\n\n  options:\n", name);
    printf("\t  help (flag) -- Show this message\n");
    printf("\t  config (string) -- Path to .json config file\n");
#define X(NAME, TYPE, VAL, DEFAULT, DOCS) \
    printf("\t  %s (%s) -- %s (default: %d)\n", NAME, #TYPE, DOCS, DEFAULT);
    SETTINGS
#undef X
}

static int LoadConfig(const char *path) {
    const char *data = NULL;
    if (!(data = LoadFile(path, NULL)))
        return 0;

    const struct json_attr_t config_attr[] = {
#define X(NAME, TYPE, VAL, DEFAULT,DOCS) \
        {(char*)#NAME, t_##TYPE, .addr.TYPE=&state.desc.VAL},
        SETTINGS
#undef X
        {NULL}
    };
    int status = json_read_object(data, config_attr, NULL);
    if (!status)
        return 0;
    free((void*)data);
    return 1;
}

#define jim_boolean jim_bool

static int ExportConfig(const char *path) {
    FILE *fh = fopen(path, "w");
    if (!fh)
        return 0;
    Jim jim = {
        .sink = fh,
        .write = (Jim_Write)fwrite
    };
    jim_object_begin(&jim);
#define X(NAME, TYPE, VAL, DEFAULT, DOCS) \
    jim_member_key(&jim, NAME);           \
    jim_##TYPE(&jim, state.desc.VAL);
    SETTINGS
#undef X
    jim_object_end(&jim);
    fclose(fh);
    return 1;
}

static int ParseArguments(int argc, char *argv[]) {
    const char *name = argv[0];
    sargs_desc desc = (sargs_desc) {
#if defined LURK_EMSCRIPTEN
        .argc = argc,
        .argv = (char**)argv
#else
        .argc = argc - 1,
        .argv = (char**)(argv + 1)
#endif
    };
    sargs_setup(&desc);

#if !defined(LURK_EMSCRIPTEN)
    if (sargs_exists("help")) {
        Usage(name);
        return 0;
    }
    if (sargs_exists("config")) {
        const char *path = sargs_value("config");
        if (!path) {
            fprintf(stderr, "[ARGUMENT ERROR] No value passed for \"config\"\n");
            Usage(name);
            return 0;
        }
        if (!DoesFileExist(path)) {
            fprintf(stderr, "[FILE ERROR] No file exists at \"%s\"\n", path);
            Usage(name);
            return 0;
        }
        LoadConfig(path);
    }
#endif // LURK_EMSCRIPTEN

#define boolean 1
#define integer 0
#define X(NAME, TYPE, VAL, DEFAULT, DOCS)                                               \
    if (sargs_exists(NAME))                                                             \
    {                                                                                   \
        const char *tmp = sargs_value_def(NAME, #DEFAULT);                              \
        if (!tmp)                                                                       \
        {                                                                               \
            fprintf(stderr, "[ARGUMENT ERROR] No value passed for \"%s\"\n", NAME);     \
            Usage(name);                                                                \
            return 0;                                                                   \
        }                                                                               \
        if (TYPE == 1)                                                                  \
            state.desc.VAL = (int)atoi(tmp);                                            \
        else                                                                            \
            state.desc.VAL = sargs_boolean(NAME);                                       \
    }
    SETTINGS
#undef X
#undef boolean
#undef integer
    return 1;
}

// MARK: Program loop

#define VALID_EXTS_LEN 9
static const char *VALID_IMAGE_EXTS[VALID_EXTS_LEN] = {
    "jpg",
    "png",
    "tga",
    "bmp",
    "psd",
    "gdr",
    "pic",
    "pnm",
    "qoi"
};

static const char* ToLower(const char *str, int length) {
    if (!length)
        length = (int)strlen(str);
    assert(length);
    char *result = malloc(sizeof(char) * length);
    for (int i = 0; i < length; i++) {
        char c = str[i];
        result[i] = isalpha(c) && isupper(c) ? tolower(c) : c;
    }
    return result;
}

static bool IsFile(const char *path) {
    struct stat st;
    assert(stat(path, &st) != -1);
    return S_ISREG(st.st_mode);
}

static int CountFilesInDir(const char *path) {
    int result = 0;
    char full[MAX_PATH];
    DIR *dir = opendir(path);
    assert(dir);
    struct dirent *ent;
    while ((ent = readdir(dir))) {
        sprintf(full, "%s%s", path, ent->d_name);
        if (IsFile(full))
            result++;
        full[0] = '\0';
    }
    closedir(dir);
    return result;
}

static const char** GetFilesInDir(const char *path, int *count_out) {
    int count = CountFilesInDir(path);
    const char** result = malloc(sizeof(char*) * count);
    int index = 0;
    char full[MAX_PATH];
    DIR *dir = opendir(path);
    assert(dir);
    struct dirent *ent;
    while ((ent = readdir(dir))) {
        sprintf(full, "%s%s", path, ent->d_name);
        if (IsFile(full))
            result[index++] = ent->d_name;
        full[0] = '\0';
    }
    closedir(dir);
    if (count_out)
        *count_out = count;
    for (int i = 0; i < count; i++)
        printf("%s\n", result[i]);
    return result;
}

static void AssetWatchCallback(dmon_watch_id watch_id,
                               dmon_action action,
                               const char* rootdir,
                               const char* filepath,
                               const char* oldfilepath,
                               void* user) {
//    if (DoesFileExist(LURK_ASSETS_PATH_OUT))
//        remove(LURK_ASSETS_PATH_OUT);

    // TODO: Check if file exists inside map
    // TODO: Compare file hashes

    switch (action) {
        case DMON_ACTION_CREATE:
            break;
        case DMON_ACTION_DELETE:
            break;
        case DMON_ACTION_MODIFY:
            break;
        case DMON_ACTION_MOVE:
            break;
    }
    return;

    int count = 0;
#if defined(LURK_POSIX)
    const char **files = GetFilesInDir(rootdir, &count);
#else
    char appended[MAX_PATH];
    memcpy(appended, rootdir, strlen(rootdir) * sizeof(char));
    const char **files = GetFilesInDir(appended, &count);
#endif
    assert(count);
    free(files);
}

static void GamepadButtonDown(struct Gamepad_device* device, unsigned int buttonID, double timestamp, void* context) {
}

static void GamepadButtonUp(struct Gamepad_device* device, unsigned int buttonID, double timestamp, void* context) {
}

static void GamepadAxisMoved(struct Gamepad_device* device, unsigned int axisID, float value, float lastValue, double timestamp, void* context) {
}

static void GamepadDeviceAttached(struct Gamepad_device* device, void* context) {
}

static void GamepadDeviceRemoved(struct Gamepad_device* device, void* context) {
}

static void InitCallback(void) {
    sg_desc desc = (sg_desc) {
        // TODO: Add more configuration options for sg_desc
        .context = sapp_sgcontext()
    };
    sg_setup(&desc);
    stm_setup();
    sgp_desc desc_sgp = (sgp_desc) {};
    sgp_setup(&desc_sgp);
    assert(sg_isvalid() && sgp_is_valid());
#if !defined(LURK_DISABLE_HOTRELOAD)
    dmon_init();
//    dmon_watch(LURK_ASSETS_PATH_IN, AssetWatchCallback, DMON_WATCHFLAGS_IGNORE_DIRECTORIES, NULL);
#endif
    Gamepad_deviceAttachFunc(GamepadDeviceAttached, NULL);
	Gamepad_deviceRemoveFunc(GamepadDeviceRemoved, NULL);
	Gamepad_buttonDownFunc(GamepadButtonDown, NULL);
	Gamepad_buttonUpFunc(GamepadButtonUp, NULL);
	Gamepad_axisMoveFunc(GamepadAxisMoved, NULL);
	Gamepad_init();

    state.textureMapCapacity = 1;
    state.textureMapCount = 0;
    state.textureMap = imap_ensure(NULL, 1);

    state.windowWidth = sapp_width();
    state.windowHeight = sapp_height();
    state.clearColor = (sg_color){0.39f, 0.58f, 0.92f, 1.f};

#if defined(LURK_MAC)
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    uint64_t frequency = info.denom;
    frequency *= 1000000000L;
    state.timerFrequency = frequency / info.numer;
#elif defined(LURK_WINDOW)
    LARGE_INTEGER frequency;
    if (!QueryPerformanceFrequency(&frequency))
        return 1000L;
    state.timerFrequency = frequency.QuadPart;
#else
    state.timerFrequency = 1000000000L;
#endif

    state.updateMultiplicity = 1;
#if defined(LURK_UNLOCKFRAME_RATE)
    state.unlockFramerate = 1;
#else
    state.unlockFramerate = 0;
#endif
    state.desiredFrameTime = state.timerFrequency * DEFAULT_TARGET_FPS;
    state.fixedDeltaTime = 1.0 / DEFAULT_TARGET_FPS;
    int64_t time60hz = state.timerFrequency / 60;
    state.snapFrequencies[0] = time60hz;
    state.snapFrequencies[1] = time60hz*2;
    state.snapFrequencies[2] = time60hz*3;
    state.snapFrequencies[3] = time60hz*4;
    state.snapFrequencies[4] = (time60hz+1)/2;
    state.snapFrequencies[5] = (time60hz+2)/3;
    state.snapFrequencies[6] = (time60hz+3)/4;
    state.maxVsyncError = state.timerFrequency * .0002;
    for (int i = 0; i < 4; i++)
        state.timeAverager[i] = state.desiredFrameTime;
    state.resync = true;
    state.prevFrameTime = stm_now();
    state.frameAccumulator = 0;

    state.world = ezEcsNewWorld();

    state.nextScene = NULL;
    lurkSwapToScene(&state, LURK_FIRST_SCENE);
    assert(ReloadLibrary(state.nextScene));
}

static void ProcessCommandQueue(void) {
    while (state.commandQueue.front) {
        lurkCommand *command = (lurkCommand*)state.commandQueue.front->data;
        ProcessCommand(command);
        FreeCommand(command);
        ezStackEntry *head = ezStackShift(&state.commandQueue);
        free(head);
    }
}

static void CallFixedUpdate(void) {
    if (state.libraryScene->fixedupdate)
        state.libraryScene->fixedupdate(&state, state.libraryContext, state.fixedDeltaTime);
#if !defined(LURK_ECS_VARIABLE_TICK)
    ezEcsStep(state.world);
#endif
}

static void CallVarUpdate(float delta) {
    if (state.libraryScene->update)
        state.libraryScene->update(&state, state.libraryContext, delta);
#if defined(LURK_ECS_VARIABLE_TICK)
    ezEcsStep(state.world);
#endif
}

static void FrameCallback(void) {
    if (state.fullscreen != state.fullscreenLast) {
        sapp_toggle_fullscreen();
        state.fullscreenLast = state.fullscreen;
    }

    if (state.cursorVisible != state.cursorVisibleLast) {
        sapp_show_mouse(state.cursorVisible);
        state.cursorVisibleLast = state.cursorVisible;
    }

    if (state.cursorLocked != state.cursorLockedLast) {
        sapp_lock_mouse(state.cursorLocked);
        state.cursorLockedLast = state.cursorLocked;
    }

    if (state.nextScene) {
        assert(ReloadLibrary(state.nextScene));
        state.nextScene = NULL;
    } else
#if !defined(LURK_DISABLE_HOTRELOAD)
        assert(ReloadLibrary(state.libraryPath));
#endif

    if (state.libraryScene->preframe) {
        state.libraryScene->preframe(&state, state.libraryContext);
        ProcessCommandQueue();
    }

    int64_t current_frame_time = stm_now();
    int64_t delta_time = current_frame_time - state.prevFrameTime;
    state.prevFrameTime = current_frame_time;

    if (delta_time > state.desiredFrameTime * 8)
        delta_time = state.desiredFrameTime;
    if (delta_time < 0)
        delta_time = 0;

    for (int i = 0; i < 7; ++i)
        if (labs(delta_time - state.snapFrequencies[i]) < state.maxVsyncError) {
            delta_time = state.snapFrequencies[i];
            break;
        }

    for (int i = 0; i < 3; ++i)
        state.timeAverager[i] = state.timeAverager[i + 1];
    state.timeAverager[3] = delta_time;
    delta_time = 0;
    for (int i = 0; i < 4; ++i)
        delta_time += state.timeAverager[i];
    delta_time /= 4.f;

    if ((state.frameAccumulator += delta_time) > state.desiredFrameTime * 8)
        state.resync = true;

    if (state.resync) {
        state.frameAccumulator = 0;
        delta_time = state.desiredFrameTime;
        state.resync = false;
    }

    double render_time = 1.0;
    if (state.unlockFramerate) {
        int64_t consumedDeltaTime = delta_time;

        while (state.frameAccumulator >= state.desiredFrameTime) {
            CallFixedUpdate();
            if (consumedDeltaTime > state.desiredFrameTime) {
                CallVarUpdate(state.fixedDeltaTime);
                consumedDeltaTime -= state.desiredFrameTime;
            }
            state.frameAccumulator -= state.desiredFrameTime;
        }

        CallVarUpdate((double)consumedDeltaTime / state.timerFrequency);
        render_time = (double)state.frameAccumulator / state.desiredFrameTime;
    } else
        while (state.frameAccumulator >= state.desiredFrameTime*state.updateMultiplicity)
            for (int i = 0; i < state.updateMultiplicity; ++i) {
                CallFixedUpdate();
                CallVarUpdate(state.fixedDeltaTime);
                state.frameAccumulator -= state.desiredFrameTime;
            }

    sgp_begin(state.windowWidth, state.windowHeight);
    if (state.libraryScene->frame)
        state.libraryScene->frame(&state, state.libraryContext, render_time);
    ProcessCommandQueue();

    state.pass_action.colors[0].clear_value = state.clearColor;
    sg_begin_default_pass(&state.pass_action, state.windowWidth, state.windowHeight);
    sgp_flush();
    sgp_end();
    sg_end_pass();
    sg_commit();

    state.modifiers = 0;
    state.mouse.scroll.x = 0.f;
    state.mouse.scroll.y = 0.f;

    if (state.libraryScene->postframe)
        state.libraryScene->postframe(&state, state.libraryContext);
}

static void EventCallback(const sapp_event* e) {
    switch (e->type) {
    case SAPP_EVENTTYPE_KEY_DOWN:
    case SAPP_EVENTTYPE_KEY_UP:
        state.keyboard[e->key_code].down = e->type == SAPP_EVENTTYPE_KEY_DOWN;
        state.keyboard[e->key_code].timestamp = stm_now();
        state.modifiers = e->modifiers;
        return;
    case SAPP_EVENTTYPE_MOUSE_DOWN:
    case SAPP_EVENTTYPE_MOUSE_UP:
        state.mouse.buttons[e->mouse_button].down = e->type == SAPP_EVENTTYPE_MOUSE_DOWN;
        state.mouse.buttons[e->mouse_button].timestamp = stm_now();
        state.modifiers = e->modifiers;
        return;
    case SAPP_EVENTTYPE_MOUSE_SCROLL:
        state.mouse.scroll.x = e->scroll_x;
        state.mouse.scroll.y = e->scroll_y;
        return;
    case SAPP_EVENTTYPE_MOUSE_MOVE:
        memcpy(&state.mouse.lastPosition, &state.mouse.position, 2 * sizeof(int));
        state.mouse.position.x = e->mouse_x;
        state.mouse.position.y = e->mouse_y;
        return;
    case SAPP_EVENTTYPE_CLIPBOARD_PASTED: {
        state.clipboard[0] = '\0';
        const char *buffer = sapp_get_clipboard_string();
        memcpy(state.clipboard, buffer, strlen(buffer) * sizeof(char));
        break;
    }
    case SAPP_EVENTTYPE_FILES_DROPPED:
        state.droppedCount = sapp_get_num_dropped_files();
        for (int i = 0; i < state.droppedCount; i++)
            state.dropped[i] = sapp_get_dropped_file_path(i);
        break;
    case SAPP_EVENTTYPE_RESIZED:
        state.windowWidth = e->window_width;
        state.windowHeight = e->window_height;
    default:
        break;
    }
    if (state.libraryScene->event)
        state.libraryScene->event(&state, state.libraryContext, e->type);
}

static void CleanupCallback(void) {
    state.running = false;
    if (state.libraryScene->deinit)
        state.libraryScene->deinit(&state, state.libraryContext);
    ezEcsFreeWorld(&state.world);
#if !defined(LURK_DISABLE_HOTRELOAD)
    dmon_deinit();
#endif
    dlclose(state.libraryHandle);
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
#if defined(LURK_ENABLE_CONFIG)
#if !defined(LURK_CONFIG_PATH)
    const char *configPath = JoinPath(UserPath(), DEFAULT_CONFIG_NAME);
#else
    const char *configPath = ResolvePath(LURK_CONFIG_PATH);
#endif

    if (DoesFileExist(configPath)) {
        if (!LoadConfig(configPath)) {
            fprintf(stderr, "[IMPORT CONFIG ERROR] Failed to import config from \"%s\"\n", configPath);
            fprintf(stderr, "errno (%d): \"%s\"\n", errno, strerror(errno));
            goto EXPORT_CONFIG;
        }
    } else {
    EXPORT_CONFIG:
        if (!ExportConfig(configPath)) {
            fprintf(stderr, "[EXPORT CONFIG ERROR] Failed to export config to \"%s\"\n", configPath);
            fprintf(stderr, "errno (%d): \"%s\"\n", errno, strerror(errno));
            abort();
        }
    }
#endif
#if defined(LURK_ENABLE_ARGUMENTS)
    if (argc > 1)
        if (!ParseArguments(argc, argv)) {
            fprintf(stderr, "[PARSE ARGUMENTS ERROR] Failed to parse arguments\n");
            abort();
        }
#endif

    state.desc.init_cb = InitCallback;
    state.desc.frame_cb = FrameCallback;
    state.desc.event_cb = EventCallback;
    state.desc.cleanup_cb = CleanupCallback;
    return state.desc;
}
#endif

#if defined(LURK_MAC)
#define DYLIB_EXT ".dylib"
#elif defined(LURK_WINDOWS)
#define DYLIB_EXT ".dll"
#elif defined(LURK_LINUX)
#define DYLIB_EXT ".so"
#else
#error Unsupported operating system
#endif

void lurkSwapToScene(lurkState *state, const char *name) {
    const char *ext = FileExt(name);
    if (ext)
        state->nextScene = name;
    else {
        static char path[MAX_PATH];
        sprintf(path, "./%s/%s%s", LURK_DYLIB_PATH, name, DYLIB_EXT);
        state->nextScene = path;
    }
}

void lurkWindowSize(lurkState *state, int *width, int *height) {
    if (width)
        *width = state->windowWidth;
    if (height)
        *height = state->windowHeight;
}

int lurkIsWindowFullscreen(lurkState *state) {
    return state->fullscreen;
}

void lurkToggleFullscreen(lurkState *state) {
    state->fullscreen = !state->fullscreen;
}

int lurkIsCursorVisible(lurkState *state) {
    return state->cursorVisible;
}

void lurkToggleCursorVisible(lurkState *state) {
    state->cursorVisible = !state->cursorVisible;
}

int lurkIsCursorLocked(lurkState *state) {
    return state->cursorLocked;
}

void lurkToggleCursorLock(lurkState *state) {
    state->cursorLocked = !state->cursorLocked;
}

uint64_t lurkFindTexture(lurkState *state, const char *name) {
    uint64_t hash = MurmurHash((void*)name, strlen(name), 0);
    return imap_lookup(state->textureMap, hash) ? hash : -1L;
}

bool lurkIsKeyDown(lurkState *state, sapp_keycode key) {
    assert(key >= SAPP_KEYCODE_SPACE && key <= SAPP_KEYCODE_MENU);
    return state->keyboard[key].down;
}

bool lurkAreAllKeysDown(lurkState *state, int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++)
        if (!state->keyboard[va_arg(args, int)].down)
            return false;
    return true;
}

bool lurkAreAnyKeysDown(lurkState *state, int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++)
        if (!state->keyboard[va_arg(args, int)].down)
            return true;
    return false;
}

bool lurkIsMouseButtonDown(lurkState *state, sapp_mousebutton button) {
    assert(button == SAPP_MOUSEBUTTON_LEFT ||
           button == SAPP_MOUSEBUTTON_RIGHT ||
           button == SAPP_MOUSEBUTTON_MIDDLE);
    return state->mouse.buttons[button].down;
}

void lurkMousePosition(lurkState *state, int* x, int* y) {
    if (x)
        *x = state->mouse.position.x;
    if (y)
        *y = state->mouse.position.y;
}

void lurkMouseDelta(lurkState *state, int *dx, int *dy) {
    if (dx)
        *dx = state->mouse.position.x - state->mouse.lastPosition.x;
    if (dy)
        *dy = state->mouse.position.y - state->mouse.lastPosition.y;
}

void lurkMouseScroll(lurkState *state, float *dx, float *dy) {
    if (dx)
        *dx = state->mouse.scroll.x;
    if (dy)
        *dy = state->mouse.scroll.y;
}

bool lurkTestKeyboardModifiers(lurkState *state, int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++)
        if (!(state->modifiers & va_arg(args, int)))
            return false;
    return true;
}
