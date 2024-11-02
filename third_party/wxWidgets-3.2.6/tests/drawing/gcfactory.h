///////////////////////////////////////////////////////////////////////////////
// Name:        tests/drawing/gcfactory.h
// Purpose:     Interface of a factory to create and destroy a wxGraphicsContext
//              and finally save the result of a rendering test
// Author:      Armel Asselin
// Created:     2014-02-26
// Copyright:   (c) 2014 Ellié Computing <opensource@elliecomputing.com>
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_TESTS_GCFACTORY_H_
#define _WX_TESTS_GCFACTORY_H_

// wxCairoRenderer::CreateMeasuringContext() is not implement for wxX11
#if wxUSE_GRAPHICS_CONTEXT && !defined(__WXX11__)
    #include "wx/graphics.h"
    #define wxUSE_TEST_GC_DRAWING 1
#else
    #define wxUSE_TEST_GC_DRAWING 0
#endif

#if wxUSE_TEST_GC_DRAWING

#include "wx/filename.h"

class WXDLLIMPEXP_FWD_CORE wxGraphicsContext;

// Implement a class derived from this one to let the drawing test create
//  graphics context, save them and destroy them.
class DrawingTestGCFactory
{
public:
    virtual ~DrawingTestGCFactory() {}

    // This identifier is used in the test and reference files to distinguish
    //  results produced by wxGraphicsContext generated by this factory
    //  use only lower case latin letters, '_' and '-'
    virtual wxString GetIdForFileName () const = 0;

    // This is the extension used when saving test and reference files
    virtual wxString GetExtensionForFileName () const = 0;

    // Returns true if a pixel per pixel comparison of rendered image should
    //  be used rather than a byte by byte comparison of files
    virtual bool UseImageComparison() const = 0;

    // Returns true this target is platform independent, rendering exactly
    //  the same result for whichever platform
    virtual bool PlatformIndependent() const = 0;

    // Builds a new context of @c expected_size, expecting to save it as
    //  @c target_file_name
    // NB: only one context is created as a time, so that the implementer
    //  can keep internal state if necessary
    virtual wxGraphicsContext *BuildNewContext (wxSize expectedSize,
        double pointsPerInch, const wxFileName &targetFileName) = 0;

    // Let's the opportunity to actually save the context and associated data
    // If saving requires deleting the wxGraphicsContext object the
    //  implementer is free to do it but @c gc must then be nulled
    virtual void SaveBuiltContext (wxGraphicsContext *&gc) = 0;

    // Cleans @c gc and internal data if any
    virtual void CleanUp (wxGraphicsContext *gc) = 0;
};

typedef DrawingTestGCFactory * (*CreateDrawingTestLifeCycleFunction)();
typedef void (*DestroyDrawingTestLifeCycleFunction) (DrawingTestGCFactory* lc);

#endif // wxUSE_TEST_GC_DRAWING

#endif // #ifndef _WX_TESTS_GCFACTORY_H_
