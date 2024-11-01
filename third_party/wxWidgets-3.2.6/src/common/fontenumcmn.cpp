/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/fontenumcmn.cpp
// Purpose:     wxFontEnumerator class
// Author:      Vadim Zeitlin
// Modified by:
// Created:     7/5/2006
// Copyright:   (c) 1999-2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_FONTENUM

#include "wx/fontenum.h"
#include "wx/module.h"

namespace
{

// Cached result of GetFacenames().
wxArrayString gs_allFacenames;

// Module used to ensure the cache is cleared on library shutdown and so is not
// reused if it re-initialized again later.
class wxFontEnumCacheCleanupModule : public wxModule
{
public:
    wxFontEnumCacheCleanupModule() { }

    bool OnInit() wxOVERRIDE { return true; }
    void OnExit() wxOVERRIDE { gs_allFacenames.clear(); }

private:
    wxDECLARE_DYNAMIC_CLASS(wxFontEnumCacheCleanupModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxFontEnumCacheCleanupModule, wxModule);

} // anonymous namespace

// ============================================================================
// implementation
// ============================================================================

// A simple wxFontEnumerator which doesn't perform any filtering and
// just returns all facenames and encodings found in the system
class wxSimpleFontEnumerator : public wxFontEnumerator
{
public:
    wxSimpleFontEnumerator() { }

    // called by EnumerateFacenames
    virtual bool OnFacename(const wxString& facename) wxOVERRIDE
    {
        m_arrFacenames.Add(facename);
        return true;
    }

    // called by EnumerateEncodings
    virtual bool OnFontEncoding(const wxString& WXUNUSED(facename),
                                const wxString& encoding) wxOVERRIDE
    {
        m_arrEncodings.Add(encoding);
        return true;
    }

public:
    wxArrayString m_arrFacenames, m_arrEncodings;
};


/* static */
wxArrayString wxFontEnumerator::GetFacenames(wxFontEncoding encoding, bool fixedWidthOnly)
{
    wxSimpleFontEnumerator temp;
    temp.EnumerateFacenames(encoding, fixedWidthOnly);
    return temp.m_arrFacenames;
}

/* static */
wxArrayString wxFontEnumerator::GetEncodings(const wxString& facename)
{
    wxSimpleFontEnumerator temp;
    temp.EnumerateEncodings(facename);
    return temp.m_arrEncodings;
}

/* static */
bool wxFontEnumerator::IsValidFacename(const wxString &facename)
{
    // we cache the result of wxFontEnumerator::GetFacenames supposing that
    // the array of face names won't change in the session of this program
    if ( gs_allFacenames.empty() )
        gs_allFacenames = wxFontEnumerator::GetFacenames();

#ifdef __WXMSW__
    // Quoting the MSDN:
    //     "MS Shell Dlg is a mapping mechanism that enables
    //     U.S. English Microsoft Windows NT, and Microsoft Windows 2000 to
    //     support locales that have characters that are not contained in code
    //     page 1252. It is not a font but a face name for a nonexistent font."
    // Thus we need to consider "Ms Shell Dlg" and "Ms Shell Dlg 2" as valid
    // font face names even if they are not enumerated by wxFontEnumerator
    if (facename.IsSameAs(wxT("Ms Shell Dlg"), false) ||
        facename.IsSameAs(wxT("Ms Shell Dlg 2"), false))
        return true;
#endif

    // is given font face name a valid one ?
    if (gs_allFacenames.Index(facename, false) == wxNOT_FOUND)
        return false;

    return true;
}

/* static */
void wxFontEnumerator::InvalidateCache()
{
    gs_allFacenames.clear();
}

#ifdef wxHAS_UTF8_FONTS
bool wxFontEnumerator::EnumerateEncodingsUTF8(const wxString& facename)
{
    // name of UTF-8 encoding: no need to use wxFontMapper for it as it's
    // unlikely to change
    const wxString utf8(wxT("UTF-8"));

    // all fonts are in UTF-8 only if this code is used
    if ( !facename.empty() )
    {
        OnFontEncoding(facename, utf8);
        return true;
    }

    // so enumerating all facenames supporting this encoding is the same as
    // enumerating all facenames
    const wxArrayString facenames(GetFacenames(wxFONTENCODING_UTF8));
    const size_t count = facenames.size();
    if ( !count )
        return false;

    for ( size_t n = 0; n < count; n++ )
    {
        if ( !OnFontEncoding(facenames[n], utf8) )
            break;
    }

    return true;
}
#endif // wxHAS_UTF8_FONTS

#endif // wxUSE_FONTENUM
