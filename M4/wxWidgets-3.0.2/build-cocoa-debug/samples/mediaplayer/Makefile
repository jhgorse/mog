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
WINDRES = 
SETFILE = SetFile
NM = nm
BK_DEPS = /Users/jhg/Documents/work/mog-master/M4/wxWidgets-3.0.2/build-cocoa-debug/bk-deps
srcdir = ../../../samples/mediaplayer
top_srcdir = ../../..
LIBS = -lz -lpthread -lc++
LDFLAGS_GUI = 
CXX = g++ -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk -mmacosx-version-min=10.7
CXXFLAGS = -DWX_PRECOMP -O2 -fno-strict-aliasing -std=c++0x -stdlib=libc++ -fno-common 
CPPFLAGS = -D_FILE_OFFSET_BITS=64 -I${wx_top_builddir}/lib/wx/include/osx_cocoa-unicode-static-3.0 -I${top_srcdir}/include -stdlib=libc++
LDFLAGS =   -framework IOKit -framework Carbon -framework Cocoa -framework AudioToolbox -framework System -framework OpenGL 
WX_LIB_FLAVOUR = 
TOOLKIT = OSX_COCOA
TOOLKIT_LOWERCASE = osx_cocoa
TOOLKIT_VERSION = 
TOOLCHAIN_FULLNAME = osx_cocoa-unicode-static-3.0
EXTRALIBS =   -framework IOKit -framework Carbon -framework Cocoa -framework AudioToolbox -framework System -framework OpenGL  -lz -lpthread -lc++ 
EXTRALIBS_XML =  -lexpat
EXTRALIBS_MEDIA = -framework QTKit
EXTRALIBS_GUI = 
CXXWARNINGS = -Wall -Wundef -Wunused-parameter -Wno-ctor-dtor-privacy -Woverloaded-virtual -Wno-deprecated-declarations
HOST_SUFFIX = 
SAMPLES_RPATH_FLAG = 
wx_top_builddir = /Users/jhg/Documents/work/mog-master/M4/wxWidgets-3.0.2/build-cocoa-debug

### Variables: ###

DESTDIR = 
WX_RELEASE = 3.0
WX_VERSION = $(WX_RELEASE).2
LIBDIRNAME = $(wx_top_builddir)/lib
MEDIAPLAYER_CXXFLAGS = -D__WX$(TOOLKIT)__ $(__WXUNIV_DEFINE_p) \
	$(__DEBUG_DEFINE_p) $(__EXCEPTIONS_DEFINE_p) $(__RTTI_DEFINE_p) \
	$(__THREAD_DEFINE_p) -I$(srcdir) $(__DLLFLAG_p) -I$(srcdir)/../../samples \
	$(CXXWARNINGS) $(CPPFLAGS) $(CXXFLAGS)
MEDIAPLAYER_OBJECTS =  \
	$(__mediaplayer___win32rc) \
	$(__mediaplayer_os2_lib_res) \
	mediaplayer_mediaplayer.o

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
#EXTRALIBS_FOR_GUI = $(EXTRALIBS_GUI)
EXTRALIBS_FOR_GUI = 
#__mediaplayer___mac_setfilecmd = @true
__mediaplayer___mac_setfilecmd = \
	$(SETFILE) -t APPL mediaplayer$(EXEEXT)
#__WXUNIV_DEFINE_p = -D__WXUNIVERSAL__
#__WXUNIV_DEFINE_p_2 = --define __WXUNIVERSAL__
#__DEBUG_DEFINE_p = -DwxDEBUG_LEVEL=0
#__DEBUG_DEFINE_p_2 = --define wxDEBUG_LEVEL=0
#__EXCEPTIONS_DEFINE_p = -DwxNO_EXCEPTIONS
#__EXCEPTIONS_DEFINE_p_2 = --define wxNO_EXCEPTIONS
#__RTTI_DEFINE_p = -DwxNO_RTTI
#__RTTI_DEFINE_p_2 = --define wxNO_RTTI
#__THREAD_DEFINE_p = -DwxNO_THREADS
#__THREAD_DEFINE_p_2 = --define wxNO_THREADS
#__DLLFLAG_p = -DWXUSINGDLL
#__DLLFLAG_p_2 = --define WXUSINGDLL
COND_PLATFORM_OS2_1___mediaplayer___os2_emxbindcmd = $(NM) \
	mediaplayer$(EXEEXT) | if grep -q pmwin.763 ; then emxbind -ep \
	mediaplayer$(EXEEXT) ; fi
#__mediaplayer___os2_emxbindcmd = $(COND_PLATFORM_OS2_1___mediaplayer___os2_emxbindcmd)
#__RCDEFDIR_p_1 = --include-dir \
#	$(LIBDIRNAME)/wx/include/$(TOOLCHAIN_FULLNAME)
#__mediaplayer___win32rc = mediaplayer_sample_rc.o
#__mediaplayer_os2_lib_res = \
#	$(top_srcdir)/include/wx/os2/wx.res
__mediaplayer_app_Contents_PkgInfo___depname \
	= mediaplayer.app/Contents/PkgInfo
__mediaplayer_bundle___depname = mediaplayer_bundle
#____mediaplayer_BUNDLE_TGT_REF_DEP = \
#	$(__mediaplayer_app_Contents_PkgInfo___depname)
#____mediaplayer_BUNDLE_TGT_REF_DEP \
#	= \
#	$(__mediaplayer_app_Contents_PkgInfo___depname)
____mediaplayer_BUNDLE_TGT_REF_DEP \
	= $(__mediaplayer_app_Contents_PkgInfo___depname)
#____mediaplayer_BUNDLE_TGT_REF_DEP \
#	= \
#	$(__mediaplayer_app_Contents_PkgInfo___depname)
#____mediaplayer_BUNDLE_TGT_REF_DEP \
#	= $(__mediaplayer_app_Contents_PkgInfo___depname)
COND_MONOLITHIC_0_USE_MEDIA_1___WXLIB_MEDIA_p = \
	-lwx_$(PORTNAME)$(WXUNIVNAME)$(WXUNICODEFLAG)$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)_media-$(WX_RELEASE)$(HOST_SUFFIX)
#__WXLIB_MEDIA_p = $(COND_MONOLITHIC_0_USE_MEDIA_1___WXLIB_MEDIA_p)
COND_MONOLITHIC_0___WXLIB_CORE_p = \
	-lwx_$(PORTNAME)$(WXUNIVNAME)$(WXUNICODEFLAG)$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)_core-$(WX_RELEASE)$(HOST_SUFFIX)
#__WXLIB_CORE_p = $(COND_MONOLITHIC_0___WXLIB_CORE_p)
COND_MONOLITHIC_0___WXLIB_BASE_p = \
	-lwx_base$(WXBASEPORT)$(WXUNICODEFLAG)$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
#__WXLIB_BASE_p = $(COND_MONOLITHIC_0___WXLIB_BASE_p)
COND_MONOLITHIC_1___WXLIB_MONO_p = \
	-lwx_$(PORTNAME)$(WXUNIVNAME)$(WXUNICODEFLAG)$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
__WXLIB_MONO_p = $(COND_MONOLITHIC_1___WXLIB_MONO_p)
__LIB_SCINTILLA_IF_MONO_p \
	= \
	-lwxscintilla$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
__LIB_TIFF_p \
	= \
	-lwxtiff$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
__LIB_JPEG_p \
	= \
	-lwxjpeg$(WXDEBUGFLAG)$(WX_LIB_FLAVOUR)-$(WX_RELEASE)$(HOST_SUFFIX)
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

all: mediaplayer$(EXEEXT) $(__mediaplayer_bundle___depname)

install: 

uninstall: 

install-strip: install

clean: 
	rm -rf ./.deps ./.pch
	rm -f ./*.o
	rm -f mediaplayer$(EXEEXT)
	rm -rf mediaplayer.app

distclean: clean
	rm -f config.cache config.log config.status bk-deps bk-make-pch shared-ld-sh Makefile

mediaplayer$(EXEEXT): $(MEDIAPLAYER_OBJECTS) $(__mediaplayer___win32rc)
	$(CXX) -o $@ $(MEDIAPLAYER_OBJECTS)    -L$(LIBDIRNAME) $(LDFLAGS_GUI) $(SAMPLES_RPATH_FLAG) $(LDFLAGS)  $(__WXLIB_MEDIA_p) $(EXTRALIBS_MEDIA) $(__WXLIB_CORE_p)  $(__WXLIB_BASE_p)  $(__WXLIB_MONO_p) $(__LIB_SCINTILLA_IF_MONO_p) $(__LIB_TIFF_p) $(__LIB_JPEG_p) $(__LIB_PNG_p)  $(EXTRALIBS_FOR_GUI) $(__LIB_ZLIB_p) $(__LIB_REGEX_p) $(__LIB_EXPAT_p) $(EXTRALIBS_FOR_BASE) $(LIBS)
	
	$(__mediaplayer___mac_setfilecmd)
	$(__mediaplayer___os2_emxbindcmd)

mediaplayer.app/Contents/PkgInfo: mediaplayer$(EXEEXT) $(top_srcdir)/src/osx/carbon/Info.plist.in $(top_srcdir)/src/osx/carbon/wxmac.icns
	mkdir -p mediaplayer.app/Contents
	mkdir -p mediaplayer.app/Contents/MacOS
	mkdir -p mediaplayer.app/Contents/Resources
	
	
	sed -e "s/IDENTIFIER/`echo $(srcdir) | sed -e 's,\.\./,,g' | sed -e 's,/,.,g'`/" \
	-e "s/EXECUTABLE/mediaplayer/" \
	-e "s/VERSION/$(WX_VERSION)/" \
	$(top_srcdir)/src/osx/carbon/Info.plist.in >mediaplayer.app/Contents/Info.plist
	
	
	/bin/echo "APPL????" >mediaplayer.app/Contents/PkgInfo
	
	
	ln -f mediaplayer$(EXEEXT) mediaplayer.app/Contents/MacOS/mediaplayer
	
	
	cp -f $(top_srcdir)/src/osx/carbon/wxmac.icns mediaplayer.app/Contents/Resources/wxmac.icns

mediaplayer_bundle: $(____mediaplayer_BUNDLE_TGT_REF_DEP)

mediaplayer_sample_rc.o: $(srcdir)/../../samples/sample.rc
	$(WINDRES) -i$< -o$@    --define __WX$(TOOLKIT)__ $(__WXUNIV_DEFINE_p_2) $(__DEBUG_DEFINE_p_2)  $(__EXCEPTIONS_DEFINE_p_2) $(__RTTI_DEFINE_p_2) $(__THREAD_DEFINE_p_2)   --include-dir $(srcdir) $(__DLLFLAG_p_2) --include-dir $(srcdir)/../../samples $(__RCDEFDIR_p_1) --include-dir $(top_srcdir)/include

mediaplayer_mediaplayer.o: $(srcdir)/mediaplayer.cpp
	$(CXXC) -c -o $@ $(MEDIAPLAYER_CXXFLAGS) $(srcdir)/mediaplayer.cpp


# Include dependency info, if present:
-include ./.deps/*.d

.PHONY: all install uninstall clean distclean mediaplayer_bundle
