/////////////////////////////////////////////////////////////////////////////
// Name:        src/xrc/xh_choic.cpp
// Purpose:     XRC resource for wxChoice
// Author:      Bob Mitchell
// Created:     2000/03/21
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_XRC && wxUSE_CHOICE

#include "wx/xrc/xh_choic.h"

#ifndef WX_PRECOMP
    #include "wx/intl.h"
    #include "wx/choice.h"
#endif

#include "wx/xml/xml.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxChoiceXmlHandler, wxXmlResourceHandler);

wxChoiceXmlHandler::wxChoiceXmlHandler()
: wxXmlResourceHandler() , m_insideBox(false)
{
    XRC_ADD_STYLE(wxCB_SORT);
    AddWindowStyles();
}

wxObject *wxChoiceXmlHandler::DoCreateResource()
{
    if( m_class == wxT("wxChoice"))
    {
        // find the selection
        long selection = GetLong(wxT("selection"), -1);

        // need to build the list of strings from children
        m_insideBox = true;
        CreateChildrenPrivately(NULL, GetParamNode(wxT("content")));

        XRC_MAKE_INSTANCE(control, wxChoice)

        control->Create(m_parentAsWindow,
                        GetID(),
                        GetPosition(), GetSize(),
                        strList,
                        GetStyle(),
                        wxDefaultValidator,
                        GetName());

        if (selection != -1)
            control->SetSelection(selection);

        SetupWindow(control);

        strList.Clear();    // dump the strings

        return control;
    }
    else
    {
        // on the inside now.
        // handle <item>Label</item>

        // add to the list
        strList.Add(GetNodeText(m_node, wxXRC_TEXT_NO_ESCAPE));

        return NULL;
    }
}

bool wxChoiceXmlHandler::CanHandle(wxXmlNode *node)
{
    return (IsOfClass(node, wxT("wxChoice")) ||
           (m_insideBox && node->GetName() == wxT("item")));
}

#endif // wxUSE_XRC && wxUSE_CHOICE
