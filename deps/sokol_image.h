/* sokol_image.h -- https://github.com/takeiteasy/suk

 The MIT License (MIT)
 
 Copyright (c) 2024 George Watson
 
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

#ifndef SUK_IMG
#define SUK_IMG
#if defined(__cplusplus)
extern "C" {
#endif

#ifndef __has_include
#define __has_include(x) 1
#endif

#ifndef SOKOL_GFX_INCLUDED
#if __has_include("sokol_gfx.h")
#include "sokol_gfx.h"
#else
#error Please include sokol_gfx.h before sokol_image.h
#endif
#endif

sg_image sg_empty_texture(int width, int height);
sg_image sg_load_texture_path(const char *path);
sg_image sg_load_texture_memory(unsigned char *data, int data_size);
sg_image sg_load_texture_path_ex(const char *path, int *width, int *height);
sg_image sg_load_texture_memory_ex(unsigned char *data, int data_size, int *width, int *height);

#if defined(__cplusplus)
}
#endif
#endif // SUK_INPUT

#ifdef SUK_IMPL
#ifdef _WIN32
#include <io.h>
#include <dirent.h>
#define F_OK 0
#define access _access
#else
#include <unistd.h>
#endif

#ifndef __has_include
#define __has_include(x) 1
#endif
#ifndef SUK_DEPS_PATH
#define SUK_DEPS_PATH "deps"
#endif

#define STB_IMAGE_IMPLEMENTATION
#if __has_include("stb_image.h")
#include "stb_image.h"
#else
#include SUK_DEPS_PATH "/stb_image.h"
#endif

#define QOI_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#if __has_include("qoi.h")
#include "qoi.h"
#else
#include SUK_DEPS_PATH "/qoi.h"
#endif

sg_image sg_empty_texture(int width, int height) {
    assert(width && height);
    sg_image_desc desc = {
        .width = width,
        .height = height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .usage = SG_USAGE_STREAM
    };
    return sg_make_image(&desc);
}

static int does_file_exist(const char *path) {
    return !access(path, F_OK);
}

sg_image sg_load_texture_path_ex(const char *path, int *width, int *height) {
    if (!does_file_exist(path))
        return (sg_image){.id=SG_INVALID_ID};
    size_t sz = -1;
    FILE *fh = fopen(path, "rb");
    assert(fh);
    fseek(fh, 0, SEEK_END);
    sz = ftell(fh);
    fseek(fh, 0, SEEK_SET);
    unsigned char *data = malloc(sz * sizeof(unsigned char));
    fread(data, sz, 1, fh);
    fclose(fh);
    sg_image result = sg_load_texture_memory_ex(data, (int)sz, width, height);
    free(data);
    return result;
}

#define QOI_MAGIC (((unsigned int)'q') << 24 | ((unsigned int)'o') << 16 | ((unsigned int)'i') <<  8 | ((unsigned int)'f'))

static int check_if_qoi(unsigned char *data) {
    return (data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]) == QOI_MAGIC;
}

#define RGBA(R, G, B, A) (((unsigned int)(A) << 24) | ((unsigned int)(B) << 16) | ((unsigned int)(G) << 8) | (R))

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

sg_image sg_load_texture_memory_ex(unsigned char *data, int data_size, int *width, int *height) {
    assert(data && data_size);
    int w, h;
    int *tmp = load_texture_data(data, data_size, &w, &h);
    assert(tmp && w && h);
    sg_image texture = sg_empty_texture(w, h);
    sg_image_data desc = {
        .subimage[0][0] = (sg_range) {
            .ptr = tmp,
            .size = w * h * sizeof(int)
        }
    };
    sg_update_image(texture, &desc);
    free(tmp);
    if (width)
        *width = w;
    if (height)
        *height = h;
    return texture;
}

sg_image sg_load_texture_path(const char *path) {
    return sg_load_texture_path_ex(path, NULL, NULL);
}

sg_image sg_load_texture_memory(unsigned char *data, int data_size) {
    return sg_load_texture_memory_ex(data, data_size, NULL, NULL);
}
#endif
