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

#ifndef SOXT_DEVICE_H
#define SOXT_DEVICE_H

#include <X11/Intrinsic.h>

#include <Inventor/SbLinear.h>

#include <Inventor/Xt/SoXtBasic.h>

class SoEvent;
class SbPList;

// *************************************************************************

class SOXT_DLL_EXPORT SoXtDevice {
public:
  virtual ~SoXtDevice(void);

  virtual void enable( Widget widget, XtEventHandler handler,
      XtPointer closure, Window window = (Window) NULL ) = 0;
  virtual void disable( Widget widget, XtEventHandler handler,
      XtPointer closure ) = 0;

  virtual const SoEvent * translateEvent( XAnyEvent * xevent ) = 0;

  void setWindowSize( const SbVec2s size );
  const SbVec2s getWindowSize(void) const;

protected:
  SoXtDevice(void);

  void setEventPosition( SoEvent * event, int x, int y ) const;

  void addEventHandler( Widget, XtEventHandler, XtPointer, Window );
  void removeEventHandler( Widget, XtEventHandler, XtPointer );

  void invokeHandlers( XEvent * const event );

private:
  SbVec2s size;

  SbPList * handlers;

}; // class SoXtDevice

// *************************************************************************

/*
  Customized XEvent-like structures
*/

#define soxt6dofDeviceButtonPressedEvent   (LASTEvent + 1)
#define soxt6dofDeviceButtonReleasedEvent  (LASTEvent + 2)
#define soxt6dofDevicePressureEvent        (LASTEvent + 3)

typedef struct {
  int type;
  unsigned long serial;
  Bool send_event;
  Display * display;
  Window window;
  Time time;
  unsigned int state;
  unsigned int button;
  unsigned int buttons;
} SoXt6dofDeviceButtonEvent;
typedef SoXt6dofDeviceButtonEvent SoXt6dofDeviceButtonPressedEvent;
typedef SoXt6dofDeviceButtonEvent SoXt6dofDeviceButtonReleasedEvent;

typedef struct {
  int type;
  unsigned long serial;
  Bool send_event;
  Display * display;
  Window window;
  Time time;
  float trans_x, trans_y, trans_z;
  float rot_x, rot_y, rot_z;
} SoXt6dofDevicePressureEvent;

// *************************************************************************

#endif // ! SOXT_DEVICE_H
