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

//  $Id$

#ifndef SOXT_EXAMINERVIEWER_H
#define SOXT_EXAMINERVIEWER_H

#include <Inventor/Xt/viewers/SoXtFullViewer.h>
#include <Inventor/Xt/viewers/SoAnyExaminerViewer.h>

class SbRotation;
class SoTimerSensor;

// *************************************************************************

class SoXtExaminerViewer :
  public SoXtFullViewer,
  public SoAnyExaminerViewer
{
  typedef SoXtFullViewer inherited;
  friend class SoAnyExaminerViewer;

public:
  SoXtExaminerViewer( Widget parent = (Widget) NULL,
      const char * name = NULL, SbBool inParent = TRUE,
      SoXtFullViewer::BuildFlag flags = BUILD_ALL,
      SoXtViewer::Type type = BROWSER );
  ~SoXtExaminerViewer(void);

protected:
  SoXtExaminerViewer( Widget parent, const char * name,
      SbBool inParent, SoXtFullViewer::BuildFlag flags,
      SoXtViewer::Type type, SbBool build );

  virtual void setCamera( SoCamera * camera );

  virtual void processEvent( XAnyEvent * event );

  virtual void leftWheelMotion( float value );
  virtual void bottomWheelMotion( float value );
  virtual void rightWheelMotion( float value );

  void openViewerHelpCard(void);

  virtual const char * getDefaultWidgetName(void) const;
  virtual const char * getDefaultTitle(void) const;
  virtual const char * getDefaultIconTitle(void) const;

  XtIntervalId spindetecttimerId;
  SbBool spindetecttimerActive;
  static void spindetecttimerCB( XtPointer user, XtIntervalId * id );

  virtual void createViewerButtons( Widget parent, SbPList * buttonlist );
  void camerabuttonClicked(void);
  static void camerabuttonCB( Widget, XtPointer, XtPointer );

private:
  void constructor( SbBool build );

  enum ViewerMode {
    IDLE,
    INTERACT,
    EXAMINE,
    DRAGGING,
    WAITING_FOR_SEEK,
    ZOOMING,
    WAITING_FOR_PAN,
    PANNING
  } mode;

  void setMode( const ViewerMode mode );
  void setModeFromState( const unsigned int state );

  Widget camerabutton;
  struct {
    Pixmap ortho, ortho_ins;
    Pixmap perspective, perspective_ins;
    Pixmap nocam, nocam_ins;
  } camerapixmaps;

}; // class SoXtExaminerViewer

// *************************************************************************

#endif // ! SOXT_EXAMINERVIEWER_H
