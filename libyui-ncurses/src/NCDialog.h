/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       NCDialog.h

   Author:     Michael Andres <ma@suse.de>

/-*/
#ifndef NCDialog_h
#define NCDialog_h

#include <iosfwd>

#include "YDialog.h"
#include "NCWidget.h"
#include "NCPushButton.h"


class NCDialog;
class NCPopupInfo;


class NCDialog : public YDialog, public NCWidget {

  friend std::ostream & operator<<( std::ostream & STREAM, const NCDialog & OBJ );
  friend std::ostream & operator<<( std::ostream & STREAM, const NCDialog * OBJ );

  NCDialog & operator=( const NCDialog & );
  NCDialog            ( const NCDialog & );

  private:

    typedef tnode<NCWidget *> * (tnode<NCWidget *>::* SeekDir)( const bool );

    NCWidget & GetNormal( NCWidget & startwith, SeekDir Direction );
    void       Activate( SeekDir Direction );

    void _init();
    void _init_size();

  protected:

    virtual const char * location() const { return "NCDialog"; }

  private:

    NCursesUserPanel<NCDialog> * pan;
    NCstyle::StyleSet            mystyleset;
    const NCstyle::Style *       dlgstyle;

    unsigned inMultiDraw_i;

    bool            active;
    NCWidget *const wActive;

    NCursesEvent pendingEvent;
    YEvent::EventReason eventReason;

    NCPopupInfo *helpPopup;

    // wrapper for wHandle... calls in processInput()
    NCursesEvent getInputEvent( wint_t ch );
    NCursesEvent getHotkeyEvent( wint_t key );

  private:

    void grabActive( NCWidget * nactive );
    virtual void grabNotify( NCWidget * mgrab );
    virtual bool wantFocus( NCWidget & ngrab );

    virtual void wCreate( const wrect & newrect );
    virtual void wMoveTo( const wpos & newpos );
    virtual void wDelete();
    virtual void wRedraw();
    virtual void wRecoded();
    virtual void wUpdate( bool forced_br = false );
    void doUpdate() { wUpdate( true ); }

    NCWidget & GetNextNormal( NCWidget & startwith );
    NCWidget & GetPrevNormal( NCWidget & startwith );

    bool Activate( NCWidget & nactive );
    void Activate();
    void Deactivate();
    void ActivateNext();
    void ActivatePrev();

    bool ActivateByKey( int key );

    void processInput( int timeout_millisec );

    std::map<int, string> describeFunctionKeys();

    wint_t getinput();		// get the input (respect terminal encoding)

    bool flushTypeahead();
    
  protected:

    wint_t getch( int timeout_millisec = -1 );

    virtual NCursesEvent wHandleInput( wint_t ch );
    virtual NCursesEvent wHandleHotkey( wint_t key );

    virtual void startMultipleChanges();
    virtual void doneMultipleChanges();
    
    /**
     * Internal open() method: Initialize what is left over to initialize after
     * all dialog children have been created.
     * YDialog::setInitialSize() is already called before this in
     * YDailog::open(), so don't call it here again (very expensive!).
     *
     * This function is called (exactly once during the life time of the
     * dialog) in YDialog::open(). 
     *
     * Implemented from YDialog.
     **/
    virtual void openInternal();

    /**
     * Wait for a user event.
     *
     * Implemented from YDialog.
     **/
    virtual YEvent * waitForEventInternal( int timeout_millisec );
    
    /**
     * Check if a user event is pending. If there is one, return it.
     * If there is none, do not wait for one - return 0.
     *
     * Implemented from YDialog.
     **/
    virtual YEvent * pollEventInternal();


  public:

    NCDialog( YDialogType 	dialogType,
	      YDialogColorMode	colorMode = YDialogNormalColor );
    virtual ~NCDialog();

    void showDialog();
    void closeDialog();

    void activate( const bool newactive );
    bool isActive() const { return active; }

    void idleInput();

    NCursesEvent userInput( int timeout_millisec = -1 );
    NCursesEvent pollInput();

    //virtual long nicesize( YUIDimension dim );
    virtual int preferredWidth();
    virtual int preferredHeight();
    
    /**
     * Set the new size of the widget.
     *
     * Reimplemented from YWidget.
     **/
    virtual void setSize( int newWidth, int newHeight );

    /**
     * Activate this dialog: Make sure that it is shown as the topmost dialog
     * of this application and that it can receive input.
     *
     * Implemented from YDialog.
     **/
    virtual void activate();

  protected:

    enum NCDopts {
      DEFAULT = 0x00,
      POPUP   = 0x01,
      NOBOX   = 0x10
    };

    typedef unsigned NCDoptflag;

    NCDoptflag ncdopts;
    wpos       popedpos;
    bool       hshaddow;
    bool       vshaddow;

    NCDialog( YDialogType dialogType, const wpos at, const bool boxed = true );

    bool isPopup() const { return  (ncdopts & POPUP); }
    bool isBoxed() const { return !(ncdopts & NOBOX); }

    virtual void initDialog();

    virtual const NCstyle::Style & wStyle() const {
      return dlgstyle ? *dlgstyle : NCurses::style()[NCstyle::DefaultStyle];
    }

    //virtual void setEnabling( bool do_bv ) { /*NOP*/ }
    virtual void setEnabled( bool do_bv ) { /*NOP*/ };
    
  private:

    friend class NCurses;
    bool getInvisible();
    bool getVisible();
    void resizeEvent();
};


#endif // NCDialog_h