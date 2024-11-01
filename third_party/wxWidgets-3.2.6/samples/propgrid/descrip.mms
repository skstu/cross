#*****************************************************************************
#                                                                            *
# Make file for VMS                                                          *
# Author : J.Jansen (joukj@hrem.nano.tudelft.nl)                             *
# Date : 22 September 2009                                                   *
#                                                                            *
#*****************************************************************************
.first
	define wx [--.include.wx]

.ifdef __WXMOTIF__
CXX_DEFINE = /define=(__WXMOTIF__=1)/name=(as_is,short)\
	   /assume=(nostdnew,noglobal_array_new)
.else
.ifdef __WXGTK__
CXX_DEFINE = /define=(__WXGTK__=1)/float=ieee/name=(as_is,short)/ieee=denorm\
	   /assume=(nostdnew,noglobal_array_new)
.else
.ifdef __WXGTK2__
CXX_DEFINE = /define=(__WXGTK__=1,VMS_GTK2=1)/float=ieee/name=(as_is,short)/ieee=denorm\
	   /assume=(nostdnew,noglobal_array_new)
.else
.ifdef __WXX11__
CXX_DEFINE = /define=(__WXX11__=1,__WXUNIVERSAL__==1)/float=ieee\
	/name=(as_is,short)/assume=(nostdnew,noglobal_array_new)
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
	$(MMS)$(MMSQUALIFIERS) propgrid.exe
.else
.ifdef __WXGTK__
	$(MMS)$(MMSQUALIFIERS) propgrid_gtk.exe
.else
.ifdef __WXGTK2__
	$(MMS)$(MMSQUALIFIERS) propgrid_gtk2.exe
.else
.ifdef __WXX11__
	$(MMS)$(MMSQUALIFIERS) propgrid_x11.exe
.endif
.endif
.endif
.endif

OBJS=propgrid.obj,propgrid_minimal.obj,sampleprops.obj,tests.obj

.ifdef __WXMOTIF__
propgrid.exe : $(OBJS)
	cxxlink $(OBJS),[--.lib]vms/opt
.else
.ifdef __WXGTK__
propgrid_gtk.exe : $(OBJS)
	cxxlink/exec=propgrid_gtk.exe $(OBJS),[--.lib]vms_gtk/opt
.else
.ifdef __WXGTK2__
propgrid_gtk2.exe : $(OBJS)
	cxxlink/exec=propgrid_gtk2.exe $(OBJS),[--.lib]vms_gtk2/opt
.else
.ifdef __WXX11__
propgrid_x11.exe : $(OBJS)
	cxxlink/exec=propgrid_x11.exe $(OBJS),[--.lib]vms_x11_univ/opt
.endif
.endif
.endif
.endif

propgrid.obj : propgrid.cpp
propgrid_minimal.obj : propgrid_minimal.cpp
sampleprops.obj : sampleprops.cpp
tests.obj : tests.cpp
