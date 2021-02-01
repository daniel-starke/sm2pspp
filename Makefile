PREFIX = 
CC = $(PREFIX)gcc

SRC = \
  src/parser.c \
  src/sm2pspp.c \
  src/tchar.c

SYS := $(shell $(CC) -dumpmachine)
ifneq (, $(findstring linux, $(SYS)))
 include src/linux.mk
else
 ifneq (, $(findstring mingw, $(SYS))$(findstring windows, $(SYS)))
  include src/mingw.mk
 else
  include src/general.mk
 endif
endif

all: bin bin/sm2pspp$(BINEXT)

.PHONY: clean
clean:
ifeq (,$(strip $(WINDRES)))
	rm -f bin/sm2pspp$(BINEXT)
else
	rm -f bin/sm2pspp$(BINEXT) bin/version$(OBJEXT)
endif

bin:
	mkdir bin

.PHONY: bin/sm2pspp$(BINEXT)
bin/sm2pspp$(BINEXT): $(SRC)
ifeq (,$(strip $(WINDRES)))
	rm -f $@
	$(CC) $(CFLAGS) $(CWFLAGS) $(PATHS) $(LDFLAGS) -o $@ $+ $(LIBS)
else
	rm -f $@ bin/sm2pspp$(OBJEXT)
	$(WINDRES) src/version.rc bin/version$(OBJEXT)
	$(CC) $(CFLAGS) $(CWFLAGS) $(PATHS) $(LDFLAGS) -o $@ $+ bin/version$(OBJEXT) $(LIBS)
endif
