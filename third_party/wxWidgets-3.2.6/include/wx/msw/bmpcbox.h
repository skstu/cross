/////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/bmpcbox.h
// Purpose:     wxBitmapComboBox
// Author:      Jaakko Salli
// Created:     2008-04-06
// Copyright:   (c) 2008 Jaakko Salli
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_BMPCBOX_H_
#define _WX_MSW_BMPCBOX_H_


#include "wx/combobox.h"


// ----------------------------------------------------------------------------
// wxBitmapComboBox: a wxComboBox that allows images to be shown
// in front of string items.
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_ADV wxBitmapComboBox : public wxComboBox,
                                         public wxBitmapComboBoxBase
{
public:
    // ctors and such
    wxBitmapComboBox() : wxComboBox(), wxBitmapComboBoxBase()
    {
        Init();
    }

    wxBitmapComboBox(wxWindow *parent,
                     wxWindowID id = wxID_ANY,
                     const wxString& value = wxEmptyString,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     int n = 0,
                     const wxString choices[] = NULL,
                     long style = 0,
                     const wxValidator& validator = wxDefaultValidator,
                     const wxString& name = wxASCII_STR(wxBitmapComboBoxNameStr))
        : wxComboBox(),
          wxBitmapComboBoxBase()
    {
        Init();

        (void)Create(parent, id, value, pos, size, n,
                     choices, style, validator, name);
    }

    wxBitmapComboBox(wxWindow *parent,
                     wxWindowID id,
                     const wxString& value,
                     const wxPoint& pos,
                     const wxSize& size,
                     const wxArrayString& choices,
                     long style,
                     const wxValidator& validator = wxDefaultValidator,
                     const wxString& name = wxASCII_STR(wxBitmapComboBoxNameStr));

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& value,
                const wxPoint& pos,
                const wxSize& size,
                int n,
                const wxString choices[],
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxBitmapComboBoxNameStr));

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxString& value,
                const wxPoint& pos,
                const wxSize& size,
                const wxArrayString& choices,
                long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = wxASCII_STR(wxBitmapComboBoxNameStr));

    virtual ~wxBitmapComboBox();

    // Sets the image for the given item.
    virtual void SetItemBitmap(unsigned int n, const wxBitmapBundle& bitmap) wxOVERRIDE;

    virtual bool SetFont(const wxFont& font) wxOVERRIDE;

    // Adds item with image to the end of the combo box.
    int Append(const wxString& item, const wxBitmapBundle& bitmap = wxBitmapBundle());
    int Append(const wxString& item, const wxBitmapBundle& bitmap, void *clientData);
    int Append(const wxString& item, const wxBitmapBundle& bitmap, wxClientData *clientData);

    // Inserts item with image into the list before pos. Not valid for wxCB_SORT
    // styles, use Append instead.
    int Insert(const wxString& item, const wxBitmapBundle& bitmap, unsigned int pos);
    int Insert(const wxString& item, const wxBitmapBundle& bitmap,
               unsigned int pos, void *clientData);
    int Insert(const wxString& item, const wxBitmapBundle& bitmap,
               unsigned int pos, wxClientData *clientData);

protected:

    WXDWORD MSWGetStyle(long style, WXDWORD *exstyle) const wxOVERRIDE;
    virtual bool MSWOnDraw(WXDRAWITEMSTRUCT *item) wxOVERRIDE;
    virtual bool MSWOnMeasure(WXMEASUREITEMSTRUCT *item) wxOVERRIDE;
    virtual void MSWUpdateFontOnDPIChange(const wxSize& newDPI) wxOVERRIDE;

    // Event handlers
    void OnSize(wxSizeEvent& event);

    virtual wxItemContainer* GetItemContainer() wxOVERRIDE { return this; }
    virtual wxWindow* GetControl() wxOVERRIDE { return this; }

    // wxItemContainer implementation
    virtual int DoInsertItems(const wxArrayStringsAdapter & items,
                              unsigned int pos,
                              void **clientData, wxClientDataType type) wxOVERRIDE;
    virtual void DoClear() wxOVERRIDE;
    virtual void DoDeleteOneItem(unsigned int n) wxOVERRIDE;

    virtual bool OnAddBitmap(const wxBitmapBundle& bitmap) wxOVERRIDE;
    virtual wxSize DoGetBestSize() const wxOVERRIDE;
    void RecreateControl();

private:
    void Init();

    bool m_inResize;

    wxDECLARE_EVENT_TABLE();

    wxDECLARE_DYNAMIC_CLASS(wxBitmapComboBox);
};

#endif // _WX_MSW_BMPCBOX_H_
