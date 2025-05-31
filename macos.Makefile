LIBEXT=dylib
PROGEXT=
CFLAGS=-x objective-c -DSOKOL_METAL -fobjc-arc -framework Metal -framework Cocoa -framework MetalKit -framework Quartz
SHDC_FLAGS=metal_macos
ifeq ($(shell uname -m),arm64)
    ARCH=osx_arm64
else
    ARCH=osx
endif
