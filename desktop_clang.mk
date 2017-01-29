CC = clang
LD = clang
CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
CFLAGS += $(DFLAGS) -g -Ddebug  -Wsign-conversion -Wunreachable-code  -Wstrict-overflow=5 -Wcast-align -fsanitize=memory -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize-memory-track-origins=2 -fsanitize-memory-use-after-dtor -O0
LDFLAGS +=  $(CFLAGS) $(shell pkg-config --libs gtk+-2.0) -rdynamic
