#include "fwt.h"

static float vertices[] = {
    -1.0f, -1.0f, -1.0f,  0.f, 0.f,
     1.0f, -1.0f, -1.0f,  1.f, 0.f,
     1.0f,  1.0f, -1.0f,  1.f, 1.f,
    -1.0f,  1.0f, -1.0f,  0.f, 1.f,

    -1.0f, -1.0f,  1.0f,  0.f, 0.f,
     1.0f, -1.0f,  1.0f,  1.f, 0.f,
     1.0f,  1.0f,  1.0f,  1.f, 1.f,
    -1.0f,  1.0f,  1.0f,  0.f, 1.f,

    -1.0f, -1.0f, -1.0f,  0.f, 0.f,
    -1.0f,  1.0f, -1.0f,  1.f,  0.f,
    -1.0f,  1.0f,  1.0f,  1.f, 1.f,
    -1.0f, -1.0f,  1.0f,  0.f, 1.f,

     1.0f, -1.0f, -1.0f,  0.f, 0.f,
     1.0f,  1.0f, -1.0f,  1.f, 0.f,
     1.0f,  1.0f,  1.0f,  1.f, 1.f,
     1.0f, -1.0f,  1.0f,  0.f, 1.f,

    -1.0f, -1.0f, -1.0f,  0.f, 0.f,
    -1.0f, -1.0f,  1.0f,  1.f, 0.f,
     1.0f, -1.0f,  1.0f,  1.f, 1.f,
     1.0f, -1.0f, -1.0f,  0.f, 1.f,

    -1.0f,  1.0f, -1.0f,  0.f, 0.f,
    -1.0f,  1.0f,  1.0f,  1.f, 0.f,
     1.0f,  1.0f,  1.0f,  1.f, 1.f,
     1.0f,  1.0f, -1.0f,  0.f, 1.f,
};

static int indices[] = {
    0,  1,  2,   0,  2,  3,
    6,  5,  4,   7,  6,  4,
    8,  9,  10,  8,  10, 11,
    14, 13, 12,  15, 14, 12,
    16, 17, 18,  16, 18, 19,
    22, 21, 20,  23, 22, 20
};

struct fwtState {
    int pear_texture;
    float rx, ry;
};

static fwtState* init(void) {
    fwtSetWorkingDir("etc"); // TODO: virtual file system
    // TODO: Port vfs to fun-with-pixels too
    fwtState *state = malloc(sizeof(fwtState));
    state->pear_texture = fwtLoadTexturePath("pear.jpg");
    state->rx = 0.f;
    state->ry = 0.f;
    return state;
}

static void deinit(fwtState *state) {
    if (state)
        free(state);
}

static void reload(fwtState *state) {
    // Called when the dynamic has been updated + reloaded
}

static void unload(fwtState *state) {
    // Called when dynamic library has been unloaded
}

static int tick(fwtState *state, double delta) {
    state->rx += 1.f * delta;
    state->ry += 2.f * delta;
    fwtMatrixMode(FWT_MATRIXMODE_PROJECTION);
    fwtLoadIdentity();
    fwtPerspective(60.f, fwtWindowAspectRatio(), .01f, 10.f);
    fwtMatrixMode(FWT_MATRIXMODE_MODELVIEW);
    fwtLoadIdentity();
    fwtLookAt(0.0f, 1.5f, 6.0f,
              0.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);
    fwtRotate(state->rx, 1.f, 0.f, 0.f);
    fwtRotate(state->ry, 0.f, 1.f, 0.f);

    fwtPushTexture(state->pear_texture);
    fwtBegin(FWT_DRAW_TRIANGLES);
    for (int i = 0; i < 36; i++) {
        int j = indices[i];
        float *data = vertices + j * 5;
        fwtTexCoord2f(data[3], data[4]);
        fwtVertex3f(data[0], data[1], data[2]);
    }
    fwtDraw();
    fwtEnd();
    fwtPopTexture();
    return 1;
}

// So fwt knows where your callbacks are a `scene` definition must be made
// The definition should be always be called scene. If the name changes fwt
// won't know where to look!
const fwtScene scene = {
    .init = init,
    .deinit = deinit,
    .reload = reload,
    .unload = unload,
    .tick = tick
};
