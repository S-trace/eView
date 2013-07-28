TOOLCHAIN_PATH := /usr/local/ereader/eabi-glibc

CC  = $(TOOLCHAIN_PATH)/bin/arm-ereader-linux-gnueabi-gcc
LD  = $(TOOLCHAIN_PATH)/bin/arm-ereader-linux-gnueabi-gcc
STRIP  = $(TOOLCHAIN_PATH)/bin/arm-ereader-linux-gnueabi-strip -s $(EXE)

PKGCONFIG_PATH := $(TOOLCHAIN_PATH)/arm-ereader-linux-gnueabi
PKGCONFIG := "PKG_CONFIG_SYSROOT_DIR=$(PKGCONFIG_PATH) PKG_CONFIG_PATH=$(PKGCONFIG_PATH)/usr/lib/pkgconfig $(TOOLCHAIN_PATH)/bin/pkg-config"
PKGCONFIG_CFLAGS := $(shell eval $(PKGCONFIG) --cflags gtk+-2.0)
PKGCONFIG_LDFLAGS := $(shell eval $(PKGCONFIG) --libs gtk+-2.0)

CFLAGS += $(PKGCONFIG_CFLAGS) $(DFLAGS)
LDFLAGS += $(PKGCONFIG_LDFLAGS) -lX11

CFLAGS += -O3 -Wall
