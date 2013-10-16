#pragma once
#ifndef _SIMAPP_HH_
#define _SIMAPP_HH_

#include "OGRE/OgreRoot.h"
#include "OGRE/OgreRenderSystem.h"
#include "OGRE/OgreRenderWindow.h"
#include "OGRE/OgreWindowEventUtilities.h"

#include "predefs.hh"

#include "SimState.hh"
#include "SimGUI.hh"

#include <stack>
#include <memory>


/* SimApp class holds the general
   resources used app-wide. */
class SimApp : public Ogre::WindowEventListener
{
  public:
	~SimApp();

	void Init();
	void Unload();
	void Update();

	bool IsClosed() const;
	void Close();
	
	// Window event listeners
	virtual void windowMoved( Ogre::RenderWindow* );
	virtual void windowResized( Ogre::RenderWindow* );
	virtual bool windowClosing( Ogre::RenderWindow* );
	virtual void windowClosed( Ogre::RenderWindow* );
	virtual void windowFocusChange( Ogre::RenderWindow* );

  private:


  protected:
	bool closed;
	std::auto_ptr<Ogre::Root> ogreRoot;
	Ogre::RenderWindow *mainWindow;
	Ogre::SceneManager *sceneManager;
	
	// States
	std::stack<SimState*> stateStack;

	// GUI
	SimGUI *gui;

	// Input
	OIS::InputManager *inputManager;
	OIS::Mouse        *mouse;
	OIS::Keyboard     *keyboard;

  // Friend classes:
  friend SimGUI;
  friend SimState;
};

#endif
