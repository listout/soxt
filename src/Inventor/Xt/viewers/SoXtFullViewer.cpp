/**************************************************************************
 *
 *  Copyright (C) 2000 by Systems in Motion.  All rights reserved.
 *
 *  This file is part of the Coin library.
 *
 *  This file may be distributed under the terms of the Q Public License
 *  as defined by Troll Tech AS of Norway and appearing in the file
 *  LICENSE.QPL included in the packaging of this file.
 *
 *  If you want to use Coin in applications not covered by licenses
 *  compatible with the QPL, you can contact SIM to aquire a
 *  Professional Edition license for Coin.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
 **************************************************************************/

static const char rcsid[] =
  "$Id$";

#if HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <assert.h>

#if HAVE_LIBXPM
#include <X11/xpm.h>
#include <Inventor/Xt/common/pixmaps/pick.xpm>
#include <Inventor/Xt/common/pixmaps/view.xpm>
#include <Inventor/Xt/common/pixmaps/help.xpm>
#include <Inventor/Xt/common/pixmaps/home.xpm>
#include <Inventor/Xt/common/pixmaps/set_home.xpm>
#include <Inventor/Xt/common/pixmaps/view_all.xpm>
#include <Inventor/Xt/common/pixmaps/seek.xpm>
#endif // HAVE_LIBXPM

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/DialogS.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/Scale.h>
#include <Xm/PanedW.h>

#include <Inventor/errors/SoDebugError.h>
#include <Inventor/misc/SoBasic.h>

#include <Inventor/Xt/SoXtBasic.h>
#include <Inventor/Xt/SoXt.h>
#include <Inventor/Xt/widgets/SoXtThumbWheel.h>
#include <Inventor/Xt/widgets/SoAnyPopupMenu.h>

#include <Inventor/Xt/viewers/SoAnyFullViewer.h>
#include <Inventor/Xt/viewers/SoXtFullViewer.h>

enum DefaultViewerButtons {
  INTERACT_BUTTON = 0,
  EXAMINE_BUTTON,
  HELP_BUTTON,
  HOME_BUTTON,
  SET_HOME_BUTTON,
  VIEW_ALL_BUTTON,
  SEEK_BUTTON
};

#define VIEWERBUTTON(button) ((Widget) ((*this->viewerButtonsList)[button]))


// *************************************************************************

/*!
  Constructor.
*/

SoXtFullViewer::SoXtFullViewer( // protected
  Widget parent,
  const char * name,
  SbBool inParent,
  BuildFlag flags,
  Type type,
  SbBool build )
: inherited( parent, name, inParent, type, FALSE )
, common( new SoAnyFullViewer( this ) )
, popupTitle( NULL )
{
  this->viewerbase = NULL;
  this->canvas = NULL;

  char axisindicator[] = { 'Y', 'X', 'Z' };
  for ( int i = FIRSTDECORATION; i <= LASTDECORATION; i++) {
    this->wheelstrings[i] = "Motion ";
    this->wheelstrings[i] += axisindicator[i - FIRSTDECORATION];
    this->decorform[i] = NULL;
    this->wheellabels[i] = NULL;
  }

  this->zoomrange = SbVec2f( 1.0f, 140.0f );

  this->popupEnabled = (flags & SoXtFullViewer::BUILD_POPUP) ? TRUE : FALSE;
  this->decorations = (flags & SoXtFullViewer::BUILD_DECORATION) ? TRUE : FALSE;

  this->appButtonsList = new SbPList;
  this->viewerButtonsList = new SbPList;

  this->setSize( SbVec2s( 500, 390 ) );
  this->setClassName( "SoXtFullViewer" );

  this->prefmenu = NULL;

  pixmaps.pick = 0;
  pixmaps.view = 0;
  pixmaps.help = 0;
  pixmaps.home = 0;
  pixmaps.set_home = 0;
  pixmaps.view_all = 0;
  pixmaps.seek = 0;

  if ( build != FALSE )
    this->buildWidget( parent );
} // SoXtFullViewer()

/*!
  The destructor.
*/

SoXtFullViewer::~SoXtFullViewer( // protected
  void )
{
  delete this->common;
  delete this->appButtonsList;
  delete this->viewerButtonsList;
} // ~SoXtFullViewer()

// *************************************************************************

/*!
*/

void
SoXtFullViewer::setDecoration(
  const SbBool enable )
{
  if ( (this->decorations != enable) && (this->viewerbase != (Widget) NULL) )
    this->showDecorationWidgets( enable );
  this->decorations = enable;
  if ( this->prefmenu ) {
    SoDebugError::postInfo( "SoXtFullViewer::setDecoration",
       "setting to %s", enable ? "true" : "false" );
    this->prefmenu->SetMenuItemMarked( DECORATION_ITEM, enable );
  }
} // setDecoration()

/*!
*/

SbBool
SoXtFullViewer::isDecoration(
  void ) const
{
  return this->decorations;
} // isDecoration()

// *************************************************************************

/*!
*/

void
SoXtFullViewer::setPopupMenuEnabled(
  const SbBool enable )
{
  this->popupEnabled = enable;
} // setPopupMenuEnabled()

/*!
*/

SbBool
SoXtFullViewer::isPopupMenuEnabled(
  void ) const
{
  return this->popupEnabled;
} // isPopupMenuEnabled()

// *************************************************************************

/*!
  Return value is either NULL or a widget of type xmFormWidgetClass (Xm/Form.h)
*/

Widget
SoXtFullViewer::getAppPushButtonParent(
  void ) const
{
  return this->appButtonsForm;
} // getAppPushButtonParent()

/*!
*/

void
SoXtFullViewer::addAppPushButton(
  Widget button )
{
  this->appButtonsList->append( button );
  this->layoutAppPushButtons( this->getAppPushButtonParent() );
} // addAppPushButton()

/*!
*/

void
SoXtFullViewer::insertAppPushButton(
  Widget button,
  int idx )
{
  this->appButtonsList->insert( button, idx );
  this->layoutAppPushButtons( this->getAppPushButtonParent() );
} // insertAppPushButton()

/*!
*/

void
SoXtFullViewer::removeAppPushButton(
  Widget button )
{
  int idx = this->appButtonsList->find( button );
  if ( idx == -1 ) {
#if SOXT_DEBUG
    SoDebugError::postWarning( "SoXtFullViewer::removeAppPushButton",
                               "tried to remove non-existant button" );
#endif // SOXT_DEBUG
    return;
  }
  this->appButtonsList->remove( idx );
  this->layoutAppPushButtons( this->getAppPushButtonParent() );
} // removeAppPushButton()

/*!
*/

int
SoXtFullViewer::findAppPushButton(
  Widget button ) const
{
  return this->appButtonsList->find( button );
} // findAppPushButton()

/*!
*/

int
SoXtFullViewer::lengthAppPushButton(
  void ) const
{
  return this->appButtonsList->getLength();
} // lengthAppPushButton()

/*!
*/

void
SoXtFullViewer::layoutAppPushButtons(
  Widget parent )
{
  SOXT_STUB();
} // layoutAppPushButtons()

// *************************************************************************

/*!
*/

Widget
SoXtFullViewer::getRenderAreaWidget(
  void ) const
{
  return this->canvas;
} // getRenderAreaWidget()

// *************************************************************************

/*!
*/

Widget
SoXtFullViewer::buildWidget( // protected
  Widget parent )
{
  this->viewerbase = XtVaCreateManagedWidget( "ViewerRoot",
      xmFormWidgetClass, parent, NULL );

  this->buildDecoration( this->viewerbase );
  
  this->canvas = inherited::buildWidget( this->viewerbase );

  XtVaSetValues( this->canvas,
      XmNtopAttachment, XmATTACH_FORM,
      XmNleftAttachment, XmATTACH_FORM,
      XmNleftOffset, 30,
      XmNbottomAttachment, XmATTACH_FORM,
      XmNbottomOffset, 30,
      XmNrightAttachment, XmATTACH_FORM,
      XmNrightOffset, 30,
      NULL );
  this->setBorder( FALSE );

  XtManageChild( this->canvas );
  this->setBaseWidget( this->viewerbase );
  return this->viewerbase;
} // buildWidget()

// *************************************************************************

void
SoXtFullViewer::buildDecoration( // virtual
  Widget parent )
{
  this->decorform[BOTTOMDECORATION] = this->buildBottomTrim( parent );
  XtManageChild( this->decorform[BOTTOMDECORATION] );
  this->decorform[LEFTDECORATION]   = this->buildLeftTrim( parent );
  XtManageChild( this->decorform[LEFTDECORATION] );
  this->decorform[RIGHTDECORATION]  = this->buildRightTrim( parent );
  XtManageChild( this->decorform[RIGHTDECORATION] );
} // buildDecorations()

/*!
*/

Widget
SoXtFullViewer::buildBottomTrim( // virtual
  Widget parent )
{
  Widget trim = XtVaCreateManagedWidget( "BottomTrim",
      xmFormWidgetClass, parent,
      XmNleftAttachment, XmATTACH_FORM,
      XmNrightAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_FORM,
      XmNheight, 30,
      NULL );

  Widget label1 = XtVaCreateManagedWidget( "Roty", xmLabelWidgetClass,
    trim, XmNleftAttachment, XmATTACH_FORM, XmNtopAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset, 5,
    NULL );

  Widget label2 = XtVaCreateManagedWidget( "Rotx", xmLabelWidgetClass,
    trim, XmNleftAttachment, XmATTACH_WIDGET, XmNleftWidget, label1,
    XmNtopAttachment, XmATTACH_FORM, XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset, 5,
    NULL );

  // add bottom thumb wheel
  Widget wheel = XtVaCreateManagedWidget( "BottomWheel",
    soxtThumbWheelWidgetClass, trim,
    XmNorientation, XmHORIZONTAL,
    XmNshadowType, XmSHADOW_OUT,
    XmNhighlightThickness, 0,
    XmNshadowThickness, 2,
    XmNtraversalOn, False,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, label2,
    XmNleftOffset, 2,
    XmNtopAttachment, XmATTACH_FORM,
    XmNtopOffset, 2,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNbottomOffset, 2,
    XmNwidth, 90,
    NULL );

  XtAddCallback( wheel,
    XmNarmCallback, SoXtFullViewer::bottomWheelStartCB, this );
  XtAddCallback( wheel,
    XmNdisarmCallback, SoXtFullViewer::bottomWheelFinishCB, this );
  XtAddCallback( wheel,
    XmNvalueChangedCallback, SoXtFullViewer::bottomWheelMotionCB, this );

  Widget label3 = XtVaCreateManagedWidget( "Dolly", xmLabelWidgetClass,
    trim, XmNtopAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM, XmNrightAttachment, XmATTACH_FORM,
    XmNrightOffset, 5, NULL );

  return trim;
} // buildBottomTrim()

/*!
*/

Widget
SoXtFullViewer::buildLeftTrim( // virtual
  Widget parent )
{
  Widget trim = XtVaCreateManagedWidget( "LeftTrim",
      xmFormWidgetClass, parent,
      XmNwidth, 30,
      XmNleftAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_WIDGET,
      XmNbottomWidget, this->decorform[BOTTOMDECORATION],
      NULL );

  // build application buttons
  this->appButtonsForm = this->buildAppButtonsForm( trim );

  XtVaSetValues( this->appButtonsForm,
    XmNleftAttachment, XmATTACH_FORM,
    XmNtopAttachment, XmATTACH_FORM,
    XmNrightAttachment, XmATTACH_FORM,
    NULL );
  XtManageChild( this->appButtonsForm );

  // add right thumb wheel
  Widget wheel = XtVaCreateManagedWidget( "LeftWheel",
    soxtThumbWheelWidgetClass, trim,
    XmNorientation, XmVERTICAL,
    XmNshadowType, XmSHADOW_OUT,
    XmNhighlightThickness, 0,
    XmNshadowThickness, 2,
    XmNtraversalOn, False,
    XmNleftAttachment, XmATTACH_FORM,
    XmNleftOffset, 2,
    XmNrightAttachment, XmATTACH_FORM,
    XmNrightOffset, 2,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNbottomOffset, 2,
    XmNheight, 90,
    NULL );

  XtAddCallback( wheel,
    XmNarmCallback, SoXtFullViewer::leftWheelStartCB, this );
  XtAddCallback( wheel,
    XmNdisarmCallback, SoXtFullViewer::leftWheelFinishCB, this );
  XtAddCallback( wheel,
    XmNvalueChangedCallback, SoXtFullViewer::leftWheelMotionCB, this );

  return trim;
} // buildLeftTrim()

/*!
*/

Widget
SoXtFullViewer::buildRightTrim( // virtual
  Widget parent )
{
  Widget trim = XtVaCreateManagedWidget( "RightTrim",
      xmFormWidgetClass, parent,
      XmNwidth, 30,
      XmNtopAttachment, XmATTACH_FORM,
      XmNrightAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_WIDGET,
      XmNbottomWidget, this->decorform[BOTTOMDECORATION],
      NULL );

  Widget buttonForm = this->buildViewerButtons( trim );
  XtManageChild( buttonForm );

  // add right thumb wheel
  Widget wheel = XtVaCreateManagedWidget( "RightWheel",
    soxtThumbWheelWidgetClass, trim,
    XmNorientation, XmVERTICAL,
    XmNshadowType, XmSHADOW_OUT,
    XmNhighlightThickness, 0,
    XmNshadowThickness, 2,
    XmNtraversalOn, False,
    XmNleftAttachment, XmATTACH_FORM,
    XmNleftOffset, 2,
    XmNrightAttachment, XmATTACH_FORM,
    XmNrightOffset, 2,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNbottomOffset, 2,
    XmNheight, 90,
    NULL );

  XtAddCallback( wheel,
    XmNarmCallback, SoXtFullViewer::rightWheelStartCB, this );
  XtAddCallback( wheel,
    XmNdisarmCallback, SoXtFullViewer::rightWheelFinishCB, this );
  XtAddCallback( wheel,
    XmNvalueChangedCallback, SoXtFullViewer::rightWheelMotionCB, this );

  return trim;
} // buildRightTrim()

// *************************************************************************

/*!
*/

void
SoXtFullViewer::setViewing( // virtual
  SbBool enable )
{
#if SOXT_DEBUG
  if ( this->isViewing() == enable )
    SoDebugError::postWarning( "SoXtFullViewer::setViewing",
      "current state already that of argument" );
#endif // SOXT_DEBUG

  inherited::setViewing( enable );
  if ( this->prefmenu ) {
    this->prefmenu->SetMenuItemMarked( EXAMINING_ITEM, enable );
    this->prefmenu->SetMenuItemEnabled( SEEK_ITEM, enable );
  }

  XtVaSetValues( this->viewerbuttons.view,
                 XmNset, enable ? True : False, NULL );
  XtVaSetValues( this->viewerbuttons.pick,
                 XmNset, enable ? False : True, NULL );

  XtUnmanageChild( this->viewerbuttons.seek );
  XtSetSensitive( this->viewerbuttons.seek,
    enable ? True : False );
  XtVaSetValues( this->viewerbuttons.seek,
    XmNwidth, 30, XmNheight, 30, NULL );
  XtManageChild( this->viewerbuttons.seek );
} // setViewing()

/*!
*/

void
SoXtFullViewer::setHeadlight( // virtual
  SbBool enable )
{
  inherited::setHeadlight( enable );
  if ( this->prefmenu )
    this->prefmenu->SetMenuItemMarked( HEADLIGHT_ITEM, enable );
} // setHeadlight()

/*!
*/

void
SoXtFullViewer::setDrawStyle( // virtual
  SoXtViewer::DrawType type,
  SoXtViewer::DrawStyle style )
{
  inherited::setDrawStyle( type, style );
  if ( this->prefmenu )
    common->setDrawStyleMenuActivation( type, style );
} // setDrawStyle()

/*!
*/

void
SoXtFullViewer::setBufferingType( // virtual
  SoXtViewer::BufferType type )
{
  inherited::setBufferingType( type );

  if ( this->prefmenu ) {
    switch ( type ) {
    case SoXtViewer::BUFFER_SINGLE:
      this->prefmenu->SetMenuItemMarked( SINGLE_BUFFER_ITEM, TRUE );
      break;
    case SoXtViewer::BUFFER_DOUBLE:
      this->prefmenu->SetMenuItemMarked( DOUBLE_BUFFER_ITEM, TRUE );
      break;
    case SoXtViewer::BUFFER_INTERACTIVE:
      this->prefmenu->SetMenuItemMarked( INTERACTIVE_BUFFER_ITEM, TRUE );
      break;
    default:
      assert( 0 && "unsupported buffer type" );
      break;
    } // switch ( type )
  }
} // setBufferingType()

/*!
*/

void
SoXtFullViewer::setCamera( // virtual
  SoCamera * camera )
{
  inherited::setCamera( camera );

#if 0
  if ( this->prefwindow ) {
    this->setZoomSliderPosition( this->getCameraZoom() );
    this->setZoomFieldString( this->getCameraZoom() );

    SbBool enable = camera ? TRUE : FALSE;
    this->zoomslider->setEnabled( enable );
    this->zoomfield->setEnabled( enable );
    this->zoomrangefrom->setEnabled( enable );
    this->zoomrangeto->setEnabled( enable );
  }
#endif // 0
} // setCamera()

/*!
*/

void
SoXtFullViewer::hide( // virtual
  void )
{
  inherited::hide();
//  if ( this->prefwindow )
//    SoXt::hide( this->prefwindow );
} // hide()

// *************************************************************************

/*!
*/

Widget
SoXtFullViewer::buildViewerButtons(
  Widget parent )
{
  Widget form = XtVaCreateManagedWidget( "ViewerButtons",
      xmFormWidgetClass, parent, NULL );

  this->createViewerButtons( form, this->viewerButtonsList );

  const int buttons = this->viewerButtonsList->getLength();

  XtVaSetValues( form,
    XmNx, 0, XmNy, 0,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNtopAttachment, XmATTACH_POSITION,
    XmNwidth, 30,
    XmNheight, buttons * 30,
    XmNfractionBase, buttons,
    NULL );

  for ( int i = 0; i < this->viewerButtonsList->getLength(); i++ ) {
    Widget button = (Widget) (*this->viewerButtonsList)[i];
    XtVaSetValues( button,
       XmNwidth, 30, XmNheight, 30,
       XmNleftAttachment, XmATTACH_POSITION,
       XmNtopAttachment, XmATTACH_POSITION,
       XmNleftPosition, 0, XmNtopPosition, i,
       NULL );
  }
  return form;
} // buildViewerButtons()

// *************************************************************************

/*
  \internal
  Does nothing if libXpm use hasn't been enabled.
*/

Pixmap
SoXtFullViewer::createPixmapFromXpmData(
  Widget button,
  char ** xpm )
{
#if HAVE_LIBXPM
  Pixel bg;
  XtVaGetValues( button, XmNbackground, &bg, NULL );
  Display * dpy = SoXt::getDisplay();
  XImage * image, * stencil;
  int error = XpmCreateImageFromData( dpy, xpm, &image, &stencil, NULL );
  if ( error != XpmSuccess ) return 0;
  for ( int x = 0; x < image->width; x++ ) {
    for ( int y = 0; y < image->height; y++ ) {
      Pixel pixel = XGetPixel( stencil, x, y );
      if ( pixel == 0 ) // background must be set in image
        XPutPixel( image, x, y, bg );
    }
  }
  const int width = 24, height = 24;
  Drawable draw = RootWindow( dpy, DefaultScreen(dpy) );
  Pixmap retval = XCreatePixmap( dpy, draw, width, height, image->depth );
  GC gc = XCreateGC( dpy, draw, 0, NULL );
  XPutImage( dpy, retval, gc, image, 0, 0, 0, 0, width, height );
  XFreeGC( dpy, gc );
  XDestroyImage( image );
  XDestroyImage( stencil );
  return retval;
#endif // HAVE_LIBXPM
  return (Pixmap) 0;
} // ButtonPixmap()

/*!
  \internal
  Does nothing if libXpm use hasn't been enabled.
*/

Pixmap
SoXtFullViewer::createInsensitivePixmapFromXpmData(
  Widget button,
  char ** xpm )
{
#if HAVE_LIBXPM
  Pixel bg;
  XtVaGetValues( button, XmNbackground, &bg, NULL );
  Display * dpy = SoXt::getDisplay();
  XImage * image, * stencil;
  int error = XpmCreateImageFromData( dpy, xpm, &image, &stencil, NULL );
  if ( error != XpmSuccess ) return 0;
  for ( int x = 0; x < image->width; x++ ) {
    for ( int y = 0; y < image->height; y++ ) {
      Pixel pixel = XGetPixel( stencil, x, y );
      if ( (pixel == 0) || (((x+y) & 1) == 1) )
        XPutPixel( image, x, y, bg );
    }
  }
  const int width = 24, height = 24;
  Drawable draw = RootWindow( dpy, DefaultScreen(dpy) );
  Pixmap retval = XCreatePixmap( dpy, draw, width, height, image->depth );
  GC gc = XCreateGC( dpy, draw, 0, NULL );
  XPutImage( dpy, retval, gc, image, 0, 0, 0, 0, width, height );
  XFreeGC( dpy, gc );
  XDestroyImage( image );
  XDestroyImage( stencil );
  return retval;
#endif // HAVE_LIBXPM
  return (Pixmap) 0;
} // createInsensitivePixmapFromXpmData()

/*!
*/

void
SoXtFullViewer::createViewerButtons(
  Widget parent,
  SbPList * buttonlist )
{
  assert( buttonlist != NULL );

  int viewerbutton;
  for ( viewerbutton = 0; viewerbutton <= SEEK_BUTTON; viewerbutton++ ) {
    XtCallbackProc proc = NULL;
    char label[2];
    label[1] = '\0';
    switch ( viewerbutton ) {
    case INTERACT_BUTTON:
      proc = SoXtFullViewer::interactbuttonCB;
      label[0] = 'I';
      break;
    case EXAMINE_BUTTON:
      proc = SoXtFullViewer::examinebuttonCB;
      label[0] = 'E';
      break;
    case HELP_BUTTON:
      proc = SoXtFullViewer::helpbuttonCB;
      label[0] = '?';
      break;
    case HOME_BUTTON:
      proc = SoXtFullViewer::homebuttonCB;
      label[0] = 'H';
      break;
    case SET_HOME_BUTTON:
      proc = SoXtFullViewer::sethomebuttonCB;
      label[0] = 'N';
      break;
    case VIEW_ALL_BUTTON:
      proc = SoXtFullViewer::viewallbuttonCB;
      label[0] = 'V';
      break;
    case SEEK_BUTTON:
      proc = SoXtFullViewer::seekbuttonCB;
      label[0] = 'S';
      break;
    default:
      assert( 0 );
      break;
    } // switch ( viewerbutton )
    Widget button;
    if ( viewerbutton == EXAMINE_BUTTON || viewerbutton == INTERACT_BUTTON ) {
      button = XtVaCreateManagedWidget( label,
        xmToggleButtonWidgetClass, parent,
        XmNindicatorOn, False,
        XmNshadowType, XmSHADOW_OUT,
        XmNhighlightThickness, 0,
        XmNshadowThickness, 2,
        XmNtraversalOn, False,
        XmNwidth, 30,
        XmNheight, 30,
        NULL );
      if ( viewerbutton == EXAMINE_BUTTON ) {
        XtVaSetValues( button, XmNset, this->isViewing() ? True : False, NULL );
      }
      if ( viewerbutton == INTERACT_BUTTON ) {
        XtVaSetValues( button, XmNset, this->isViewing() ? False : True, NULL );
      }
    } else {
      button = XtVaCreateManagedWidget( label,
        xmPushButtonWidgetClass, parent,
        XmNshadowType, XmSHADOW_OUT,
        XmNhighlightThickness, 0,
        XmNshadowThickness, 2,
        XmNtraversalOn, False,
        XmNwidth, 30,
        XmNheight, 30,
        NULL );
    }

    switch ( viewerbutton ) {
    case INTERACT_BUTTON:
      this->viewerbuttons.pick = button;
      break;
    case EXAMINE_BUTTON:
      this->viewerbuttons.view = button;
      break;
    case HELP_BUTTON:
      this->viewerbuttons.help = button;
      break;
    case HOME_BUTTON:
      this->viewerbuttons.home = button;
      break;
    case SET_HOME_BUTTON:
      this->viewerbuttons.set_home = button;
      break;
    case VIEW_ALL_BUTTON:
      this->viewerbuttons.view_all = button;
      break;
    case SEEK_BUTTON:
      this->viewerbuttons.seek = button;
      break;
    default:
      assert( 0 && "impossible" );
      break;
    } // switch ( viewerbutton )
#if HAVE_LIBXPM
    Pixmap pixmap, pixmap_ins;
    switch ( viewerbutton ) {
    case INTERACT_BUTTON:
      pixmap = pixmap_ins = pixmaps.pick =
        createPixmapFromXpmData( button, pick_xpm );
      break;
    case EXAMINE_BUTTON:
      pixmap = pixmap_ins = pixmaps.view =
        createPixmapFromXpmData( button, view_xpm );
      break;
    case HELP_BUTTON:
      pixmap = pixmap_ins = pixmaps.help =
        createPixmapFromXpmData( button, help_xpm );
      break;
    case HOME_BUTTON:
      pixmap = pixmap_ins = pixmaps.home =
        createPixmapFromXpmData( button, home_xpm );
      break;
    case SET_HOME_BUTTON:
      pixmap = pixmap_ins = pixmaps.set_home =
        createPixmapFromXpmData( button, set_home_xpm );
      break;
    case VIEW_ALL_BUTTON:
      pixmap = pixmap_ins = pixmaps.view_all =
        createPixmapFromXpmData( button, view_all_xpm );
      break;
    case SEEK_BUTTON:
      pixmap = pixmaps.seek =
        createPixmapFromXpmData( button, seek_xpm );
      pixmap_ins = pixmaps.seek_ins =
        createInsensitivePixmapFromXpmData( button, seek_xpm );
      break;
    default:
      assert( 0 && "impossible" );
      break;
    } // switch ( viewerbutton )
    XtVaSetValues( button,
      XmNlabelType, XmPIXMAP,
      XmNlabelPixmap, pixmap,
      XmNlabelInsensitivePixmap, pixmap_ins,
      XmNselectPixmap, pixmap,
      XmNselectInsensitivePixmap, pixmap_ins,
      XmNwidth, 30, XmNheight, 30,
      NULL );
#endif // HAVE_LIBXPM

    if ( proc != NULL ) {
      if ( viewerbutton == INTERACT_BUTTON || viewerbutton == EXAMINE_BUTTON ) {
        XtAddCallback( button, XmNdisarmCallback, proc, this );
      } else {
        XtAddCallback( button, XmNactivateCallback, proc, this );
      }
    }
    buttonlist->append( button );
  }
} // createViewerButtons()

/*!
*/

void
SoXtFullViewer::buildPopupMenu(
  void )
{
  if ( this->prefmenu == NULL )
    this->prefmenu = common->setupStandardPopupMenu();

  this->setDrawStyle(
    SoXtViewer::STILL, this->getDrawStyle( SoXtViewer::STILL ) );
  this->setDrawStyle(
    SoXtViewer::INTERACTIVE, this->getDrawStyle( SoXtViewer::INTERACTIVE ) );
  this->setBufferingType( this->getBufferingType() );

  this->prefmenu->SetMenuItemMarked( EXAMINING_ITEM, this->isViewing() );
  this->prefmenu->SetMenuItemMarked( DECORATION_ITEM, this->decorations );
  this->prefmenu->SetMenuItemMarked( HEADLIGHT_ITEM, this->isHeadlight() );
  this->prefmenu->SetMenuItemEnabled( SEEK_ITEM, this->isViewing() );
} // buildPopupMenu()

// *************************************************************************

/*!
*/

void
SoXtFullViewer::leftWheelStart( // virtual
  void )
{
  this->interactiveCountInc();
} // leftWheelStart()

/*!
*/

void
SoXtFullViewer::leftWheelStartCB( // static
  Widget, // w,
  XtPointer client_data,
  XtPointer ) // call_data )
{
  assert( client_data != NULL );
  ((SoXtFullViewer *) client_data)->leftWheelStart();
} // leftWheelStartCB()

/*!
*/

void
SoXtFullViewer::leftWheelMotion( // virtual
  float value )
{
  this->wheelvalues[LEFTDECORATION] = value;
} // leftWheelMotion()

/*!
*/

void
SoXtFullViewer::leftWheelMotionCB( // static
  Widget, // w,
  XtPointer client_data,
  XtPointer call_data )
{
  assert( client_data != NULL && call_data != NULL );
  float * valueptr = (float *) call_data;
  ((SoXtFullViewer *) client_data)->leftWheelMotion( *valueptr );
} // leftWheelMotionCB()

/*!
*/

void
SoXtFullViewer::leftWheelFinish( // virtual
  void )
{
  this->interactiveCountDec();
} // leftWheelFinished()

/*!
*/

void
SoXtFullViewer::leftWheelFinishCB( // static
  Widget, // w,
  XtPointer client_data,
  XtPointer ) // call_data )
{
  assert( client_data != NULL );
  ((SoXtFullViewer *) client_data)->leftWheelFinish();
} // leftWheelFinishCB()

/*!
*/

float
SoXtFullViewer::getLeftWheelValue(
  void ) const
{
  return this->wheelvalues[LEFTDECORATION];
} // getLeftWheelValue()

// *************************************************************************

/*!
*/

void
SoXtFullViewer::bottomWheelStart( // virtual
  void )
{
  this->interactiveCountInc();
} // bottomWheelStart()

/*!
*/

void
SoXtFullViewer::bottomWheelStartCB( // static
  Widget, // w,
  XtPointer client_data,
  XtPointer ) // call_data )
{
  assert( client_data != NULL );
  ((SoXtFullViewer *) client_data)->bottomWheelStart();
} // bottomWheelStart()

/*!
*/

void
SoXtFullViewer::bottomWheelMotion( // virtual
  float value )
{
  this->wheelvalues[BOTTOMDECORATION] = value;
} // bottomWheelMode()

/*!
*/

void
SoXtFullViewer::bottomWheelMotionCB( // static
  Widget, // w,
  XtPointer client_data,
  XtPointer call_data )
{
  assert( client_data != NULL && call_data != NULL );
  float * valueptr = (float *) call_data;
  ((SoXtFullViewer *) client_data)->bottomWheelMotion( *valueptr );
} // bottomWheelStart()

/*!
*/

void
SoXtFullViewer::bottomWheelFinish( // virtual
  void )
{
  this->interactiveCountDec();
} // bottomWheelFinish()

/*!
*/

void
SoXtFullViewer::bottomWheelFinishCB( // static
  Widget, // w,
  XtPointer client_data,
  XtPointer ) // call_data )
{
  assert( client_data != NULL );
  ((SoXtFullViewer *) client_data)->bottomWheelFinish();
} // bottomWheelStart()

/*!
*/

float
SoXtFullViewer::getBottomWheelValue(
  void ) const
{
  return this->wheelvalues[BOTTOMDECORATION];
} // getBottomWheelValue()

// *************************************************************************

/*!
*/

void
SoXtFullViewer::rightWheelStart( // virtual
  void )
{
  this->interactiveCountInc();
} // rightWheelStart()

/*!
*/

void
SoXtFullViewer::rightWheelStartCB( // static
  Widget, // w,
  XtPointer client_data,
  XtPointer ) // call_data )
{
  assert( client_data != NULL );
  ((SoXtFullViewer *) client_data)->rightWheelStart();
} // rightWheelStartCB()

/*!
*/

void
SoXtFullViewer::rightWheelMotion( // virtual
  float value )
{
  this->wheelvalues[RIGHTDECORATION] = value;
} // rightWheelMotion()

/*!
*/

void
SoXtFullViewer::rightWheelMotionCB(
  Widget, // w,
  XtPointer client_data,
  XtPointer call_data )
{
  assert( client_data != NULL && call_data != NULL );
  float * valueptr = (float *) call_data;
  ((SoXtFullViewer *) client_data)->rightWheelMotion( *valueptr );
} // rightWheelMotionCB()

/*!
*/

void
SoXtFullViewer::rightWheelFinish( // virtual
  void )
{
  this->interactiveCountDec();
} // rightWheelFinish()

/*!
*/

void
SoXtFullViewer::rightWheelFinishCB(
  Widget, // w,
  XtPointer client_data,
  XtPointer ) // call_data
{
  assert( client_data != NULL );
  ((SoXtFullViewer *) client_data)->rightWheelFinish();
} // rightWheelFinishCB()

/*!
*/

float
SoXtFullViewer::getRightWheelValue(
  void ) const
{
  return this->wheelvalues[RIGHTDECORATION];
} // getRightWheelValue()

// *************************************************************************

/*!
  Default implementation does nothing.
*/

void
SoXtFullViewer::openViewerHelpCard( // virtual
  void )
{
  SOXT_STUB();
} // openViewerHelpCard()

// *************************************************************************

/*!
*/

Widget
SoXtFullViewer::buildAppButtonsForm(
  Widget parent )
{
  Widget form = XtVaCreateManagedWidget( "appbuttons", xmFormWidgetClass,
    parent, NULL );

  return form;
} // buildAppButtonsForm()

// *************************************************************************

void
SoXtFullViewer::showDecorationWidgets(
  SbBool enable )
{
  assert( this->canvas != (Widget) NULL );
  assert( this->decorform[0] != (Widget) NULL );
  assert( this->decorform[1] != (Widget) NULL );
  assert( this->decorform[2] != (Widget) NULL );

  if ( enable ) {
    XtRealizeWidget( this->decorform[0] );
    XtManageChild( this->decorform[0] );
    XtRealizeWidget( this->decorform[1] );
    XtManageChild( this->decorform[1] );
    XtRealizeWidget( this->decorform[2] );
    XtManageChild( this->decorform[2] );
    XtVaSetValues( this->canvas,
      XmNleftOffset, 30, XmNrightOffset, 30, XmNbottomOffset, 30, NULL );
  } else {
    XtUnrealizeWidget( this->decorform[0] );
    XtUnrealizeWidget( this->decorform[1] );
    XtUnrealizeWidget( this->decorform[2] );
    XtVaSetValues( this->canvas,
      XmNleftOffset, 0, XmNrightOffset, 0, XmNbottomOffset, 0, NULL );
  }
} // showDecorationWidgets()

// *************************************************************************

/*!
  This method sets the title text for the viewer preferences window.
*/

void
SoXtFullViewer::setPrefSheetString( // protected
  const char * name )
{
  if ( this->prefstring )
    delete [] this->prefstring;
  this->prefstring = NULL;
  if ( name )
    this->prefstring = strcpy( new char [strlen(name) + 1], name );

  if ( this->prefshell )
    XtVaSetValues( this->prefshell, XtNtitle, this->prefstring, NULL );
} // setPrefSheetString()

/*!
  This is a hook for constructing the preferences window in the derived
  viewer classes.

  Basically, derived classes will do the following in this method:

  \code
    if ( ! this->prefshell ) {
      createPrefSheetShellAndForm( this->prefshell, this->prefsheet );
      createDefaultPrefSheetParts( this->prefparts, this->numprefparts,
        this->prefsheet );
      // add parts specific for derived viewer
    }
    layoutPartsAndMapPrefSheet( this->prefparts, this->numprefparts,
      this->prefsheet, this->prefshell );
*/

void
SoXtFullViewer::createPrefSheet( // protected, virtual
  void )
{
/*�
  Method could be made abstract.
*/
} // createPrefSheet()

/*!
*/

void
SoXtFullViewer::prefSheetDestroyCB( // protected, static
  Widget w,
  XtPointer client_data,
  XtPointer ) // call_data )
{
  SoXtFullViewer * viewer = (SoXtFullViewer *) client_data;
  XtUnrealizeWidget( viewer->prefshell );
  viewer->prefshell = NULL;
  viewer->prefsheet = NULL;
} // prefSheetDestroyCB()

/*!
*/

void
SoXtFullViewer::createPrefSheetShellAndForm(  // protected
  Widget & shell,
  Widget & form )
{
  assert( this->prefshell == NULL );

  if ( ! this->prefstring )
    this->setPrefSheetString( "Xt Viewer Preferences" );

  shell = XtVaAppCreateShell( NULL, "SoXt",
    topLevelShellWidgetClass, SoXt::getDisplay(),
    XtNtitle, this->prefstring,
    NULL );
  XtAddCallback( shell, XmNdestroyCallback,
    prefSheetDestroyCB, (XtPointer) this );
  this->prefshell = shell;

  form = XtVaCreateWidget( "form", xmFormWidgetClass, shell, NULL );
  this->prefsheet = form;
} // createPrefSheetShellAndForm()

/*!
*/

void
SoXtFullViewer::createDefaultPrefSheetParts( // protected
  Widget * widgets,
  int & num,
  Widget form )
{
  assert( widgets != NULL );
  num = 0;
  widgets[num++] = this->createSeekPrefSheetGuts( form );
  if ( ! (widgets[num++] = this->createSeekDistPrefSheetGuts( form )) ) num--;
  if ( ! (widgets[num++] = this->createZoomPrefSheetGuts( form ) )) num--;
  if ( ! (widgets[num++] = this->createClippingPrefSheetGuts( form )) ) num--;
  if ( ! (widgets[num++] = this->createStereoPrefSheetGuts( form )) ) num--;
  if ( ! (widgets[num++] = this->createSpeedPrefSheetGuts( form )) ) num--;
} // createDefaultPrefSheetParts()

/*!
*/

void
SoXtFullViewer::layoutPartsAndMapPrefSheet( // protected
  Widget * widgets,
  int num,
  Widget form,
  Widget shell )
{
  assert( widgets != NULL );
  for ( int i = 0; i < num; i++ ) {
    int attachment_top, attachment_bottom;
    if ( i == 0 ) {
      attachment_top = XmATTACH_FORM;
      attachment_bottom = XmATTACH_NONE;
    } else if ( i == (num - 1) ) {
      attachment_top = XmATTACH_WIDGET;
      attachment_bottom = XmATTACH_FORM;
    } else {
      attachment_top = XmATTACH_WIDGET;
      attachment_bottom = XmATTACH_NONE;
    }
    XtVaSetValues( widgets[i],
      XmNtopAttachment, attachment_top,
      XmNleftAttachment, XmATTACH_FORM,
      XmNrightAttachment, XmATTACH_FORM,
      XmNbottomAttachment, attachment_bottom,
      NULL );
    if ( attachment_top == XmATTACH_WIDGET )
      XtVaSetValues( widgets[i], XmNtopWidget, widgets[i-1], NULL );
  }
  XtManageChild( form );
  XtPopup( shell, XtGrabNone );
} // layoutPartsAndMapPrefSheet()
 
// *************************************************************************

/*!
*/

Widget
SoXtFullViewer::createSeekPrefSheetGuts( // protected
  Widget parent )
{
  //  Seek animation time  |NNN|  seconds
  //  Seek to:   <> point   <> object

  Widget form = XtVaCreateManagedWidget( "seekprefs",
    xmFormWidgetClass, parent, NULL );

  XmString labelstring;

  Widget line1 = XtVaCreateManagedWidget( "line 1",
    xmFormWidgetClass, form,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightAttachment, XmATTACH_FORM,
    NULL );

  labelstring = SoXt::encodeString( "Seek animation time: " );
  Widget label = XtVaCreateManagedWidget( "seektime",
    xmLabelWidgetClass, line1,
    XmNlabelString, labelstring,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    NULL );
  XtFree( (char *) labelstring );

  Widget input = XtVaCreateManagedWidget( "seektimeinput",
    xmTextWidgetClass, line1,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, label,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNwidth, 60,
    NULL );

  Widget denotion = XtVaCreateManagedWidget( "seconds",
    xmLabelWidgetClass, line1,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, input,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNbottomWidget, input,
    NULL );

  Widget line2 = XtVaCreateManagedWidget( "line 2",
    xmFormWidgetClass, form,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightAttachment, XmATTACH_FORM,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, line1,
    XmNbottomAttachment, XmATTACH_FORM,
    NULL );

  labelstring = SoXt::encodeString( "Seek to: " );
  Widget tolabel = XtVaCreateManagedWidget( "tolabel",
    xmLabelWidgetClass, line2,
    XmNlabelString, labelstring,
    XmNleftAttachment, XmATTACH_FORM,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    NULL );
  XtFree( (char *) labelstring );

  Widget pointb = XtVaCreateManagedWidget( "point",
    xmToggleButtonWidgetClass, line2,
    XmNindicatorType, XmONE_OF_MANY,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, tolabel,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    NULL );

  Widget objectb = XtVaCreateManagedWidget( "object",
    xmToggleButtonWidgetClass, line2,
    XmNindicatorType, XmONE_OF_MANY,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, pointb,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    NULL );

  return form;
} // createSeekPrefSheetGuts()

/*!
*/

Widget
SoXtFullViewer::createSeekDistPrefSheetGuts( // protected
  Widget parent )
{
  //   Seek distance:   |||||||||||||   |NN.NNN|
  //      <> percentage   <> absolute

  Widget form = XtVaCreateManagedWidget( "seekdistprefs",
    xmFormWidgetClass, parent, NULL );

  Widget line1 = XtVaCreateManagedWidget( "seekdistprefs",
    xmFormWidgetClass, form,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightAttachment, XmATTACH_FORM,
    XmNtopAttachment, XmATTACH_FORM,
    NULL );

  XmString labelstring;
  labelstring = SoXt::encodeString( "Seek distance: " );
  Widget label = XtVaCreateManagedWidget( "seekdistlabel",
    xmLabelWidgetClass, line1,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_FORM,
    XmNlabelString, labelstring,
    XmNbottomAttachment, XmATTACH_FORM,
    NULL );
  XtFree( (char *) labelstring );

  Widget wheel = XtVaCreateManagedWidget( "distance",
    soxtThumbWheelWidgetClass, line1,
    XmNtopAttachment, XmATTACH_FORM,
    XmNtopOffset, 2,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, label,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNbottomOffset, 2,
    XmNorientation, XmHORIZONTAL,
    XmNshadowType, XmSHADOW_OUT,
    XmNshadowThickness, 2,
    XmNtraversalOn, False,
    XmNwidth, 90,
    NULL );

  Widget input = XtVaCreateManagedWidget( "input",
    xmTextWidgetClass, line1,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, wheel,
    XmNwidth, 80,
    NULL );


  Widget line2 = XtVaCreateManagedWidget( "line 2",
    xmFormWidgetClass, form,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, line1,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightAttachment, XmATTACH_FORM,
    NULL );

  labelstring = SoXt::encodeString( "percentage" );
  Widget percentb = XtVaCreateManagedWidget( "percentage",
    xmToggleButtonWidgetClass, line2,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNlabelString, labelstring,
    XmNindicatorType, XmONE_OF_MANY,
    NULL );
  XtFree( (char *) labelstring );

  labelstring = SoXt::encodeString( "absolute" );
  Widget absb = XtVaCreateManagedWidget( "absolute",
    xmToggleButtonWidgetClass, line2,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, percentb,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNindicatorType, XmONE_OF_MANY,
    XmNlabelString, labelstring,
    NULL );
  XtFree( (char *) labelstring );

  return form;
} // createSeekDistPrefSheetGuts()

/*!
*/

Widget
SoXtFullViewer::createZoomPrefSheetGuts( // protected
  Widget parent )
{
  //  Camera zoom  |          ||||        |     |NN.NN|
  //  Zoom slider ranges from:  |N.N|  to |N.N|

  Widget form = XtVaCreateManagedWidget( "zoomprefs",
    xmFormWidgetClass, parent,
    NULL );

  Widget line1 = XtVaCreateManagedWidget( "line 1",
    xmFormWidgetClass, form,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightAttachment, XmATTACH_FORM,
    NULL );

  XmString labelstring;
  labelstring = SoXt::encodeString( "Camera zoom: " );
  Widget label1 = XtVaCreateManagedWidget( "label1",
    xmLabelWidgetClass, line1,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNlabelString, labelstring,
    NULL );
  XtFree( (char *) labelstring );

  Widget zoomscale = XtVaCreateManagedWidget( "scale",
    xmScaleWidgetClass, line1,
    XmNtopAttachment, XmATTACH_FORM,
    XmNtopOffset, 6,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, label1,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNbottomOffset, 6,
    XmNorientation, XmHORIZONTAL,
    XmNwidth, 120,
    NULL );

  Widget zoominput = XtVaCreateManagedWidget( "zoominput",
    xmTextWidgetClass, line1,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, zoomscale,
    XmNleftOffset, 2,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNwidth, 80,
    NULL );

  Widget line2 = XtVaCreateManagedWidget( "line 2",
    xmFormWidgetClass, form,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, line1,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightAttachment, XmATTACH_FORM,
    NULL );

  labelstring = SoXt::encodeString( "Zoom slider ranges from " );
  Widget rangelabel = XtVaCreateManagedWidget( "rangelabel",
    xmLabelWidgetClass, line2,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNlabelString, labelstring,
    NULL );
  XtFree( (char *) labelstring );

  Widget from = XtVaCreateManagedWidget( "from",
    xmTextWidgetClass, line2,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, rangelabel,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNwidth, 80,
    NULL );

  labelstring = SoXt::encodeString( " to " );
  Widget tolabel = XtVaCreateManagedWidget( "rangelabel",
    xmLabelWidgetClass, line2,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, from,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNlabelString, labelstring,
    NULL );
  XtFree( (char *) labelstring );

  Widget to = XtVaCreateManagedWidget( "to",
    xmTextWidgetClass, line2,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, tolabel,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNwidth, 80,
    NULL );

  return form;
} // createZoomPrefSheetGuts()

/*!
*/

Widget
SoXtFullViewer::createClippingPrefSheetGuts( // protected
  Widget parent )
{
  //     []  Auto clipping planes
  //      near plane    ||||||||||||||   |N.NNNNNN|
  //       far plane    ||||||||||||||   |N.NNNNNN|
  Widget form = XtVaCreateManagedWidget( "clippingprefs",
    xmFormWidgetClass, parent,
    NULL );

  Widget line1 = XtVaCreateManagedWidget( "line 1",
    xmFormWidgetClass, form,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightAttachment, XmATTACH_FORM,
    NULL );

  XmString labelstring;
  labelstring = SoXt::encodeString( "Auto clipping planes" );
  Widget enable = XtVaCreateManagedWidget( "enable",
    xmToggleButtonWidgetClass, line1,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_FORM,
    XmNlabelString, labelstring,
    NULL );
  XtFree( (char *) labelstring );

  Widget line2 = XtVaCreateManagedWidget( "line 2",
    xmFormWidgetClass, form,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, line1,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightAttachment, XmATTACH_FORM,
    NULL );

  labelstring = SoXt::encodeString( "near plane" );
  Widget label2 = XtVaCreateManagedWidget( "label2",
    xmLabelWidgetClass, line2,
    XmNleftAttachment, XmATTACH_FORM,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNlabelString, labelstring,
    NULL );
  XtFree( (char *) labelstring );

  Widget wheel2 = XtVaCreateManagedWidget( "wheel2",
    soxtThumbWheelWidgetClass, line2,
    XmNtopAttachment, XmATTACH_FORM,
    XmNtopOffset, 2,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNbottomOffset, 2,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, label2,
    XmNwidth, 90,
    XmNorientation, XmHORIZONTAL,
    NULL );

  Widget input2 = XtVaCreateManagedWidget( "input 2",
    xmTextWidgetClass, line2,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, wheel2,
    XmNwidth, 80,
    NULL );

  Widget line3 = XtVaCreateManagedWidget( "line 3",
    xmFormWidgetClass, form,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, line2,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightAttachment, XmATTACH_FORM,
    NULL );

  labelstring = SoXt::encodeString( "far plane" );
  Widget label3 = XtVaCreateManagedWidget( "label3",
    xmLabelWidgetClass, line3,
    XmNleftAttachment, XmATTACH_FORM,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNlabelString, labelstring,
    NULL );
  XtFree( (char *) labelstring );

  Widget wheel3 = XtVaCreateManagedWidget( "wheel3",
    soxtThumbWheelWidgetClass, line3,
    XmNtopAttachment, XmATTACH_FORM,
    XmNtopOffset, 2,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNbottomOffset, 2,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, label3,
    XmNorientation, XmHORIZONTAL,
    XmNwidth, 90,
    NULL );

  Widget input3 = XtVaCreateManagedWidget( "input 3",
    xmTextWidgetClass, line3,
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, wheel3,
    XmNwidth, 80,
    NULL );

  return form;
} // createClippingPrefSheetGuts()

/*!
*/

Widget
SoXtFullViewer::createStereoPrefSheetGuts( // protected
  Widget parent )
{
  SOXT_STUB();
  return (Widget) NULL;
} // createStereoPrefSheetGuts()

/*!
*/

Widget
SoXtFullViewer::createSpeedPrefSheetGuts( // protected
  Widget parent )
{
  SOXT_STUB();
  return (Widget) NULL;
} // createSpeedPrefSheetGuts()

// *************************************************************************

/*!
*/

void
SoXtFullViewer::selectedViewing(
  void )
{
  this->setViewing( this->isViewing() ? FALSE : TRUE );
} // selectedViewing()

/*!
*/

void
SoXtFullViewer::selectedDecoration(
  void )
{
  SOXT_STUB();
} // selectedDecoration();

/*!
*/

void
SoXtFullViewer::selectedHeadlight(
  void )
{
  SOXT_STUB();
} // selectedHeadlight();

/*!
*/

void
SoXtFullViewer::selectedPrefs(
  void )
{
  if ( ! this->prefshell )
    this->createPrefSheet();
} // selectedPrefs()

/*!
*/

void
SoXtFullViewer::interactbuttonClicked(
  Boolean set )
{
#if SOXT_DEBUG && 0
  SoDebugError::postInfo( "SoXtFullViewer::interactbuttonClicked",
    "%s", set ? "Set" : "Unset" );
#endif // SOXT_DEBUG
  if ( this->isViewing() && set )
    this->setViewing( FALSE );
  else if ( ! this->isViewing() && ! set )
    this->setViewing( TRUE );
} // interactbuttonClicked()

/*!
*/

void
SoXtFullViewer::interactbuttonCB(
  Widget,
  XtPointer client_data,
  XtPointer call_data )
{
  XmToggleButtonCallbackStruct * data =
    (XmToggleButtonCallbackStruct *) call_data;
  ((SoXtFullViewer *) client_data)->interactbuttonClicked( data->set );
} // interactbuttonCB()

/*!
*/

void
SoXtFullViewer::examinebuttonClicked(
  Boolean set )
{
#if SOXT_DEBUG && 0
  SoDebugError::postInfo( "SoXtFullViewer::examinebuttonClicked",
    "%s", set ? "Set" : "Unset" );
#endif // SOXT_DEBUG
  if ( this->isViewing() && ! set )
    this->setViewing( FALSE );
  else if ( ! this->isViewing() && set )
    this->setViewing( TRUE );
} // examinebuttonClicked();

/*!
*/

void
SoXtFullViewer::examinebuttonCB(
  Widget,
  XtPointer client_data,
  XtPointer call_data )
{
  XmToggleButtonCallbackStruct * data =
    (XmToggleButtonCallbackStruct *) call_data;
  ((SoXtFullViewer *) client_data)->examinebuttonClicked( data->set );
} // examinebuttonCB()

/*!
*/

void
SoXtFullViewer::helpbuttonClicked(
  void )
{
  this->openViewerHelpCard();
} // helpbuttonClicked()

/*!
*/

void
SoXtFullViewer::helpbuttonCB(
  Widget,
  XtPointer client_data,
  XtPointer )
{
  ((SoXtFullViewer *) client_data)->helpbuttonClicked();
} // helpbuttonCB()

/*!
*/

void
SoXtFullViewer::homebuttonClicked(
  void )
{
  this->resetToHomePosition();
} // homebuttonClicked()

/*!
*/

void
SoXtFullViewer::homebuttonCB(
  Widget,
  XtPointer client_data,
  XtPointer )
{
  ((SoXtFullViewer *) client_data)->homebuttonClicked();
} // homebuttonCB()

/*!
*/

void
SoXtFullViewer::sethomebuttonClicked(
  void )
{
  this->saveHomePosition();
} // sethomebuttonClicked()

/*!
*/

void
SoXtFullViewer::sethomebuttonCB(
  Widget,
  XtPointer client_data,
  XtPointer )
{
  ((SoXtFullViewer *) client_data)->sethomebuttonClicked();
} // sethomebuttonCB()

/*!
*/

void
SoXtFullViewer::viewallbuttonClicked(
  void )
{
  this->viewAll();
} // viewallbuttonClicked()

/*!
*/

void
SoXtFullViewer::viewallbuttonCB(
  Widget,
  XtPointer client_data,
  XtPointer )
{
  ((SoXtFullViewer *) client_data)->viewallbuttonClicked();
} // viewallbuttonCB()

/*!
*/

void
SoXtFullViewer::seekbuttonClicked(
  void )
{
  if ( ! this->isViewing() ) {
#if SOXT_DEBUG
    SoDebugError::postInfo( "SoXtFullViewer::seekbuttonClicked",
      "seek button should not be enabled in interaction mode!" );
#endif // SOXT_DEBUG
    return;
  }
  this->setSeekMode(this->isSeekMode() ? FALSE : TRUE);
} // seekbuttonClicked()

/*!
*/

void
SoXtFullViewer::seekbuttonCB(
  Widget,
  XtPointer client_data,
  XtPointer )
{
  ((SoXtFullViewer *) client_data)->seekbuttonClicked();
} // seekbuttonCB()

/*!
*/

void
SoXtFullViewer::copyviewSelected(
  void )
{
  this->copyView( SbTime::getTimeOfDay() );
} // copyviewSelected()

/*!
*/

void
SoXtFullViewer::pasteviewSelected(
  void )
{
  this->pasteView( SbTime::getTimeOfDay() );
} // pasteviewSelected()

/*!
*/

void
SoXtFullViewer::drawstyleActivated(
  int item )
{
  SOXT_STUB();
} // drawstyleActivated()

// *************************************************************************
// preferences sheet hooks:

// *************************************************************************
