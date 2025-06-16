#include "fwt.h"

struct fwtState {
    int dummy;
};

static fwtState* init(void) {
    fwtState *state = malloc(sizeof(fwtState));
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
    // Called every frame, this is your update callback
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
