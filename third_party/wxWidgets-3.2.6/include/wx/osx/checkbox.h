/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/checkbox.h
// Purpose:     wxCheckBox class
// Author:      Stefan Csomor
// Modified by:
// Created:     1998-01-01
// Copyright:   (c) Stefan Csomor
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_CHECKBOX_H_
#define _WX_CHECKBOX_H_

// Checkbox item (single checkbox)
class WXDLLIMPEXP_CORE wxCheckBox : public wxCheckBoxBase
{
public:
    wxCheckBox() { }
    wxCheckBox(wxWindow *parent, wxWindowID id, const wxString& label,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxCheckBoxNameStr))
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

    bool Create(wxWindow *parent, wxWindowID id, const wxString& label,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxCheckBoxNameStr));
    virtual void SetValue(bool) wxOVERRIDE;
    virtual bool GetValue() const wxOVERRIDE;

    virtual void Command(wxCommandEvent& event) wxOVERRIDE;

    // osx specific event handling common for all osx-ports

    virtual bool        OSXHandleClicked( double timestampsec ) wxOVERRIDE;
protected:
    void DoSet3StateValue(wxCheckBoxState val) wxOVERRIDE;
    virtual wxCheckBoxState DoGet3StateValue() const wxOVERRIDE;

    wxDECLARE_DYNAMIC_CLASS(wxCheckBox);
};

class WXDLLIMPEXP_FWD_CORE wxBitmap;
class WXDLLIMPEXP_CORE wxBitmapCheckBox: public wxCheckBox
{
public:
    int checkWidth;
    int checkHeight;

    wxBitmapCheckBox()
        : checkWidth(-1), checkHeight(-1)
        { }

    wxBitmapCheckBox(wxWindow *parent, wxWindowID id, const wxBitmap *label,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxCheckBoxNameStr))
    {
        Create(parent, id, label, pos, size, style, validator, name);
    }

    bool Create(wxWindow *parent, wxWindowID id, const wxBitmap *bitmap,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize, long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxASCII_STR(wxCheckBoxNameStr));
    virtual void SetValue(bool) wxOVERRIDE;
    virtual bool GetValue() const wxOVERRIDE;
    virtual void SetLabel(const wxBitmap *bitmap);
    virtual void SetLabel( const wxString & WXUNUSED(name) ) wxOVERRIDE {}

    wxDECLARE_DYNAMIC_CLASS(wxBitmapCheckBox);
};
#endif
    // _WX_CHECKBOX_H_
