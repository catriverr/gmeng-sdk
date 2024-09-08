HAS_NCURSES := $(shell pkg-config --exists ncurses && echo yes || echo no)
HAS_PKG_CONFIG := $(shell command -v pkg-config >/dev/null 2>&1 && echo yes || echo no)

# Check for ncurses if pkg-config is available
ifeq ($(HAS_PKG_CONFIG),yes)
  HAS_NCURSES := $(shell pkg-config --exists ncurses && echo yes || echo no)
else
  HAS_NCURSES := no
endif
