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
srcdir = ../../../samples/menu
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
MENU_CXXFLAGS = -D__WX$(TOOLKIT)__ $(__WXUNIV_DEFINE_p) $(__DEBUG_DEFINE_p) \
	$(__EXCEPTIONS_DEFINE_p) $(__RTTI_DEFINE_p) $(__THREAD_DEFINE_p) \
	-I$(srcdir) $(__DLLFLAG_p) -I$(srcdir)/../../samples $(CXXWARNINGS) \
	$(CPPFLAGS) $(CXXFLAGS)
MENU_OBJECTS =  \
	$(__menu___win32rc) \
	$(__menu_os2_lib_res) \
	menu_menu.o

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
#__menu___mac_setfilecmd = @true
__menu___mac_setfilecmd = \
	$(SETFILE) -t APPL menu$(EXEEXT)
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
COND_PLATFORM_OS2_1___menu___os2_emxbindcmd = $(NM) menu$(EXEEXT) | if grep -q \
	pmwin.763 ; then emxbind -ep menu$(EXEEXT) ; fi
#__menu___os2_emxbindcmd = $(COND_PLATFORM_OS2_1___menu___os2_emxbindcmd)
#__RCDEFDIR_p_1 = --include-dir \
#	$(LIBDIRNAME)/wx/include/$(TOOLCHAIN_FULLNAME)
#__menu___win32rc = menu_sample_rc.o
#__menu_os2_lib_res = \
#	$(top_srcdir)/include/wx/os2/wx.res
__menu_app_Contents_PkgInfo___depname \
	= menu.app/Contents/PkgInfo
__menu_bundle___depname = menu_bundle
#____menu_BUNDLE_TGT_REF_DEP = \
#	$(__menu_app_Contents_PkgInfo___depname)
#____menu_BUNDLE_TGT_REF_DEP \
#	= $(__menu_app_Contents_PkgInfo___depname)
____menu_BUNDLE_TGT_REF_DEP \
	= $(__menu_app_Contents_PkgInfo___depname)
#____menu_BUNDLE_TGT_REF_DEP \
#	= $(__menu_app_Contents_PkgInfo___depname)
#____menu_BUNDLE_TGT_REF_DEP = \
#	$(__menu_app_Contents_PkgInfo___depname)
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

all: menu$(EXEEXT) $(__menu_bundle___depname)

install: 

uninstall: 

install-strip: install

clean: 
	rm -rf ./.deps ./.pch
	rm -f ./*.o
	rm -f menu$(EXEEXT)
	rm -rf menu.app

distclean: clean
	rm -f config.cache config.log config.status bk-deps bk-make-pch shared-ld-sh Makefile

menu$(EXEEXT): $(MENU_OBJECTS) $(__menu___win32rc)
	$(CXX) -o $@ $(MENU_OBJECTS)    -L$(LIBDIRNAME) $(LDFLAGS_GUI) $(SAMPLES_RPATH_FLAG) $(LDFLAGS)  $(__WXLIB_CORE_p)  $(__WXLIB_BASE_p)  $(__WXLIB_MONO_p) $(__LIB_SCINTILLA_IF_MONO_p) $(__LIB_TIFF_p) $(__LIB_JPEG_p) $(__LIB_PNG_p)  $(EXTRALIBS_FOR_GUI) $(__LIB_ZLIB_p) $(__LIB_REGEX_p) $(__LIB_EXPAT_p) $(EXTRALIBS_FOR_BASE) $(LIBS)
	
	$(__menu___mac_setfilecmd)
	$(__menu___os2_emxbindcmd)

menu.app/Contents/PkgInfo: menu$(EXEEXT) $(top_srcdir)/src/osx/carbon/Info.plist.in $(top_srcdir)/src/osx/carbon/wxmac.icns
	mkdir -p menu.app/Contents
	mkdir -p menu.app/Contents/MacOS
	mkdir -p menu.app/Contents/Resources
	
	
	sed -e "s/IDENTIFIER/`echo $(srcdir) | sed -e 's,\.\./,,g' | sed -e 's,/,.,g'`/" \
	-e "s/EXECUTABLE/menu/" \
	-e "s/VERSION/$(WX_VERSION)/" \
	$(top_srcdir)/src/osx/carbon/Info.plist.in >menu.app/Contents/Info.plist
	
	
	/bin/echo "APPL????" >menu.app/Contents/PkgInfo
	
	
	ln -f menu$(EXEEXT) menu.app/Contents/MacOS/menu
	
	
	cp -f $(top_srcdir)/src/osx/carbon/wxmac.icns menu.app/Contents/Resources/wxmac.icns

menu_bundle: $(____menu_BUNDLE_TGT_REF_DEP)

menu_sample_rc.o: $(srcdir)/../../samples/sample.rc
	$(WINDRES) -i$< -o$@    --define __WX$(TOOLKIT)__ $(__WXUNIV_DEFINE_p_2) $(__DEBUG_DEFINE_p_2)  $(__EXCEPTIONS_DEFINE_p_2) $(__RTTI_DEFINE_p_2) $(__THREAD_DEFINE_p_2)   --include-dir $(srcdir) $(__DLLFLAG_p_2) --include-dir $(srcdir)/../../samples $(__RCDEFDIR_p_1) --include-dir $(top_srcdir)/include

menu_menu.o: $(srcdir)/menu.cpp
	$(CXXC) -c -o $@ $(MENU_CXXFLAGS) $(srcdir)/menu.cpp


# Include dependency info, if present:
-include ./.deps/*.d

.PHONY: all install uninstall clean distclean menu_bundle
