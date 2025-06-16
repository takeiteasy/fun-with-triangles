#include "fwt.h"

struct fwtContext {
    uint64_t texture;
};

typedef struct {
    int dummy;
} TestComponent;

static fwtContext* init(fwtState* state) {
    fwtContext *result = malloc(sizeof(struct fwtContext));
    result->texture = fwtFindTexture(state, "test2.png");
    return result;
}

static void deinit(fwtState* state, fwtContext *context) {
    free(context);
}

static void reload(fwtState* state, fwtContext *context) {

}

static void unload(fwtState* state, fwtContext *context) {

}

static void event(fwtState* state, fwtContext *context, fwtEventType event) {

}

static void frame(fwtState* state, fwtContext *context, float delta) {
    int width, height;
    fwtWindowSize(state, &width, &height);
    float ratio = (float)width / (float)height;
    fwtViewport(state, 0, 0, width, height);
    fwtProject(state, -ratio, ratio, 1.f, -1.f);
    fwtSetColor(state, 1.f, 0.f, 0.f, 1.f);
    fwtClear(state);

    fwtSetColor(state, 0.f, 0.f, 0.f, 1.f);
    fwtDrawFilledRect(state, -.5f, -.5f, 1.f, 1.f);
}

EXPORT const fwtScene scene = {
    .init = init,
    .deinit = deinit,
    .reload = reload,
    .unload = unload,
    .event = event,
    .frame = frame
};
