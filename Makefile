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

INCLUDE=-Ideps -Isrc -Ibuild
SHDC_PATH=bin/$(ARCH)/sokol-shdc$(PROG_EXT)

default: program

shader:
	$(SHDC_PATH) -i etc/default.glsl -o src/fwt.glsl.h -l $(SHDC_FLAGS)

ALL_SRC := $(wildcard src/*.c)
SRC := $(filter-out src/fwt.c, $(ALL_SRC))
SCENES := scenes
INC := -Ideps -Isrc -Ibuild
BIN := build
TARGETS := $(foreach file,$(foreach src,$(wildcard $(SCENES)/*.c),$(notdir $(src))),$(patsubst %.c,$(BIN)/%.$(LIBEXT),$(file)))

.PHONY: FORCE scenes

FORCE: ;

$(BIN)/%.$(LIBEXT): $(SCENES)/%.c FORCE | $(BIN)
	$(CC) $(INC) -shared -fpic -DFWT_SCENE $(CFLAGS) $(SRC) $(LINK) -o $@ $<

scenes: $(TARGETS)

program: shader
	$(CC) $(INC) $(CFLAGS) $(ALL_SRC) $(LINK) -o $(BIN)/fwp$(PROGEXT)

all: shader scenes program

.PHONY: default all scenes program shader
