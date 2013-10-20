CC = clang
LD = clang
CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
CFLAGS += $(DFLAGS) -g -Ddebug_printf  -Wsign-conversion -Wunreachable-code  -Wstrict-overflow=5 -Wcast-align #-Wconversion
LDFLAGS += $(shell pkg-config --libs gtk+-2.0) -rdynamic
