CC = gcc
LD = gcc
CFLAGS += $(shell pkg-config --cflags gtk+-2.0)
CFLAGS += -Wall -g -Werror -Ddebug_printf $(DFLAGS) -std=c99 -D_GNU_SOURCE
LDFLAGS += $(shell pkg-config --libs gtk+-2.0) -lX11 -rdynamic -ldl -lpthread
