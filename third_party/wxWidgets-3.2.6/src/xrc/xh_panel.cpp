/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_panel.cpp
// Purpose:     XRC resource for panels
// Author:      Vaclav Slavik
// Created:     2000/03/05
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_XRC

#include "wx/xrc/xh_panel.h"

#ifndef WX_PRECOMP
    #include "wx/panel.h"
    #include "wx/frame.h"
#endif

wxIMPLEMENT_DYNAMIC_CLASS(wxPanelXmlHandler, wxXmlResourceHandler);

wxPanelXmlHandler::wxPanelXmlHandler() : wxXmlResourceHandler()
{
    XRC_ADD_STYLE(wxTAB_TRAVERSAL);
    XRC_ADD_STYLE(wxWS_EX_VALIDATE_RECURSIVELY);

    AddWindowStyles();
}

wxObject *wxPanelXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(panel, wxPanel)

    panel->Create(m_parentAsWindow,
                  GetID(),
                  GetPosition(), GetSize(),
                  GetStyle(wxT("style"), wxTAB_TRAVERSAL),
                  GetName());

    SetupWindow(panel);
    CreateChildren(panel);

    return panel;
}

bool wxPanelXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, wxT("wxPanel"));
}

#endif // wxUSE_XRC
