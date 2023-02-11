
APP        = serter.exe
OBJECTS    = app.res serialio.o
FLAGS      = -std=c11

.PHONY: all all-before all-after clean-custom run-custom lib demo
all: all-before $(APP) all-after

clean: clean-custom
	del /S *.exe *.a *.dll
#	rm -rf *.exe *.a *.dll

purge: clean-custom
	del /S *.o *.exe *.a *.dll
#	rm -rf *.o *.exe *.a *.dll

run: run-custom
	$(APP)


$(APP): main.o $(OBJECTS)
	gcc main.o $(OBJECTS) $(FLAGS) -o $(APP)

serialio.o: serialio.c
	gcc -c serialio.c -o serialio.o $(FLAGS)

app.res: app.rc
	windres -i app.rc --input-format=rc -o app.res -O coff 