LIB_EXT=dylib
PROG_EXT=
CFLAGS=-x objective-c -DSOKOL_METAL -fobjc-arc -framework Metal -framework Cocoa -framework MetalKit -framework Quartz -framework AudioToolbox
SHDC_FLAGS=metal_macos
ifeq ($(shell uname -m),arm64)
    ARCH=osx_arm64
else
    ARCH=osx
endif
