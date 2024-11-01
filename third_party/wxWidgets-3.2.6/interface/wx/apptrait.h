/////////////////////////////////////////////////////////////////////////////
// Name:        apptrait.h
// Purpose:     interface of wxAppTraits
// Author:      wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

/**
    @class wxAppTraits

    The wxAppTraits class defines various configurable aspects of a wxApp.
    You can access it using wxApp::GetTraits() function and you can create your
    own wxAppTraits overriding the wxApp::CreateTraits() function.

    Note that wxAppTraits is an abstract class since it contains many
    pure virtual functions.
    In fact, by default, wxWidgets creates a @c wxConsoleAppTraits object for
    console applications (i.e. those applications linked against wxBase library
    only - see the @ref page_libs page) and a @c wxGUIAppTraits object for GUI
    applications.
    Both these classes are derived by wxAppTraits and represent concrete
    implementation of the wxAppTraits interface.

    @library{wxbase}
    @category{cfg}

    @see @ref overview_app, wxApp
*/
class wxAppTraits
{
public:
    /**
        Called by wxWidgets to create the default configuration object for the
        application. The default version creates a registry-based wxRegConfig
        class under MSW and wxFileConfig under all other platforms.

        The wxApp::GetAppName and wxApp::GetVendorName methods are used to
        determine the registry key or file name.
    */
    virtual wxConfigBase* CreateConfig();

    /**
        Used by wxWidgets to create the main event loop used by wxApp::OnRun().

        The default implementation of this method in wxGUIAppTraits returns the
        usual platform-specific GUI event loop. The version in wxConsoleAppTraits
        returns a console-specific event loop which can be used to handle timer
        and socket events in console programs under Unix and MSW or @NULL under
        the other platforms where console event loops are not supported yet.
     */
    virtual wxEventLoopBase *CreateEventLoop() = 0;

    /**
        Creates the global font mapper object used for encodings/charset mapping.
    */
    virtual wxFontMapper* CreateFontMapper() = 0;

    /**
        Creates a wxLog class for the application to use for logging errors.
        The default implementation returns a new wxLogGui class.

        @see wxLog
    */
    virtual wxLog* CreateLogTarget() = 0;

    /**
        Creates the global object used for printing out messages.
    */
    virtual wxMessageOutput* CreateMessageOutput() = 0;

    /**
        Returns the renderer to use for drawing the generic controls (return
        value may be @NULL in which case the default renderer for the current
        platform is used); this is used in GUI mode only and always returns @NULL
        in console.

        @note the returned pointer needs to be deleted by the caller.
    */
    virtual wxRendererNative* CreateRenderer() = 0;

    /**
        Returns the name of the desktop environment currently running on a Unix
        desktop. It returns an empty string for platforms other than wxGTK, or
        if the desktop environment could not be determined.
    */
    virtual wxString GetDesktopEnvironment() const = 0;

    /**
        Returns the wxStandardPaths object for the application.
        It's normally the same for wxBase and wxGUI except in the case of wxMac
        and wxCocoa.

        @note
        The returned reference is to a @c wxStandardPathsBase class but you
        can consider it to be equivalent to wxStandardPaths (which is documented).
    */
    virtual wxStandardPaths& GetStandardPaths();

    /**
        Returns the wxWidgets port ID used by the running program and eventually
        fills the given pointers with the values of the major, minor, and micro
        digits of the native toolkit currently used.

        The version numbers returned are thus detected at run-time and not compile-time
        (except when this is not possible e.g. wxMotif).

        E.g. if your program is using wxGTK port this function will return wxPORT_GTK
        and put in given pointers the versions of the GTK library in use.
        See wxPlatformInfo for more details.

        If a micro version is not available it will have a value of 0.
    */
    virtual wxPortId GetToolkitVersion(int* major = NULL,
                                       int* minor = NULL,
                                       int* micro = NULL) const = 0;

    /**
        Returns @true if @c fprintf(stderr) goes somewhere, @false otherwise.
    */
    virtual bool HasStderr() = 0;

    /**
        Returns @true if the library was built as wxUniversal.
        Always returns @false for wxBase-only apps.
    */
    virtual bool IsUsingUniversalWidgets() const = 0;

    /**
        Shows the assert dialog with the specified message in GUI mode or just prints
        the string to stderr in console mode.
        Returns @true to suppress subsequent asserts, @false to continue as before.
    */
    virtual bool ShowAssertDialog(const wxString& msg) = 0;

    /**
        Shows a message box with the given text and title if possible.

        In some ports, e.g. wxMSW, a message box will always be shown, while in
        the other ones it will be only done if the GUI is available (e.g. X11
        display was successfully opened for X11-based ports) and the function
        simply returns @false without doing anything otherwise.

        This function is safe in the sense that it can always be called, even
        before creating wxApp, similarly to wxSafeShowMessage() which is
        implemented by calling this function and then logging the message to
        standard error stream if it returned @false.

        @since 3.1.5

        @param text
            The text to show to the user.
        @param title
            The title of the message box shown to the user.
        @return @true if the message box was shown or @false otherwise.
     */
    virtual bool SafeMessageBox(const wxString& text, const wxString& title) = 0;

    /**
        Helper function mostly useful for derived classes ShowAssertDialog()
        implementation.

        Returns the stack frame as a plain (and possibly empty) wxString.

        This function is only available when @c wxUSE_STACKWALKER is 1.

        @since 3.1.5
     */
    virtual wxString GetAssertStackTrace();
};

