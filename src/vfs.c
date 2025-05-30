/* vfs.c -- https://github.com/takeiteasy/fun-with-triangles

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

// virtual filesystem (registered directories and/or compressed zip archives).
// - rlyeh, public domain.
//
// - note: vfs_mount() order matters (the most recent the higher priority).

#include "fwt.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define ZIP_C
#include "zip.h"
#define DIR_C
#include "dir.h"
#pragma clang diagnostic pop

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#define chdir(x) _chdir(x)
#else
#include <unistd.h>
#endif

bool fwtSetWorkingDir(const char *path) {
    return chdir(path);
}

unsigned char* read_file(const char *path, size_t *length) {
    unsigned char *result = NULL;
    size_t sz = -1;
    FILE *fh = fopen(path, "rb");
    if (!fh)
        goto BAIL;
    fseek(fh, 0, SEEK_END);
    sz = ftell(fh);
    fseek(fh, 0, SEEK_SET);

    result = malloc(sz * sizeof(unsigned char));
    fread(result, sz, 1, fh);
    fclose(fh);

BAIL:
    if (length)
        *length = sz;
    return result;
}

#ifdef PLATFORM_WINDOWS
#include <io.h>
#include <dirent.h>
#ifndef F_OK
#define F_OK 0
#endif
#ifndef access
#define access _access
#endif
#else
#include <unistd.h>
#endif

static bool file_exists(const char *path) {
    return !access(path, F_OK);
}

typedef struct vfs_dir {
    char* path;
    // const
    zip* archive;
    int is_directory;
    struct vfs_dir *next;
} vfs_dir;

static vfs_dir *dir_head = NULL;

void fwtMountFileSystem(const char *path) {
    zip *z = NULL;
    int is_directory = ('/' == path[strlen(path)-1]);
    if( !is_directory ) z = zip_open(path, "rb");
    if( !is_directory && !z ) return;

    vfs_dir *prev = dir_head, zero = {0};
    *(dir_head = realloc(0, sizeof(vfs_dir))) = zero;
    dir_head->next = prev;
    dir_head->path = STRDUP(path);
    dir_head->archive = z;
    dir_head->is_directory = is_directory;
}

static bool vfs_has(vfs_dir *dir, const char *filename) {
    if (dir->is_directory) {
        char buf[512];
        snprintf(buf, sizeof(buf), "%s%s", dir->path, filename);
        if (file_exists(buf))
            return true;
    } else {
        int index = zip_find(dir->archive, filename);
        if (index != -1)
            return true;
    }
    return false;
}

bool fwtFileExists(const char *filename) {
    vfs_dir *dir = dir_head;
    if (!dir)
        return file_exists(filename);
    while (dir) {
        if (vfs_has(dir, filename))
            return true;
        dir = dir->next;
    }
    return false;
}

unsigned char* fwtReadFile(const char *filename, size_t *size) { // must free() after use
    vfs_dir *dir = dir_head;
    if (!dir)
        return read_file(filename, size);
    while (dir) {
        if (vfs_has(dir, filename)) {
            if (dir->is_directory)
                return read_file(filename, size);
            else {
                int index = zip_find(dir->archive, filename);
                unsigned char *data = zip_extract(dir->archive, index);
                if (size)
                    *size = zip_size(dir->archive, index);
                return data;
            }
        }
        dir = dir->next;
    }
    if (size)
        *size = 0;
    return NULL;
}

bool fwtWriteFile(const char *filename, unsigned char *data, size_t size) {
    // TODO: Write to all mounts, overwrite if existing
    return false;
}
