ifeq ($(OS),Windows_NT)
	include windows.Makefile
else
	UNAME:=$(shell uname -s)
	ifeq ($(UNAME),Darwin)
		include macos.Makefile
	else ifeq ($(UNAME),Linux)
		include linux.Makefile
	else
		$(error OS not supported by this Makefile)
	endif
endif

default: program

SHDC_PATH=bin/$(ARCH)/sokol-shdc$(PROG_EXT)

shader:
	$(SHDC_PATH) -i etc/shader.glsl -o src/shader.glsl.h -l $(SHDC_FLAGS)

sokol:
	$(CC) $(INC) -shared -fpic $(CFLAGS) src/sokol.c $(LINK) -o $(BIN)/libsokol.$(LIBEXT)

SRC := src/fwt.c
SCENES := scenes
INC := -Ideps -Isrc -Lbuild
BIN := build
TARGETS := $(foreach file,$(foreach src,$(wildcard $(SCENES)/*.c),$(notdir $(src))),$(patsubst %.c,$(BIN)/%.$(LIBEXT),$(file)))

.PHONY: FORCE scenes

FORCE: ;

$(BIN)/%.$(LIBEXT): $(SCENES)/%.c FORCE | $(BIN)
	$(CC) $(INC) -shared -fpic $(CFLAGS) $(SRC) $(LINK) -lsokol -o $@ $<

scenes: sokol $(TARGETS)

program: sokol shader
	$(CC) $(INC) $(CFLAGS) -DFWT_MAIN_PROGRAM $(SRC) $(LINK) -lsokol -o $(BIN)/fwt$(PROGEXT)

all: shader scenes program

.PHONY: default all sokol scenes program shader
