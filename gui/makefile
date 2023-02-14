
APP        = SerialTerminal.exe
OBJECTS    = main.o app.res terminal.o interface.o
LIBOBJECTS = ../lib/serial.o
FLAGS      = -std=c++20
LIBS       = -lgdi32 -lcomdlg32 -lwinmm
# -L"../lib" -lserialapi
#-mwindows

.PHONY: all all-before all-after clean-custom run-custom prod prod-custom
all: all-before $(APP) all-after

clean: clean-custom
	del /S *.exe *.a *.dll
#	rm -rf *.exe *.a *.dll

purge: clean-custom
	del /S *.o *.exe *.a *.dll *.res
#	rm -rf *.o *.exe *.a *.dll *.res

run: run-custom
	$(APP)

prod: prod-custom
	g++ $(OBJECTS) -L"../lib" -lserialapi $(LIBS) $(FLAGS) -mwindows -o $(APP)


$(APP): $(OBJECTS) $(LIBOBJECTS)
	g++ $(OBJECTS) $(LIBOBJECTS) $(LIBS) $(FLAGS) -o $(APP)

main.o: main.cpp
	g++ -c main.cpp -o main.o $(FLAGS)

terminal.o: terminal.cpp
	g++ -c terminal.cpp -o terminal.o $(FLAGS)

interface.o: interface.cpp
	g++ -c interface.cpp -o interface.o $(FLAGS)

../lib/serial.o: ../lib/serial.cpp
	g++ -c ../lib/serial.cpp -o ../lib/serial.o $(FLAGS)

app.res: app.rc
	windres -i app.rc --input-format=rc -o app.res -O coff 