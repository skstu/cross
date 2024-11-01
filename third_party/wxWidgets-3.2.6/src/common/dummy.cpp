/*
 * File:        src/common/dummy.cpp
 * Purpose:     See below
 * Author:      Julian Smart
 * Created:     1993
 * Updated:
 * Copyright:   (c) 1993, AIAI, University of Edinburgh
 */

/* A dummy file to include wx.h. If precompiling wx.h,
 * always start by compiling this and producing the PCH file.
 * Then subsequent source files use the PCH file.
 *
 * If precompiling wx.h for wxWidgets and derived apps,
 * link dummy.obj with your program.
 *
 * This will produce a big PCH file.
 */

#include "wx/wxprec.h"


#ifdef __WINDOWS__
    #include "wx/msw/msvcrt.h"
#endif

#ifdef WXMAKINGDLL_ADV

// This is a hack required to create an import library for the "adv" DLL which
// doesn't contain anything any more and is preserved only for compatibility.
extern WXEXPORT void wxAdvLibraryIsObsoleteDontUse()
{
}

#endif // WXMAKINGDLL_ADV
