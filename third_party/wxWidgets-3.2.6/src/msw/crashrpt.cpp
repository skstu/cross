/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/crashrpt.cpp
// Purpose:     code to generate crash dumps (minidumps)
// Author:      Vadim Zeitlin
// Modified by:
// Created:     13.07.03
// Copyright:   (c) 2003 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_CRASHREPORT

#ifndef WX_PRECOMP
    #include "wx/wxcrtvararg.h"
#endif  //WX_PRECOMP

#include "wx/msw/debughlp.h"
#include "wx/msw/crashrpt.h"
#include "wx/msw/private.h"

// ----------------------------------------------------------------------------
// classes
// ----------------------------------------------------------------------------

// low level wxBusyCursor replacement: we use Win32 API directly here instead
// of going through wxWidgets calls as this could be dangerous
class BusyCursor
{
public:
    BusyCursor()
    {
        HCURSOR hcursorBusy = ::LoadCursor(NULL, IDC_WAIT);
        m_hcursorOld = ::SetCursor(hcursorBusy);
    }

    ~BusyCursor()
    {
        if ( m_hcursorOld )
        {
            ::SetCursor(m_hcursorOld);
        }
    }

private:
    HCURSOR m_hcursorOld;
};

// the real crash report generator
class wxCrashReportImpl
{
public:
    wxCrashReportImpl(const wxChar *filename);

    bool Generate(int flags, EXCEPTION_POINTERS *ep);

    ~wxCrashReportImpl()
    {
        if ( m_hFile != INVALID_HANDLE_VALUE )
        {
            ::CloseHandle(m_hFile);
        }
    }

private:

    // formatted output to m_hFile
    void Output(const wxChar *format, ...);

    // output end of line
    void OutputEndl() { Output(wxT("\r\n")); }

    // the handle of the report file
    HANDLE m_hFile;
};

// ----------------------------------------------------------------------------
// globals
// ----------------------------------------------------------------------------

// the file name where the report about exception is written
//
// we use fixed buffer to avoid (big) dynamic allocations when the program
// crashes
static wxChar gs_reportFilename[MAX_PATH];

// this is defined in msw/main.cpp
extern EXCEPTION_POINTERS *wxGlobalSEInformation;

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxCrashReportImpl
// ----------------------------------------------------------------------------

wxCrashReportImpl::wxCrashReportImpl(const wxChar *filename)
{
    m_hFile = ::CreateFile
                (
                    filename,
                    GENERIC_WRITE,
                    0,                          // no sharing
                    NULL,                       // default security
                    CREATE_ALWAYS,
                    FILE_FLAG_WRITE_THROUGH,
                    NULL                        // no template file
                );
}

void wxCrashReportImpl::Output(const wxChar *format, ...)
{
    va_list argptr;
    va_start(argptr, format);

    DWORD cbWritten;

    wxString s = wxString::FormatV(format, argptr);

    wxCharBuffer buf(s.mb_str(wxConvUTF8));
    ::WriteFile(m_hFile, buf.data(), strlen(buf.data()), &cbWritten, 0);

    va_end(argptr);
}

bool wxCrashReportImpl::Generate(int flags, EXCEPTION_POINTERS *ep)
{
    if ( m_hFile == INVALID_HANDLE_VALUE )
        return false;

#if wxUSE_DBGHELP
    if ( !ep )
        ep = wxGlobalSEInformation;

    if ( !ep )
    {
        Output(wxT("Context for crash report generation not available."));
        return false;
    }

    // show to the user that we're doing something...
    BusyCursor busyCursor;

    // user-specified crash report flags override those specified by the
    // programmer
    TCHAR envFlags[64];
    DWORD dwLen = ::GetEnvironmentVariable
                    (
                        wxT("WX_CRASH_FLAGS"),
                        envFlags,
                        WXSIZEOF(envFlags)
                    );

    int flagsEnv;
    if ( dwLen && dwLen < WXSIZEOF(envFlags) &&
            wxSscanf(envFlags, wxT("%d"), &flagsEnv) == 1 )
    {
        flags = flagsEnv;
    }

    if ( wxDbgHelpDLL::Init() )
    {
        MINIDUMP_EXCEPTION_INFORMATION minidumpExcInfo;

        minidumpExcInfo.ThreadId = ::GetCurrentThreadId();
        minidumpExcInfo.ExceptionPointers = ep;
        minidumpExcInfo.ClientPointers = FALSE; // in our own address space

        // do generate the dump
        MINIDUMP_TYPE dumpFlags;
        if ( flags & wxCRASH_REPORT_LOCALS )
        {
            // the only way to get local variables is to dump the entire
            // process memory space -- but this makes for huge (dozens or
            // even hundreds of Mb) files
            dumpFlags = MiniDumpWithFullMemory;
        }
        else if ( flags & wxCRASH_REPORT_GLOBALS )
        {
            // MiniDumpWriteDump() has the option for dumping just the data
            // segment which contains all globals -- exactly what we need
            dumpFlags = MiniDumpWithDataSegs;
        }
        else // minimal dump
        {
            // the file size is not much bigger than when using MiniDumpNormal
            // if we use the flags below, but the minidump is much more useful
            // as it contains the values of many (but not all) local variables
            dumpFlags = (MINIDUMP_TYPE)(MiniDumpScanMemory
                                        |MiniDumpWithIndirectlyReferencedMemory
                                        );
        }

        if ( !wxDbgHelpDLL::MiniDumpWriteDump
              (
                ::GetCurrentProcess(),
                ::GetCurrentProcessId(),
                m_hFile,                    // file to write to
                dumpFlags,                  // kind of dump to craete
                &minidumpExcInfo,
                NULL,                       // no extra user-defined data
                NULL                        // no callbacks
              ) )
        {
            Output(wxT("MiniDumpWriteDump() failed."));

            return false;
        }

        return true;
    }
    else // dbghelp.dll couldn't be loaded
    {
        Output(wxT("%s"), static_cast<const wxChar*>(
                    wxDbgHelpDLL::GetErrorMessage().c_str()
              ));
    }
#else // !wxUSE_DBGHELP
    wxUnusedVar(flags);
    wxUnusedVar(ep);

    Output(wxT("Support for crash report generation was not included ")
           wxT("in this wxWidgets version."));
#endif // wxUSE_DBGHELP/!wxUSE_DBGHELP

    return false;
}

// ----------------------------------------------------------------------------
// wxCrashReport
// ----------------------------------------------------------------------------

/* static */
void wxCrashReport::SetFileName(const wxString& filename)
{
    wxStrlcpy(gs_reportFilename, filename.t_str(), WXSIZEOF(gs_reportFilename));
}

/* static */
wxString wxCrashReport::GetFileName()
{
    return gs_reportFilename;
}

/* static */
bool wxCrashReport::Generate(int flags, EXCEPTION_POINTERS *ep)
{
    wxCrashReportImpl impl(gs_reportFilename);

    return impl.Generate(flags, ep);
}

/* static */
bool wxCrashReport::GenerateNow(int flags)
{
    bool rc = false;

    __try
    {
        RaiseException(0x1976, 0, 0, NULL);
    }
    __except( rc = Generate(flags, (EXCEPTION_POINTERS *)GetExceptionInformation()),
              EXCEPTION_CONTINUE_EXECUTION )
    {
        // never executed because of EXCEPTION_CONTINUE_EXECUTION above
    }

    return rc;
}

// ----------------------------------------------------------------------------
// wxCrashContext
// ----------------------------------------------------------------------------

wxCrashContext::wxCrashContext(_EXCEPTION_POINTERS *ep)
{
    wxZeroMemory(*this);

    if ( !ep )
    {
        wxCHECK_RET( wxGlobalSEInformation, wxT("no exception info available") );
        ep = wxGlobalSEInformation;
    }

    // TODO: we could also get the operation (read/write) and address for which
    //       it failed for EXCEPTION_ACCESS_VIOLATION code
    const EXCEPTION_RECORD& rec = *ep->ExceptionRecord;
    code = rec.ExceptionCode;
    addr = rec.ExceptionAddress;

#ifdef __INTEL__
    const CONTEXT& ctx = *ep->ContextRecord;
    regs.eax = ctx.Eax;
    regs.ebx = ctx.Ebx;
    regs.ecx = ctx.Ecx;
    regs.edx = ctx.Edx;
    regs.esi = ctx.Esi;
    regs.edi = ctx.Edi;

    regs.ebp = ctx.Ebp;
    regs.esp = ctx.Esp;
    regs.eip = ctx.Eip;

    regs.cs = ctx.SegCs;
    regs.ds = ctx.SegDs;
    regs.es = ctx.SegEs;
    regs.fs = ctx.SegFs;
    regs.gs = ctx.SegGs;
    regs.ss = ctx.SegSs;

    regs.flags = ctx.EFlags;
#endif // __INTEL__
}

wxString wxCrashContext::GetExceptionString() const
{
    wxString s;

    #define CASE_EXCEPTION( x ) case EXCEPTION_##x: s = wxT(#x); break

    switch ( code )
    {
        CASE_EXCEPTION(ACCESS_VIOLATION);
        CASE_EXCEPTION(DATATYPE_MISALIGNMENT);
        CASE_EXCEPTION(BREAKPOINT);
        CASE_EXCEPTION(SINGLE_STEP);
        CASE_EXCEPTION(ARRAY_BOUNDS_EXCEEDED);
        CASE_EXCEPTION(FLT_DENORMAL_OPERAND);
        CASE_EXCEPTION(FLT_DIVIDE_BY_ZERO);
        CASE_EXCEPTION(FLT_INEXACT_RESULT);
        CASE_EXCEPTION(FLT_INVALID_OPERATION);
        CASE_EXCEPTION(FLT_OVERFLOW);
        CASE_EXCEPTION(FLT_STACK_CHECK);
        CASE_EXCEPTION(FLT_UNDERFLOW);
        CASE_EXCEPTION(INT_DIVIDE_BY_ZERO);
        CASE_EXCEPTION(INT_OVERFLOW);
        CASE_EXCEPTION(PRIV_INSTRUCTION);
        CASE_EXCEPTION(IN_PAGE_ERROR);
        CASE_EXCEPTION(ILLEGAL_INSTRUCTION);
        CASE_EXCEPTION(NONCONTINUABLE_EXCEPTION);
        CASE_EXCEPTION(STACK_OVERFLOW);
        CASE_EXCEPTION(INVALID_DISPOSITION);
        CASE_EXCEPTION(GUARD_PAGE);
        CASE_EXCEPTION(INVALID_HANDLE);

        default:
            // unknown exception, ask NTDLL for the name
            s = wxMSWFormatMessage(code, ::GetModuleHandle(wxT("NTDLL.DLL")));
    }

    #undef CASE_EXCEPTION

    return s;
}

#endif // wxUSE_CRASHREPORT/!wxUSE_CRASHREPORT

