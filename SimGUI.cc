#include "SimGUI.hh"
#include "SimApp.hh"


SimGUI::SimGUI( SimApp &app ) : app( app )
{
	myGui = nullptr;
	platform = nullptr;
}



void SimGUI::Load()
{
	platform = new MyGUI::OgrePlatform();
	platform->initialise( app.mainWindow, app.sceneManager );

	myGui = new MyGUI::Gui();
	myGui->initialise();
	myGui->hidePointer();
	
	MyGUI::LayoutManager::getInstance().loadLayout( "Test.layout" );

	// Set MenuBar width
	MyGUI::MenuBarPtr menuBar = myGui->findWidget<MyGUI::MenuBar>( "MenuBar" );
	menuBar->setSize( myGui->getViewWidth(), menuBar->getHeight() );

	// Handlers
	MyGUI::MenuItemPtr fileQuitItem = myGui->findWidget<MyGUI::MenuItem>("FileQuit-Button");
	fileQuitItem->eventMouseButtonClick += MyGUI::newDelegate( this, &SimGUI::MenuBarWidgetClicked );

	// Set callbacks
	app.mouse->setEventCallback( this );
	app.keyboard->setEventCallback( this );
}



void SimGUI::Unload()
{
	myGui->shutdown();
	delete myGui;
	myGui = nullptr;

	platform->shutdown();
	delete platform;
	platform = nullptr;
}



void SimGUI::Update()
{
	app.mouse->capture();
	app.keyboard->capture();
}



void SimGUI::Resize( int w, int h )
{
	const OIS::MouseState &mouseState = app.mouse->getMouseState();
	mouseState.width = w;
	mouseState.height = h;
	myGui->_resizeWindow( MyGUI::IntSize( w, h ) );
}


/*
 * OIS Event Handlers
 */
bool SimGUI::mouseMoved( const OIS::MouseEvent &arg )
{
    MyGUI::InputManager::getInstance().injectMouseMove(arg.state.X.abs, arg.state.Y.abs, arg.state.Z.abs);
	return true;
}
 
bool SimGUI::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    MyGUI::InputManager::getInstance().injectMousePress(arg.state.X.abs, arg.state.Y.abs, MyGUI::MouseButton::Enum(id));
	return true;
}
 
bool SimGUI::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    MyGUI::InputManager::getInstance().injectMouseRelease(arg.state.X.abs, arg.state.Y.abs, MyGUI::MouseButton::Enum(id));
	return true;
}

bool SimGUI::keyPressed( const OIS::KeyEvent &arg )
{
    MyGUI::InputManager::getInstance().injectKeyPress(MyGUI::KeyCode::Enum(arg.key), arg.text);
	return true;
}
 
bool SimGUI::keyReleased( const OIS::KeyEvent &arg )
{
    MyGUI::InputManager::getInstance().injectKeyRelease(MyGUI::KeyCode::Enum(arg.key));
	return true;
}



/*
 * MyGUI Events
 */
void SimGUI::MenuBarWidgetClicked( MyGUI::Widget *widget )
{
	if( widget->getName() == "FileQuit-Button" )
	{
		app.closed = true;
	}
}
