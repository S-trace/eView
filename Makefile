name ?= eView
version ?= 064t2
lang ?= russian
CFLAGS+=-std=c99 -D_GNU_SOURCE -Winit-self -Wformat=2 -Wmissing-include-dirs -Wswitch-default -Wfloat-equal -Wundef -Wshadow -Wcast-qual -Wwrite-strings -Wall -Werror -Wno-error=missing-field-initializers -Wno-error=cast-align -Wno-error=format-nonliteral -Wno-format-nonliteral -Wbad-function-cast -Winline -Wnested-externs -Wpointer-arith -DVERSION="\"eView $(version) $(shell LANG=en_US date '+%d.%b.%Y')\""
# -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes -Wcast-align
LDFLAGS+=-lX11 -ldl -lpthread
DFLAGS = -MD
ifeq ($(MAKECMDGOALS), arm)
include libro.mk
CFLAGS += -O3
T_ARCH=ARM
else ifeq ($(MAKECMDGOALS), debug)
include libro.mk
T_ARCH=ARM
CFLAGS += -Ddebug
LDFLAGS += -rdynamic -funwind-tables -g -O0
else ifeq ($(MAKECMDGOALS), clang)
T_ARCH=x86
include desktop_clang.mk
else
T_ARCH=x86
include desktop_gcc.mk
endif

CFLAGS += -Wextra  -Dlanguage_$(lang)

SOURCE_PATH=src

OBJS = gtk_file_manager.o mylib.o mygtk.o ViewImageWindow.o digma_hw.o crop.o cfg.o debug_msg_win.o frames_search.o shift.o archive_handler.o interface.o contrast.o
OBJ = $(addprefix src/, $(OBJS))
EXE = $(name)$(version)_$(lang).app


.PHONY: all	arm


clang arm debug all: cleanup $(OBJ)
	$(LD) -o $(EXE) $(OBJ) $(LDFLAGS)
	$(STRIP)

include $(wildcard src/*.d)

cleanup:
	@for i in $(OBJ); do \
		if [ "q$$(file $$i | cut -d ',' -f 2 | cut -c 2-4)" != "q$(T_ARCH)" ]; then \
			rm -f $(OBJ) $(OBJ:.o=.d); \
			break; \
		fi; \
	done

clean:
	-rm -f $(EXE) $(name) $(name)$(version)_* $(OBJ) $(EXE).sh $(name)$(version)_$(lang).tar.gz $(OBJ:.o=.d)  src/*~

installer:
	cp installer.head.sh $(name)$(version)_$(lang)-installer.sh
	cp $(EXE) $(name)
	tar -czf ./$(EXE).tar.gz ./$(name) ./desktop_$(name)-QbiX_edit.png
	cat ./$(EXE).tar.gz >> $(name)$(version)_$(lang)-installer.sh
	chmod +x $(name)$(version)_$(lang)-installer.sh

release:
	mkdir $(name)$(version)
	make clean
	make debug
	mv $(name)$(version)_russian.app $(name)$(version)/$(name)$(version)_russian-debug.app
	make clean
	make arm
	cp $(name)$(version)_russian.app $(name)$(version)/
	make installer
	mv $(name)$(version)_russian-installer.sh $(name)$(version)/
	make clean
	lang=english make debug
	mv $(name)$(version)_english.app $(name)$(version)/$(name)$(version)_english-debug.app
	make clean
	lang=english make arm
	cp $(name)$(version)_english.app $(name)$(version)/
	lang=english make installer
	mv $(name)$(version)_english-installer.sh $(name)$(version)/
	make clean
	cp $(name)_remover.sh $(name)$(version)/
	cp readme.txt $(name)$(version)/
	cp S-trace-changelog.txt $(name)$(version)/
	cp hardware_support.txt $(name)$(version)/
	cp LICENSE $(name)$(version)/
	zip -r $(name)$(version).zip $(name)$(version)/
	mv $(name)$(version).zip $(name)$(version)/
	
