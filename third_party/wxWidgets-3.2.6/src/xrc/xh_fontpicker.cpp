/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_fontpicker.cpp
// Purpose:     XML resource handler for wxFontPickerCtrl
// Author:      Francesco Montorsi
// Created:     2006-04-17
// Copyright:   (c) 2006 Francesco Montorsi
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_XRC && wxUSE_FONTPICKERCTRL

#include "wx/xrc/xh_fontpicker.h"
#include "wx/fontpicker.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxFontPickerCtrlXmlHandler, wxXmlResourceHandler);

wxFontPickerCtrlXmlHandler::wxFontPickerCtrlXmlHandler() : wxXmlResourceHandler()
{
    XRC_ADD_STYLE(wxFNTP_USE_TEXTCTRL);
    XRC_ADD_STYLE(wxFNTP_FONTDESC_AS_LABEL);
    XRC_ADD_STYLE(wxFNTP_USEFONT_FOR_LABEL);
    XRC_ADD_STYLE(wxFNTP_DEFAULT_STYLE);
    AddWindowStyles();
}

wxObject *wxFontPickerCtrlXmlHandler::DoCreateResource()
{
   XRC_MAKE_INSTANCE(picker, wxFontPickerCtrl)

    wxFont f = *wxNORMAL_FONT;
    if (HasParam(wxT("value")))
        f = GetFont(wxT("value"));

   picker->Create(m_parentAsWindow,
                  GetID(),
                  f,
                  GetPosition(), GetSize(),
                  GetStyle(wxT("style"), wxFNTP_DEFAULT_STYLE),
                  wxDefaultValidator,
                  GetName());

    SetupWindow(picker);

    return picker;
}

bool wxFontPickerCtrlXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, wxT("wxFontPickerCtrl"));
}

#endif // wxUSE_XRC && wxUSE_FONTPICKERCTRL
