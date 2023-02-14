
APP        = SerialTerminal.exe
OBJECTS    = app.res terminal.o interface.o ../lib/serial.o
FLAGS      = -std=c++20
LIBS       = -lgdi32 -lcomdlg32 -lwinmm
# -L"../lib" -lserialapi
#-mwindows

.PHONY: all all-before all-after clean-custom run-custom lib demo
all: all-before $(APP) all-after

clean: clean-custom
	del /S *.exe *.a *.dll
#	rm -rf *.exe *.a *.dll

purge: clean-custom
	del /S *.o *.exe *.a *.dll *.res
#	rm -rf *.o *.exe *.a *.dll *.res

run: run-custom
	$(APP)


$(APP): main.o $(OBJECTS) rescodes.hpp terminal.hpp interface.hpp
	g++ main.o $(OBJECTS) $(LIBS) $(FLAGS) -o $(APP)

terminal.o: terminal.cpp
	g++ -c terminal.cpp -o terminal.o $(FLAGS)

interface.o: interface.cpp
	g++ -c interface.cpp -o interface.o $(FLAGS)

../lib/serial.o: ../lib/serial.cpp
	g++ -c ../lib/serial.cpp -o ../lib/serial.o $(FLAGS)

app.res: app.rc
	windres -i app.rc --input-format=rc -o app.res -O coff 