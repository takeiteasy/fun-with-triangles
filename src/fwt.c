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
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_app.h"
#include "sokol/sokol_glue.h"
#include "sokol/sokol_time.h"
#include "sokol/sokol_log.h"

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
