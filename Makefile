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

SRC := $(wildcard "src/*.c")
SCENES := scenes
INC := -Ideps -Isrc -Lbuild
BIN := build

default: program

builddir:
	mkdir -p $(BIN)

SHDC_PATH=bin/$(ARCH)/sokol-shdc$(PROG_EXT)

shader:
	$(SHDC_PATH) -i etc/shader.glsl -o $(BUILD)/shader.glsl.h -l $(SHDC_FLAGS)

sokol: builddir
	$(CC) $(INC) -shared -fpic $(CFLAGS) etc/sokol.c $(LINK) -o $(BIN)/libsokol.$(LIBEXT)

TARGETS := $(foreach file,$(foreach src,$(wildcard $(SCENES)/*.c),$(notdir $(src))),$(patsubst %.c,$(BIN)/%.$(LIBEXT),$(file)))

.PHONY: FORCE scenes

FORCE: ;

$(BIN)/%.$(LIBEXT): $(SCENES)/%.c FORCE | $(BIN)
	$(CC) $(INC) -shared -fpic $(CFLAGS) $(SRC) $(LINK) -lsokol -o $@ $<

scenes: sokol $(TARGETS)

program: builddir sokol shader
	$(CC) $(INC) $(CFLAGS) -DFWT_MAIN_PROGRAM $(SRC) $(LINK) -lsokol -o $(BIN)/fwt$(PROGEXT)

all: sokol scenes program

.PHONY: default all builddir sokol scenes program shader
