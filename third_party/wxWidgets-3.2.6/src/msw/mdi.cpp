/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/mdi.cpp
// Purpose:     MDI classes for wxMSW
// Author:      Julian Smart
// Modified by: Vadim Zeitlin on 2008-11-04 to use the base classes
// Created:     04/01/98
// Copyright:   (c) 1998 Julian Smart
//              (c) 2008-2009 Vadim Zeitlin
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ===========================================================================
// declarations
// ===========================================================================

// ---------------------------------------------------------------------------
// headers
// ---------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_MDI && !defined(__WXUNIVERSAL__)

#include "wx/mdi.h"

#ifndef WX_PRECOMP
    #include "wx/frame.h"
    #include "wx/menu.h"
    #include "wx/app.h"
    #include "wx/utils.h"
    #include "wx/dialog.h"
    #include "wx/statusbr.h"
    #include "wx/settings.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/sizer.h"
    #include "wx/toolbar.h"
#endif

#include "wx/stockitem.h"
#include "wx/msw/private.h"
#include "wx/msw/private/winstyle.h"

#include <string.h>

// ---------------------------------------------------------------------------
// global variables
// ---------------------------------------------------------------------------

extern wxMenu *wxCurrentPopupMenu;

extern void wxRemoveHandleAssociation(wxWindow *win);

namespace
{

// ---------------------------------------------------------------------------
// constants
// ---------------------------------------------------------------------------

// First ID for the MDI child menu item in the "Window" menu.
const int wxFIRST_MDI_CHILD = 4100;

// There can be no more than 9 children in the "Window" menu as beginning with
// the tenth one they're not shown and "More windows..." menu item is used
// instead.
const int wxLAST_MDI_CHILD = wxFIRST_MDI_CHILD + 8;

// The ID of the "More windows..." menu item is the next one after the last
// child.
const int wxID_MDI_MORE_WINDOWS = wxLAST_MDI_CHILD + 1;

// ---------------------------------------------------------------------------
// private functions
// ---------------------------------------------------------------------------

// set the MDI menus (by sending the WM_MDISETMENU message) and update the menu
// of the parent of win (which is supposed to be the MDI client window)
void MDISetMenu(wxWindow *win, HMENU hmenuFrame, HMENU hmenuWindow);

// insert the window menu (subMenu) into menu just before "Help" submenu or at
// the very end if not found
void MDIInsertWindowMenu(wxWindow *win, WXHMENU hMenu, HMENU subMenu, const wxString& windowMenuLabelTranslated);

// Remove the window menu
void MDIRemoveWindowMenu(wxWindow *win, WXHMENU hMenu, const wxString& windowMenuLabelTranslated);

// unpack the parameters of WM_MDIACTIVATE message
void UnpackMDIActivate(WXWPARAM wParam, WXLPARAM lParam,
                       WXWORD *activate, WXHWND *hwndAct, WXHWND *hwndDeact);

// return the HMENU of the MDI menu
//
// this function works correctly even when we don't have a window menu and just
// returns 0 then
inline HMENU GetMDIWindowMenu(wxMDIParentFrame *frame)
{
    wxMenu *menu = frame->GetWindowMenu();
    return menu ? GetHmenuOf(menu) : 0;
}

} // anonymous namespace

// ===========================================================================
// implementation
// ===========================================================================

// ---------------------------------------------------------------------------
// wxWin macros
// ---------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxMDIParentFrame, wxFrame);
wxIMPLEMENT_DYNAMIC_CLASS(wxMDIChildFrame, wxFrame);
wxIMPLEMENT_DYNAMIC_CLASS(wxMDIClientWindow, wxWindow);

wxBEGIN_EVENT_TABLE(wxMDIParentFrame, wxFrame)
    EVT_ACTIVATE(wxMDIParentFrame::OnActivate)
    EVT_SIZE(wxMDIParentFrame::OnSize)
    EVT_ICONIZE(wxMDIParentFrame::OnIconized)
    EVT_SYS_COLOUR_CHANGED(wxMDIParentFrame::OnSysColourChanged)

#if wxUSE_MENUS
    EVT_MENU_RANGE(wxFIRST_MDI_CHILD, wxLAST_MDI_CHILD,
                   wxMDIParentFrame::OnMDIChild)
    EVT_MENU_RANGE(wxID_MDI_WINDOW_FIRST, wxID_MDI_WINDOW_LAST,
                   wxMDIParentFrame::OnMDICommand)
#endif // wxUSE_MENUS
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(wxMDIChildFrame, wxFrame)
    EVT_IDLE(wxMDIChildFrame::OnIdle)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(wxMDIClientWindow, wxWindow)
    EVT_SCROLL(wxMDIClientWindow::OnScroll)
wxEND_EVENT_TABLE()

// ===========================================================================
// wxMDIParentFrame: the frame which contains the client window which manages
// the children
// ===========================================================================

void wxMDIParentFrame::Init()
{
#if wxUSE_MENUS && wxUSE_ACCEL
  // the default menu doesn't have any accelerators (even if we have it)
  m_accelWindowMenu = NULL;
#endif // wxUSE_MENUS && wxUSE_ACCEL

  m_activationNotHandled = false;
}

bool wxMDIParentFrame::Create(wxWindow *parent,
                              wxWindowID id,
                              const wxString& title,
                              const wxPoint& pos,
                              const wxSize& size,
                              long style,
                              const wxString& name)
{
  // this style can be used to prevent a window from having the standard MDI
  // "Window" menu
  if ( !(style & wxFRAME_NO_WINDOW_MENU) )
  {
      // normal case: we have the window menu, so construct it
      m_windowMenu = new wxMenu;

      m_windowMenu->Append(wxID_MDI_WINDOW_CASCADE, _("&Cascade"));
      m_windowMenu->Append(wxID_MDI_WINDOW_TILE_HORZ, _("Tile &Horizontally"));
      m_windowMenu->Append(wxID_MDI_WINDOW_TILE_VERT, _("Tile &Vertically"));
      m_windowMenu->AppendSeparator();
      m_windowMenu->Append(wxID_MDI_WINDOW_ARRANGE_ICONS, _("&Arrange Icons"));
      m_windowMenu->Append(wxID_MDI_WINDOW_NEXT, _("&Next"));
      m_windowMenu->Append(wxID_MDI_WINDOW_PREV, _("&Previous"));
  }

  if (!parent)
    wxTopLevelWindows.Append(this);

  SetName(name);
  m_windowStyle = style;

  if ( parent )
      parent->AddChild(this);

  if ( id != wxID_ANY )
    m_windowId = id;
  else
    m_windowId = NewControlId();

  WXDWORD exflags;
  WXDWORD msflags = MSWGetCreateWindowFlags(&exflags);
  msflags &= ~WS_VSCROLL;
  msflags &= ~WS_HSCROLL;

  if ( !wxWindow::MSWCreate(wxApp::GetRegisteredClassName(
                                    wxT("wxMDIFrame"), -1, 0,
                                    (style & wxFULL_REPAINT_ON_RESIZE) ? wxApp::RegClass_Default
                                                                       : wxApp::RegClass_ReturnNR
                                   ),
                            title.t_str(),
                            pos, size,
                            msflags,
                            exflags) )
  {
      return false;
  }

  SetOwnBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));

  // unlike (almost?) all other windows, frames are created hidden
  m_isShown = false;

  return true;
}

wxMDIParentFrame::~wxMDIParentFrame()
{
    // see comment in ~wxMDIChildFrame
#if wxUSE_TOOLBAR
    m_frameToolBar = NULL;
#endif
#if wxUSE_STATUSBAR
    m_frameStatusBar = NULL;
#endif // wxUSE_STATUSBAR

#if wxUSE_MENUS && wxUSE_ACCEL
    delete m_accelWindowMenu;
#endif // wxUSE_MENUS && wxUSE_ACCEL

    DestroyChildren();

    // the MDI frame menubar is not automatically deleted by Windows unlike for
    // the normal frames
    if ( m_hMenu )
        ::DestroyMenu((HMENU)m_hMenu);

    if ( m_clientWindow )
    {
        if ( m_clientWindow->MSWGetOldWndProc() )
            m_clientWindow->UnsubclassWin();

        m_clientWindow->SetHWND(0);
        delete m_clientWindow;
    }
}

// ----------------------------------------------------------------------------
// wxMDIParentFrame child management
// ----------------------------------------------------------------------------

wxMDIChildFrame *wxMDIParentFrame::GetActiveChild() const
{
    HWND hWnd = (HWND)::SendMessage(GetWinHwnd(GetClientWindow()),
                                    WM_MDIGETACTIVE, 0, 0L);
    if ( !hWnd )
        return NULL;

    return static_cast<wxMDIChildFrame *>(wxFindWinFromHandle(hWnd));
}

int wxMDIParentFrame::GetChildFramesCount() const
{
    int count = 0;
    for ( wxWindowList::const_iterator i = GetChildren().begin();
          i != GetChildren().end();
          ++i )
    {
        if ( wxDynamicCast(*i, wxMDIChildFrame) )
            count++;
    }

    return count;
}

#if wxUSE_MENUS

void wxMDIParentFrame::AddMDIChild(wxMDIChildFrame * WXUNUSED(child))
{
    switch ( GetChildFramesCount() )
    {
        case 1:
            // first MDI child was just added, we need to insert the window
            // menu now if we have it
            AddWindowMenu();

            // and disable the items which can't be used until we have more
            // than one child
            UpdateWindowMenu(false);
            break;

        case 2:
            // second MDI child was added, enable the menu items which were
            // disabled because they didn't make sense for a single window
            UpdateWindowMenu(true);
            break;
    }
}

void wxMDIParentFrame::RemoveMDIChild(wxMDIChildFrame * WXUNUSED(child))
{
    switch ( GetChildFramesCount() )
    {
        case 1:
            // last MDI child is being removed, remove the now unnecessary
            // window menu too
            RemoveWindowMenu();

            // there is no need to call UpdateWindowMenu(true) here so this is
            // not quite symmetric to AddMDIChild() above
            break;

        case 2:
            // only one MDI child is going to remain, disable the menu commands
            // which don't make sense for a single child window
            UpdateWindowMenu(false);
            break;
    }
}

// ----------------------------------------------------------------------------
// wxMDIParentFrame window menu handling
// ----------------------------------------------------------------------------

void wxMDIParentFrame::AddWindowMenu()
{
    if ( m_windowMenu )
    {
        // For correct handling of the events from this menu we also must
        // attach it to the menu bar.
        m_windowMenu->Attach(GetMenuBar());

        // Store the current translation, we can't use _("Window") later in
        // case the locale changes.
        m_currentWindowMenuLabel = _("&Window");

        MDIInsertWindowMenu(GetClientWindow(), m_hMenu, GetMDIWindowMenu(this),
                            m_currentWindowMenuLabel);
    }
}

void wxMDIParentFrame::RemoveWindowMenu()
{
    if ( m_windowMenu )
    {
        MDIRemoveWindowMenu(GetClientWindow(), m_hMenu,
                            m_currentWindowMenuLabel);

        m_windowMenu->Detach();
    }
}

void wxMDIParentFrame::UpdateWindowMenu(bool enable)
{
    if ( m_windowMenu )
    {
        m_windowMenu->Enable(wxID_MDI_WINDOW_NEXT, enable);
        m_windowMenu->Enable(wxID_MDI_WINDOW_PREV, enable);
    }
}

#if wxUSE_MENUS_NATIVE

void wxMDIParentFrame::InternalSetMenuBar()
{
    if ( GetActiveChild() )
    {
        AddWindowMenu();
    }
    else // we don't have any MDI children yet
    {
        // wait until we do to add the window menu but do set the main menu for
        // now (this is done by AddWindowMenu() as a side effect)
        MDISetMenu(GetClientWindow(), (HMENU)m_hMenu, NULL);
    }
}

#endif // wxUSE_MENUS_NATIVE

void wxMDIParentFrame::SetWindowMenu(wxMenu* menu)
{
    if ( menu != m_windowMenu )
    {
        // We may not be showing the window menu currently if we don't have any
        // children, and in this case we shouldn't remove/add it back right now.
        const bool hasWindowMenu = GetActiveChild() != NULL;

        if ( hasWindowMenu )
            RemoveWindowMenu();

        delete m_windowMenu;

        m_windowMenu = menu;

        if ( hasWindowMenu )
            AddWindowMenu();
    }

#if wxUSE_ACCEL
    wxDELETE(m_accelWindowMenu);

    if ( menu && menu->HasAccels() )
        m_accelWindowMenu = menu->CreateAccelTable();
#endif // wxUSE_ACCEL
}

// ----------------------------------------------------------------------------
// wxMDIParentFrame other menu-related stuff
// ----------------------------------------------------------------------------

void wxMDIParentFrame::DoMenuUpdates(wxMenu* menu)
{
    wxMDIChildFrame *child = GetActiveChild();
    if ( child )
    {
        wxMenuBar* bar = child->GetMenuBar();

        if (menu)
        {
            menu->UpdateUI();
        }
        else
        {
            if ( bar != NULL )
            {
                int nCount = bar->GetMenuCount();
                for (int n = 0; n < nCount; n++)
                    bar->GetMenu(n)->UpdateUI();
            }
        }
    }
    else
    {
        wxFrameBase::DoMenuUpdates(menu);
    }
}

wxMenuItem *wxMDIParentFrame::FindItemInMenuBar(int menuId) const
{
    // We must look in the child menu first: if it has an item with the same ID
    // as in our own menu bar, the child item should be used to determine
    // whether it's currently enabled.
    wxMenuItem *item = GetActiveChild()
                            ? GetActiveChild()->FindItemInMenuBar(menuId)
                            : NULL;
    if ( !item )
        item = wxFrame::FindItemInMenuBar(menuId);

    if ( !item && m_windowMenu )
        item = m_windowMenu->FindItem(menuId);

    return item;
}

wxMenu* wxMDIParentFrame::MSWFindMenuFromHMENU(WXHMENU hMenu)
{
    wxMenu* menu = GetActiveChild()
                        ? GetActiveChild()->MSWFindMenuFromHMENU(hMenu)
                        : NULL;
    if ( !menu )
        menu = wxFrame::MSWFindMenuFromHMENU(hMenu);

    if ( !menu && m_windowMenu && GetHmenuOf(m_windowMenu) == hMenu )
        menu = m_windowMenu;

    return menu;
}

WXHMENU wxMDIParentFrame::MSWGetActiveMenu() const
{
    wxMDIChildFrame * const child  = GetActiveChild();
    if ( child )
    {
        const WXHMENU hmenu = child->MSWGetActiveMenu();
        if ( hmenu )
            return hmenu;
    }

    return wxFrame::MSWGetActiveMenu();
}

#endif // wxUSE_MENUS

// ----------------------------------------------------------------------------
// wxMDIParentFrame event handling
// ----------------------------------------------------------------------------

void wxMDIParentFrame::UpdateClientSize()
{
    int width, height;
    GetClientSize(&width, &height);

    if ( wxSizer* sizer = GetSizer() )
    {
        sizer->SetDimension(0, 0, width, height);
    }
    else
    {
        if ( GetClientWindow() )
            GetClientWindow()->SetSize(0, 0, width, height);
    }
}

void wxMDIParentFrame::OnSize(wxSizeEvent& WXUNUSED(event))
{
    UpdateClientSize();

    // do not call event.Skip() here, it somehow messes up MDI client window
}

void wxMDIParentFrame::OnIconized(wxIconizeEvent& event)
{
    event.Skip();

    if ( !event.IsIconized() )
        UpdateClientSize();
}

// Responds to colour changes, and passes event on to children.
void wxMDIParentFrame::OnSysColourChanged(wxSysColourChangedEvent& event)
{
    if ( m_clientWindow )
    {
        m_clientWindow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
        m_clientWindow->Refresh();
    }

    event.Skip();
}

WXHICON wxMDIParentFrame::GetDefaultIcon() const
{
    // we don't have any standard icons (any more)
    return (WXHICON)0;
}

// ---------------------------------------------------------------------------
// MDI operations
// ---------------------------------------------------------------------------

void wxMDIParentFrame::Cascade()
{
    ::SendMessage(GetWinHwnd(GetClientWindow()), WM_MDICASCADE, 0, 0);
}

void wxMDIParentFrame::Tile(wxOrientation orient)
{
    wxASSERT_MSG( orient == wxHORIZONTAL || orient == wxVERTICAL,
                  wxT("invalid orientation value") );

    ::SendMessage(GetWinHwnd(GetClientWindow()), WM_MDITILE,
                  orient == wxHORIZONTAL ? MDITILE_HORIZONTAL
                                         : MDITILE_VERTICAL, 0);
}

void wxMDIParentFrame::ArrangeIcons()
{
    ::SendMessage(GetWinHwnd(GetClientWindow()), WM_MDIICONARRANGE, 0, 0);
}

void wxMDIParentFrame::ActivateNext()
{
    ::SendMessage(GetWinHwnd(GetClientWindow()), WM_MDINEXT, 0, 0);
}

void wxMDIParentFrame::ActivatePrevious()
{
    ::SendMessage(GetWinHwnd(GetClientWindow()), WM_MDINEXT, 0, 1);
}

// ---------------------------------------------------------------------------
// the MDI parent frame window proc
// ---------------------------------------------------------------------------

WXLRESULT wxMDIParentFrame::MSWWindowProc(WXUINT message,
                                          WXWPARAM wParam,
                                          WXLPARAM lParam)
{
    WXLRESULT rc = 0;
    bool processed = false;

    switch ( message )
    {
        case WM_ACTIVATE:
            {
                WXWORD state, minimized;
                WXHWND hwnd;
                UnpackActivate(wParam, lParam, &state, &minimized, &hwnd);

                processed = HandleActivate(state, minimized != 0, hwnd);
            }
            break;

        case WM_COMMAND:
            // system messages such as SC_CLOSE are sent as WM_COMMANDs to the
            // parent MDI frame and we must let the DefFrameProc() have them
            // for these commands to work (without it, closing the maximized
            // MDI children doesn't work, for example)
            {
                WXWORD id, cmd;
                WXHWND hwnd;
                UnpackCommand(wParam, lParam, &id, &hwnd, &cmd);

                if ( id == wxID_MDI_MORE_WINDOWS ||
                     (cmd == 0 /* menu */ &&
                        id >= SC_SIZE /* first system menu command */) )
                {
                    MSWDefWindowProc(message, wParam, lParam);
                    processed = true;
                }
            }
            break;

        case WM_CREATE:
            m_clientWindow = OnCreateClient();
            // Uses own style for client style
            if ( !m_clientWindow->CreateClient(this, GetWindowStyleFlag()) )
            {
                wxLogMessage(_("Failed to create MDI parent frame."));

                rc = -1;
            }

            processed = true;
            break;
    }

    if ( !processed )
        rc = wxFrame::MSWWindowProc(message, wParam, lParam);

    return rc;
}

void wxMDIParentFrame::OnActivate(wxActivateEvent& WXUNUSED(event))
{
    // The base class version saves the current focus when we are being
    // deactivated and restores it when the window is activated again, but this
    // is not necessary here as DefWindowProc() for MDI parent frame already
    // takes care of re-activating the MDI child that had been active the last
    // time, and MDI children remember their own last focused child already,
    // being subclasses of wxTLW.
    //
    // Moreover, in addition to being unnecessary, this can be actively harmful
    // if we somehow don't have the focus any more at the moment of activation
    // loss as happens when showing a standard file dialog under Windows 7, see
    // #16635: in this case the base class just gives the focus to its first
    // child, meaning that we can switch to a different MDI child, which is
    // worse than losing focus inside the current child.
    //
    // So we don't let the base class have this event to prevent this from
    // happening. But the event is not really processed, so we set a flag here
    // which is used in HandleActivate() below to check if the event was really
    // processed (and not skipped) in the user code or just reached this dummy
    // handler.
    m_activationNotHandled = true;
}

bool wxMDIParentFrame::HandleActivate(int state, bool minimized, WXHWND activate)
{
    bool processed = false;

    // Set the flag before testing it to ensure the only way for it to be true
    // is to be set in our OnActivate() -- and not just remain set from the
    // last time.
    m_activationNotHandled = false;

    if ( wxWindow::HandleActivate(state, minimized, activate) )
    {
        // already processed, unless we artificially marked the event as
        // handled in our own handler without really processing it
        processed = !m_activationNotHandled;
    }

    // Also generate the (de)activation event for the current child, if any, to
    // allow updating its state and, in particular, remembering or restoring
    // its last focused window.
    if ( GetActiveChild() )
    {
        if ( GetActiveChild()->HandleActivate(state, minimized, activate) )
            processed = true;
    }

    return processed;
}

#if wxUSE_MENUS

void wxMDIParentFrame::OnMDIChild(wxCommandEvent& event)
{
    wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
    while ( node )
    {
        wxWindow *child = node->GetData();
        if ( child->GetHWND() )
        {
            int childId = wxGetWindowId(child->GetHWND());
            if ( childId == event.GetId() )
            {
                wxStaticCast(child, wxMDIChildFrame)->Activate();
                return;
            }
        }

        node = node->GetNext();
    }

    wxFAIL_MSG( "unknown MDI child selected?" );
}

void wxMDIParentFrame::OnMDICommand(wxCommandEvent& event)
{
    WXWPARAM wParam = 0;
    WXLPARAM lParam = 0;
    int msg;
    switch ( event.GetId() )
    {
        case wxID_MDI_WINDOW_CASCADE:
            msg = WM_MDICASCADE;
            wParam = MDITILE_SKIPDISABLED;
            break;

        case wxID_MDI_WINDOW_TILE_HORZ:
            wParam |= MDITILE_HORIZONTAL;
            wxFALLTHROUGH;

        case wxID_MDI_WINDOW_TILE_VERT:
            if ( !wParam )
                wParam = MDITILE_VERTICAL;
            msg = WM_MDITILE;
            wParam |= MDITILE_SKIPDISABLED;
            break;

        case wxID_MDI_WINDOW_ARRANGE_ICONS:
            msg = WM_MDIICONARRANGE;
            break;

        case wxID_MDI_WINDOW_NEXT:
            msg = WM_MDINEXT;
            lParam = 0;         // next child
            break;

        case wxID_MDI_WINDOW_PREV:
            msg = WM_MDINEXT;
            lParam = 1;         // previous child
            break;

        default:
            wxFAIL_MSG( "unknown MDI command" );
            return;
    }

    ::SendMessage(GetWinHwnd(GetClientWindow()), msg, wParam, lParam);
}

#endif // wxUSE_MENUS

WXLRESULT wxMDIParentFrame::MSWDefWindowProc(WXUINT message,
                                        WXWPARAM wParam,
                                        WXLPARAM lParam)
{
    WXHWND clientWnd;
    if ( GetClientWindow() )
        clientWnd = GetClientWindow()->GetHWND();
    else
        clientWnd = 0;

    return DefFrameProc(GetHwnd(), (HWND)clientWnd, message, wParam, lParam);
}

bool wxMDIParentFrame::MSWTranslateMessage(WXMSG* msg)
{
    MSG *pMsg = (MSG *)msg;

    // first let the current child get it
    wxMDIChildFrame * const child = GetActiveChild();
    if ( child && child->MSWTranslateMessage(msg) )
    {
        return true;
    }

    // then try out accelerator table (will also check the accelerators for the
    // normal menu items)
    if ( wxFrame::MSWTranslateMessage(msg) )
    {
        return true;
    }

#if wxUSE_MENUS && wxUSE_ACCEL
    // but it doesn't check for the (custom) accelerators of the window menu
    // items as it's not part of the menu bar as it's handled by Windows itself
    // so we need to do this explicitly
    if ( m_accelWindowMenu && m_accelWindowMenu->Translate(this, msg) )
        return true;
#endif // wxUSE_MENUS && wxUSE_ACCEL

    // finally, check for MDI specific built-in accelerators
    if ( pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN )
    {
        if ( ::TranslateMDISysAccel(GetWinHwnd(GetClientWindow()), pMsg))
            return true;
    }

    return false;
}

// ===========================================================================
// wxMDIChildFrame
// ===========================================================================

void wxMDIChildFrame::Init()
{
    m_needsResize = true;
}

bool wxMDIChildFrame::Create(wxMDIParentFrame *parent,
                             wxWindowID id,
                             const wxString& title,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxString& name)
{
    m_mdiParent = parent;

  SetName(name);

  if ( id != wxID_ANY )
    m_windowId = id;
  else
    m_windowId = NewControlId();

  if ( parent )
  {
      parent->AddChild(this);
  }

  int x = pos.x;
  int y = pos.y;
  int width = size.x;
  int height = size.y;

  MDICREATESTRUCT mcs;

  wxString className = wxApp::GetRegisteredClassName(
                               wxT("wxMDIChildFrame"), COLOR_WINDOW, 0,
                               (style & wxFULL_REPAINT_ON_RESIZE) ? wxApp::RegClass_Default
                                                                  : wxApp::RegClass_ReturnNR
                              );

  mcs.szClass = className.t_str();
  mcs.szTitle = title.t_str();
  mcs.hOwner = wxGetInstance();
  if (x != wxDefaultCoord)
      mcs.x = x;
  else
      mcs.x = CW_USEDEFAULT;

  if (y != wxDefaultCoord)
      mcs.y = y;
  else
      mcs.y = CW_USEDEFAULT;

  if (width != wxDefaultCoord)
      mcs.cx = width;
  else
      mcs.cx = CW_USEDEFAULT;

  if (height != wxDefaultCoord)
      mcs.cy = height;
  else
      mcs.cy = CW_USEDEFAULT;

  DWORD msflags = WS_OVERLAPPED | WS_CLIPCHILDREN;
  if (style & wxMINIMIZE_BOX)
    msflags |= WS_MINIMIZEBOX;
  if (style & wxMAXIMIZE_BOX)
    msflags |= WS_MAXIMIZEBOX;
  if (style & wxRESIZE_BORDER)
    msflags |= WS_THICKFRAME;
  if (style & wxSYSTEM_MENU)
    msflags |= WS_SYSMENU;
  if (style & wxMINIMIZE)
    msflags |= WS_MINIMIZE;
  if (style & wxMAXIMIZE)
    msflags |= WS_MAXIMIZE;
  if (style & wxCAPTION)
    msflags |= WS_CAPTION;

  mcs.style = msflags;

  mcs.lParam = 0;

  wxWindowCreationHook hook(this);

  m_hWnd = (WXHWND)::SendMessage(GetWinHwnd(parent->GetClientWindow()),
                                 WM_MDICREATE, 0, (LPARAM)&mcs);

  if ( !m_hWnd )
  {
      wxLogLastError(wxT("WM_MDICREATE"));
      return false;
  }

  SubclassWin(m_hWnd);

  parent->AddMDIChild(this);

  return true;
}

wxMDIChildFrame::~wxMDIChildFrame()
{
    // if we hadn't been created, there is nothing to destroy
    if ( !m_hWnd )
        return;

    wxMDIParentFrame * const parent = GetMDIParent();

    parent->RemoveMDIChild(this);

    // will be destroyed by DestroyChildren() but reset them before calling it
    // to avoid using dangling pointers if a callback comes in the meanwhile
#if wxUSE_TOOLBAR
    m_frameToolBar = NULL;
#endif
#if wxUSE_STATUSBAR
    m_frameStatusBar = NULL;
#endif // wxUSE_STATUSBAR

    DestroyChildren();

    MDIRemoveWindowMenu(NULL, m_hMenu, parent->MSWGetCurrentWindowMenuLabel());

    // MDIRemoveWindowMenu() doesn't update the MDI menu when called with NULL
    // window, so do it ourselves.
    MDISetMenu(parent->GetClientWindow(),
               (HMENU)parent->MSWGetActiveMenu(),
               GetMDIWindowMenu(parent));

    MSWDestroyWindow();
}

bool wxMDIChildFrame::Show(bool show)
{
    if (!wxFrame::Show(show))
        return false;

    // KH: Without this call, new MDI children do not become active.
    // This was added here after the same BringWindowToTop call was
    // removed from wxTopLevelWindow::Show (November 2005)
    if ( show )
        ::BringWindowToTop(GetHwnd());

    // we need to refresh the MDI frame window menu to include (or exclude if
    // we've been hidden) this frame
    wxMDIParentFrame * const parent = GetMDIParent();
    MDISetMenu(parent->GetClientWindow(), NULL, NULL);

    return true;
}

void
wxMDIChildFrame::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
    // we need to disable client area origin adjustments used for the child
    // windows for the frame itself
    wxMDIChildFrameBase::DoSetSize(x, y, width, height, sizeFlags);
}

// Set the client size (i.e. leave the calculation of borders etc.
// to wxWidgets)
void wxMDIChildFrame::DoSetClientSize(int width, int height)
{
  HWND hWnd = GetHwnd();

  RECT rect;
  ::GetClientRect(hWnd, &rect);

  RECT rect2;
  GetWindowRect(hWnd, &rect2);

  // Find the difference between the entire window (title bar and all)
  // and the client area; add this to the new client size to move the
  // window
  int actual_width = rect2.right - rect2.left - rect.right + width;
  int actual_height = rect2.bottom - rect2.top - rect.bottom + height;

#if wxUSE_STATUSBAR
  if (GetStatusBar() && GetStatusBar()->IsShown())
  {
    int sx, sy;
    GetStatusBar()->GetSize(&sx, &sy);
    actual_height += sy;
  }
#endif // wxUSE_STATUSBAR

  POINT point;
  point.x = rect2.left;
  point.y = rect2.top;

  // If there's an MDI parent, must subtract the parent's top left corner
  // since MoveWindow moves relative to the parent
  wxMDIParentFrame * const mdiParent = GetMDIParent();
  ::ScreenToClient(GetHwndOf(mdiParent->GetClientWindow()), &point);

  MoveWindow(hWnd, point.x, point.y, actual_width, actual_height, (BOOL)true);

  wxSize size(width, height);
  wxSizeEvent event(size, m_windowId);
  event.SetEventObject( this );
  HandleWindowEvent(event);
}

// Unlike other wxTopLevelWindowBase, the mdi child's "GetPosition" is not the
//  same as its GetScreenPosition
void wxMDIChildFrame::DoGetScreenPosition(int *x, int *y) const
{
  HWND hWnd = GetHwnd();

  RECT rect;
  ::GetWindowRect(hWnd, &rect);
  if (x)
     *x = rect.left;
  if (y)
     *y = rect.top;
}


void wxMDIChildFrame::DoGetPosition(int *x, int *y) const
{
  RECT rect;
  GetWindowRect(GetHwnd(), &rect);
  POINT point;
  point.x = rect.left;
  point.y = rect.top;

  // Since we now have the absolute screen coords,
  // if there's a parent we must subtract its top left corner
  wxMDIParentFrame * const mdiParent = GetMDIParent();
  ::ScreenToClient(GetHwndOf(mdiParent->GetClientWindow()), &point);

  if (x)
      *x = point.x;
  if (y)
      *y = point.y;
}

void wxMDIChildFrame::InternalSetMenuBar()
{
    wxMDIParentFrame * const parent = GetMDIParent();

    MDIInsertWindowMenu(parent->GetClientWindow(),
                        m_hMenu, GetMDIWindowMenu(parent),
                        parent->MSWGetCurrentWindowMenuLabel());
}

void wxMDIChildFrame::DetachMenuBar()
{
    wxMDIParentFrame * const parent = GetMDIParent();

    MDIRemoveWindowMenu(NULL, m_hMenu, parent->MSWGetCurrentWindowMenuLabel());
    wxFrame::DetachMenuBar();
}

WXHICON wxMDIChildFrame::GetDefaultIcon() const
{
    // we don't have any standard icons (any more)
    return (WXHICON)0;
}

// ---------------------------------------------------------------------------
// MDI operations
// ---------------------------------------------------------------------------

void wxMDIChildFrame::Maximize(bool maximize)
{
    wxMDIParentFrame * const parent = GetMDIParent();
    if ( parent && parent->GetClientWindow() )
    {
        if ( !IsShown() )
        {
            // Turn off redrawing in the MDI client window because otherwise
            // maximizing it would also show it and we don't want this for
            // hidden windows.
            ::SendMessage(GetWinHwnd(parent->GetClientWindow()), WM_SETREDRAW,
                          FALSE, 0L);
        }

        ::SendMessage(GetWinHwnd(parent->GetClientWindow()),
                      maximize ? WM_MDIMAXIMIZE : WM_MDIRESTORE,
                      (WPARAM)GetHwnd(), 0);

        if ( !IsShown() )
        {
            // Hide back the child window shown by maximizing it.
            ::ShowWindow(GetHwnd(), SW_HIDE);

            // Turn redrawing in the MDI client back on.
            ::SendMessage(GetWinHwnd(parent->GetClientWindow()), WM_SETREDRAW,
                          TRUE, 0L);
        }
    }
}

void wxMDIChildFrame::Restore()
{
    wxMDIParentFrame * const parent = GetMDIParent();
    if ( parent && parent->GetClientWindow() )
    {
        ::SendMessage(GetWinHwnd(parent->GetClientWindow()), WM_MDIRESTORE,
                      (WPARAM) GetHwnd(), 0);
    }
}

void wxMDIChildFrame::Activate()
{
    wxMDIParentFrame * const parent = GetMDIParent();
    if ( parent && parent->GetClientWindow() )
    {
        // Activating an iconized MDI frame doesn't do anything, so restore it
        // first to really present it to the user.
        if ( IsIconized() )
            Restore();

        ::SendMessage(GetWinHwnd(parent->GetClientWindow()), WM_MDIACTIVATE,
                      (WPARAM) GetHwnd(), 0);
    }
}

// ---------------------------------------------------------------------------
// MDI window proc and message handlers
// ---------------------------------------------------------------------------

WXLRESULT wxMDIChildFrame::MSWWindowProc(WXUINT message,
                                         WXWPARAM wParam,
                                         WXLPARAM lParam)
{
    WXLRESULT rc = 0;
    bool processed = false;

    switch ( message )
    {
        case WM_GETMINMAXINFO:
            processed = HandleGetMinMaxInfo((MINMAXINFO *)lParam);
            break;

        case WM_MDIACTIVATE:
            {
                WXWORD act;
                WXHWND hwndAct, hwndDeact;
                UnpackMDIActivate(wParam, lParam, &act, &hwndAct, &hwndDeact);

                processed = HandleMDIActivate(act, hwndAct, hwndDeact);
            }
            wxFALLTHROUGH;

        case WM_MOVE:
            // must pass WM_MOVE to DefMDIChildProc() to recalculate MDI client
            // scrollbars if necessary

            // fall through

        case WM_SIZE:
            // must pass WM_SIZE to DefMDIChildProc(), otherwise many weird
            // things happen
            MSWDefWindowProc(message, wParam, lParam);
            break;

        case WM_WINDOWPOSCHANGING:
            processed = HandleWindowPosChanging((LPWINDOWPOS)lParam);
            break;
    }

    if ( !processed )
        rc = wxFrame::MSWWindowProc(message, wParam, lParam);

    return rc;
}

bool wxMDIChildFrame::HandleMDIActivate(long WXUNUSED(activate),
                                        WXHWND hwndAct,
                                        WXHWND hwndDeact)
{
    wxMDIParentFrame * const parent = GetMDIParent();

    WXHMENU hMenuToSet = 0;

    bool activated;

    if ( m_hWnd == hwndAct )
    {
        activated = true;
        parent->SetActiveChild(this);

        WXHMENU hMenuChild = m_hMenu;
        if ( hMenuChild )
            hMenuToSet = hMenuChild;
    }
    else if ( m_hWnd == hwndDeact )
    {
        wxASSERT_MSG( parent->GetActiveChild() == this,
                      wxT("can't deactivate MDI child which wasn't active!") );

        activated = false;
        parent->SetActiveChild(NULL);

        WXHMENU hMenuParent = parent->m_hMenu;

        // activate the parent menu only when there is no other child
        // that has been activated
        if ( hMenuParent && !hwndAct )
            hMenuToSet = hMenuParent;
    }
    else
    {
        // we have nothing to do with it
        return false;
    }

    if ( hMenuToSet )
    {
        MDISetMenu(parent->GetClientWindow(),
                   (HMENU)hMenuToSet, GetMDIWindowMenu(parent));
    }

    wxActivateEvent event(wxEVT_ACTIVATE, activated, m_windowId);
    event.SetEventObject( this );

    ResetWindowStyle(NULL);

    return HandleWindowEvent(event);
}

bool wxMDIChildFrame::HandleWindowPosChanging(void *pos)
{
    WINDOWPOS *lpPos = (WINDOWPOS *)pos;

    if (!(lpPos->flags & SWP_NOSIZE))
    {
        RECT rectClient;
        DWORD dwExStyle = ::GetWindowLong(GetHwnd(), GWL_EXSTYLE);
        DWORD dwStyle = ::GetWindowLong(GetHwnd(), GWL_STYLE);
        if (ResetWindowStyle((void *) & rectClient) && (dwStyle & WS_MAXIMIZE))
        {
            ::AdjustWindowRectEx(&rectClient, dwStyle, false, dwExStyle);
            lpPos->x = rectClient.left;
            lpPos->y = rectClient.top;
            lpPos->cx = rectClient.right - rectClient.left;
            lpPos->cy = rectClient.bottom - rectClient.top;
        }
    }

    return false;
}

bool wxMDIChildFrame::HandleGetMinMaxInfo(void *mmInfo)
{
    // Get the window max size from DefMDIChildProc() as it calculates it
    // correctly from the size of the MDI parent frame.
    MSWDefWindowProc(WM_GETMINMAXINFO, 0, (LPARAM)mmInfo);

    // But then handle the message as usual at the base class level to allow
    // overriding min/max frame size as for the normal frames.
    return false;
}

// ---------------------------------------------------------------------------
// MDI specific message translation/preprocessing
// ---------------------------------------------------------------------------

WXLRESULT wxMDIChildFrame::MSWDefWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    return DefMDIChildProc(GetHwnd(),
                           (UINT)message, (WPARAM)wParam, (LPARAM)lParam);
}

bool wxMDIChildFrame::MSWTranslateMessage(WXMSG* msg)
{
    // we must pass the parent frame to ::TranslateAccelerator(), otherwise it
    // doesn't do its job correctly for MDI child menus
    return MSWDoTranslateMessage(GetMDIParent(), msg);
}

// ---------------------------------------------------------------------------
// misc
// ---------------------------------------------------------------------------

void wxMDIChildFrame::MSWDestroyWindow()
{
    wxMDIParentFrame * const parent = GetMDIParent();

    // Must make sure this handle is invalidated (set to NULL) since all sorts
    // of things could happen after the child client is destroyed, but before
    // the wxFrame is destroyed.

    HWND oldHandle = (HWND)GetHWND();
    SendMessage(GetWinHwnd(parent->GetClientWindow()), WM_MDIDESTROY,
                (WPARAM)oldHandle, 0);

    if (parent->GetActiveChild() == NULL)
        ResetWindowStyle(NULL);

    if (m_hMenu)
    {
        ::DestroyMenu((HMENU) m_hMenu);
        m_hMenu = 0;
    }
    wxRemoveHandleAssociation(this);
    m_hWnd = 0;
}

// Change the client window's extended style so we don't get a client edge
// style when a child is maximised (a double border looks silly.)
bool wxMDIChildFrame::ResetWindowStyle(void *vrect)
{
    RECT *rect = (RECT *)vrect;
    wxMDIParentFrame * const pFrameWnd = GetMDIParent();
    wxMDIChildFrame* pChild = pFrameWnd->GetActiveChild();

    if (!pChild || (pChild == this))
    {
        HWND hwndClient = GetWinHwnd(pFrameWnd->GetClientWindow());

        wxMSWWinStyleUpdater updateStyle(hwndClient);

        // we want to test whether there is a maximized child, so just set
        // dwThisStyle to 0 if there is no child at all
        DWORD dwThisStyle = pChild
            ? ::GetWindowLong(GetWinHwnd(pChild), GWL_STYLE) : 0;
        updateStyle.TurnOnOrOff(!(dwThisStyle & WS_MAXIMIZE), WS_EX_CLIENTEDGE);

        if ( updateStyle.Apply() )
        {
            // force update of everything
            ::RedrawWindow(hwndClient, NULL, NULL,
                           RDW_INVALIDATE | RDW_ALLCHILDREN);
            ::SetWindowPos(hwndClient, NULL, 0, 0, 0, 0,
                           SWP_FRAMECHANGED | SWP_NOACTIVATE |
                           SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                           SWP_NOCOPYBITS);
            if (rect)
                ::GetClientRect(hwndClient, rect);

            return true;
        }
    }

    return false;
}

// ===========================================================================
// wxMDIClientWindow: the window of predefined (by Windows) class which
// contains the child frames
// ===========================================================================

bool wxMDIClientWindow::CreateClient(wxMDIParentFrame *parent, long style)
{
    m_backgroundColour = wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE);

    CLIENTCREATESTRUCT ccs;
    m_windowStyle = style;
    m_parent = parent;

    ccs.hWindowMenu = GetMDIWindowMenu(parent);
    ccs.idFirstChild = wxFIRST_MDI_CHILD;

    DWORD msStyle = MDIS_ALLCHILDSTYLES | WS_VISIBLE | WS_CHILD |
                    WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    if ( style & wxHSCROLL )
        msStyle |= WS_HSCROLL;
    if ( style & wxVSCROLL )
        msStyle |= WS_VSCROLL;

    DWORD exStyle = WS_EX_CLIENTEDGE;

    wxWindowCreationHook hook(this);
    m_hWnd = (WXHWND)::CreateWindowEx
                       (
                        exStyle,
                        wxT("MDICLIENT"),
                        NULL,
                        msStyle,
                        0, 0, 0, 0,
                        GetWinHwnd(parent),
                        NULL,
                        wxGetInstance(),
                        (LPSTR)(LPCLIENTCREATESTRUCT)&ccs);
    if ( !m_hWnd )
    {
        wxLogLastError(wxT("CreateWindowEx(MDI client)"));

        return false;
    }

    SubclassWin(m_hWnd);

    return true;
}

// Explicitly call default scroll behaviour
void wxMDIClientWindow::OnScroll(wxScrollEvent& event)
{
    // Note: for client windows, the scroll position is not set in
    // WM_HSCROLL, WM_VSCROLL, so we can't easily determine what
    // scroll position we're at.
    // This makes it hard to paint patterns or bitmaps in the background,
    // and have the client area scrollable as well.

    if ( event.GetOrientation() == wxHORIZONTAL )
        m_scrollX = event.GetPosition(); // Always returns zero!
    else
        m_scrollY = event.GetPosition(); // Always returns zero!

    event.Skip();
}

void wxMDIClientWindow::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
    // Try to fix a problem whereby if you show an MDI child frame, then reposition the
    // client area, you can end up with a non-refreshed portion in the client window
    // (see OGL studio sample). So check if the position is changed and if so,
    // redraw the MDI child frames.

    const wxPoint oldPos = GetPosition();

    wxWindow::DoSetSize(x, y, width, height, sizeFlags | wxSIZE_FORCE);

    const wxPoint newPos = GetPosition();

    if ((newPos.x != oldPos.x) || (newPos.y != oldPos.y))
    {
        if (GetParent())
        {
            wxWindowList::compatibility_iterator node = GetParent()->GetChildren().GetFirst();
            while (node)
            {
                wxWindow *child = node->GetData();
                if (wxDynamicCast(child, wxMDIChildFrame))
                {
                   ::RedrawWindow(GetHwndOf(child),
                                  NULL,
                                  NULL,
                                  RDW_FRAME |
                                  RDW_ALLCHILDREN |
                                  RDW_INVALIDATE);
                }
                node = node->GetNext();
            }
        }
    }
}

void wxMDIChildFrame::OnIdle(wxIdleEvent& event)
{
    // MDI child frames get their WM_SIZE when they're constructed but at this
    // moment they don't have any children yet so all child windows will be
    // positioned incorrectly when they are added later - to fix this, we
    // generate an artificial size event here
    if ( m_needsResize )
    {
        m_needsResize = false; // avoid any possibility of recursion

        SendSizeEvent();
    }

    event.Skip();
}

// ---------------------------------------------------------------------------
// private helper functions
// ---------------------------------------------------------------------------

namespace
{

void MDISetMenu(wxWindow *win, HMENU hmenuFrame, HMENU hmenuWindow)
{
    if ( hmenuFrame || hmenuWindow )
    {
        // Under XP, the last error seems to be not reset by this function, so
        // ensure we don't report spurious errors below when setting the menu
        // initially.
        ::SetLastError(ERROR_SUCCESS);

        if ( !::SendMessage(GetWinHwnd(win),
                            WM_MDISETMENU,
                            (WPARAM)hmenuFrame,
                            (LPARAM)hmenuWindow) )
        {
            const DWORD err = ::GetLastError();
            if ( err != ERROR_SUCCESS )
            {
                wxLogApiError(wxT("SendMessage(WM_MDISETMENU)"), err);
            }
        }
    }

    // update menu bar of the parent window
    wxWindow *parent = win->GetParent();
    wxCHECK_RET( parent, wxT("MDI client without parent frame? weird...") );

    ::SendMessage(GetWinHwnd(win), WM_MDIREFRESHMENU, 0, 0L);

    ::DrawMenuBar(GetWinHwnd(parent));
}

class MenuIterator
{
public:
    explicit MenuIterator(HMENU hmenu)
        : m_hmenu(hmenu),
          m_numItems(::GetMenuItemCount(hmenu)),
          m_pos(-1)
    {
        m_mii.fMask = MIIM_STRING;
        m_mii.dwTypeData = m_buf;
    }

    bool GetNext(wxString& str)
    {
        // Loop until we get the label of the next menu item.
        for ( m_pos++; m_pos < m_numItems; m_pos++ )
        {
            // As cch field is updated by GetMenuItemInfo(), it's important to
            // reset it to the size of the buffer before each call.
            m_mii.cch = WXSIZEOF(m_buf);

            if ( !::GetMenuItemInfo(m_hmenu, m_pos, TRUE, &m_mii) )
            {
                wxLogLastError(wxString::Format("GetMenuItemInfo(%d)", m_pos));
                continue;
            }

            if ( !m_mii.cch )
            {
                // This isn't a string menu at all.
                continue;
            }

            str = m_buf;
            return true;
        }

        return false;
    }

    int GetPos() const { return m_pos; }

private:
    const HMENU m_hmenu;
    const int m_numItems;
    int m_pos;

    wxChar m_buf[1024];
    WinStruct<MENUITEMINFO> m_mii;

    wxDECLARE_NO_COPY_CLASS(MenuIterator);
};

void MDIInsertWindowMenu(wxWindow *win, WXHMENU hMenu, HMENU menuWin, const wxString& windowMenuLabelTranslated)
{
    HMENU hmenu = (HMENU)hMenu;

    if ( menuWin )
    {
        // Try to insert Window menu in front of Help, otherwise append it.
        bool inserted = false;
        wxString buf;
        MenuIterator it(hmenu);
        while ( it.GetNext(buf) )
        {
            const wxString label = wxStripMenuCodes(buf, wxStrip_Menu);
            if ( label == wxGetStockLabel(wxID_HELP, wxSTOCK_NOFLAGS) )
            {
                inserted = true;
                ::InsertMenu(hmenu, it.GetPos(),
                             MF_BYPOSITION | MF_POPUP | MF_STRING,
                             (UINT_PTR)menuWin,
                             windowMenuLabelTranslated.t_str());
                break;
            }
        }

        if ( !inserted )
        {
            ::AppendMenu(hmenu, MF_POPUP,
                         (UINT_PTR)menuWin,
                         windowMenuLabelTranslated.t_str());
        }
    }

    MDISetMenu(win, hmenu, menuWin);
}

void MDIRemoveWindowMenu(wxWindow *win, WXHMENU hMenu, const wxString& windowMenuLabelTranslated)
{
    HMENU hmenu = (HMENU)hMenu;

    if ( hmenu )
    {
        wxString buf;
        MenuIterator it(hmenu);
        while ( it.GetNext(buf) )
        {
            if ( wxStrcmp(buf, windowMenuLabelTranslated) == 0 )
            {
                if ( !::RemoveMenu(hmenu, it.GetPos(), MF_BYPOSITION) )
                {
                    wxLogLastError(wxT("RemoveMenu"));
                }

                break;
            }
        }
    }

    if ( win )
    {
        // we don't change the windows menu, but we update the main one
        MDISetMenu(win, hmenu, NULL);
    }
}

void UnpackMDIActivate(WXWPARAM wParam, WXLPARAM lParam,
                              WXWORD *activate, WXHWND *hwndAct, WXHWND *hwndDeact)
{
    *activate = true;
    *hwndAct = (WXHWND)lParam;
    *hwndDeact = (WXHWND)wParam;
}

} // anonymous namespace

#endif // wxUSE_MDI && !defined(__WXUNIVERSAL__)
