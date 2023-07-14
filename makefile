
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

OBJECTS_LIB = lib/dllinfo.res lib/serialport.o lib/serialdevice.o
TARGET_LIB	= libserial.dll

libshared: $(TARGET_LIB)
$(TARGET_LIB): $(OBJECTS_LIB)
	g++ $(OBJECTS_LIB) $(FLAGS) -s -shared -o $(TARGET_LIB) -Wl,--out-implib,$(TARGET_LIB).a

lib/dllinfo.res: lib/dllinfo.rc
	windres -i lib/dllinfo.rc --input-format=rc -o lib/dllinfo.res -O coff

lib/serialport.o: lib/serialport.cpp
	g++ -c lib/serialport.cpp -o lib/serialport.o $(FLAGS)

lib/serialdevice.o: lib/serialdevice.cpp
	g++ -c lib/serialdevice.cpp -o lib/serialdevice.o $(FLAGS)


# ----
#	Gui app
# ----

OBJECTS_APP	= app/app.res app/main.o app/terminal.o app/interface.o app/settings.o app/serial.o
TARGET_APP	= SerialTerminal.exe
LIBS_APP	= -lgdi32 -lcomdlg32 -lwinmm

app: $(TARGET_APP)

$(TARGET_APP): $(OBJECTS_APP)
	g++ $(OBJECTS_APP) $(LIBS_APP) $(FLAGS) -o $(TARGET_APP)

app/app.res: app/app.rc
	windres -i app/app.rc --input-format=rc -o app/app.res -O coff

app/main.o: app/main.cpp
	g++ -c app/main.cpp -o app/main.o $(FLAGS)

app/terminal.o: app/terminal.cpp
	g++ -c app/terminal.cpp -o app/terminal.o $(FLAGS)

app/interface.o: app/interface.cpp
	g++ -c app/interface.cpp -o app/interface.o $(FLAGS)

app/settings.o: app/settings.cpp
	g++ -c app/settings.cpp -o app/settings.o $(FLAGS)	

app/serial.o: app/serial.cpp
	g++ -c app/serial.cpp -o app/serial.o $(FLAGS)
