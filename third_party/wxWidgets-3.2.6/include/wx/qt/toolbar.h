/////////////////////////////////////////////////////////////////////////////
// Name:        wx/qt/toolbar.h
// Author:      Sean D'Epagnier, Mariano Reingart
// Copyright:   (c) 2010 wxWidgets dev team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


#ifndef _WX_QT_TOOLBAR_H_
#define _WX_QT_TOOLBAR_H_

class QActionGroup;
class QToolBar;

class WXDLLIMPEXP_CORE wxToolBar : public wxToolBarBase
{
public:

    wxToolBar() { Init(); }
    wxToolBar(wxWindow *parent,
              wxWindowID id,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              long style = wxTB_DEFAULT_STYLE | wxNO_BORDER,
              const wxString& name = wxASCII_STR(wxToolBarNameStr))
    {
        Init();

        Create(parent, id, pos, size, style, name);
    }

    virtual ~wxToolBar();

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxTB_DEFAULT_STYLE | wxNO_BORDER,
                const wxString& name = wxASCII_STR(wxToolBarNameStr));

    virtual wxToolBarToolBase *FindToolForPosition(wxCoord x, wxCoord y) const wxOVERRIDE;

    virtual void SetWindowStyleFlag( long style ) wxOVERRIDE;

    virtual void SetToolShortHelp(int id, const wxString& helpString) wxOVERRIDE;
    virtual void SetToolNormalBitmap(int id, const wxBitmapBundle& bitmap) wxOVERRIDE;
    virtual void SetToolDisabledBitmap(int id, const wxBitmapBundle& bitmap) wxOVERRIDE;

    virtual bool Realize() wxOVERRIDE;

    virtual wxToolBarToolBase *CreateTool(int toolid,
                                          const wxString& label,
                                          const wxBitmapBundle& bmpNormal,
                                          const wxBitmapBundle& bmpDisabled = wxNullBitmap,
                                          wxItemKind kind = wxITEM_NORMAL,
                                          wxObject *clientData = NULL,
                                          const wxString& shortHelp = wxEmptyString,
                                          const wxString& longHelp = wxEmptyString) wxOVERRIDE;

    virtual wxToolBarToolBase *CreateTool(wxControl *control,
                                          const wxString& label) wxOVERRIDE;
    QWidget *GetHandle() const wxOVERRIDE;

    // Private, only used by wxFrame.
    QToolBar *GetQToolBar() const { return m_qtToolBar; }

protected:
    QActionGroup* GetActionGroup(size_t pos);
    virtual bool DoInsertTool(size_t pos, wxToolBarToolBase *tool) wxOVERRIDE;
    virtual bool DoDeleteTool(size_t pos, wxToolBarToolBase *tool) wxOVERRIDE;
    virtual void DoEnableTool(wxToolBarToolBase *tool, bool enable) wxOVERRIDE;
    virtual void DoToggleTool(wxToolBarToolBase *tool, bool toggle) wxOVERRIDE;
    virtual void DoSetToggle(wxToolBarToolBase *tool, bool toggle) wxOVERRIDE;

private:
    void Init();

    long GetButtonStyle();

    QToolBar *m_qtToolBar;

    wxDECLARE_DYNAMIC_CLASS(wxToolBar);
};

#endif // _WX_QT_TOOLBAR_H_
