# =========================================================================
#     This makefile was generated by
#     Bakefile 0.2.9 (http://www.bakefile.org)
#     Do not modify, all changes will be overwritten!
# =========================================================================




prefix = /usr/local
exec_prefix = ${prefix}
datarootdir = ${prefix}/share
INSTALL = /usr/bin/install -c
EXEEXT = 
SETFILE = SetFile
STRIP = strip
INSTALL_PROGRAM = cp -fp
INSTALL_DIR = mkdir -p
BK_DEPS = /Users/jhg/Documents/work/mog-master/M4/wxWidgets-3.0.2/build-cocoa-debug/bk-deps
srcdir = ../../../utils/execmon
top_srcdir = ../../..
bindir = ${exec_prefix}/bin
LIBS = -lz -lpthread -lc++
CXX = g++ -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk -mmacosx-version-min=10.7
CXXFLAGS = -DWX_PRECOMP -O2 -fno-strict-aliasing -std=c++0x -stdlib=libc++ -fno-common 
CPPFLAGS = -D_FILE_OFFSET_BITS=64 -I${wx_top_builddir}/lib/wx/include/osx_cocoa-unicode-static-3.0 -I${top_srcdir}/include -stdlib=libc++
LDFLAGS =   -framework IOKit -framework Carbon -framework Cocoa -framework AudioToolbox -framework System -framework OpenGL 
WX_LIB_FLAVOUR = 
TOOLKIT = OSX_COCOA
TOOLKIT_LOWERCASE = osx_cocoa
TOOLKIT_VERSION = 
EXTRALIBS =   -framework IOKit -framework Carbon -framework Cocoa -framework AudioToolbox -framework System -framework OpenGL  -lz -lpthread -lc++ 
EXTRALIBS_XML =  -lexpat
EXTRALIBS_GUI = 
CXXWARNINGS = -Wall -Wundef -Wunused-parameter -Wno-ctor-dtor-privacy -Woverloaded-virtual -Wno-deprecated-declarations
HOST_SUFFIX = 
wx_top_builddir = /Users/jhg/Documents/work/mog-master/M4/wxWidgets-3.0.2/build-cocoa-debug

### Variables: ###

DESTDIR = 
WX_RELEASE = 3.0
LIBDIRNAME = $(wx_top_builddir)/lib
EXECMON_CXXFLAGS = -D__WX$(TOOLKIT)__ $(__WXUNIV_DEFINE_p) $(__DEBUG_DEFINE_p) \
	$(__EXCEPTIONS_DEFINE_p) $(__RTTI_DEFINE_p) $(__THREAD_DEFINE_p) \
	-I$(srcdir) $(__DLLFLAG_p) -DwxUSE_GUI=0 $(CXXWARNINGS) $(CPPFLAGS) \
	$(CXXFLAGS)
EXECMON_OBJECTS =  \
	execmon_execmon.o

### Conditionally set variables: ###

#CXXC = $(CXX)
CXXC = $(BK_DEPS) $(CXX)
#PORTNAME = base
PORTNAME = $(TOOLKIT_LOWERCASE)$(TOOLKIT_VERSION)
#WXBASEPORT = _carbon
#WXDEBUGFLAG = d
WXUNICODEFLAG = u
#WXUNIVNAME = univ
#EXTRALIBS_FOR_BASE = $(EXTRALIBS)
EXTRALIBS_FOR_BASE = $(EXTRALIBS) \
	$(EXTRALIBS_XML) $(EXTRALIBS_GUI)
__execmon___depname = execmon$(EXEEXT)
#__execmon___mac_setfilecmd = @true
__execmon___mac_setfilecmd = \
	$(SETFILE) -t APPL execmon$(EXEEXT)
#__WXUNIV_DEFINE_p = -D__WXUNIVERSAL__
#__DEBUG_DEFINE_p = -DwxDEBUG_LEVEL=0
#__EXCEPTIONS_DEFINE_p = -DwxNO_EXCEPTIONS
#__RTTI_DEFINE_p = -DwxNO_RTTI
#__THREAD_DEFINE_p = -DwxNO_THREADS
#__DLLFLAG_p = -DWXUSINGDLL
COND_MONOLITHIC_0___WXLIB_BASE_p = \
	-lwx_base$(WXBASEPORT)$(WXUNICODEFLAG)$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
#__WXLIB_BASE_p = $(COND_MONOLITHIC_0___WXLIB_BASE_p)
__install_execmon___depname = install_execmon
__uninstall_execmon___depname = uninstall_execmon
COND_MONOLITHIC_1___WXLIB_MONO_p = \
	-lwx_$(PORTNAME)$(WXUNIVNAME)$(WXUNICODEFLAG)$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
__WXLIB_MONO_p = $(COND_MONOLITHIC_1___WXLIB_MONO_p)
__LIB_PNG_IF_MONO_p = $(__LIB_PNG_p)
__LIB_PNG_p \
	= \
	-lwxpng$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
#__LIB_ZLIB_p = \
#	-lwxzlib$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
COND_wxUSE_REGEX_builtin___LIB_REGEX_p = \
	-lwxregex$(WXUNICODEFLAG)$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
__LIB_REGEX_p = $(COND_wxUSE_REGEX_builtin___LIB_REGEX_p)
#__LIB_EXPAT_p = \
#	-lwxexpat$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)

### Targets: ###

all: $(__execmon___depname)

install: $(__install_execmon___depname)

uninstall: $(__uninstall_execmon___depname)

install-strip: install
	$(STRIP) $(DESTDIR)$(bindir)/execmon$(EXEEXT)

clean: 
	rm -rf ./.deps ./.pch
	rm -f ./*.o
	rm -f execmon$(EXEEXT)

distclean: clean
	rm -f config.cache config.log config.status bk-deps bk-make-pch shared-ld-sh Makefile

execmon$(EXEEXT): $(EXECMON_OBJECTS)
	$(CXX) -o $@ $(EXECMON_OBJECTS)    -L$(LIBDIRNAME)  $(LDFLAGS)  $(__WXLIB_BASE_p)  $(__WXLIB_MONO_p) $(__LIB_PNG_IF_MONO_p) $(__LIB_ZLIB_p) $(__LIB_REGEX_p) $(__LIB_EXPAT_p) $(EXTRALIBS_FOR_BASE) $(LIBS)
	
	$(__execmon___mac_setfilecmd)

install_execmon: $(__execmon___depname)
	$(INSTALL_DIR) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) execmon$(EXEEXT) $(DESTDIR)$(bindir)

uninstall_execmon: 
	rm -f $(DESTDIR)$(bindir)/execmon$(EXEEXT)

execmon_execmon.o: $(srcdir)/execmon.cpp
	$(CXXC) -c -o $@ $(EXECMON_CXXFLAGS) $(srcdir)/execmon.cpp


# Include dependency info, if present:
-include ./.deps/*.d

.PHONY: all install uninstall clean distclean install_execmon uninstall_execmon
