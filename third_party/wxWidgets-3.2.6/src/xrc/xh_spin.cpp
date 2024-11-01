/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_spin.cpp
// Purpose:     XRC resource for wxSpinButton
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_XRC

#include "wx/xrc/xh_spin.h"

#if wxUSE_SPINBTN

#include "wx/spinbutt.h"

static const long DEFAULT_VALUE = 0;
static const long DEFAULT_MIN = 0;
static const long DEFAULT_MAX = 100;
static const int DEFAULT_INCREMENT = 1;
static const int DEFAULT_DIGITS = 0;

wxIMPLEMENT_DYNAMIC_CLASS(wxSpinButtonXmlHandler, wxXmlResourceHandler);

wxSpinButtonXmlHandler::wxSpinButtonXmlHandler()
: wxXmlResourceHandler()
{
    XRC_ADD_STYLE(wxSP_HORIZONTAL);
    XRC_ADD_STYLE(wxSP_VERTICAL);
    XRC_ADD_STYLE(wxSP_ARROW_KEYS);
    XRC_ADD_STYLE(wxSP_WRAP);
    AddWindowStyles();
}

wxObject *wxSpinButtonXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(control, wxSpinButton)

    control->Create(m_parentAsWindow,
                    GetID(),
                    GetPosition(), GetSize(),
                    GetStyle(wxT("style"), wxSP_ARROW_KEYS),
                    GetName());

    control->SetValue(GetLong( wxT("value"), DEFAULT_VALUE));
    control->SetRange(GetLong( wxT("min"), DEFAULT_MIN),
                      GetLong(wxT("max"), DEFAULT_MAX));
    control->SetValue(GetLong(wxT( "inc" ), DEFAULT_INCREMENT));
    SetupWindow(control);

    return control;
}

bool wxSpinButtonXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, wxT("wxSpinButton"));
}

#endif // wxUSE_SPINBTN

#if wxUSE_SPINCTRL

#include "wx/spinctrl.h"

static const float DEFAULT_INC = 1;

static void AddSpinCtrlStyles(wxXmlResourceHandler& handler)
{
    handler.XRC_ADD_STYLE(wxSP_HORIZONTAL);
    handler.XRC_ADD_STYLE(wxSP_VERTICAL);
    handler.XRC_ADD_STYLE(wxSP_ARROW_KEYS);
    handler.XRC_ADD_STYLE(wxSP_WRAP);
    handler.XRC_ADD_STYLE(wxALIGN_LEFT);
    handler.XRC_ADD_STYLE(wxALIGN_CENTER);
    handler.XRC_ADD_STYLE(wxALIGN_RIGHT);
    handler.XRC_ADD_STYLE(wxTE_PROCESS_ENTER);
}

wxIMPLEMENT_DYNAMIC_CLASS(wxSpinCtrlXmlHandler, wxXmlResourceHandler);

wxSpinCtrlXmlHandler::wxSpinCtrlXmlHandler()
    : wxXmlResourceHandler()
{
    AddSpinCtrlStyles(*this);
}

wxObject *wxSpinCtrlXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(control, wxSpinCtrl)

    control->Create(m_parentAsWindow,
                    GetID(),
                    GetText(wxT("value")),
                    GetPosition(), GetSize(),
                    GetStyle(wxT("style"), wxSP_ARROW_KEYS),
                    GetLong(wxT("min"), DEFAULT_MIN),
                    GetLong(wxT("max"), DEFAULT_MAX),
                    GetLong(wxT("value"), DEFAULT_VALUE),
                    GetName());
    control->SetIncrement(GetLong(wxT("inc"), DEFAULT_INCREMENT));

    const long base = GetLong(wxS("base"), 10);
    if ( base != 10 )
        control->SetBase(base);

    SetupWindow(control);

    return control;
}

bool wxSpinCtrlXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, wxT("wxSpinCtrl"));
}


wxIMPLEMENT_DYNAMIC_CLASS(wxSpinCtrlDoubleXmlHandler, wxXmlResourceHandler);

wxSpinCtrlDoubleXmlHandler::wxSpinCtrlDoubleXmlHandler()
    : wxXmlResourceHandler()
{
    AddSpinCtrlStyles(*this);
}

wxObject *wxSpinCtrlDoubleXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(control, wxSpinCtrlDouble)

    control->Create(m_parentAsWindow,
                    GetID(),
                    GetText(wxS("value")),
                    GetPosition(), GetSize(),
                    GetStyle(wxS("style"), wxSP_ARROW_KEYS),
                    double(GetFloat(wxS("min"), DEFAULT_MIN)),
                    double(GetFloat(wxS("max"), DEFAULT_MAX)),
                    double(GetFloat(wxS("value"), DEFAULT_VALUE)),
                    double(GetFloat(wxS("inc"), DEFAULT_INC)),
                    GetName());

    int digits = GetLong("digits", DEFAULT_DIGITS);
    if (digits != DEFAULT_DIGITS)
        control->SetDigits(digits);

    SetupWindow(control);

    return control;
}

bool wxSpinCtrlDoubleXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, wxS("wxSpinCtrlDouble"));
}

#endif // wxUSE_SPINCTRL

#endif // wxUSE_XRC
