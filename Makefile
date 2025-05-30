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

SRC := scenes
INC := -Ideps -Isrc -Ibuild
BIN := build
TARGETS := $(foreach file,$(foreach src,$(wildcard $(SRC)/*.c),$(notdir $(src))),$(patsubst %.c,$(BIN)/%.$(LIBEXT),$(file)))

.PHONY: FORCE scenes

FORCE: ;

$(BIN)/%.$(LIBEXT): $(SRC)/%.c FORCE | $(BIN)
	$(CC) $(INC) -shared -fpic $(CFLAGS) src/*.c $(LINK) -o $@ $<

scenes: $(TARGETS)

program:
	$(CC) $(INC) $(CFLAGS) src/*.c $(LINK) -o $(BIN)/fwp$(PROGEXT)

all: shader scenes program

.PHONY: default all scenes program shader
