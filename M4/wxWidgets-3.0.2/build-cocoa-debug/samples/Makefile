#
# Makefile : Builds wxWindows samples for Unix.
#

SAMPLES_SUBDIRS=animate artprov aui calendar caret clipboard collpane combo config controls dataview debugrpt dialogs display dnd docview dragimag drawing erase event except exec font fswatcher grid help htlbox html html/about html/help html/helpview html/htmlctrl html/printing html/test html/virtual html/widget html/zip image internat ipc joytest keyboard layout listctrl mdi mediaplayer menu minimal notebook opengl opengl/cube opengl/isosurf opengl/penguin popup prefseditor printing propgrid render ribbon richtext sashtest scroll shaped sockets sound splash splitter statbar stc svg taborder taskbar text thread toolbar treectrl treelist typetest uiaction validate vscroll webview widgets wizard wrapsizer xrc 

all:
	@for d in $(SAMPLES_SUBDIRS); do (cd $$d && $(MAKE)); done

clean:
	@for d in $(SAMPLES_SUBDIRS); do (cd $$d && $(MAKE) clean); done

distclean:
	@for d in $(SAMPLES_SUBDIRS); do (cd $$d && $(MAKE) distclean); done

.PHONY: all clean distclean
