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

#include <Xm/PushB.h>

#include <Inventor/errors/SoDebugError.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/projectors/SbSphereSheetProjector.h>
#include <Inventor/sensors/SoTimerSensor.h>

#include <Inventor/Xt/SoXt.h>
#include <Inventor/Xt/SoXtBasic.h>
#include <Inventor/Xt/viewers/SoAnyExaminerViewer.h>

#include <Inventor/Xt/viewers/SoXtExaminerViewer.h>

#if HAVE_LIBXPM
#include "icons/ortho.xpm"
#include "icons/perspective.xpm"
#endif // HAVE_LIBXPM

// *************************************************************************

SoXtExaminerViewer::SoXtExaminerViewer(
  Widget parent,
  const char * name,
  SbBool inParent,
  SoXtFullViewer::BuildFlag flag,
  SoXtViewer::Type type )
: inherited( parent, name, inParent, flag, type, FALSE )
, SoAnyExaminerViewer( this )
{
  this->constructor( TRUE );
} // SoXtExaminerViewer()

SoXtExaminerViewer::SoXtExaminerViewer( // protected
  Widget parent,
  const char * name,
  SbBool inParent,
  SoXtFullViewer::BuildFlag flag,
  SoXtViewer::Type type,
  SbBool build )
: inherited( parent, name, inParent, flag, type, FALSE )
, SoAnyExaminerViewer( this )
{
  this->constructor( build );
} // SoXtExaminerViewer()

void
SoXtExaminerViewer::constructor( // private
  SbBool build )
{
// don't uncomment this until constructor sends FALSE to FullViewer class
  this->mode = EXAMINE;

  this->spindetecttimerId = 0;
  this->spindetecttimerActive = FALSE;

  this->setClassName( "SoXtExaminerViewer" );
  this->camerabutton = (Widget) NULL;

  if ( build ) {
    Widget viewer = this->buildWidget( this->getParentWidget() );
    this->setBaseWidget( viewer );
  }
} // constructor()

/*!
  The destructor.
*/

SoXtExaminerViewer::~SoXtExaminerViewer(
  void )
{
} // ~SoXtExaminerViewer()

// *************************************************************************

void
SoXtExaminerViewer::processEvent(
  XAnyEvent * event )
{
  if ( this->processCommonEvents(event) ) return;

  SbVec2s canvassize = this->getGLSize();
  SbVec2s mousepos( 0, 0 );
  switch ( event->type ) {
  case ButtonPress:
  case ButtonRelease:
    mousepos[0] = ((XButtonEvent *)event)->x;
    mousepos[1] = ((XButtonEvent *)event)->y;
    break;
    
  case MotionNotify:
    mousepos[0] = ((XPointerMovedEvent *)event)->x;
    mousepos[1] = ((XPointerMovedEvent *)event)->y;
    break;

  default:
    break;
  }

  mousepos[1] = canvassize[1] - mousepos[1];

  SbVec2f norm_mousepos( (float)mousepos[0]/(float)canvassize[0],
                         (float)mousepos[1]/(float)canvassize[1] );

  switch ( event->type ) {

  case ButtonPress:
    this->lastmouseposition = norm_mousepos;
    this->lastspinposition = norm_mousepos;
    if ( ((XButtonEvent *) event)->button == Button3 )
      break;

    if ( ((XButtonEvent *) event)->button == Button4 ) {
      this->zoom( 0.1f );
      break;
    }
      
    if ( ((XButtonEvent *) event)->button == Button5 ) {
      this->zoom( -0.1f );
      break;
    }

    this->interactiveCountInc();

    if ( this->mode == WAITING_FOR_SEEK ) {
      this->seekToPoint( mousepos );
    } else {
      do {
        unsigned int state = ((XButtonEvent *)event)->state;
        switch ( ((XButtonEvent *)event)->button ) {
        case Button1: state |= Button1Mask; break;
        case Button2: state |= Button2Mask; break;
        case Button3: state |= Button3Mask; break;
        case Button4: state |= Button4Mask; break;
        case Button5: state |= Button5Mask; break;
        default: break;
        }
        this->setModeFromState( state );
        if ( this->isAnimating() )
          this->stopAnimating();
      } while ( FALSE );
    }

    break;

  case ButtonRelease:
    if ( ((XButtonEvent *)event)->button != Button1 &&
         ((XButtonEvent *)event)->button != Button2 )
      break;

    if ( this->mode == DRAGGING &&
         this->isAnimationEnabled() &&
         this->spindetecttimerActive ) {
      XtRemoveTimeOut( this->spindetecttimerId );
      this->spindetecttimerId = 0;
      this->spindetecttimerActive = FALSE;
      this->spinanimating = TRUE;
      this->spintimertrigger->schedule();
      this->interactiveCountInc();
    }

    this->interactiveCountDec();
    do {
      unsigned int state = ((XButtonEvent *)event)->state;
      switch ( ((XButtonEvent *)event)->button ) {
      case Button1: state &= ~Button1Mask; break;
      case Button2: state &= ~Button2Mask; break;
      case Button3: state &= ~Button3Mask; break;
      case Button4: state &= ~Button4Mask; break;
      case Button5: state &= ~Button5Mask; break;
      default: break;
      }
      this->setModeFromState( state );
    } while ( FALSE );
    break;

  case MotionNotify:
    // XPointerMovedEvent
    switch ( this->mode ) {
    case DRAGGING:
      if ( this->spindetecttimerId )
        XtRemoveTimeOut( this->spindetecttimerId );
      this->spindetecttimerId =
        XtAppAddTimeOut( SoXt::getAppContext(), 10,
          SoXtExaminerViewer::spindetecttimerCB, this );
        this->spindetecttimerActive = TRUE;
        this->spin( norm_mousepos );
      break;

    case PANNING:
      this->pan( norm_mousepos );
      break;

    case ZOOMING:
      this->zoomByCursor( norm_mousepos );
      break;

    default:
      break;
    }
    break;

  case KeyPress:
    switch ( ((XKeyPressedEvent *) event)->keycode ) {
    default:
      break;
    } // switch ( ((XKeyPressedEvent *) event)->keycode )
    break;

  case KeyRelease:
    switch ( ((XKeyReleasedEvent *) event)->keycode ) {
    default:
      break;
    } // switch ( ((XKeyReleasedEvent *) event)->keycode )
    break;

  default:
    break;

  } // switch ( event )

  this->lastmouseposition = norm_mousepos;

  inherited::processEvent( event );
} // processEvent()

// *************************************************************************

/*!
*/

void
SoXtExaminerViewer::leftWheelMotion( // virtual, protected
  float value )
{
  if ( this->isAnimating() )
    this->stopAnimating();
  inherited::leftWheelMotion(
    rotXWheelMotion( value, this->getLeftWheelValue() ) );
} // leftWheelMotion()

/*!
*/

void
SoXtExaminerViewer::bottomWheelMotion( // virtual, protected
  float value )
{
  if ( this->isAnimating() )
    this->stopAnimating();
  inherited::bottomWheelMotion(
    rotYWheelMotion( value, this->getLeftWheelValue() ) );
} // bottomWheelMotion()

/*!
*/

void
SoXtExaminerViewer::rightWheelMotion( // virtual, protected
  float value )
{
  this->zoom( this->getRightWheelValue() - value );
  inherited::rightWheelMotion( value );
} // rightWheelMotion()

// *************************************************************************

/*!
*/

const char *
SoXtExaminerViewer::getDefaultWidgetName(
  void ) const
{
  static const char defaultWidgetName[] = "SoXtExaminerViewer";
  return defaultWidgetName;
} // getDefaultWidgetName()

/*!
*/

const char *
SoXtExaminerViewer::getDefaultTitle(
  void ) const
{
  static const char defaultTitle[] = "Examiner Viewer";
  return defaultTitle;
} // getDefaultTitle()

/*!
*/

const char *
SoXtExaminerViewer::getDefaultIconTitle(
  void ) const
{
  static const char defaultIconTitle[] = "Examiner Viewer";
  return defaultIconTitle;
} // getDefaultIconTitle()

// *************************************************************************

/*!
*/

void
SoXtExaminerViewer::openViewerHelpCard(
  void )
{
  this->openHelpCard( "SoXtExaminerViewer.help" );
} // openViewerHelpCard()

// *************************************************************************

void
SoXtExaminerViewer::setModeFromState(
  const unsigned int state )
{
  ViewerMode mode = EXAMINE;

  const unsigned int maskedstate =
    state & (Button1Mask | Button2Mask | ControlMask);

  switch ( maskedstate ) {
  case 0:
    mode = EXAMINE;
    break;

  case Button1Mask:
    mode = DRAGGING;
    break;

  case Button2Mask:
  case (Button1Mask | ControlMask):
    mode = PANNING;
    break;

  case ControlMask:
    mode = WAITING_FOR_PAN;
    break;

  case (Button2Mask | ControlMask):
  case (Button1Mask | Button2Mask):
  case (Button1Mask | Button2Mask | ControlMask):
    mode = ZOOMING;
    break;

  default:
    assert(0);
    break;
  }

  this->setMode( mode );
} // setModeFromState()

void
SoXtExaminerViewer::setMode(
  const ViewerMode mode )
{
  switch ( mode ) {

  case INTERACT:
    if ( this->isAnimating() )
      this->stopAnimating();
    while ( this->getInteractiveCount() )
      this->interactiveCountDec();
    break;

  case DRAGGING:
    this->spinprojector->project( this->lastmouseposition );
    break;

  case PANNING:
    do {
      SoCamera * camera = this->getCamera();
      SbViewVolume volume = camera->getViewVolume( this->getGLAspectRatio() );
      this->panningplane = volume.getPlane( camera->focalDistance.getValue() );
    } while ( FALSE );
    break;

  default:
    break;

  } // switch ( mode )

  this->mode = mode;
} // setMode()

// *************************************************************************

void
SoXtExaminerViewer::spindetecttimerCB( // static
  XtPointer user,
  XtIntervalId * id )
{
  SoXtExaminerViewer * that = (SoXtExaminerViewer *) user;
  that->spindetecttimerId = 0;
  that->spindetecttimerActive = FALSE;
#if SOXT_DEBUG && 0
  SoDebugError::postInfo( "SoXtExaminerViewer::spindetecttimerCB",
    "called - but doin' squat" );
#endif // SOXT_DEBUG
} // spindetecttimerCB()

// *************************************************************************

/*!
*/

void
SoXtExaminerViewer::createViewerButtons( // virtual
  Widget parent,
  SbPList * buttonlist )
{
  assert( this->camerabutton == (Widget) NULL );

  inherited::createViewerButtons( parent, buttonlist );

  this->camerabutton = XtVaCreateManagedWidget( "C",
    xmPushButtonWidgetClass, parent,
    XmNshadowType, XmSHADOW_OUT,
    XmNhighlightThickness, 2,
    XmNshadowThickness, 2,
    XmNtraversalOn, False,
    XmNwidth, 30,
    XmNheight, 30,
    NULL );

  XtAddCallback( this->camerabutton,
    XmNdisarmCallback, SoXtExaminerViewer::camerabuttonCB, this );

  buttonlist->append( this->camerabutton );

  this->camerapixmaps.ortho =
    createPixmapFromXpmData( this->camerabutton, ortho_xpm );
  this->camerapixmaps.ortho_ins =
    createInsensitivePixmapFromXpmData( this->camerabutton, ortho_xpm );
  this->camerapixmaps.perspective =
    createPixmapFromXpmData( this->camerabutton, perspective_xpm );
  this->camerapixmaps.perspective_ins =
    createInsensitivePixmapFromXpmData( this->camerabutton, perspective_xpm );

} // createViewerButtons()

// *************************************************************************

/*!
*/

void
SoXtExaminerViewer::camerabuttonClicked(
  void )
{
  this->toggleCameraType();

/*
  SoCamera * camera = this->getCamera();
  if ( ! camera ) {
#if SOXT_DEBUG
    SoDebugError::postWarning( "SoXtExaminerViewer::camerabuttonClicked",
      "no camera" );
#endif // SOXT_DEBUG
    return;
  }

  if ( camera->isOfType( SoPerspectiveCamera::getClassTypeId() ) ) {
    this->setCamera( new SoOrthographicCamera );
  } else if ( camera->isOfType( SoPerspectiveCamera::getClassTypeId() ) ) {
    this->setCamera( new SoPerspectiveCamera );
  } else {
#if SOXT_DEBUG
    SoDebugError::postWarning( "SoXtExaminerViewer::camerabuttonClicked",
      "unknown camera - got no pixmap" );
#endif // SOXT_DEBUG
  }

*/

} // camerabuttonClicked()

/*!
*/

void
SoXtExaminerViewer::camerabuttonCB( // static
  Widget w,
  XtPointer client_data,
  XtPointer call_data )
{
  SoXtExaminerViewer * viewer = (SoXtExaminerViewer *) client_data;
  viewer->camerabuttonClicked();
} // camerabuttonCB()

// *************************************************************************

/*!
*/

void
SoXtExaminerViewer::setCamera( // virtual
  SoCamera * camera )
{
  Pixmap pixmap, pixmap_ins;
  if ( camera == NULL ) {
    // find better pixmaps for this...
    pixmap = this->camerapixmaps.ortho;
    pixmap_ins = this->camerapixmaps.ortho_ins;
  } else if ( camera->isOfType( SoPerspectiveCamera::getClassTypeId() ) ) {
    pixmap = this->camerapixmaps.perspective;
    pixmap_ins = this->camerapixmaps.perspective_ins;
  } else if ( camera->isOfType( SoOrthographicCamera::getClassTypeId() ) ) {
    pixmap = this->camerapixmaps.ortho;
    pixmap_ins = this->camerapixmaps.ortho_ins;
  } else {
    SoDebugError::postWarning( "SoXtExaminerViewer::setCamera",
      "unknown camera type - got no pixmap" );
    // find better pixmaps for this...
    pixmap = this->camerapixmaps.ortho;
    pixmap_ins = this->camerapixmaps.ortho_ins;
  }

/*
  FIXME: this somehow disturbs the layout of the hole viewer button row..
  XtVaSetValues( this->camerabutton,
    XmNlabelType, XmPIXMAP,
    XmNlabelPixmap, pixmap,
    XmNselectPixmap, pixmap,
    XmNlabelInsensitivePixmap, pixmap_ins,
    XmNselectInsensitivePixmap, pixmap_ins,
    XmNwidth, 30,
    XmNheight, 30,
    NULL );
*/

  // FIXME: update zoom/dolly string

  inherited::setCamera( camera );
} // setCamera()

// *************************************************************************
