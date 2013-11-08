
#ifndef _SIMCAMERA_HH_
#define _SIMCAMERA_HH_

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreFrameListener.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>



class SimCamera: public OIS::KeyListener, public OIS::MouseListener
{
  public:
	SimCamera( Ogre::SceneManager* );
	~SimCamera();

	Ogre::Camera *GetCamera();

	void Update( Ogre::Real );

	virtual bool keyPressed( const OIS::KeyEvent &arg );
	virtual bool keyReleased( const OIS::KeyEvent &arg );

	virtual bool mouseMoved( const OIS::MouseEvent &arg );
	virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );


  protected:
	Ogre::Camera       *cam;
	Ogre::SceneNode    *camNode;
	Ogre::SceneManager *sceneMan;

	Ogre::Vector3 velocity;
	Ogre::Real turnVelocity;

	bool upKey,
	     downKey,
	     rightKey,
	     leftKey,
	     turnLeftKey,
	     turnRightKey,
		 turbo,
		 menuMode;
};

#endif
