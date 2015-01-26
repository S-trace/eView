CC = gcc
LD = gcc
CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
CFLAGS += $(DFLAGS) -g -Ddebug_printf -Wsign-conversion -Wunreachable-code  -Wstrict-overflow=5 -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -fipa-pure-const -Wlogical-op -Wcast-align #-Wconversion
LDFLAGS += $(shell pkg-config --libs gtk+-2.0) -rdynamic
