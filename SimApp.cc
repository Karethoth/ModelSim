#include "SimApp.hh"

#include <exception>

using std::cout;
using std::string;


SimApp::SimApp()
{
	gui = nullptr;
	curl = nullptr;
}

SimApp::~SimApp()
{
}



void SimApp::Init()
{
	try
	{
		closed = false;

		// Initialize curl
		curl = curl_easy_init();
		if( !curl )
		{
			throw std::exception( "Fatal error: CURL could not be initialized!\n" );
		}

		// Create Ogre root
		Ogre::String ogreConfigFile = "ogre.cfg";
		Ogre::String ogrePluginsFile = "";
		Ogre::String ogreLogFile = "log.txt";
		ogreRoot = std::auto_ptr<Ogre::Root>( new Ogre::Root() );
 
		// Load required plugins
		{
			std::vector<Ogre::String> lPluginNames;
			lPluginNames.push_back( "RenderSystem_GL" ); // For now, just GL is supported.
			{
				std::vector<Ogre::String>::iterator lIter = lPluginNames.begin();
				std::vector<Ogre::String>::iterator lIterEnd = lPluginNames.end();
				for( ; lIter != lIterEnd; lIter++ )
				{
					Ogre::String& lPluginName = (*lIter);
					// In debug mode, the plugins names are differents, they got a '_d' at the end.
					// So I add it. This is the main reason why I needed to put plugins names in a vector.
					// A macro or a function would have done the trick too...
					bool lIsInDebugMode = OGRE_DEBUG_MODE;
					if( lIsInDebugMode )
					{
						lPluginName.append( "_d" );
					}
					ogreRoot->loadPlugin( lPluginName );
				}
			}
		}

		
		// Load resource groups
		Ogre::String lNameOfResourceGroup = "Data";
		{
			Ogre::ResourceGroupManager& lRgMgr = Ogre::ResourceGroupManager::getSingleton();
			lRgMgr.createResourceGroup( lNameOfResourceGroup );
 
			Ogre::String lDirectoryToLoad = "data/gui";
			bool lIsRecursive = true;
			lRgMgr.addResourceLocation( lDirectoryToLoad, "FileSystem", "General", lIsRecursive );

			lDirectoryToLoad = "data/meshes";
			lIsRecursive = false;
			lRgMgr.addResourceLocation( lDirectoryToLoad, "FileSystem", lNameOfResourceGroup, lIsRecursive );
 
			// The function 'initialiseResourceGroup' parses scripts if any in the locations.
			lRgMgr.initialiseResourceGroup( lNameOfResourceGroup );
 
			// Files that can be loaded are loaded.
			lRgMgr.loadResourceGroup( lNameOfResourceGroup );

		}


		// Load or configure rendering settings
		ogreRoot->restoreConfig();
		if( ogreRoot->getRenderSystem() == nullptr )
		{
			ogreRoot->showConfigDialog();
		}

        mainWindow = ogreRoot->initialise( true, "SimApp" );

		sceneManager = ogreRoot->createSceneManager( Ogre::ST_GENERIC );

		Ogre::SceneNode* rootNode = sceneManager->getRootSceneNode();

		// Create camera
		/*Ogre::Camera* camera = sceneManager->createCamera( "MainCamera" );
		Ogre::SceneNode* cameraNode = rootNode->createChildSceneNode( "MainCameraNode" );
		cameraNode->attachObject( camera );*/
		camera = new SimCamera( sceneManager );

		

		// Create viewport
		float lViewportWidth = 1.0f;
		float lViewportHeight = 1.0f;
		float lViewportLeft =  (1.0f - lViewportWidth) * 0.5f;
		float lViewportTop = (1.0f - lViewportHeight) * 0.5f;
		unsigned short lMainViewpointZOrder = 100;

		Ogre::Viewport* vp = mainWindow->addViewport( camera->GetCamera(), lMainViewpointZOrder, lViewportLeft,
			                                          lViewportTop, lViewportWidth, lViewportHeight );
		vp->setAutoUpdated( true );
		vp->setBackgroundColour( Ogre::ColourValue( 0.1f, 0.1f, 0.1f ) );
		float ratio = float( vp->getActualWidth() ) / float( vp->getActualHeight() );
		camera->GetCamera()->setAspectRatio( ratio );
		camera->GetCamera()->setNearClipDistance( 1.5 );
		camera->GetCamera()->setFarClipDistance( 6000.0 );

		mainWindow->setActive( true );
		mainWindow->setAutoUpdated( false );


		// Init OIS
		OIS::ParamList pl;
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;
		mainWindow->getCustomAttribute( "WINDOW", &windowHnd );
		windowHndStr << windowHnd;
		pl.insert( std::make_pair( std::string("WINDOW"), windowHndStr.str() ) );

		// Use the OS mouse pointer
		#if defined OIS_WIN32_PLATFORM
			pl.insert(std::make_pair( std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )) );
			pl.insert(std::make_pair( std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")) );
			pl.insert(std::make_pair( std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")) );
			pl.insert(std::make_pair( std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")) );
		#elif defined OIS_LINUX_PLATFORM
			pl.insert(std::make_pair( std::string("x11_mouse_grab"), std::string("false")) );
			pl.insert(std::make_pair( std::string("x11_mouse_hide"), std::string("false")) );
			pl.insert(std::make_pair( std::string("x11_keyboard_grab"), std::string("false")) );
			pl.insert(std::make_pair( std::string("XAutoRepeatOn"), std::string("true")) );
		#endif

		inputManager = OIS::InputManager::createInputSystem( pl );
		mouse = static_cast<OIS::Mouse*>( inputManager->createInputObject( OIS::OISMouse, true ) );
		keyboard = static_cast<OIS::Keyboard*>( inputManager->createInputObject( OIS::OISKeyboard, true ) );

		const OIS::MouseState &mouseState = mouse->getMouseState();
		mouseState.width = mainWindow->getWidth();
		mouseState.height = mainWindow->getHeight();


		// Load GUI
		gui = new SimGUI( *this );
		gui->Load();
		
		// Temporary, used for testing TerrainLoader
		// Load some terrain:
		GeoCoord topLeft = GeoCoord( 63.631625, 26.967101, COORDSYS_DDDDDDDD );
		GeoCoord currentPoint;

		TerrainLoader terrainLoader( *this );
		terrainLoader.SetReferencePoint( topLeft );

		std::stringstream ss;
		Ogre::SceneNode* lNode = rootNode->createChildSceneNode();

		double latitudeStep = topLeft.Offset( GeoCoord( -1000.0, 0.0, COORDSYS_METERS ) ).latitude - topLeft.latitude;
		double longitudeStep = topLeft.Offset( GeoCoord( 0.0, 1000.0, COORDSYS_METERS ) ).longitude - topLeft.longitude;

		#define GRID_Y_SIZE 3
		#define GRID_X_SIZE 3

		for( int y=0; y < GRID_Y_SIZE; ++y )
		{
			currentPoint = topLeft.Offset( GeoCoord( y*latitudeStep, 0.0, COORDSYS_DDDDDDDD ) );
			currentPoint = currentPoint.Offset( GeoCoord( 0.0, longitudeStep/2.0, COORDSYS_DDDDDDDD ) );
		
			for( int x=0; x < GRID_X_SIZE; ++x )
			{
				std::cout << "\rLoading area...\t";
				std::cout << (y*GRID_X_SIZE + x) / (GRID_Y_SIZE*GRID_X_SIZE) * 100.0 << "%";

				ss.clear();
				ss << x << "_" << y;
				std::string id = ss.str();

				terrainLoader.LoadArea( currentPoint, latitudeStep/2.0, longitudeStep/2.0, 6, "TestArea"+id );

				// Create isntance of the mesh
				Ogre::String lNameOfTheMesh = "MeshTestArea"+id;
				Ogre::Entity* lEntity = sceneManager->createEntity( lNameOfTheMesh );

				Ogre::SceneNode* tmpNode = lNode->createChildSceneNode();
				tmpNode->attachObject( lEntity );
				//lEntity->setCastShadows( true );

				currentPoint = currentPoint.Offset( GeoCoord( 0.0, longitudeStep, COORDSYS_DDDDDDDD ) );
			}
		}
		std::cout << "\nFinished!\n";
		lNode->translate( 0, -100.0, 0.0f );

		// Create Light
		Ogre::SceneNode* lLightSceneNode = NULL;
		{
			Ogre::Light* lLight = sceneManager->createLight();
			lLight->setType( Ogre::Light::LT_DIRECTIONAL );
 
			lLight->setDiffuseColour( 0.6f, 0.6f, 0.6f ); // this will be a red light
			lLight->setSpecularColour( 1.0f, 1.0f, 1.0f );// color of 'reflected' light
 
			lLightSceneNode = rootNode->createChildSceneNode();
			lLightSceneNode->attachObject( lLight );
		}

		Ogre::ColourValue lAmbientColour( 0.2f, 0.2f, 0.2f, 1.0f );
		sceneManager->setAmbientLight( lAmbientColour );
		

		Ogre::WindowEventUtilities::addWindowEventListener( mainWindow, this );

		// Clear event times
		ogreRoot->clearEventTimes();
	}
	catch( Ogre::Exception &e )
	{
		throw std::exception( e.what() );
	}
	catch( std::exception &e )
	{
		throw e;
	}
	catch( ... )
	{
		throw std::exception( "Initializing failed." );
	}
}



void SimApp::Update()
{
	if( !mainWindow->isClosed() )
	{
		camera->Update( 0.1 );
		gui->Update();
		mainWindow->update( false );
		mainWindow->swapBuffers( true );
		ogreRoot->renderOneFrame();
		Ogre::WindowEventUtilities::messagePump();
	}
	else
	{
		closed = true;
	}
}



bool SimApp::IsClosed() const
{
	return closed;
}



void SimApp::Close()
{
	closed = true;
}



void SimApp::Unload()
{
	if( gui )
	{
		gui->Unload();
		gui = nullptr;
	}

	if( curl )
	{
		curl_easy_cleanup( curl );
	}
}



void SimApp::windowMoved( Ogre::RenderWindow *win )
{
}



void SimApp::windowResized( Ogre::RenderWindow *win )
{
	win->windowMovedOrResized();
	if( mainWindow == win )
	{
		gui->Resize( mainWindow->getWidth(), mainWindow->getHeight() );
	}
}



bool SimApp::windowClosing( Ogre::RenderWindow *win )
{
	return true;
}



void SimApp::windowClosed( Ogre::RenderWindow *win )
{
}



void SimApp::windowFocusChange( Ogre::RenderWindow *win )
{
}
