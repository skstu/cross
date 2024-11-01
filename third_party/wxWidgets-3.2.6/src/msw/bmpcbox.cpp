/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/bmpcbox.cpp
// Purpose:     wxBitmapComboBox
// Author:      Jaakko Salli
// Created:     2008-04-06
// Copyright:   (c) 2008 Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wx/wxprec.h"


#if wxUSE_BITMAPCOMBOBOX

#include "wx/bmpcbox.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
#endif

#include "wx/settings.h"
#include "wx/vector.h"

#include "wx/msw/dcclient.h"
#include "wx/msw/private.h"

// For wxODCB_XXX flags
#include "wx/odcombo.h"



// ============================================================================
// implementation
// ============================================================================


wxBEGIN_EVENT_TABLE(wxBitmapComboBox, wxComboBox)
    EVT_SIZE(wxBitmapComboBox::OnSize)
wxEND_EVENT_TABLE()


wxIMPLEMENT_DYNAMIC_CLASS(wxBitmapComboBox, wxComboBox);


// ----------------------------------------------------------------------------
// wxBitmapComboBox creation
// ----------------------------------------------------------------------------

void wxBitmapComboBox::Init()
{
    m_inResize = false;
}

wxBitmapComboBox::wxBitmapComboBox(wxWindow *parent,
                                  wxWindowID id,
                                  const wxString& value,
                                  const wxPoint& pos,
                                  const wxSize& size,
                                  const wxArrayString& choices,
                                  long style,
                                  const wxValidator& validator,
                                  const wxString& name)
    : wxComboBox(),
      wxBitmapComboBoxBase()
{
    Init();

    Create(parent,id,value,pos,size,choices,style,validator,name);
}

bool wxBitmapComboBox::Create(wxWindow *parent,
                              wxWindowID id,
                              const wxString& value,
                              const wxPoint& pos,
                              const wxSize& size,
                              const wxArrayString& choices,
                              long style,
                              const wxValidator& validator,
                              const wxString& name)
{
    wxCArrayString chs(choices);
    return Create(parent, id, value, pos, size, chs.GetCount(),
                  chs.GetStrings(), style, validator, name);
}

bool wxBitmapComboBox::Create(wxWindow *parent,
                              wxWindowID id,
                              const wxString& value,
                              const wxPoint& pos,
                              const wxSize& size,
                              int n,
                              const wxString choices[],
                              long style,
                              const wxValidator& validator,
                              const wxString& name)
{
    if ( !wxComboBox::Create(parent, id, value, pos, size,
                             n, choices, style, validator, name) )
        return false;

    UpdateInternals();

    return true;
}

WXDWORD wxBitmapComboBox::MSWGetStyle(long style, WXDWORD *exstyle) const
{
    return wxComboBox::MSWGetStyle(style, exstyle) | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS;
}

void wxBitmapComboBox::RecreateControl()
{
    //
    // Recreate control so that WM_MEASUREITEM gets called again.
    // Can't use CBS_OWNERDRAWVARIABLE because it has odd
    // mouse-wheel behaviour.
    //
    MSWRecreate();

    ::SendMessage(GetHwnd(), CB_SETITEMHEIGHT, 0, MeasureItem(0));
}

wxBitmapComboBox::~wxBitmapComboBox()
{
    Clear();
}

wxSize wxBitmapComboBox::DoGetBestSize() const
{
    wxSize best = wxComboBox::DoGetBestSize();
    wxSize bitmapSize = GetBitmapSize();

    wxCoord useHeightBitmap = EDIT_HEIGHT_FROM_CHAR_HEIGHT(bitmapSize.y);
    if ( best.y < useHeightBitmap )
        best.y = useHeightBitmap;
    return best;
}

// ----------------------------------------------------------------------------
// Item manipulation
// ----------------------------------------------------------------------------

void wxBitmapComboBox::SetItemBitmap(unsigned int n, const wxBitmapBundle& bitmap)
{
    OnAddBitmap(bitmap);
    DoSetItemBitmap(n, bitmap);

    if ( (int)n == GetSelection() )
        Refresh();
}

int wxBitmapComboBox::Append(const wxString& item, const wxBitmapBundle& bitmap)
{
    OnAddBitmap(bitmap);
    const int n = wxComboBox::Append(item);
    if ( n != wxNOT_FOUND )
        DoSetItemBitmap(n, bitmap);
    return n;
}

int wxBitmapComboBox::Append(const wxString& item, const wxBitmapBundle& bitmap,
                             void *clientData)
{
    OnAddBitmap(bitmap);
    const int n = wxComboBox::Append(item, clientData);
    if ( n != wxNOT_FOUND )
        DoSetItemBitmap(n, bitmap);
    return n;
}

int wxBitmapComboBox::Append(const wxString& item, const wxBitmapBundle& bitmap,
                             wxClientData *clientData)
{
    OnAddBitmap(bitmap);
    const int n = wxComboBox::Append(item, clientData);
    if ( n != wxNOT_FOUND )
        DoSetItemBitmap(n, bitmap);
    return n;
}

int wxBitmapComboBox::Insert(const wxString& item,
                             const wxBitmapBundle& bitmap,
                             unsigned int pos)
{
    OnAddBitmap(bitmap);
    const int n = wxComboBox::Insert(item, pos);
    if ( n != wxNOT_FOUND )
        DoSetItemBitmap(n, bitmap);
    return n;
}

int wxBitmapComboBox::Insert(const wxString& item, const wxBitmapBundle& bitmap,
                             unsigned int pos, void *clientData)
{
    OnAddBitmap(bitmap);
    const int n = wxComboBox::Insert(item, pos, clientData);
    if ( n != wxNOT_FOUND )
        DoSetItemBitmap(n, bitmap);
    return n;
}

int wxBitmapComboBox::Insert(const wxString& item, const wxBitmapBundle& bitmap,
                             unsigned int pos, wxClientData *clientData)
{
    OnAddBitmap(bitmap);
    const int n = wxComboBox::Insert(item, pos, clientData);
    if ( n != wxNOT_FOUND )
        DoSetItemBitmap(n, bitmap);
    return n;
}

int wxBitmapComboBox::DoInsertItems(const wxArrayStringsAdapter & items,
                                    unsigned int pos,
                                    void **clientData, wxClientDataType type)
{
    const unsigned int numItems = items.GetCount();

    int index;
    if ( HasFlag(wxCB_SORT) )
    {
        // Since we don't know at what positions new elements will be actually inserted
        // we need to add them one by one, check for each one the position it was added at
        // and reserve the slot for corresponding bitmap at the same postion in the bitmap array.
        index = pos;
        for ( unsigned int i = 0; i < numItems; i++ )
        {
            if ( clientData )
                index = wxComboBox::DoInsertItems(items[i], pos+i, clientData+i, type);
            else
                index = wxComboBox::DoInsertItems(items[i], pos+i, NULL, wxClientData_None);

            wxASSERT_MSG( index != wxNOT_FOUND, wxS("Invalid wxBitmapComboBox state") );
            if ( index == wxNOT_FOUND )
            {
                continue;
            }

            // Update the bitmap array.
            if ( GetCount() > m_bitmapbundles.size() )
            {
                wxASSERT_MSG( GetCount() == m_bitmapbundles.size() + 1,
                              wxS("Invalid wxBitmapComboBox state") );
                // Control is in the normal state.
                // New item has been just added.
                // Insert bitmap at the given index into the array.
                wxASSERT_MSG( (size_t)index <= m_bitmapbundles.size(),
                              wxS("wxBitmapComboBox item index out of bound") );
                m_bitmapbundles.insert(m_bitmapbundles.begin() + index, wxBitmapBundle());
            }
            else
            {
                // No. of items after insertion <= No. bitmaps:
                // (This can happen if control is e.g. recreated with RecreateControl).
                // In this case existing bitmaps are reused.
                // Required and actual indices should be the same to assure
                // consistency between list of items and bitmap array.
                wxASSERT_MSG( (size_t)index < m_bitmapbundles.size(),
                              wxS("wxBitmapComboBox item index out of bound") );
                wxASSERT_MSG( (unsigned int)index == pos+i,
                              wxS("Invalid index for wxBitmapComboBox item") );
            }
        }
    }
    else
    {
        if ( GetCount() == m_bitmapbundles.size() )
        {
            // Control is in the normal state.
            // Just insert new bitmaps into the array.
            const unsigned int countNew = GetCount() + numItems;
            m_bitmapbundles.reserve(countNew);

            for ( unsigned int i = 0; i < numItems; i++ )
            {
                m_bitmapbundles.insert(m_bitmapbundles.begin() + pos + i, wxBitmapBundle());
            }
        }
        else
        {
            wxASSERT_MSG( GetCount() < m_bitmapbundles.size(),
                          wxS("Invalid wxBitmapComboBox state") );
            // There are less items then bitmaps.
            // (This can happen if control is e.g. recreated with RecreateControl).
            // In this case existing bitmaps are reused.
            // The whole block of inserted items should be within the range
            // of indices of the existing bitmap array.
            wxASSERT_MSG( pos + numItems <= m_bitmapbundles.size(),
                      wxS("wxBitmapComboBox item index out of bound") );
        }

        index = wxComboBox::DoInsertItems(items, pos,
                                          clientData, type);
        // This returns index of the last item in the inserted block.

        if ( index == wxNOT_FOUND )
        {
            for ( int i = numItems-1; i >= 0; i-- )
                BCBDoDeleteOneItem(pos + i);
        }
        else
        {
            // Index of the last inserted item should be consistent
            // with required position and number of items.
            wxASSERT_MSG( (unsigned int)index == pos+numItems-1,
                           wxS("Invalid index for wxBitmapComboBox item") );
        }
    }

    return index;
}

bool wxBitmapComboBox::OnAddBitmap(const wxBitmapBundle& bitmap)
{
    if ( wxBitmapComboBoxBase::OnAddBitmap(bitmap) || !GetCount() )
    {
        // Need to recreate control for a new measureitem call?
        int prevItemHeight = ::SendMessage(GetHwnd(), CB_GETITEMHEIGHT, 0, 0);

        if ( prevItemHeight != MeasureItem(0) )
            RecreateControl();

        return true;
    }

    return false;
}

void wxBitmapComboBox::DoClear()
{
    wxComboBox::DoClear();
    wxBitmapComboBoxBase::BCBDoClear();
}

void wxBitmapComboBox::DoDeleteOneItem(unsigned int n)
{
    wxComboBox::DoDeleteOneItem(n);
    wxBitmapComboBoxBase::BCBDoDeleteOneItem(n);
}

// ----------------------------------------------------------------------------
// wxBitmapComboBox event handlers and such
// ----------------------------------------------------------------------------

void wxBitmapComboBox::OnSize(wxSizeEvent& event)
{
    // Prevent infinite looping
    if ( !m_inResize )
    {
        m_inResize = true;
        DetermineIndent();
        m_inResize = false;
    }

    event.Skip();
}

// ----------------------------------------------------------------------------
// wxBitmapComboBox miscellaneous
// ----------------------------------------------------------------------------

bool wxBitmapComboBox::SetFont(const wxFont& font)
{
    bool res = wxComboBox::SetFont(font);
    UpdateInternals();
    return res;
}

// ----------------------------------------------------------------------------
// wxBitmapComboBox item drawing and measuring
// ----------------------------------------------------------------------------

bool wxBitmapComboBox::MSWOnDraw(WXDRAWITEMSTRUCT *item)
{
    LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT) item;
    int pos = lpDrawItem->itemID;

    // Draw default for item -1, which means 'focus rect only'
    if ( pos == -1 )
        return false;

    int flags = 0;
    if ( lpDrawItem->itemState & ODS_COMBOBOXEDIT )
        flags |= wxODCB_PAINTING_CONTROL;
    if ( lpDrawItem->itemState & ODS_SELECTED )
        flags |= wxODCB_PAINTING_SELECTED;

    wxPaintDCEx dc(this, lpDrawItem->hDC);
    wxRect rect = wxRectFromRECT(lpDrawItem->rcItem);
    wxBitmapComboBoxBase::DrawBackground(dc, rect, pos, flags);

    wxString text;

    if ( flags & wxODCB_PAINTING_CONTROL )
    {
        // Don't draw anything in the editable selection field.
        if ( !HasFlag(wxCB_READONLY) )
            return true;

        pos = GetSelection();
        // Skip drawing if there is nothing selected.
        if ( pos < 0 )
            return true;

        text = GetValue();
    }
    else
    {
        text = GetString(pos);
    }

    wxBitmapComboBoxBase::DrawItem(dc, rect, pos, text, flags);

    // If the item has the focus, draw focus rectangle.
    // Commented out since regular combo box doesn't
    // seem to do it either.
    //if ( lpDrawItem->itemState & ODS_FOCUS )
    //    DrawFocusRect(lpDrawItem->hDC, &lpDrawItem->rcItem);

    return true;
}

bool wxBitmapComboBox::MSWOnMeasure(WXMEASUREITEMSTRUCT *item)
{
    LPMEASUREITEMSTRUCT lpMeasureItem = (LPMEASUREITEMSTRUCT) item;
    int pos = lpMeasureItem->itemID;

    // Measure edit field height if item list is not empty,
    // otherwise leave default system value.
    if ( m_usedImgSize.y >= 0 || pos >= 0 )
    {
        lpMeasureItem->itemHeight = wxBitmapComboBoxBase::MeasureItem(pos);
    }

    return true;
}

void wxBitmapComboBox::MSWUpdateFontOnDPIChange(const wxSize& newDPI)
{
    wxComboBox::MSWUpdateFontOnDPIChange(newDPI);

    UpdateInternals();

    RecreateControl();
}

#endif // wxUSE_BITMAPCOMBOBOX
