CC=gcc
CPP=g++
INCLUDES=-I/Library/Frameworks/GStreamer.framework/Versions/1.0/Headers/
CFLAGS=-c -Wall $(INCLUDES)
CPPFLAGS=-c -Wall $(INCLUDES)
LDFLAGS=-L/Library/Frameworks/GStreamer.framework/Versions/1.0/lib/ -lgstreamer-1.0 -lglib-2.0 -lgobject-2.0 -lc++
SOURCES=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SOURCES:.c=.o))
EXECUTABLE=m3

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.cpp.o:
	$(CPP) $(CPPFLAGS) $< -o $@