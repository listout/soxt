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

#ifndef SOXT_PRINTDIALOG_H
#define SOXT_PRINTDIALOG_H

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

#include <Inventor/misc/SoCallbackList.h>

#include <Inventor/Xt/SoXtComponent.h>

class SoGLRenderAction;
class SoXtPrintDialog;

// *************************************************************************

typedef void SoXtPrintDialogCB( void * userdata, SoXtPrintDialog * dialog );

class SoXtPrintDialog : public SoXtComponent {
  typedef SoXtComponent inherited;

public:
  SoXtPrintDialog(
    Widget parent = NULL,
    const char * const name = NULL,
    SbBool inParent = TRUE );
  SoXtPrintDialog(
    SoGLRenderAction * const action,
    Widget parent = NULL,
    const char * const name = NULL,
    SbBool inParent = TRUE );
  ~SoXtPrintDialog(void);

  void setSceneGraph( SoPath * path );
  void setSceneGraph( SoNode * root );

  SoNode * getSceneGraph(void);
  SoPath * getSceneGraphPath(void);

  void setGLRenderAction( const SoGLRenderAction * const action );
  SoGLRenderAction * getGLRenderAction(void);

  void setPrintSize( const SbVec2f & size );
  void setPrintSize( const SbVec2s size );

  void setBeforePrintCallback(
    SoXtPrintDialogCB * callback, void * user = NULL );
  void setAfterPrintCallback(
    SoXtPrintDialogCB * callback, void * user = NULL );

protected:
  SoXtPrintDialog(
    Widget parent, const char * const name, SbBool inParent, SbBool build );

  virtual const char * getDefaultWidgetName(void) const;
  virtual const char * getDefaultTitle(void) const;
  virtual const char * getDefaultIconTitle(void) const;

  SoNode * rootNode;
  SoPath * rootPath;
  SbBool printDone;
  SbBool highQuality;
  SbBool portraitFormat;
  SbBool printerOutput, postScriptOutput;
  SbBool nodeMostRecent;
  SbBool WYSIWYGflag;
  Widget messageWidget, printButton, quitButton;
  Widget messageLabelWidget, fileFormatWidget;
  Widget toPrinterWidget, toPostScriptFileWidget, toRGBFileWidget;
  Widget printerHorizSize, printerVertSize;
  Widget postScriptHorizSize, postScriptVertSize;
  Widget rgbHorizSize, rgbVertSize;
  Widget printerDPIField, postScriptDPIField;
  Widget rgbFilenameWidget, postScriptFilenameWidget;
  SbPList * printers;
  char * defaultPrinter;
  int whichPrinter;
  SbVec2f printSize;
  SbVec2s printRes;
  SoCallbackList beforeList, afterList;
  SbBool alreadyUpdated;

  void print(void);
  void getPrinterList(void);
  void printToPostScript( SoNode *, char *, int, int );

  void buildToPrinterWidget( Widget parent );
  void buildToPostScriptFileWidget( Widget parent );
  void buildToRGBFileWidget( Widget parent );
  void placeBottomOfDialog( SoXtPrintDialog * );
  void buildRadioButton( char *,  char *, char *, int, int,
    Widget, XtCallbackProc );
  void buildSizeFields( char *, int, Widget, Widget &, Widget &,
    XtCallbackProc, XtCallbackProc );
  void buildDPIField( int, Widget, Widget &, XtCallbackProc );
  void updateTextports(void);

// callbacks
  static void qualityCB( Widget, SoXtPrintDialog *, XmAnyCallbackStruct * );
  static void pageFormatCB( Widget, SoXtPrintDialog *, XmAnyCallbackStruct * );
  static void fileFormatCB( Widget, SoXtPrintDialog *, XmAnyCallbackStruct * );
  static void printerHorizSizeCB(
    Widget, SoXtPrintDialog *, XmAnyCallbackStruct * );
  static void printerVertSizeCB(
    Widget, SoXtPrintDialog *, XmAnyCallbackStruct * );
  static void postScriptHorizSizeCB(
    Widget, SoXtPrintDialog *, XmAnyCallbackStruct * );
  static void postScriptVertSizeCB(
    Widget, SoXtPrintDialog *, XmAnyCallbackStruct * );
  static void rgbHorizSizeCB(
    Widget, SoXtPrintDialog *, XmAnyCallbackStruct * );
  static void rgbVertSizeCB( Widget, SoXtPrintDialog *, XmAnyCallbackStruct * );
  static void printerDPICB( Widget, SoXtPrintDialog *, XmAnyCallbackStruct * );
  static void postScriptDPICB(
    Widget, SoXtPrintDialog *, XmAnyCallbackStruct * );
  static void outputCB( Widget, SoXtPrintDialog *, XmAnyCallbackStruct * );
  static void printCB( Widget, SoXtPrintDialog *, XmAnyCallbackStruct * );
  static void quitCB( Widget, SoXtPrintDialog *, XmAnyCallbackStruct * );
  static void listPick( Widget, SoXtPrintDialog *, XmAnyCallbackStruct * );

  Widget styleButton;
  int currentStyle;

private:
  void constructor( SbBool build );
  Widget buildWidget( Widget parent );

}; // class SoXtPrintDialog

// *************************************************************************

#endif // ! SOXT_PRINTDIALOG_H