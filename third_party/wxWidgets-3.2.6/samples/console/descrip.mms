#*****************************************************************************
#                                                                            *
# Make file for VMS                                                          *
# Author : J.Jansen (joukj@hrem.nano.tudelft.nl)                             *
# Date : 6 October 2009                                                      *
#                                                                            *
#*****************************************************************************
.first
	define wx [--.include.wx]

.ifdef __WXMOTIF__
CXX_DEFINE = /define=(__WXMOTIF__=1)/name=(as_is,short)\
	   /assume=(nostdnew,noglobal_array_new)/incl=([],[-])
.else
.ifdef __WXGTK__
CXX_DEFINE = /define=(__WXGTK__=1)/float=ieee/name=(as_is,short)/ieee=denorm\
	   /assume=(nostdnew,noglobal_array_new)/incl=([],[-])
.else
.ifdef __WXGTK2__
CXX_DEFINE = /define=(__WXGTK__=1,VMS_GTK2=1)/float=ieee/name=(as_is,short)/ieee=denorm\
	   /assume=(nostdnew,noglobal_array_new)/incl=([],[-])
.else
.ifdef __WXX11__
CXX_DEFINE = /define=(__WXX11__=1,__WXUNIVERSAL__==1)/float=ieee\
	/name=(as_is,short)/assume=(nostdnew,noglobal_array_new)/incl=([],[-])
.else
CXX_DEFINE =
.endif
.endif
.endif
.endif

.suffixes : .cpp

.cpp.obj :
	cxx $(CXXFLAGS)$(CXX_DEFINE) $(MMS$TARGET_NAME).cpp

all :
.ifdef __WXMOTIF__
	$(MMS)$(MMSQUALIFIERS) console.exe
.else
.ifdef __WXGTK__
	$(MMS)$(MMSQUALIFIERS) console_gtk.exe
.else
.ifdef __WXGTK2__
	$(MMS)$(MMSQUALIFIERS) console_gtk2.exe
.else
.ifdef __WXX11__
	$(MMS)$(MMSQUALIFIERS) console_x11.exe
.endif
.endif
.endif
.endif

OBJS=console.obj

.ifdef __WXMOTIF__
console.exe : $(OBJS)
	cxxlink $(OBJS),[--.lib]vms/opt
.else
.ifdef __WXGTK__
console_gtk.exe : $(OBJS)
	cxxlink/exec=console_gtk.exe $(OBJS),[--.lib]vms_gtk/opt
.else
.ifdef __WXGTK2__
console_gtk2.exe : $(OBJS)
	cxxlink/exec=console_gtk2.exe $(OBJS),[--.lib]vms_gtk2/opt
.else
.ifdef __WXX11__
console_x11.exe : $(OBJS)
	cxxlink/exec=console_x11.exe $(OBJS),[--.lib]vms_x11_univ/opt
.endif
.endif
.endif
.endif

console.obj : console.cpp
	cxx $(CXXFLAGS)$(CXX_DEFINE)/nowarn console.cpp
