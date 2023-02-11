
APP		= serial.exe
OBJECTS = serial.o
FLAGS   = -std=c++20
LIBS	= -lwinmm
LIBNAME = serialapi


.PHONY: all all-before all-after clean-custom
all: all-before $(APP) all-after

clean: clean-custom
	del /S *.exe *.a *.dll
#	rm -rf *.exe *.a *.dll

purge: clean-custom
	del /S *.o *.exe *.a *.dll
#	rm -rf *.o *.exe *.a *.dll


# ----
#	dev app
# ----
$(APP): main.o $(OBJECTS) serial.hpp
	g++ main.o serial.o $(LIBS) $(FLAGS) -o $(APP)

main.o: main.cpp
	g++ -c main.cpp $(FLAGS)

# ----
#	lib
# ----
#	make static lib
libstatic: $(OBJECTS)
	ar rvs lib$(LIBNAME).a $(OBJECTS)

#	make dll
libshared: $(OBJECTS)
	g++ $(OBJECTS) $(LIBS) $(FLAGS) -s -shared -o $(LIBNAME).dll -Wl,--out-implib,lib$(LIBNAME).dll.a


# ----
#	objects
# ----

serial.o: serial.cpp serial.hpp
	g++ -c serial.cpp $(FLAGS)
