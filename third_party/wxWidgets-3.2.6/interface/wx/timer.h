/////////////////////////////////////////////////////////////////////////////
// Name:        timer.h
// Purpose:     interface of wxTimer
// Author:      wxWidgets team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// generate notifications periodically until the timer is stopped (default)
#define wxTIMER_CONTINUOUS false

// only send the notification once and then stop the timer
#define wxTIMER_ONE_SHOT true

wxEventType wxEVT_TIMER;


/**
    @class wxTimer

    The wxTimer class allows you to execute code at specified intervals.
    Its precision is platform-dependent, but in general will not be better than
    @c 1ms nor worse than @c 1s.

    There are three different ways to use this class:

    - You may derive a new class from wxTimer and override the
      wxTimer::Notify member to perform the required action.
    - You may redirect the notifications to any wxEvtHandler derived object by
      using the non-default constructor or wxTimer::SetOwner.
      Then use the @c EVT_TIMER macro to connect it to the event handler which
      will receive wxTimerEvent notifications.
    - You may use a derived class and the @c EVT_TIMER macro to connect it to
      an event handler defined in the derived class. If the default constructor
      is used, the timer object will be its own owner object, since it is
      derived from wxEvtHandler.

    In any case, you must start the timer with wxTimer::Start() after constructing
    it before it actually starts sending notifications.
    It can be stopped later with wxTimer::Stop().

    @note A timer can only be used from the main thread.

    @library{wxbase}
    @category{misc}

    @see wxStopWatch
*/
class wxTimer : public wxEvtHandler
{
public:
    /**
        Default constructor.
        If you use it to construct the object and don't call SetOwner() later,
        you must override Notify() method to process the notifications.
    */
    wxTimer();

    /**
        Creates a timer and associates it with @a owner.
        Please see SetOwner() for the description of parameters.
    */
    wxTimer(wxEvtHandler* owner, int id = -1);

    /**
        Destructor. Stops the timer if it is running.
    */
    virtual ~wxTimer();

    /**
        Returns the ID of the events generated by this timer.
    */
    int GetId() const;

    /**
        Returns the current interval for the timer (in milliseconds).
    */
    int GetInterval() const;

    /**
        Returns the current @e owner of the timer.

        If non-@NULL this is the event handler which will receive the
        timer events (see wxTimerEvent) when the timer is running.
    */
    wxEvtHandler* GetOwner() const;

    /**
        Returns @true if the timer is one shot, i.e.\ if it will stop after firing
        the first notification automatically.
    */
    bool IsOneShot() const;

    /**
        Returns @true if the timer is running, @false if it is stopped.
    */
    bool IsRunning() const;

    /**
        This member should be overridden by the user if the default constructor was
        used and SetOwner() wasn't called.

        Perform whatever action which is to be taken periodically here.

        Notice that throwing exceptions from this method is currently not
        supported, use event-based timer handling approach if an exception can
        be thrown while handling timer notifications.
    */
    virtual void Notify();

    /**
        Associates the timer with the given @a owner object.

        When the timer is running, the owner will receive timer events (see wxTimerEvent)
        with @a id equal to @a id specified here.
    */
    void SetOwner(wxEvtHandler* owner, int id = -1);

    /**
        (Re)starts the timer. If @a milliseconds parameter is -1 (value by default),
        the previous value is used. Returns @false if the timer could not be started,
        @true otherwise (in MS Windows timers are a limited resource).

        If @a oneShot is @false (the default), the Notify() function will be called
        repeatedly until the timer is stopped.
        If @true, it will be called only once and the timer will stop automatically.

        To make your code more readable you may also use the following symbolic constants:
        - wxTIMER_CONTINUOUS: Start a normal, continuously running, timer
        - wxTIMER_ONE_SHOT: Start a one shot timer
        Alternatively, use StartOnce().

        If the timer was already running, it will be stopped by this method before
        restarting it.
    */
    virtual bool Start(int milliseconds = -1, bool oneShot = wxTIMER_CONTINUOUS);

    /**
        Starts the timer for a once-only notification.

        This is a simple wrapper for Start() with @c wxTIMER_ONE_SHOT parameter.

        @since 2.9.5
     */
    bool StartOnce(int milliseconds = -1);

    /**
        Stops the timer.
    */
    virtual void Stop();
};


/**
   @class wxTimerRunner

   Starts the timer in its ctor, stops in the dtor.
*/
class wxTimerRunner
{
public:
    wxTimerRunner(wxTimer& timer);
    wxTimerRunner(wxTimer& timer, int milli, bool oneShot = false);
    void Start(int milli, bool oneShot = false);
    ~wxTimerRunner();
};

/**
    @class wxTimerEvent

    wxTimerEvent object is passed to the event handler of timer events
    (see wxTimer::SetOwner).

    For example:

    @code
    class MyFrame : public wxFrame
    {
    public:
        ...
        void OnTimer(wxTimerEvent& event);

    private:
        wxTimer m_timer;
        wxDECLARE_EVENT_TABLE();
    };

    wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
        EVT_TIMER(TIMER_ID, MyFrame::OnTimer)
    wxEND_EVENT_TABLE()

    MyFrame::MyFrame()
           : m_timer(this, TIMER_ID)
    {
        m_timer.Start(1000);    // 1 second interval
    }

    void MyFrame::OnTimer(wxTimerEvent& event)
    {
        // do whatever you want to do every second here
    }
    @endcode

    @library{wxbase}
    @category{events}

    @see wxTimer
*/
class wxTimerEvent : public wxEvent
{
public:
    wxTimerEvent(wxTimer& timer);

    /**
        Returns the interval of the timer which generated this event.
    */
    int GetInterval() const;

    /**
        Returns the timer object which generated this event.
    */
    wxTimer& GetTimer() const;
};

