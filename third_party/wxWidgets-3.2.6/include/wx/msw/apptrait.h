///////////////////////////////////////////////////////////////////////////////
// Name:        wx/msw/apptrait.h
// Purpose:     class implementing wxAppTraits for MSW
// Author:      Vadim Zeitlin
// Modified by:
// Created:     21.06.2003
// Copyright:   (c) 2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_MSW_APPTRAIT_H_
#define _WX_MSW_APPTRAIT_H_

// ----------------------------------------------------------------------------
// wxGUI/ConsoleAppTraits: must derive from wxAppTraits, not wxAppTraitsBase
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_BASE wxConsoleAppTraits : public wxConsoleAppTraitsBase
{
public:
    virtual wxEventLoopBase *CreateEventLoop() wxOVERRIDE;
    virtual void *BeforeChildWaitLoop() wxOVERRIDE;
    virtual void AfterChildWaitLoop(void *data) wxOVERRIDE;
#if wxUSE_TIMER
    virtual wxTimerImpl *CreateTimerImpl(wxTimer *timer) wxOVERRIDE;
#endif // wxUSE_TIMER
#if wxUSE_THREADS
    virtual bool DoMessageFromThreadWait() wxOVERRIDE;
    virtual WXDWORD WaitForThread(WXHANDLE hThread, int flags) wxOVERRIDE;
#endif // wxUSE_THREADS
    virtual bool CanUseStderr() wxOVERRIDE { return true; }
    virtual bool WriteToStderr(const wxString& text) wxOVERRIDE;
    virtual WXHWND GetMainHWND() const wxOVERRIDE { return NULL; }
};

#if wxUSE_GUI

#if defined(__WXMSW__)

class WXDLLIMPEXP_CORE wxGUIAppTraits : public wxGUIAppTraitsBase
{
public:
    virtual wxEventLoopBase *CreateEventLoop() wxOVERRIDE;
    virtual void *BeforeChildWaitLoop() wxOVERRIDE;
    virtual void AfterChildWaitLoop(void *data) wxOVERRIDE;
#if wxUSE_TIMER
    virtual wxTimerImpl *CreateTimerImpl(wxTimer *timer) wxOVERRIDE;
#endif // wxUSE_TIMER
#if wxUSE_THREADS
    virtual bool DoMessageFromThreadWait() wxOVERRIDE;
    virtual WXDWORD WaitForThread(WXHANDLE hThread, int flags) wxOVERRIDE;
#endif // wxUSE_THREADS
    wxPortId GetToolkitVersion(int *majVer = NULL,
                               int *minVer = NULL,
                               int *microVer = NULL) const wxOVERRIDE;

    virtual bool CanUseStderr() wxOVERRIDE;
    virtual bool WriteToStderr(const wxString& text) wxOVERRIDE;
    virtual WXHWND GetMainHWND() const wxOVERRIDE;
};

#elif defined(__WXGTK__)

class WXDLLIMPEXP_CORE wxGUIAppTraits : public wxGUIAppTraitsBase
{
public:
    virtual wxEventLoopBase *CreateEventLoop() wxOVERRIDE;
    virtual void *BeforeChildWaitLoop() wxOVERRIDE { return NULL; }
    virtual void AfterChildWaitLoop(void *WXUNUSED(data)) wxOVERRIDE { }
#if wxUSE_TIMER
    virtual wxTimerImpl *CreateTimerImpl(wxTimer *timer) wxOVERRIDE;
#endif

#if wxUSE_THREADS && defined(__WXGTK20__)
    virtual void MutexGuiEnter() wxOVERRIDE;
    virtual void MutexGuiLeave() wxOVERRIDE;
#endif

#if wxUSE_THREADS
    virtual bool DoMessageFromThreadWait() wxOVERRIDE { return true; }
    virtual WXDWORD WaitForThread(WXHANDLE hThread, int WXUNUSED(flags)) wxOVERRIDE
        { return DoSimpleWaitForThread(hThread); }
#endif // wxUSE_THREADS
    virtual wxPortId GetToolkitVersion(int *majVer = NULL,
                                       int *minVer = NULL,
                                       int *microVer = NULL) const wxOVERRIDE;

    virtual bool CanUseStderr() wxOVERRIDE { return false; }
    virtual bool WriteToStderr(const wxString& WXUNUSED(text)) wxOVERRIDE
        { return false; }
    virtual WXHWND GetMainHWND() const wxOVERRIDE { return NULL; }
};

#elif defined(__WXQT__)

class WXDLLIMPEXP_CORE wxGUIAppTraits : public wxGUIAppTraitsBase
{
public:
    virtual wxEventLoopBase *CreateEventLoop() wxOVERRIDE;
    virtual void *BeforeChildWaitLoop() wxOVERRIDE { return NULL; }
    virtual void AfterChildWaitLoop(void*) wxOVERRIDE { }
#if wxUSE_TIMER
    virtual wxTimerImpl *CreateTimerImpl(wxTimer *timer) wxOVERRIDE;
#endif

#if wxUSE_THREADS
    virtual bool DoMessageFromThreadWait() wxOVERRIDE { return true; }
    virtual WXDWORD WaitForThread(WXHANDLE hThread, int WXUNUSED(flags)) wxOVERRIDE
        { return DoSimpleWaitForThread(hThread); }
#endif // wxUSE_THREADS
    virtual wxPortId GetToolkitVersion(int *majVer = NULL,
                                       int *minVer = NULL,
                                       int *microVer = NULL) const wxOVERRIDE;

    virtual bool CanUseStderr() wxOVERRIDE { return false; }
    virtual bool WriteToStderr(const wxString&) wxOVERRIDE { return false; }
    virtual WXHWND GetMainHWND() const wxOVERRIDE { return NULL; }
};

#endif

#endif // wxUSE_GUI

#endif // _WX_MSW_APPTRAIT_H_
