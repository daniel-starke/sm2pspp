WINDRES = $(PREFIX)windres
CWFLAGS = -Wall -Wextra -Wformat -pedantic -Wshadow -Wno-format -std=c99
CFLAGS = -O2 -DNDEBUG -mtune=core2 -march=core2 -mstackrealign -flto -ffat-lto-objects -fomit-frame-pointer -fno-ident -fgraphite -municode
LDFLAGS = -s -static -fno-ident -municode
#CFLAGS = -Og -g3 -ggdb -gdwarf-3 -fno-omit-frame-pointer -fvar-tracking-assignments -fbounds-check -fstack-protector-strong -fno-ident
#LDFLAGS = -fno-ident -static
PATHS = 
LIBS = 
OBJEXT = .o
BINEXT = .exe

ifeq (, $(findstring __MINGW64__, $(shell $(CC) -dM -E - </dev/null 2>/dev/null)))
 # patch to handle missing symbols in mingw32 correctly
 CFLAGS += -D__MINGW64__=1
endif

CFLAGS += -D_WIN32_WINNT=0x0600 -D_FILE_OFFSET_BITS=64
