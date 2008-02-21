/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:		YUI.h

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YUI_h
#define YUI_h

#include <pthread.h>
#include <string>

#include "YTypes.h"

using std::string;

class YApplication;
class YWidget;
class YWidgetFactory;
class YOptionalWidgetFactory;
class YEvent;
class YBuiltinCaller;
class YDialog;
class YMacroPlayer;
class YMacroRecorder;


/**
 * Abstract base class of a YaST2 user interface.
 **/
class YUI
{
    friend class YUIFunction;

protected:
    /**
     * Constructor.
     **/
    YUI( bool withThreads );

public:

    /**
     * Destructor.
     **/
    virtual ~YUI();

    /**
     * Shut down multithreading. This needs to be called before the destructor
     * if the UI was created with threads. If the UI was created without
     * threads, this does nothing.
     **/
    void shutdownThreads();

    /**
     * Access the global UI.
     **/
    static YUI * ui();

    /**
     * Return the widget factory that provides all the createXY() methods for
     * standard (mandatory, i.e. non-optional) widgets.
     *
     * This will create the factory upon the first call and return a pointer to
     * the one and only (singleton) factory upon each subsequent call.
     * This may throw exceptions if the factory cannot be created.
     **/
    static YWidgetFactory * widgetFactory();

    /**
     * Return the widget factory that provides all the createXY() methods for
     * optional ("special") widgets and the corresponding hasXYWidget()
     * methods.
     *
     * This will create the factory upon the first call and return a pointer to
     * the one and only (singleton) factory upon each subsequent call.
     * This may throw exceptions if the factory cannot be created.
     **/
    static YOptionalWidgetFactory * optionalWidgetFactory();

    /**
     * Return the global YApplication object.
     *
     * This will create the YApplication upon the first call and return a
     * pointer to the one and only (singleton) YApplication upon each
     * subsequent call.  This may throw exceptions if the YApplication cannot
     * be created.
     **/
    static YApplication * app();

    /**
     * Aliases for YUI::app()
     **/
    static YApplication * application() { return app(); }
    static YApplication * yApp()	{ return app(); }

    /**
     * Make sure there is a UI (with a UI plug-in) created.
     *
     * If there is none yet, this will use all-default parameters to load a UI
     * plug-in and create a UI (without threads).
     **/
    static void ensureUICreated();


protected:
    /**
     * Create the widget factory that provides all the createXY() methods for
     * standard (mandatory, i.e. non-optional) widgets.
     *
     * Derived classes are required to implement this.
     **/
    virtual YWidgetFactory * createWidgetFactory() = 0;

    /**
     * Create the widget factory that provides all the createXY() methods for
     * optional ("special") widgets and the corresponding hasXYWidget()
     * methods.
     *
     * Derived classes are required to implement this.
     **/
    virtual YOptionalWidgetFactory * createOptionalWidgetFactory() = 0;

    /**
     * Create the YApplication object that provides global methods.
     *
     * Derived classes are required to implement this.
     **/
    virtual YApplication * createApplication() = 0;

public:

    /**
     * Block (or unblock) events. If events are blocked, any event sent
     * should be ignored until events are unblocked again.
     *
     * This default implementation keeps track of a simple internal flag that
     * can be queried with eventsBlocked(), so if you reimplement
     * blockEvents(), be sure to reimplement eventsBlocked() as well.
     **/
    virtual void blockEvents( bool block = true ) { _eventsBlocked = block; }

    /**
     * Unblock events previously blocked. This is just an alias for
     * blockEvents( false) for better readability.
     *
     * Note: This method is intentionally not virtual.
     **/
    void unblockEvents() { blockEvents( false ); }

    /**
     * Returns 'true' if events are currently blocked.
     *
     * Reimplent this if you reimplement blockEvents().
     **/
    virtual bool eventsBlocked() const { return _eventsBlocked; }

    /**
     * Must be called after the constructor of the Qt/NCurses ui
     * is ready. Starts the ui thread.
     **/
    void topmostConstructorHasFinished();

    /**
     * Running with threads?
     **/
    bool runningWithThreads() const { return _withThreads; }

    /**
     * This method implements the UI thread in case it is existing.
     * The loop consists of calling idleLoop, getting the next
     * command from the @ref YCPUIComponent, evaluating it, which
     * possibly invovles calling userInput() or pollInput()
     * and writes the answer back to the other thread where the request
     * came from.
     **/
    void uiThreadMainLoop();

    /**
     * Return the transparent inter-thread communication.
     * This will return 0 until set from the outside.
     **/
    YBuiltinCaller * builtinCaller() const { return _builtinCaller; }

    /**
     * Set the transparent inter-thread communication.
     * Built-ins are only really called if there is a valid YBuiltinCaller set.
     **/
    void setBuiltinCaller( YBuiltinCaller * caller )
	{ _builtinCaller = caller; }

    /**
     * UI-specific runPkgSelection method.
     *
     * Derived classes are required to implement this.
     **/
    virtual YEvent * runPkgSelection( YWidget * packageSelector ) = 0;


protected:

    /**
     * This virtual method is called when threads are activated in case the
     * execution control is currently on the side of the module.  This means
     * that no UserInput() or PollInput() is pending. The module just does some
     * work. The UI <-> module protocol is in the "UI waits for the next
     * command" state. The UI can override this method when it wants to react
     * to user input or other external events such as repaint requests from the
     * X server.
     *
     * 'fd_ycp' file descriptor that should be used to determine when
     * to leave the idle loop. As soon as it is readable, the loop must
     * be left. In order to avoid polling you can combine it with other
     * ui-specific fds and do a common select() call.
     **/
    virtual void idleLoop( int fd_ycp ) = 0;

    /**
     * Tells the ui thread that it should terminate and waits
     * until it does so.
     **/
    void terminateUIThread();

    /**
     * Creates and launches the ui thread.
     **/
    void createUIThread();
    friend void *start_ui_thread( void *ui_int );

    /**
     * Signals the ui thread by sending one byte through the pipe
     * to it.
     **/
    void signalUIThread();

    /**
     * Waits for the ui thread to send one byte through the pipe
     * to the ycp thread and reads this byte from the pipe.
     **/
    bool waitForUIThread();

    /**
     * Signals the ycp thread by sending one byte through the pipe
     * to it.
     **/
    void signalYCPThread();

    /**
     * Waits for the ycp thread to send one byte through the pipe
     * to the ycp thread and reads this byte from the pipe.
     **/
    bool waitForYCPThread();

    /**
     * Check if debug logging is enabled.
     **/
    bool debugLoggingEnabled() const;

    /**
     * Enable or disable debug logging.
     * This will propagate the parameter to YUILog::enableDebugLogging(),
     * but it might do more than just that.
     **/
    void enableDebugLogging( bool enable = true );


    //
    // Data members
    //

    /**
     * true if a seperate UI thread is created
     **/
    bool _withThreads;

    /**
     * Handle to the ui thread.
     **/
    pthread_t _uiThread;

    /**
     * Inter-thread communication between the YCP thread and the UI thread:
     * The YCP thread supplies data here and signals the UI thread,
     * the UI thread picks up the data, executes the function, puts
     * the result here and signals the YCP thread that waits until
     * the result is available.
     **/
    YBuiltinCaller * _builtinCaller;

    /**
     * Used to synchronize data transfer with the ui thread.
     * It stores a pair of file descriptors of a pipe. For each YCP value
     * we send to the ui thread, we write one aribrary byte here.
     **/
    int pipe_to_ui[2];

    /**
     * Used to synchronize data transfer with the ui thread.
     * It stores a pair of file descriptors of a pipe. For each YCP value
     * we get from the ui thread, we read one aribrary byte from here.
     **/
    int pipe_from_ui[2];

    /**
     * This is a flag that signals the ui thread that it should
     * terminate. This is done by setting the flag to true. The ui
     * thread replies by setting the flag back to false directly
     * after terminating itself.
     **/
    bool _terminate_ui_thread;

    /**
     * Flag that keeps track of blocked events.
     * Never query this directly, use eventsBlocked() instead.
     **/
    bool _eventsBlocked;

private:
    
    static YUI * _ui;
};



#endif // YUI_h