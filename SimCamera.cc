#include "SimCamera.hh"


SimCamera::SimCamera( Ogre::SceneManager *sMan )
{
  sceneMan = sMan;
  camNode  = sceneMan->getRootSceneNode()->createChildSceneNode( "MainCamera" );
  cam      = sceneMan->createCamera( "MainCamera" );

  camNode->attachObject( cam );

  upKey    = false;
  downKey  = false;
  leftKey  = false;
  rightKey = false;
  turnLeftKey  = false;
  turnRightKey = false;

  turbo = false;
  menuMode = false;

  cam->setPolygonMode( Ogre::PolygonMode::PM_WIREFRAME );
}



SimCamera::~SimCamera()
{
  camNode->detachAllObjects();
  delete cam;
  sceneMan->destroySceneNode( "MainCamera" );
}



Ogre::Camera *SimCamera::GetCamera()
{
  return cam;
}



void SimCamera::Update( Ogre::Real timeSinceLastFrame )
{
	float speed = 0.1;
	if( turbo )
	{
		speed = 5.0;
	}

	if( upKey )
		camNode->setPosition( camNode->getPosition() + cam->getRealDirection() * Ogre::Vector3::Vector3( speed ) );
	if( downKey )
		camNode->setPosition( camNode->getPosition() + cam->getRealDirection() * Ogre::Vector3::Vector3( -speed ) );
	if( rightKey )
		camNode->setPosition( camNode->getPosition() + cam->getRealRight() * Ogre::Vector3::Vector3( speed ) );
	if( leftKey )
		camNode->setPosition( camNode->getPosition() + cam->getRealRight() * Ogre::Vector3::Vector3( -speed ) );
	if( turnRightKey )
		camNode->setPosition( camNode->getPosition() + Ogre::Vector3::Vector3( 0.0, speed, 0.0 ) );
	if( turnLeftKey )
		camNode->setPosition( camNode->getPosition() + Ogre::Vector3::Vector3( 0.0, -speed, 0.0  ) );
}



bool SimCamera::keyPressed( const OIS::KeyEvent &arg )
{
  switch( arg.key )
  {
    case( OIS::KC_UP ):
    case( OIS::KC_W ):
      upKey = true;
      break;

    case( OIS::KC_DOWN ):
    case( OIS::KC_S ):
      downKey = true;
      break;

    case( OIS::KC_RIGHT ):
    case( OIS::KC_D ):
      rightKey = true;
      break;

    case( OIS::KC_LEFT ):
    case( OIS::KC_A ):
      leftKey = true;
      break;

    case( OIS::KC_Q ):
      turnRightKey = true;
      break;

    case( OIS::KC_E ):
      turnLeftKey = true;
      break;

	case( OIS::KC_LSHIFT ):
      turbo = true;
      break;

	case( OIS::KC_LMENU ):
      menuMode = true;
      break;
  }
  return true;
}



bool SimCamera::keyReleased( const OIS::KeyEvent &arg )
{
  switch( arg.key )
  {
    case( OIS::KC_UP ):
    case( OIS::KC_W ):
      upKey = false;
      break;

    case( OIS::KC_DOWN ):
    case( OIS::KC_S ):
      downKey = false;
      break;

    case( OIS::KC_RIGHT ):
    case( OIS::KC_D ):
      rightKey = false;
      break;

    case( OIS::KC_LEFT ):
    case( OIS::KC_A ):
      leftKey = false;
      break;

    case( OIS::KC_Q ):
      turnRightKey = false;
      break;

    case( OIS::KC_E ):
      turnLeftKey = false;
      break;

	case( OIS::KC_LSHIFT ):
      turbo = false;
      break;

	case( OIS::KC_LMENU ):
      menuMode = false;
      break;
  }

  return true;
}



bool SimCamera::mouseMoved( const OIS::MouseEvent &arg )
{
  if( menuMode )
  {
	  return true;
  }

  cam->yaw( Ogre::Radian( Ogre::Degree( arg.state.X.rel ) ) );
  cam->pitch( Ogre::Radian( Ogre::Degree( -arg.state.Y.rel ) ) );
  return true;
}



bool SimCamera::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  return true;
}



bool SimCamera::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
  return true;
}
