
APP		= serial.exe
FLAGS   = -std=c++20
LIBS	= -lwinmm


.PHONY: all all-before all-after clean-custom
all: all-before $(APP) all-after

clean: clean-custom
	del /S *.exe *.a *.dll
#	rm -rf *.exe *.a *.dll

purge: clean-custom
	del /S *.o *.exe *.a *.dll
#	rm -rf *.o *.exe *.a *.dll


$(APP): main.o serial.o serial.hpp
	g++ main.o serial.o $(LIBS) $(FLAGS) -o $(APP)

main.o: main.cpp
	g++ -c main.cpp $(FLAGS)

serial.o: serial.cpp serial.hpp
	g++ -c serial.cpp $(FLAGS)
