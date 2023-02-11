
APP        = SerialTerminal.exe
OBJECTS    = app.res appops.o serialcomms.o
FLAGS      = -std=c++20 -mwindows
LIBS       = -lgdi32 -lcomdlg32

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
	g++ main.o $(OBJECTS) $(LIBS) $(FLAGS) -o $(APP)

appops.o: appops.cpp
	g++ -c appops.cpp -o appops.o $(FLAGS)

serialcomms.o: serialcomms.cpp
	g++ -c serialcomms.cpp -o serialcomms.o $(FLAGS)

app.res: app.rc
	windres -i app.rc --input-format=rc -o app.res -O coff 