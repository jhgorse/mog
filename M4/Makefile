CC=gcc
CPP=g++

WX_BASE=/Users/justinhartman/Documents/BoxCast/JingSelfStudy/OneCommunity/libs/wxWidgets-3.0.2

INCLUDES= \
	-I/Library/Frameworks/GStreamer.framework/Versions/1.0/Headers/ \
	-I$(WX_BASE)/build-cocoa-debug/lib/wx/include/osx_cocoa-unicode-static-3.0 \
	-I$(WX_BASE)/include
CFLAGS=-c -O0 -g -Wall $(INCLUDES) -D_FILE_OFFSET_BITS=64 -D__WXMAC__ -D__WXOSX__ -D__WXOSX_COCOA__ -D__APPLE__
CPPFLAGS=-c -O0 -g -Wall $(INCLUDES) -D_FILE_OFFSET_BITS=64 -D__WXMAC__ -D__WXOSX__ -D__WXOSX_COCOA__ -D__APPLE__
LDFLAGS= \
	-L/Library/Frameworks/GStreamer.framework/Versions/1.0/lib/ \
	-lgstreamer-1.0 -lglib-2.0 -lgobject-2.0 -lgstbase-1.0 -lgstvideo-1.0 -lc++ -lpthread \
	-L$(WX_BASE)/build-cocoa-debug/lib \
	-framework IOKit -framework Carbon -framework Cocoa -framework AudioToolbox -framework System -framework OpenGL \
	$(WX_BASE)/build-cocoa-debug/lib/libwx_osx_cocoau-3.0.a \
	$(WX_BASE)/build-cocoa-debug/lib/libwx_osx_cocoau_gl-3.0.a \
	-framework WebKit -framework AVFoundation -framework CoreAudio -framework CoreMedia -framework QTKit \
	-lwxregexu-3.0 -lwxtiff-3.0 -lwxjpeg-3.0 -lwxpng-3.0 -lz -lpthread -lc++ -llzma
SOURCES=$(wildcard *.c)
SOURCES+=$(wildcard *.cpp)
SOURCES+=$(wildcard *.mm)
OBJECTS=$(patsubst %.cpp,%.o,$(patsubst %.mm,%.o,$(SOURCES:.c=.o)))
EXECUTABLE=m4

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.cpp.o:
	$(CPP) $(CPPFLAGS) $< -o $@

.SUFFIXES: .o .mm
.mm.o:
	$(CPP) $(CPPFLAGS) $< -o $@