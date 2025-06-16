/* fwt.c -- https://github.com/takeiteasy/fwt

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

#define BLA_IMPLEMENTATION
#define SOKOL_IMPL
#include "fwt.h"
#include "sokol_args.h"
#include "sokol_time.h"
#define JIM_IMPLEMENTATION
#include "jim.h"
#define MJSON_IMPLEMENTATION
#include "mjson.h"
#define DMON_IMPL
#include "dmon.h"
#define QOI_IMPLEMENTATION
#include "qoi.h"
#define STB_NO_GIF
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TABLE_IMPLEMENTATION
#include "table.h"
#define GARRY_IMPLEMENTATION
#include "garry.h"
#if defined(FWT_WINDOW)
#include "dirent_win32.h"
#include "dlfcn_win32.h"
#include "dlfcn_win32.c"
#endif

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
//    if (DoesFileExist(FWT_ASSETS_PATH_OUT))
//        remove(FWT_ASSETS_PATH_OUT);

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
#if defined(FWT_POSIX)
    const char **files = GetFilesInDir(rootdir, &count);
#else
    char appended[MAX_PATH];
    memcpy(appended, rootdir, strlen(rootdir) * sizeof(char));
    const char **files = GetFilesInDir(appended, &count);
#endif
    assert(count);
    free(files);
}

#if !defined(FWT_SCENE)
static fwtTexture* NewTexture(sg_image_desc *desc) {
    fwtTexture *result = malloc(sizeof(fwtTexture));
    result->internal = sg_make_image(desc);
    result->w = desc->width;
    result->h = desc->height;
    return result;
}

static fwtTexture* EmptyTexture(unsigned int w, unsigned int h) {
    sg_image_desc desc = {
        .width = w,
        .height = h,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .usage = SG_USAGE_STREAM
    };
    return NewTexture(&desc);
}

static void DestroyTexture(fwtTexture *texture) {
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

static void UpdateTexture(fwtTexture *texture, int *data, int w, int h) {
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

fwtState state = {
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

typedef enum {
    fwtCommandProject,
    fwtCommandResetProject,
    fwtCommandPushTransform,
    fwtCommandPopTransform,
    fwtCommandResetTransform,
    fwtCommandTranslate,
    fwtCommandRotate,
    fwtCommandRotateAt,
    fwtCommandScale,
    fwtCommandScaleAt,
    fwtCommandResetPipeline,
    fwtCommandSetUniform,
    fwtCommandResetUniform,
    fwtCommandSetBlendMode,
    fwtCommandResetBlendMode,
    fwtCommandSetColor,
    fwtCommandResetColor,
    fwtCommandSetImage,
    fwtCommandUnsetImage,
    fwtCommandResetImage,
    fwtCommandResetSampler,
    fwtCommandViewport,
    fwtCommandResetViewport,
    fwtCommandScissor,
    fwtCommandResetScissor,
    fwtCommandResetState,
    fwtCommandClear,
    fwtCommandDrawPoints,
    fwtCommandDrawPoint,
    fwtCommandDrawLines,
    fwtCommandDrawLine,
    fwtCommandDrawLinesStrip,
    fwtCommandDrawFilledTriangles,
    fwtCommandDrawFilledTriangle,
    fwtCommandDrawFilledTrianglesStrip,
    fwtCommandDrawFilledRects,
    fwtCommandDrawFilledRect,
    fwtCommandDrawTexturedRects,
    fwtCommandDrawTexturedRect,
    fwtCommandCreateTexture
} fwtCommandType;

typedef struct {
    fwtCommandType type;
    void* data;
} fwtCommand;

static void PushCommand(fwtState* state, fwtCommand* command) {
    ezStackAppend(&state->commandQueue, command->type, (void*)command);
}

typedef struct {
    float left;
    float right;
    float top;
    float bottom;
} fwtProjectData;

void fwtProject(fwtState *state, float left, float right, float top, float bottom) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandProject;
    fwtProjectData* cmdData = malloc(sizeof(fwtProjectData));
    cmdData->left = left;
    cmdData->right = right;
    cmdData->top = top;
    cmdData->bottom = bottom;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

void fwtResetProject(fwtState *state) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandResetProject;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

void fwtPushTransform(fwtState *state) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandPushTransform;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

void fwtPopTransform(fwtState *state) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandPopTransform;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

void fwtResetTransform(fwtState *state) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandResetTransform;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

typedef struct {
    float x;
    float y;
} fwtTranslateData;

void fwtTranslate(fwtState *state, float x, float y) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandTranslate;
    fwtTranslateData* cmdData = malloc(sizeof(fwtTranslateData));
    cmdData->x = x;
    cmdData->y = y;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    float theta;
} fwtRotateData;

void fwtRotate(fwtState *state, float theta) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandRotate;
    fwtRotateData* cmdData = malloc(sizeof(fwtRotateData));
    cmdData->theta = theta;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    float theta;
    float x;
    float y;
} fwtRotateAtData;

void fwtRotateAt(fwtState *state, float theta, float x, float y) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandRotateAt;
    fwtRotateAtData* cmdData = malloc(sizeof(fwtRotateAtData));
    cmdData->theta = theta;
    cmdData->x = x;
    cmdData->y = y;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    float sx;
    float sy;
} fwtScaleData;

void fwtScale(fwtState *state, float sx, float sy) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandScale;
    fwtScaleData* cmdData = malloc(sizeof(fwtScaleData));
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
} fwtScaleAtData;

void fwtScaleAt(fwtState *state, float sx, float sy, float x, float y) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandScaleAt;
    fwtScaleAtData* cmdData = malloc(sizeof(fwtScaleAtData));
    cmdData->sx = sx;
    cmdData->sy = sy;
    cmdData->x = x;
    cmdData->y = y;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

void fwtResetPipeline(fwtState *state) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandResetPipeline;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

typedef struct {
    void* data;
    int size;
} fwtSetUniformData;

void fwtSetUniform(fwtState *state, void* data, int size) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandSetUniform;
    fwtSetUniformData* cmdData = malloc(sizeof(fwtSetUniformData));
    cmdData->data = data;
    cmdData->size = size;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

void fwtResetUniform(fwtState *state) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandResetUniform;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

typedef struct {
    sgp_blend_mode blend_mode;
} fwtSetBlendModeData;

void fwtSetBlendMode(fwtState *state, sgp_blend_mode blend_mode) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandSetBlendMode;
    fwtSetBlendModeData* cmdData = malloc(sizeof(fwtSetBlendModeData));
    cmdData->blend_mode = blend_mode;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

void fwtResetBlendMode(fwtState *state) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandResetBlendMode;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

typedef struct {
    float r;
    float g;
    float b;
    float a;
} fwtSetColorData;

void fwtSetColor(fwtState *state, float r, float g, float b, float a) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandSetColor;
    fwtSetColorData* cmdData = malloc(sizeof(fwtSetColorData));
    cmdData->r = r;
    cmdData->g = g;
    cmdData->b = b;
    cmdData->a = a;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

void fwtResetColor(fwtState *state) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandResetColor;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

typedef struct {
    int channel;
    fwtTexture* texture;
} fwtSetImageData;

void fwtSetImage(fwtState* state, uint64_t texture_id, int channel) {
    assert(texture_id);
    imap_slot_t* slot = imap_lookup(state->textureMap, texture_id);
    assert(slot);
    fwtTexture* texture = (fwtTexture*)imap_getval64(state->textureMap, slot);
    assert(texture);

    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandSetImage;
    fwtSetImageData* cmdData = malloc(sizeof(fwtSetImageData));
    cmdData->channel = channel;
    cmdData->texture = texture;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    int channel;
} fwtUnsetImageData;

void fwtUnsetImage(fwtState *state, int channel) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandUnsetImage;
    fwtUnsetImageData* cmdData = malloc(sizeof(fwtUnsetImageData));
    cmdData->channel = channel;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    int channel;
} fwtResetImageData;

void fwtResetImage(fwtState *state, int channel) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandResetImage;
    fwtResetImageData* cmdData = malloc(sizeof(fwtResetImageData));
    cmdData->channel = channel;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    int channel;
} fwtResetSamplerData;

void fwtResetSampler(fwtState *state, int channel) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandResetSampler;
    fwtResetSamplerData* cmdData = malloc(sizeof(fwtResetSamplerData));
    cmdData->channel = channel;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    int x;
    int y;
    int w;
    int h;
} fwtViewportData;

void fwtViewport(fwtState *state, int x, int y, int w, int h) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandViewport;
    fwtViewportData* cmdData = malloc(sizeof(fwtViewportData));
    cmdData->x = x;
    cmdData->y = y;
    cmdData->w = w;
    cmdData->h = h;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

void fwtResetViewport(fwtState *state) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandResetViewport;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

typedef struct {
    int x;
    int y;
    int w;
    int h;
} fwtScissorData;

void fwtScissor(fwtState *state, int x, int y, int w, int h) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandScissor;
    fwtScissorData* cmdData = malloc(sizeof(fwtScissorData));
    cmdData->x = x;
    cmdData->y = y;
    cmdData->w = w;
    cmdData->h = h;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

void fwtResetScissor(fwtState *state) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandResetScissor;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

void fwtResetState(fwtState *state) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandResetState;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

void fwtClear(fwtState *state) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandClear;
    cmd->data = NULL;
    PushCommand(state, cmd);
}

typedef struct {
    sgp_point* points;
    int count;
} fwtDrawPointsData;

void fwtDrawPoints(fwtState *state, sgp_point* points, int count) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandDrawPoints;
    fwtDrawPointsData* cmdData = malloc(sizeof(fwtDrawPointsData));
    cmdData->points = points;
    cmdData->count = count;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    float x;
    float y;
} fwtDrawPointData;

void fwtDrawPoint(fwtState *state, float x, float y) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandDrawPoint;
    fwtDrawPointData* cmdData = malloc(sizeof(fwtDrawPointData));
    cmdData->x = x;
    cmdData->y = y;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    sgp_line* lines;
    int count;
} fwtDrawLinesData;

void fwtDrawLines(fwtState *state, sgp_line* lines, int count) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandDrawLines;
    fwtDrawLinesData* cmdData = malloc(sizeof(fwtDrawLinesData));
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
} fwtDrawLineData;

void fwtDrawLine(fwtState *state, float ax, float ay, float bx, float by) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandDrawLine;
    fwtDrawLineData* cmdData = malloc(sizeof(fwtDrawLineData));
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
} fwtDrawLinesStripData;

void fwtDrawLinesStrip(fwtState *state, sgp_point* points, int count) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandDrawLinesStrip;
    fwtDrawLinesStripData* cmdData = malloc(sizeof(fwtDrawLinesStripData));
    cmdData->points = points;
    cmdData->count = count;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    sgp_triangle* triangles;
    int count;
} fwtDrawFilledTrianglesData;

void fwtDrawFilledTriangles(fwtState *state, sgp_triangle* triangles, int count) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandDrawFilledTriangles;
    fwtDrawFilledTrianglesData* cmdData = malloc(sizeof(fwtDrawFilledTrianglesData));
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
} fwtDrawFilledTriangleData;

void fwtDrawFilledTriangle(fwtState *state, float ax, float ay, float bx, float by, float cx, float cy) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandDrawFilledTriangle;
    fwtDrawFilledTriangleData* cmdData = malloc(sizeof(fwtDrawFilledTriangleData));
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
} fwtDrawFilledTrianglesStripData;

void fwtDrawFilledTrianglesStrip(fwtState *state, sgp_point* points, int count) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandDrawFilledTrianglesStrip;
    fwtDrawFilledTrianglesStripData* cmdData = malloc(sizeof(fwtDrawFilledTrianglesStripData));
    cmdData->points = points;
    cmdData->count = count;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    sgp_rect* rects;
    int count;
} fwtDrawFilledRectsData;

void fwtDrawFilledRects(fwtState *state, sgp_rect* rects, int count) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandDrawFilledRects;
    fwtDrawFilledRectsData* cmdData = malloc(sizeof(fwtDrawFilledRectsData));
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
} fwtDrawFilledRectData;

void fwtDrawFilledRect(fwtState *state, float x, float y, float w, float h) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandDrawFilledRect;
    fwtDrawFilledRectData* cmdData = malloc(sizeof(fwtDrawFilledRectData));
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
} fwtDrawTexturedRectsData;

void fwtDrawTexturedRects(fwtState *state, int channel, sgp_textured_rect* rects, int count) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandDrawTexturedRects;
    fwtDrawTexturedRectsData* cmdData = malloc(sizeof(fwtDrawTexturedRectsData));
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
} fwtDrawTexturedRectData;

void fwtDrawTexturedRect(fwtState *state, int channel, sgp_rect dest_rect, sgp_rect src_rect) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandDrawTexturedRect;
    fwtDrawTexturedRectData* cmdData = malloc(sizeof(fwtDrawTexturedRectData));
    cmdData->channel = channel;
    cmdData->dest_rect = dest_rect;
    cmdData->src_rect = src_rect;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

typedef struct {
    ezImage *image;
    const char *name;
} fwtCreateTextureData;

void fwtCreateTexture(fwtState *state, const char *name, ezImage *image) {
    fwtCommand* cmd = malloc(sizeof(fwtCommand));
    cmd->type = fwtCommandDrawTexturedRect;
    fwtCreateTextureData* cmdData = malloc(sizeof(fwtCreateTextureData));
    cmdData->name = name;
    cmdData->image = image;
    cmd->data = cmdData;
    PushCommand(state, cmd);
}

#if !defined(FWT_SCENE)
static void FreeCommand(fwtCommand* command) {
    fwtCommandType type = command->type;
    switch
(type) {
    case fwtCommandProject: {
        fwtProjectData* data = (fwtProjectData*)command->data;
        free(data);
        break;
    }
    case fwtCommandTranslate: {
        fwtTranslateData* data = (fwtTranslateData*)command->data;
        free(data);
        break;
    }
    case fwtCommandRotate: {
        fwtRotateData* data = (fwtRotateData*)command->data;
        free(data);
        break;
    }
    case fwtCommandRotateAt: {
        fwtRotateAtData* data = (fwtRotateAtData*)command->data;
        free(data);
        break;
    }
    case fwtCommandScale: {
        fwtScaleData* data = (fwtScaleData*)command->data;
        free(data);
        break;
    }
    case fwtCommandScaleAt: {
        fwtScaleAtData* data = (fwtScaleAtData*)command->data;
        free(data);
        break;
    }
    case fwtCommandSetUniform: {
        fwtSetUniformData* data = (fwtSetUniformData*)command->data;
        free(data);
        break;
    }
    case fwtCommandSetBlendMode: {
        fwtSetBlendModeData* data = (fwtSetBlendModeData*)command->data;
        free(data);
        break;
    }
    case fwtCommandSetColor: {
        fwtSetColorData* data = (fwtSetColorData*)command->data;
        free(data);
        break;
    }
    case fwtCommandSetImage: {
        fwtSetImageData* data = (fwtSetImageData*)command->data;
        free(data);
        break;
    }
    case fwtCommandUnsetImage: {
        fwtUnsetImageData* data = (fwtUnsetImageData*)command->data;
        free(data);
        break;
    }
    case fwtCommandResetImage: {
        fwtResetImageData* data = (fwtResetImageData*)command->data;
        free(data);
        break;
    }
    case fwtCommandResetSampler: {
        fwtResetSamplerData* data = (fwtResetSamplerData*)command->data;
        free(data);
        break;
    }
    case fwtCommandViewport: {
        fwtViewportData* data = (fwtViewportData*)command->data;
        free(data);
        break;
    }
    case fwtCommandScissor: {
        fwtScissorData* data = (fwtScissorData*)command->data;
        free(data);
        break;
    }
    case fwtCommandDrawPoints: {
        fwtDrawPointsData* data = (fwtDrawPointsData*)command->data;
        free(data);
        break;
    }
    case fwtCommandDrawPoint: {
        fwtDrawPointData* data = (fwtDrawPointData*)command->data;
        free(data);
        break;
    }
    case fwtCommandDrawLines: {
        fwtDrawLinesData* data = (fwtDrawLinesData*)command->data;
        free(data);
        break;
    }
    case fwtCommandDrawLine: {
        fwtDrawLineData* data = (fwtDrawLineData*)command->data;
        free(data);
        break;
    }
    case fwtCommandDrawLinesStrip: {
        fwtDrawLinesStripData* data = (fwtDrawLinesStripData*)command->data;
        free(data);
        break;
    }
    case fwtCommandDrawFilledTriangles: {
        fwtDrawFilledTrianglesData* data = (fwtDrawFilledTrianglesData*)command->data;
        free(data);
        break;
    }
    case fwtCommandDrawFilledTriangle: {
        fwtDrawFilledTriangleData* data = (fwtDrawFilledTriangleData*)command->data;
        free(data);
        break;
    }
    case fwtCommandDrawFilledTrianglesStrip: {
        fwtDrawFilledTrianglesStripData* data = (fwtDrawFilledTrianglesStripData*)command->data;
        free(data);
        break;
    }
    case fwtCommandDrawFilledRects: {
        fwtDrawFilledRectsData* data = (fwtDrawFilledRectsData*)command->data;
        free(data);
        break;
    }
    case fwtCommandDrawFilledRect: {
        fwtDrawFilledRectData* data = (fwtDrawFilledRectData*)command->data;
        free(data);
        break;
    }
    case fwtCommandDrawTexturedRects: {
        fwtDrawTexturedRectsData* data = (fwtDrawTexturedRectsData*)command->data;
        free(data);
        break;
    }
    case fwtCommandDrawTexturedRect: {
        fwtDrawTexturedRectData* data = (fwtDrawTexturedRectData*)command->data;
        free(data);
        break;
    }
    default:
        break;
    }
    free(command);
}

static void ProcessCommand(fwtCommand* command) {
    fwtCommandType type = command->type;
    switch (type) {
    case fwtCommandProject: {
        fwtProjectData* data = (fwtProjectData*)command->data;
        sgp_project(data->left, data->right, data->top, data->bottom);
        break;
    }
    case fwtCommandResetProject:
        sgp_reset_project();
        break;
    case fwtCommandPushTransform:
        sgp_push_transform();
        break;
    case fwtCommandPopTransform:
        sgp_pop_transform();
        break;
    case fwtCommandResetTransform:
        sgp_reset_transform();
        break;
    case fwtCommandTranslate: {
        fwtTranslateData* data = (fwtTranslateData*)command->data;
        sgp_translate(data->x, data->y);
        break;
    }
    case fwtCommandRotate: {
        fwtRotateData* data = (fwtRotateData*)command->data;
        sgp_rotate(data->theta);
        break;
    }
    case fwtCommandRotateAt: {
        fwtRotateAtData* data = (fwtRotateAtData*)command->data;
        sgp_rotate_at(data->theta, data->x, data->y);
        break;
    }
    case fwtCommandScale: {
        fwtScaleData* data = (fwtScaleData*)command->data;
        sgp_scale(data->sx, data->sy);
        break;
    }
    case fwtCommandScaleAt: {
        fwtScaleAtData* data = (fwtScaleAtData*)command->data;
        sgp_scale_at(data->sx, data->sy, data->x, data->y);
        break;
    }
    case fwtCommandResetPipeline:
        sgp_reset_pipeline();
        break;
    case fwtCommandSetUniform: {
        fwtSetUniformData* data = (fwtSetUniformData*)command->data;
        sgp_set_uniform(data->data, data->size);
        break;
    }
    case fwtCommandResetUniform:
        sgp_reset_uniform();
        break;
    case fwtCommandSetBlendMode: {
        fwtSetBlendModeData* data = (fwtSetBlendModeData*)command->data;
        sgp_set_blend_mode(data->blend_mode);
        break;
    }
    case fwtCommandResetBlendMode:
        sgp_reset_blend_mode();
        break;
    case fwtCommandSetColor: {
        fwtSetColorData* data = (fwtSetColorData*)command->data;
        sgp_set_color(data->r, data->g, data->b, data->a);
        break;
    }
    case fwtCommandResetColor:
        sgp_reset_color();
        break;
    case fwtCommandSetImage: {
        fwtSetImageData* data = (fwtSetImageData*)command->data;
        sgp_set_image(data->channel, data->texture->internal);
        break;
    }
    case fwtCommandUnsetImage: {
        fwtUnsetImageData* data = (fwtUnsetImageData*)command->data;
        sgp_unset_image(data->channel);
        break;
    }
    case fwtCommandResetImage: {
        fwtResetImageData* data = (fwtResetImageData*)command->data;
        sgp_reset_image(data->channel);
        break;
    }
    case fwtCommandResetSampler: {
        fwtResetSamplerData* data = (fwtResetSamplerData*)command->data;
        sgp_reset_sampler(data->channel);
        break;
    }
    case fwtCommandViewport: {
        fwtViewportData* data = (fwtViewportData*)command->data;
        sgp_viewport(data->x, data->y, data->w, data->h);
        break;
    }
    case fwtCommandResetViewport:
        sgp_reset_viewport();
        break;
    case fwtCommandScissor: {
        fwtScissorData* data = (fwtScissorData*)command->data;
        sgp_scissor(data->x, data->y, data->w, data->h);
        break;
    }
    case fwtCommandResetScissor:
        sgp_reset_scissor();
        break;
    case fwtCommandResetState:
        sgp_reset_state();
        break;
    case fwtCommandClear:
        sgp_clear();
        break;
    case fwtCommandDrawPoints: {
        fwtDrawPointsData* data = (fwtDrawPointsData*)command->data;
        sgp_draw_points(data->points, data->count);
        break;
    }
    case fwtCommandDrawPoint: {
        fwtDrawPointData* data = (fwtDrawPointData*)command->data;
        sgp_draw_point(data->x, data->y);
        break;
    }
    case fwtCommandDrawLines: {
        fwtDrawLinesData* data = (fwtDrawLinesData*)command->data;
        sgp_draw_lines(data->lines, data->count);
        break;
    }
    case fwtCommandDrawLine: {
        fwtDrawLineData* data = (fwtDrawLineData*)command->data;
        sgp_draw_line(data->ax, data->ay, data->bx, data->by);
        break;
    }
    case fwtCommandDrawLinesStrip: {
        fwtDrawLinesStripData* data = (fwtDrawLinesStripData*)command->data;
        sgp_draw_lines_strip(data->points, data->count);
        break;
    }
    case fwtCommandDrawFilledTriangles: {
        fwtDrawFilledTrianglesData* data = (fwtDrawFilledTrianglesData*)command->data;
        sgp_draw_filled_triangles(data->triangles, data->count);
        break;
    }
    case fwtCommandDrawFilledTriangle: {
        fwtDrawFilledTriangleData* data = (fwtDrawFilledTriangleData*)command->data;
        sgp_draw_filled_triangle(data->ax, data->ay, data->bx, data->by, data->cx, data->cy);
        break;
    }
    case fwtCommandDrawFilledTrianglesStrip: {
        fwtDrawFilledTrianglesStripData* data = (fwtDrawFilledTrianglesStripData*)command->data;
        sgp_draw_filled_triangles_strip(data->points, data->count);
        break;
    }
    case fwtCommandDrawFilledRects: {
        fwtDrawFilledRectsData* data = (fwtDrawFilledRectsData*)command->data;
        sgp_draw_filled_rects(data->rects, data->count);
        break;
    }
    case fwtCommandDrawFilledRect: {
        fwtDrawFilledRectData* data = (fwtDrawFilledRectData*)command->data;
        sgp_draw_filled_rect(data->x, data->y, data->w, data->h);
        break;
    }
    case fwtCommandDrawTexturedRects: {
        fwtDrawTexturedRectsData* data = (fwtDrawTexturedRectsData*)command->data;
        sgp_draw_textured_rects(data->channel, data->rects, data->count);
        break;
    }
    case fwtCommandDrawTexturedRect: {
        fwtDrawTexturedRectData* data = (fwtDrawTexturedRectData*)command->data;
        sgp_draw_textured_rect(data->channel, data->dest_rect, data->src_rect);
        break;
    }
    case fwtCommandCreateTexture: {
        fwtCreateTextureData* data = (fwtCreateTextureData*)command->data;
        uint64_t hash = MurmurHash((void*)data->name, strlen(data->name), 0);
        imap_slot_t *slot = imap_assign(state.textureMap, hash);
        assert(!slot);
        fwtTexture* texture = EmptyTexture(data->image->w, data->image->h);
        UpdateTexture(texture, data->image->buf, data->image->w, data->image->h);
        imap_setval64(state.textureMap, slot, (uint64_t)texture);
        break;
    }
    default:
        abort();
    }
}
#endif

#if defined(FWT_WINDOWS)
static FILETIME Win32GetLastWriteTime(char* path) {
    FILETIME time;
    WIN32_FILE_ATTRIBUTE_DATA data;

    if (GetFileAttributesEx(path, GetFileExInfoStandard, &data))
        time = data.ftLastWriteTime;

    return time;
}
#endif

#if !defined(FWT_SCENE)
static bool ReloadLibrary(const char *path) {
#if defined(FWT_DISABLE_HOTRELOAD)
    return true;
#endif

#if defined(FWT_WINDOWS)
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

#if defined(FWT_WINDOWS)
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
#if defined(FWT_WINDOWS)
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
#if defined FWT_EMSCRIPTEN
        .argc = argc,
        .argv = (char**)argv
#else
        .argc = argc - 1,
        .argv = (char**)(argv + 1)
#endif
    };
    sargs_setup(&desc);

#if !defined(FWT_EMSCRIPTEN)
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
#endif // FWT_EMSCRIPTEN

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
#if !defined(FWT_DISABLE_HOTRELOAD)
    dmon_init();
//    dmon_watch(FWT_ASSETS_PATH_IN, AssetWatchCallback, DMON_WATCHFLAGS_IGNORE_DIRECTORIES, NULL);
#endif

    state.textureMapCapacity = 1;
    state.textureMapCount = 0;
    state.textureMap = imap_ensure(NULL, 1);
    state.windowWidth = sapp_width();
    state.windowHeight = sapp_height();
    state.clearColor = (sg_color){0.39f, 0.58f, 0.92f, 1.f};

    state.nextScene = NULL;
    fwtSwapToScene(&state, FWT_FIRST_SCENE);
    assert(ReloadLibrary(state.nextScene));
}

static void ProcessCommandQueue(void) {
    while (state.commandQueue.front) {
        fwtCommand *command = (fwtCommand*)state.commandQueue.front->data;
        ProcessCommand(command);
        FreeCommand(command);
        ezStackEntry *head = ezStackShift(&state.commandQueue);
        free(head);
    }
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
#if !defined(FWT_DISABLE_HOTRELOAD)
        assert(ReloadLibrary(state.libraryPath));
#endif

    if (state.libraryScene->preframe) {
        state.libraryScene->preframe(&state, state.libraryContext);
        ProcessCommandQueue();
    }

    int64_t current_frame_time = stm_now();
    int64_t delta_time = current_frame_time - state.prevFrameTime;
    state.prevFrameTime = current_frame_time;
    if (state.libraryScene->update)
        state.libraryScene->update(&state, state.libraryContext, delta);

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
#if !defined(FWT_DISABLE_HOTRELOAD)
    dmon_deinit();
#endif
    dlclose(state.libraryHandle);
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
#if defined(FWT_ENABLE_CONFIG)
#if !defined(FWT_CONFIG_PATH)
    const char *configPath = JoinPath(UserPath(), DEFAULT_CONFIG_NAME);
#else
    const char *configPath = ResolvePath(FWT_CONFIG_PATH);
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
#if defined(FWT_ENABLE_ARGUMENTS)
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

#if defined(FWT_MAC)
#define DYLIB_EXT ".dylib"
#elif defined(FWT_WINDOWS)
#define DYLIB_EXT ".dll"
#elif defined(FWT_LINUX)
#define DYLIB_EXT ".so"
#else
#error Unsupported operating system
#endif

void fwtSwapToScene(fwtState *state, const char *name) {
    const char *ext = FileExt(name);
    if (ext)
        state->nextScene = name;
    else {
        static char path[MAX_PATH];
        sprintf(path, "./%s/%s%s", FWT_DYLIB_PATH, name, DYLIB_EXT);
        state->nextScene = path;
    }
}

void fwtWindowSize(fwtState *state, int *width, int *height) {
    if (width)
        *width = state->windowWidth;
    if (height)
        *height = state->windowHeight;
}

int fwtIsWindowFullscreen(fwtState *state) {
    return state->fullscreen;
}

void fwtToggleFullscreen(fwtState *state) {
    state->fullscreen = !state->fullscreen;
}

int fwtIsCursorVisible(fwtState *state) {
    return state->cursorVisible;
}

void fwtToggleCursorVisible(fwtState *state) {
    state->cursorVisible = !state->cursorVisible;
}

int fwtIsCursorLocked(fwtState *state) {
    return state->cursorLocked;
}

void fwtToggleCursorLock(fwtState *state) {
    state->cursorLocked = !state->cursorLocked;
}

uint64_t fwtFindTexture(fwtState *state, const char *name) {
    uint64_t hash = MurmurHash((void*)name, strlen(name), 0);
    return imap_lookup(state->textureMap, hash) ? hash : -1L;
}

bool fwtIsKeyDown(fwtState *state, sapp_keycode key) {
    assert(key >= SAPP_KEYCODE_SPACE && key <= SAPP_KEYCODE_MENU);
    return state->keyboard[key].down;
}

bool fwtAreAllKeysDown(fwtState *state, int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++)
        if (!state->keyboard[va_arg(args, int)].down)
            return false;
    return true;
}

bool fwtAreAnyKeysDown(fwtState *state, int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++)
        if (!state->keyboard[va_arg(args, int)].down)
            return true;
    return false;
}

bool fwtIsMouseButtonDown(fwtState *state, sapp_mousebutton button) {
    assert(button == SAPP_MOUSEBUTTON_LEFT ||
           button == SAPP_MOUSEBUTTON_RIGHT ||
           button == SAPP_MOUSEBUTTON_MIDDLE);
    return state->mouse.buttons[button].down;
}

void fwtMousePosition(fwtState *state, int* x, int* y) {
    if (x)
        *x = state->mouse.position.x;
    if (y)
        *y = state->mouse.position.y;
}

void fwtMouseDelta(fwtState *state, int *dx, int *dy) {
    if (dx)
        *dx = state->mouse.position.x - state->mouse.lastPosition.x;
    if (dy)
        *dy = state->mouse.position.y - state->mouse.lastPosition.y;
}

void fwtMouseScroll(fwtState *state, float *dx, float *dy) {
    if (dx)
        *dx = state->mouse.scroll.x;
    if (dy)
        *dy = state->mouse.scroll.y;
}

bool fwtTestKeyboardModifiers(fwtState *state, int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++)
        if (!(state->modifiers & va_arg(args, int)))
            return false;
    return true;
}
