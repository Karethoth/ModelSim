#pragma once
#ifndef _SIMGUI_HH_
#define _SIMGUI_HH_

#include "predefs.hh"
#include "MyGUI.h"
#include "MyGUI_OgrePlatform.h"
#include "OIS/OIS.h"


class SimGUI : public OIS::MouseListener, public OIS::KeyListener
{
  public:
	SimGUI( SimApp& );
	void Load();
	void Unload();
	void Update();
	void Resize( int, int );

	// OIS Event Handlers
	bool mouseMoved( const OIS::MouseEvent& );
	bool mousePressed( const OIS::MouseEvent&, OIS::MouseButtonID );
	bool mouseReleased( const OIS::MouseEvent&, OIS::MouseButtonID );
	bool keyPressed( const OIS::KeyEvent& );
	bool keyReleased( const OIS::KeyEvent& );
 
	// MyGUI events:
	void MenuBarWidgetClicked( MyGUI::Widget* );


  protected:
	SimApp &app;
	MyGUI::Gui *myGui;
	MyGUI::OgrePlatform *platform;
};

#endif
