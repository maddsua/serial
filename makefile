
MAINTARGET	= libshared
FLAGS		= -std=c++20

.PHONY: all all-before all-after clean-custom
all: all-before $(MAINTARGET) all-after

clean: clean-custom
	del /S *.o *.exe *.a *.dll
#	rm -rf *.o *.exe *.a *.dll

# ----
#	Libserial
# ----


OBJECTS_LIB = lib/serialport.o lib/serialdevice.o lib/dllinfo.res
TARGET_LIB = libserial.dll

libshared: $(TARGET_LIB)
$(TARGET_LIB): $(OBJECTS_LIB)
	g++ $(OBJECTS_LIB) $(FLAGS) -s -shared -o $(TARGET_LIB) -Wl,--out-implib,$(TARGET_LIB).a

lib/serialport.o: lib/serialport.cpp
	g++ -c lib/serialport.cpp -o lib/serialport.o $(FLAGS)

lib/serialdevice.o: lib/serialdevice.cpp
	g++ -c lib/serialdevice.cpp -o lib/serialdevice.o $(FLAGS)

lib/dllinfo.res: lib/dllinfo.rc
	windres -i lib/dllinfo.rc --input-format=rc -o lib/dllinfo.res -O coff
