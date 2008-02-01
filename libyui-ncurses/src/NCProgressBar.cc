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

   File:       NCProgressBar.cc

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/
#include "Y2Log.h"
#include "NCurses.h"
#include "NCProgressBar.h"

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : NCProgressBar::NCProgressBar
//	METHOD TYPE : Constructor
//
//	DESCRIPTION :
//
NCProgressBar::NCProgressBar( YWidget * parent,
			      const string & nlabel,
			      int maxValue )
    : YProgressBar( parent, nlabel, maxValue )
    , NCWidget( parent )
    , label( nlabel )
    , maxval( maxValue )
    , cval( 0 )
    , lwin( 0 )
    , twin( 0 )
{
  WIDDBG << endl;
  if ( maxval <= 0 )
    maxval = 1;
  hotlabel = &label;
  setLabel( nlabel );
  // initial progress isn't an argument any longer 
  //setProgress( progress );
  wstate = NC::WSdumb;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : NCProgressBar::~NCProgressBar
//	METHOD TYPE : Destructor
//
//	DESCRIPTION :
//
NCProgressBar::~NCProgressBar()
{
  delete lwin;
  delete twin;
  WIDDBG << endl;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : NCProgressBar::nicesize
//	METHOD TYPE : long
//
//	DESCRIPTION :
//
long NCProgressBar::nicesize( YUIDimension dim )
{
  return dim == YD_HORIZ ? wGetDefsze().W : wGetDefsze().H;
}

int NCProgressBar::preferredWidth()
{
     return wGetDefsze().W;
}

int NCProgressBar::preferredHeight()
{
    return wGetDefsze().H;
}

void NCProgressBar::setEnabled( bool do_bv )
{
    NCWidget::setEnabled( do_bv );
    YProgressBar::setEnabled( do_bv );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : NCProgressBar::setSize
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void NCProgressBar::setSize( int newwidth, int newheight )
{
  wRelocate( wpos( 0 ), wsze( newheight, newwidth ) );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : NCProgressBar::setDefsze
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void NCProgressBar::setDefsze()
{
  defsze = wsze( label.height() + 1,
		 label.width() < 5 ? 5 : label.width() );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : NCProgressBar::wCreate
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void NCProgressBar::wCreate( const wrect & newrect )
{
  NCWidget::wCreate( newrect );

  if ( !win )
      return;

  wrect lrect( 0, wsze::min( newrect.Sze,
			     wsze( label.height(), newrect.Sze.W ) ) );
  wrect trect( 0, wsze( 1, newrect.Sze.W ) );

  if ( lrect.Sze.H == newrect.Sze.H )
    lrect.Sze.H -= 1;

  trect.Pos.L = lrect.Sze.H > 0 ? lrect.Sze.H : 0;

  lwin = new NCursesWindow( *win,
			    lrect.Sze.H, lrect.Sze.W,
			    lrect.Pos.L, lrect.Pos.C,
			    'r' );
  twin = new NCursesWindow( *win,
			    trect.Sze.H, trect.Sze.W,
			    trect.Pos.L, trect.Pos.C,
			    'r' );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : NCProgressBar::wDelete
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void NCProgressBar::wDelete()
{
  delete lwin;
  delete twin;
  lwin = 0;
  twin = 0;
  NCWidget::wDelete();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : NCProgressBar::setLabel
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void NCProgressBar::setLabel( const string & nlabel )
{
  label = NCstring( nlabel );
  setDefsze();
  YProgressBar::setLabel( nlabel );
  Redraw();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : NCProgressBar::setValue
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void NCProgressBar::setValue( int newValue )
{
  cval = newValue;
  if ( cval < 0 )
    cval = 0;
  else if ( cval > maxval )
    cval = maxval;
  Redraw();
  YProgressBar::setValue( newValue );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : NCProgressBar::wRedraw
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void NCProgressBar::wRedraw()
{
  if ( !win )
    return;

  // label
  chtype bg = wStyle().dumb.text;
  lwin->bkgdset( bg );
  lwin->clear();
  label.drawAt( *lwin, bg, bg );
  tUpdate();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : NCProgressBar::tUpdate
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void NCProgressBar::tUpdate()
{
  if ( !win )
    return;

  double split = double(twin->maxx()+1) * cval / maxval;
  int cp = int(split);
  if ( cp == 0 && split > 0.0 )
    cp = 1;

  const NCstyle::StProgbar & style( wStyle().progbar );
  twin->bkgdset( style.bar.chattr );
  twin->clear();

  if ( cp <= twin->maxx() ) {
    twin->bkgdset( NCattribute::getNonChar( style.nonbar.chattr ) );
    twin->move( 0, cp );
    for ( int i = 0; i < twin->width()-cp; ++i ) {
      twin->addch( NCattribute::getChar( style.nonbar.chattr ) );
    }
  }

  if ( twin->maxx() >= 6 ) {
    Value_t pc  = 100 * cval / maxval;
    Value_t off = twin->maxx() / 2 - ( pc == 100 ? 2
		 			         : pc >= 10 ? 1
							    : 0 );
    char buf[5];
    sprintf( buf, "%lld%%", pc );
    twin->move( 0, off );
    for ( char * ch = buf; *ch; ++ch ) {
      chtype a = twin->inch();
      NCattribute::setChar( a, *ch );
      twin->addch( a );
    }
  }
}
