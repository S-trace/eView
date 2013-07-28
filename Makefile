name ?= eView
prev_version ?= 061
version ?= 062
lang ?= russian
DFLAGS = -MD
ifeq ($(MAKECMDGOALS), arm)
include libro.mk
T_ARCH=ARM
else
include desktop.mk
T_ARCH=x86
endif

CFLAGS += -Wextra  -Dlanguage_$(lang)
ifeq ($(MAKECMDGOALS), debug)
include libro.mk
T_ARCH=ARM
CFLAGS += -Ddebug_printf 
endif

SOURCE_PATH=src

OBJS = gtk_file_manager.o mylib.o mygtk.o ViewImageWindow.o digma_hw.o crop.o cfg.o debug_msg_win.o frames_search.o shift.o archive_handler.o interface.o
OBJ = $(addprefix src/, $(OBJS))
EXE = $(name)$(version)_$(lang)


.PHONY: all	arm


arm debug all: cleanup $(OBJ)
	$(LD) -o $(EXE) $(OBJ) $(LDFLAGS)
	$(STRIP)

include $(wildcard src/*.d)

cleanup:
	@for i in $(OBJ); do \
		if [ `file $$i | cut -d "," -f 2 | cut -c 2-4` != $(T_ARCH) ]; then \
			rm -f $(OBJ) $(OBJ:.o=.d); \
			break; \
		fi; \
	done

clean:
	-rm -f $(EXE) $(name) $(name)$(version)_* $(OBJ) $(EXE).sh $(EXE).tar.gz $(OBJ:.o=.d)  src/*~

installer:
	cp installer.head.sh $(EXE)-installer.sh
	cp $(EXE) $(name)
	tar -czf ./$(EXE).tar.gz ./$(name) ./desktop_$(name).png desktop_picture.png
	cat ./$(EXE).tar.gz >> $(EXE)-installer.sh
	chmod +x $(EXE)-installer.sh

release:
	make clean
	rm -rf .eView/
	cd ../../; diff -ru eView$(prev_version)/ eView$(version) > eView$(version)/eView$(prev_version)-$(version).diff || true
	cd -
	make debug
	mv $(name)$(version)_russian ../$(name)$(version)_russian-debug
	make clean
	make arm
	cp $(name)$(version)_russian ../
	make installer
	mv $(name)$(version)_russian-installer.sh ../
	make clean
	lang=english make debug
	mv $(name)$(version)_english ../$(name)$(version)_english-debug
	make clean
	lang=english make arm
	cp $(name)$(version)_english ../
	lang=english make installer
	mv $(name)$(version)_english-installer.sh ../
	make clean
	cd ../../; zip -r eView$(version).zip eView$(version)/;cd -