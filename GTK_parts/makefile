EXE = GTK_parts
all:
	cp installer.head.sh $(EXE).sh
	tar -czf ./$(EXE).tar.gz etc home usr
	cat ./$(EXE).tar.gz >> $(EXE).sh
	chmod +x $(EXE).sh
clean:
	-rm -f $(EXE).sh $(EXE).tar.gz 
